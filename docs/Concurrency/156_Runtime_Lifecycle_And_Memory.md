# RUNTIME LIFECYCLE AND MEMORY - DETAILED FLOW DESCRIPTIONS

## SHARED STATE LIFECYCLE - Detailed Description

```
Creation:
─────────
auto fut = std::async(...);
    │
    ├──► Allocate on heap:
    │    ┌────────────────────────────┐
    │    │ Shared State Object        │
    │    ├────────────────────────────┤
    │    │ std::mutex mutex           │
    │    │ std::condition_variable cv │
    │    │ std::atomic<status> state  │
    │    │ union { T value; }         │
    │    │ std::exception_ptr excep   │
    │    │ int ref_count = 2          │
    │    └────────────────────────────┘
    │           ▲           ▲
    │           │           │
    │      promise      future
    │      (writer)     (reader)
```

**PHASE 1: CREATION - Detailed Flow**

When `std::async()` or a `std::promise<T>` constructor executes, a shared state object is allocated. Let's trace this step-by-step:

**1.1 Memory Allocation**

```cpp
// Inside std::async or std::promise constructor
template<typename T>
struct __shared_state_base {
    std::mutex __mutex_;
    std::condition_variable __cv_;
    std::atomic<__state_value> __state_;
    std::exception_ptr __exception_;
    int __ref_count_;
    
    // Alignment and storage for result
    typename std::aligned_storage<sizeof(T), alignof(T)>::type __storage_;
};

// Allocate on heap
auto* state = new __shared_state_base<T>();
```

**Memory Layout** (example for `std::promise<int>`):

```
Heap Address 0x7f1234560000:
+0x00: std::mutex (typically 40 bytes on Linux/x64)
       - Futex word (4 bytes)
       - Padding and alignment
       
+0x28: std::condition_variable (typically 48 bytes)
       - Futex word for waiters
       - Mutex pointer
       - Clock ID
       
+0x58: std::atomic<status> (4 bytes)
       Values: 0=not_ready, 1=ready, 2=exception
       
+0x5C: Padding (4 bytes for alignment)

+0x60: std::exception_ptr (16 bytes on 64-bit)
       - Pointer to exception object (or nullptr)
       - Control block pointer
       
+0x70: int ref_count (4 bytes)
       Initial value: 2 (one for promise, one for future)
       
+0x74: Padding (4 bytes for alignment)

+0x78: aligned_storage for T (4 bytes for int)
       - Not initialized yet (uninitialized memory)
       
Total size: ~128 bytes (varies by platform and type T)
```

**Why Heap Allocation?**
- The shared state must outlive the `async()` call
- Multiple objects (promise, future) need to reference it
- Size is not known at compile time in generic code
- Allows reference counting for automatic cleanup

**1.2 Initialization**

```cpp
// Initialize synchronization primitives
state->__mutex_.__init();           // Initialize mutex (calls pthread_mutex_init)
state->__cv_.__init(&state->__mutex_); // Initialize condition variable
state->__state_.store(0);          // Set status to "not_ready"
state->__exception_ = nullptr;      // No exception yet
state->__ref_count_ = 2;           // Promise + Future
// Note: __storage_ is NOT initialized yet (union/aligned_storage)
```

**1.3 Object Creation**

```cpp
// Create future object
std::future<T> fut;
fut.__state_ = state;  // Store pointer to shared state

// If using promise:
std::promise<T> prom;
prom.__state_ = state; // Same shared state pointer
```

Both `promise` and `future` are lightweight wrappers around a pointer to the shared state. The objects themselves are typically just 8 bytes (one pointer on 64-bit systems).

**1.4 Reference Count = 2**

At this point, two objects hold references to the shared state:
1. **Promise side** (or async thread's internal promise): Can write to it via `set_value()` or `set_exception()`
2. **Future side** (returned to user): Can read from it via `get()`, `wait()`, etc.

The reference count ensures the shared state isn't deleted while either side is still alive.

---

```
Access Pattern (Thread Synchronization):
─────────────────────────────────────────

Producer Thread                      Consumer Thread
───────────────                      ───────────────
    │                                      │
    │ compute result                       │
    │                                      │ future.get()
    ├──► lock mutex                        ├──► lock mutex
    │                                      │    check status
    │                                      │    status == NOT_READY
    │ set value                            │    cv.wait(mutex)
    │ status = READY                       │    [BLOCKED]
    │ cv.notify_all()                      │    [releases mutex]
    │ unlock mutex                         │
    ├─────────────────────────────────────►│    [WAKES UP]
    │                                      │    [reacquires mutex]
    │                                      │    status == READY
    │                                      │    copy value
    │                                      │    unlock mutex
    │                                      │    return value
    ▼                                      ▼
```

**PHASE 2: ACCESS PATTERN - Detailed Synchronization Flow**

This is the heart of promise-future synchronization. Let's examine each step in detail:

**CONSUMER THREAD (calling future.get())**

**Step C1: Lock Mutex**
```cpp
// Inside future<T>::get()
std::unique_lock<std::mutex> __lock(__state_->__mutex_);
```

- The consumer thread attempts to acquire the shared state's mutex
- If the mutex is free, it acquires immediately (~20 ns)
- If the producer holds it, this blocks until released (~100 ns - 10 µs depending on contention)
- The mutex is now held by the consumer thread

**Step C2: Check Status**
```cpp
// Check if result is ready
while (__state_->__state_.load(std::memory_order_acquire) == __state_not_ready) {
    // Not ready, need to wait
```

- Read the atomic status flag with `memory_order_acquire`
- This ensures we see any writes the producer made before setting the flag
- If status is `__state_not_ready`, we need to wait
- If status is `__state_ready` or `__state_exception`, skip the wait

**Step C3: Wait on Condition Variable** (if not ready)
```cpp
    __state_->__cv_.wait(__lock); // BLOCKS HERE
}
```

This is where the magic happens. `condition_variable::wait()` does three things **atomically**:

1. **Releases the mutex**: Other threads (like the producer) can now acquire it
2. **Blocks the thread**: Puts the thread into SLEEPING state in the kernel
3. **Adds to wait queue**: Registers this thread as waiting on this condition variable

**What happens in the kernel:**

```c
// Simplified kernel pseudocode (Linux futex)
futex_wait(address_of_cv, current_value) {
    // Add current thread to wait queue for this futex
    wait_queue_add(&futex_wait_queues[hash(address_of_cv)], current_task);
    
    // Change thread state to INTERRUPTIBLE or UNINTERRUPTIBLE
    current_task->state = TASK_INTERRUPTIBLE;
    
    // Remove from CPU scheduler's run queue
    dequeue_task(rq, current_task);
    
    // Schedule another thread
    schedule();
    
    // Thread is now OFF-CPU, blocked, waiting for signal
}
```

**Important**: The consumer thread is now **off-CPU**. It's not busy-waiting or consuming CPU cycles. The CPU core is free to run other threads. This is efficient waiting.

**PRODUCER THREAD (calling promise.set_value())**

**Step P1: Compute Result**
```cpp
// User's code
int result = expensive_computation(); // Takes time...
```

- The producer performs whatever computation is needed
- Consumer thread is still blocked (from Step C3)
- This happens in parallel - consumer isn't wasting CPU

**Step P2: Lock Mutex**
```cpp
// Inside promise<T>::set_value(T value)
std::unique_lock<std::mutex> __lock(__state_->__mutex_);
```

- Producer acquires the mutex
- Since consumer released it in `wait()`, this succeeds immediately
- Mutex protects the shared state from concurrent modification

**Step P3: Store Value**
```cpp
// Check that we haven't already set a value
if (__state_->__state_.load() != __state_not_ready) {
    throw std::future_error(std::future_errc::promise_already_satisfied);
}

// Construct the value in-place using placement new
new (&__state_->__storage_) T(std::move(value));
```

**Placement New Explained:**
- `&__state_->__storage_` is raw memory (aligned_storage)
- `new (&addr) T(value)` constructs a T at that address without allocating
- Uses move constructor if available for efficiency
- The value now lives in the shared state

**Step P4: Set Status Flag**
```cpp
// Mark as ready
__state_->__state_.store(__state_ready, std::memory_order_release);
```

- Sets the atomic status flag to `__state_ready`
- `memory_order_release` ensures all previous writes (the value) are visible
- This is the critical synchronization point - the "release" that pairs with the consumer's "acquire"

**Step P5: Notify Waiting Threads**
```cpp
// Wake up any threads waiting on the condition variable
__state_->__cv_.notify_all();
```

This triggers a kernel operation:

```c
// Simplified kernel pseudocode
futex_wake(address_of_cv, num_to_wake = INT_MAX) {
    // Find all threads waiting on this futex
    wait_queue_t* waiters = &futex_wait_queues[hash(address_of_cv)];
    
    // For each waiting thread
    for_each_waiter(waiters, waiter) {
        // Change state from SLEEPING to READY
        waiter->task->state = TASK_RUNNING;
        
        // Add back to scheduler's run queue
        enqueue_task(rq, waiter->task);
        
        // Remove from wait queue
        wait_queue_remove(waiters, waiter);
    }
    
    // Trigger scheduler to consider newly runnable threads
    wake_up_process(waiters);
}
```

**Step P6: Unlock Mutex**
```cpp
__lock.unlock(); // Releases mutex
```

- Mutex is released
- Producer thread continues (or terminates if it was the async thread)

**CONSUMER THREAD WAKES UP**

The consumer thread is woken by the kernel scheduler (might take 10 µs - 1 ms):

**Step C4: Reacquire Mutex**

After waking, the consumer thread attempts to reacquire the mutex:

```cpp
// After waking from wait(), unique_lock automatically reacquires mutex
// This happens inside condition_variable::wait() implementation
```

- The thread wakes up but doesn't immediately proceed
- It must reacquire the mutex before `wait()` returns
- If the producer still holds the mutex, it waits here briefly
- Once acquired, `wait()` returns and the loop condition is checked again

**Step C5: Recheck Status**
```cpp
while (__state_->__state_.load(std::memory_order_acquire) == __state_not_ready) {
    // Loop condition is false now (status == __state_ready)
}
// Exits loop!
```

- Check status again (could be spurious wakeup, though rare)
- Status is now `__state_ready`, so the loop exits
- The `memory_order_acquire` ensures we see the value the producer wrote

**Step C6: Extract Value**
```cpp
// Get the result value
T __result = std::move(*reinterpret_cast<T*>(&__state_->__storage_));

// Destroy the value in the shared state (calls destructor)
reinterpret_cast<T*>(&__state_->__storage_)->~T();

// Mark this future as invalid (can't call get() again)
__state_ = nullptr;

__lock.unlock();

return __result; // Return to user
```

- Move the value out of the shared state
- Call the destructor on the shared state's copy (placement delete)
- Invalidate the future by nulling its state pointer
- Return the value to the user's code

**Step C7: Return to User**

The `get()` call completes, returning the value to the user. The consumer thread continues executing with the result.

**Thread Coordination Summary:**

This synchronization achieves:
1. **Thread-safe data transfer**: Mutex protects concurrent access
2. **Efficient waiting**: Condition variable avoids busy-waiting
3. **Proper memory ordering**: Atomic operations ensure visibility
4. **One-time transfer**: Future becomes invalid after `get()`

The condition variable is essential because:
- **Without it**, consumer would need to busy-wait: `while(not ready) { /* spin */ }` - wastes CPU
- **With it**, consumer sleeps and OS wakes it only when needed - efficient

---

```
Destruction:
────────────
    future goes out of scope
         │
         ├──► ref_count--
         │    if (ref_count == 0)
         │       delete shared_state;
         │
    promise goes out of scope
         │
         └──► ref_count--
              if (ref_count == 0)
                 delete shared_state;
```

**PHASE 3: DESTRUCTION - Detailed Cleanup Flow**

Cleanup involves multiple objects and reference counting. Let's trace different scenarios:

**SCENARIO A: Normal Completion - Future Destroyed First**

**Step 1: future.get() Called**
```cpp
{
    auto fut = std::async(task);
    int result = fut.get(); // Value retrieved
    // fut goes out of scope here
}
```

Inside `get()`, the future already decremented its own reference:
```cpp
// Inside future<T>::get()
T result = /* ... extract value ... */;

// Decrement reference count (future is done with shared state)
if (--__state_->__ref_count_ == 1) {
    // Promise side still holds a reference, don't delete yet
}

__state_ = nullptr; // Future no longer points to shared state
return result;
```

**Step 2: Future Destructor** (when fut goes out of scope)
```cpp
// Inside ~future()
if (__state_ != nullptr) {
    // Would decrement here, but __state_ is already null from get()
    // Nothing to do
}
```

Since `get()` already handled the reference count and nulled the pointer, the destructor does nothing.

**Step 3: Promise Side Cleanup** (async thread terminates)

The async wrapper function (running in the worker thread) completes:
```cpp
// Inside async wrapper (worker thread)
try {
    T result = task();
    __state->set_value(std::move(result));
} catch (...) {
    __state->set_exception(std::current_exception());
}

// Wrapper function ends, implicit promise cleanup:
// (The worker thread held an internal promise object)
if (--__state->__ref_count_ == 0) {
    delete __state; // Last reference gone, delete shared state
}
```

**Deletion Process:**
```cpp
// Inside ~__shared_state_base()
~__shared_state_base() {
    // Destroy the stored value (if constructed)
    if (__state_.load() == __state_ready) {
        reinterpret_cast<T*>(&__storage_)->~T(); // Call destructor
    }
    
    // Destroy synchronization primitives
    __cv_.~condition_variable();  // Calls pthread_cond_destroy
    __mutex_.~mutex();            // Calls pthread_mutex_destroy
    
    // Exception pointer destructor runs automatically
    // (decrements exception object's reference count)
}
// Memory returned to heap allocator
```

**SCENARIO B: Exception Case - Promise Destroyed Without Setting Value**

```cpp
{
    std::promise<int> prom;
    auto fut = prom.get_future();
    
    // Oops, forgot to call prom.set_value()!
    // Or maybe an exception was thrown
    
} // prom goes out of scope
```

**Step 1: Promise Destructor**
```cpp
// Inside ~promise()
if (__state_ != nullptr) {
    // Check if we set a value
    if (__state_->__state_.load() == __state_not_ready) {
        // ERROR: Promise destroyed without satisfying it
        // Set a broken_promise exception
        __state_->set_exception(
            std::make_exception_ptr(
                std::future_error(std::future_errc::broken_promise)
            )
        );
    }
    
    // Decrement reference count
    if (--__state_->__ref_count_ == 1) {
        // Future still exists, keep shared state alive
    }
    
    __state_ = nullptr;
}
```

This is a critical safety feature: If a promise is destroyed without setting a value, the shared state automatically stores a "broken promise" exception. This prevents the future from waiting forever.

**Step 2: Future Calls get()**
```cpp
try {
    int result = fut.get(); // Will throw!
} catch (const std::future_error& e) {
    // e.code() == std::future_errc::broken_promise
    // Handle the error
}
```

The future receives the exception and throws it to the caller.

**Step 3: Future Destructor**
```cpp
// When fut goes out of scope
if (--__state_->__ref_count_ == 0) {
    delete __state_; // Last reference, clean up
}
```

**SCENARIO C: Unused Future - Never Called get()**

```cpp
{
    auto fut = std::async(task);
    // Do other stuff, never call fut.get()
} // fut goes out of scope
```

**Step 1: Future Destructor**
```cpp
// Inside ~future()
if (__state_ != nullptr) {
    // Special behavior: If get() was never called, BLOCK!
    // This ensures the async task completes
    
    if (__state_->__state_.load() != __state_ready &&
        __state_->__state_.load() != __state_exception) {
        // Task hasn't completed yet, wait for it
        wait(); // Blocks until ready
    }
    
    // Now decrement reference
    if (--__state_->__ref_count_ == 1) {
        // Worker thread still holds reference
    }
}
```

**This is important!** If you create a future with `std::async` and never call `get()`, the future's destructor will **block** waiting for the task to complete. This ensures the task's side effects (file writes, network operations, etc.) complete even if you don't need the return value.

**Step 2: Worker Thread Completes**
```cpp
// Worker thread finishes and decrements ref count
if (--__state_->__ref_count_ == 0) {
    delete __state_;
}
```

**SCENARIO D: shared_future - Multiple Copies**

```cpp
{
    auto fut = std::async(task);
    std::shared_future<int> sf1 = fut.share(); // ref_count = 2
    std::shared_future<int> sf2 = sf1;         // ref_count = 3
    std::shared_future<int> sf3 = sf1;         // ref_count = 4
    
    int a = sf1.get(); // Doesn't invalidate, ref_count still 4
    int b = sf2.get(); // ref_count still 4
    
} // All shared_futures go out of scope
```

**Step 1: Each shared_future Destructor**
```cpp
// ~shared_future() for sf1
if (--__state_->__ref_count_ == 3) { /* Keep alive */ }

// ~shared_future() for sf2
if (--__state_->__ref_count_ == 2) { /* Keep alive */ }

// ~shared_future() for sf3
if (--__state_->__ref_count_ == 1) { /* Keep alive (worker still has ref) */ }
```

**Step 2: Worker Thread Reference**
```cpp
// Worker completes
if (--__state_->__ref_count_ == 0) {
    delete __state_; // Finally delete
}
```

**Reference Counting Rules Summary:**

1. **Initial Count**: 2 (promise + future)
2. **shared_future Copies**: Increment count
3. **get() on Regular Future**: Doesn't decrement (handled in destructor)
4. **Destructor**: Decrements count, deletes if zero
5. **Thread Safety**: Reference count is atomic or mutex-protected

---

## MEMORY LAYOUT IN RUNTIME - Detailed Description

```
Process Memory Space:
─────────────────────

High Address
    │
    ├─────────────────────────────┐
    │   Stack (Main Thread)       │  ← Local variables, fut
    ├─────────────────────────────┤
    │   Stack (Worker Thread 1)   │  ← Thread-local storage
    ├─────────────────────────────┤
    │   Stack (Worker Thread 2)   │
    ├─────────────────────────────┤
    │         ...                 │
    ├─────────────────────────────┤
    │                             │
    │         [gap]               │
    │                             │
    ├─────────────────────────────┤
    │          HEAP               │
    │                             │
    │  ┌─────────────────────┐    │  ← Shared state objects
    │  │ Shared State #1     │    │     allocated here
    │  ├─────────────────────┤    │
    │  │ Shared State #2     │    │
    │  └─────────────────────┘    │
    │                             │
    │  Thread control blocks,     │
    │  mutexes, condition vars    │
    ├─────────────────────────────┤
    │   Data Segment (.data)      │
    ├─────────────────────────────┤
    │   Code Segment (.text)      │  ← std::async code
Low Address                           libstdc++ code
```

**Detailed Memory Analysis:**

**STACK REGIONS (High Addresses)**

**Main Thread Stack:**
- **Size**: Typically 8 MB on Linux, 1 MB on Windows
- **Location**: Starts near the top of user address space (e.g., 0x7fffffffffff on x64 Linux)
- **Contents**:
  - Local variables: `auto fut = std::async(...);`
    - `fut` object: 8 bytes (pointer to shared state)
  - Function call frames (stack frames)
  - Return addresses
  - Saved registers
- **Growth**: Grows downward (toward lower addresses)
- **Guard Page**: A protected page at the bottom to detect stack overflow

**Example Stack Frame:**
```
High address: 0x7fffffffe800
├─ Return address (8 bytes)
├─ Saved RBP (8 bytes)
├─ Local var: fut (8 bytes) ──> Points to 0x55555557a040 (heap)
├─ Other local variables
└─ Function parameters
Low address:  0x7fffffffe700
```

**Worker Thread Stacks:**
- **Size**: Customizable, default 2 MB (Linux) or 1 MB (Windows)
- **Location**: Allocated dynamically by pthread_create/CreateThread
- **Separate**: Each thread has its own stack in a different memory region
- **Contents**:
  - Worker function's local variables
  - Lambda captures (if using lambda)
  - Temporary objects during computation
  - Thread-local storage (TLS) variables

**HEAP REGION (Middle Addresses)**

The heap contains dynamically allocated memory:

**Shared State Objects:**
```
Address: 0x55555557a040
┌─────────────────────────────┐
│ Shared State Object         │
│                             │
│ + 0x00: vtable pointer      │ (8 bytes, for polymorphism)
│ + 0x08: mutex               │ (40 bytes)
│ + 0x30: condition_variable  │ (48 bytes)
│ + 0x60: atomic<status>      │ (4 bytes)
│ + 0x64: padding             │ (4 bytes)
│ + 0x68: exception_ptr       │ (16 bytes)
│ + 0x78: ref_count           │ (4 bytes, atomic)
│ + 0x7C: padding             │ (4 bytes)
│ + 0x80: storage<T>          │ (sizeof(T) bytes)
└─────────────────────────────┘
Total: ~144 bytes + sizeof(T)
```

**Thread Control Blocks:**

On Linux, pthread structures are also in the heap:
```
Address: 0x7ffff7800000 (example)
┌────────────────────────────┐
│ pthread_t structure        │
│                            │
│ - Thread ID (TID)          │
│ - Stack pointer            │
│ - Stack size               │
│ - Thread-specific data     │
│ - Cleanup handlers         │
│ - Cancellation state       │
└────────────────────────────┘
Size: ~1200 bytes on x64 Linux
```

**Heap Allocator Metadata:**

The heap allocator (malloc/free, or new/delete which use malloc internally) adds metadata:

```
For a shared state allocation:
┌──────────────────┐ ← User pointer: 0x55555557a040
│ User Data (144B) │
├──────────────────┤ ← Start of malloc block: 0x55555557a030
│ Size: 160        │ (8 bytes)
│ Prev Size: ...   │ (8 bytes)
└──────────────────┘

Actual allocated size: 160 bytes (rounded up to alignment)
User-visible size: 144 bytes
Overhead: 16 bytes
```

**DATA SEGMENT (Lower Addresses)**

Contains global and static variables:
```
Address: 0x555555558000 (example)
┌───────────────────────────┐
│ Global variables          │
│ Static class members      │
│ String literals           │
│ Virtual function tables   │
└───────────────────────────┘
```

**CODE SEGMENT (Lowest Addresses)**

Contains executable machine code:
```
Address: 0x555555554000 (example)
┌──────────────────────────────┐
│ Your compiled code           │
│ std::async implementation    │
│ Template instantiations      │
│ libstdc++ functions          │
└──────────────────────────────┘

Read-only and executable (RX permissions)
```

**Memory Access Patterns:**

**When main thread creates async:**
1. CPU fetches instructions from Code Segment (0x555555554...)
2. Executes `new` operator, calls into heap allocator
3. Allocator finds free block in Heap (0x55555557...)
4. Returns pointer, stored in stack variable `fut` (0x7fffffffe...)

**When worker thread accesses shared state:**
1. Worker stack (0x7ffff7000...) contains pointer to shared state
2. Pointer points to heap (0x55555557a040)
3. CPU loads data from heap via pointer
4. May cause cache line transfers between CPU cores

**Cache Coherency:**

Modern CPUs have cache hierarchies:
```
Core 0:            Core 1:
L1 Cache (32KB)    L1 Cache (32KB)
L2 Cache (256KB)   L2 Cache (256KB)
     └────────┬────────┘
          L3 Cache (8MB, shared)
              │
          Main RAM (16GB)
```

When both threads access the shared state:
- Each core loads the cache line containing the shared state
- Mutex operations cause cache line invalidation (MESI protocol)
- Atomic operations ensure cache coherency
- This causes performance overhead (~50-200 CPU cycles per cache miss)

**Virtual vs Physical Memory:**

The addresses shown are **virtual addresses**. The MMU (Memory Management Unit) translates them to physical RAM addresses:

```
Virtual Address: 0x55555557a040
     ↓ (Page Table Translation)
Physical Address: 0x1a4f8a040 (example)
     ↓
Actual DRAM location
```

Each process has its own virtual address space but shares physical RAM. The kernel manages this mapping via page tables.

---

This completes the detailed flow descriptions for all the diagrams! The C++ async components and runtime work together through multiple layers of abstraction, from high-level standard library APIs down to kernel thread management and CPU execution.