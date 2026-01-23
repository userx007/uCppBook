# C++ RUNTIME COMPONENTS - DETAILED FLOW DESCRIPTIONS

## HIGH-LEVEL ARCHITECTURE - Flow Description

```
┌─────────────────────────────────────────────────────────────────────────┐
│                        APPLICATION CODE                                 │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │  std::async(), std::promise, std::future, std::thread            │   │
│  └─────────────────────────────┬────────────────────────────────────┘   │
└────────────────────────────────┼────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                    C++ STANDARD LIBRARY (libstdc++/libc++)              │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │  Thread Management | Task Scheduling | Shared State Management   │   │
│  └─────────────────────────────┬────────────────────────────────────┘   │
└────────────────────────────────┼────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                      PLATFORM THREADING LAYER                           │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │  pthread (Linux/Unix) | WinAPI Threads (Windows)                 │   │
│  └─────────────────────────────┬────────────────────────────────────┘   │
└────────────────────────────────┼────────────────────────────────────────┘
                                 │
                                 ▼
┌─────────────────────────────────────────────────────────────────────────┐
│                        OPERATING SYSTEM KERNEL                          │
│  ┌──────────────────────────────────────────────────────────────────┐   │
│  │  • Kernel Thread Scheduler                                       │   │
│  │  • CPU Context Switching                                         │   │
│  │  • Synchronization Primitives (futex, mutexes, condition vars)   │   │
│  └──────────────────────────────────────────────────────────────────┘   │
└─────────────────────────────────────────────────────────────────────────┘
                                 │
                                 ▼
                          [Hardware CPUs]
```

**Detailed Flow Through Layers:**

**LAYER 1: APPLICATION CODE**

This is your C++ source code where you write `std::async()`, create promises and futures, or directly create threads. When you write:

```cpp
auto future = std::async(std::launch::async, my_function);
```

You're making a call to the C++ Standard Library API. At this layer:
- Your code is compiled to machine instructions
- Template instantiations occur for the specific types you're using
- The compiler generates calls to library functions

**Flow**: Your code → Template instantiation → Library function call

**LAYER 2: C++ STANDARD LIBRARY (libstdc++/libc++)**

The standard library (libstdc++ on GCC/Linux, libc++ on Clang, or MSVC's STL on Windows) provides the implementation of `std::async`, `std::thread`, `std::promise`, etc. This layer:

1. **Manages Thread Objects**: Maintains an internal representation of threads with metadata like thread IDs, join states, and function objects to execute.

2. **Handles Shared States**: Allocates and manages the shared state objects that connect promises and futures. These include:
   - Result storage (aligned storage or union for the value type)
   - Synchronization primitives (mutexes, condition variables)
   - Reference counting for lifetime management
   - Status flags (ready/not ready/exception)

3. **Implements Synchronization**: Provides type-safe C++ wrappers around OS synchronization primitives:
   - `std::mutex` wraps pthread_mutex or Windows CRITICAL_SECTION
   - `std::condition_variable` wraps pthread_cond or Windows CONDITION_VARIABLE
   - `std::atomic` uses CPU atomic instructions or locks

4. **Task Scheduling (Optional)**: Some implementations use internal thread pools for `std::async` to avoid creating a new OS thread for every call. The library might maintain a queue of tasks and a pool of worker threads.

**Flow**: Library function → Allocate shared state → Prepare thread function → Call platform layer

**LAYER 3: PLATFORM THREADING LAYER**

This is an OS-specific abstraction layer that provides a unified threading interface. On Linux/Unix, this is the POSIX Threads (pthreads) library. On Windows, it's the Win32 threading API.

**On Linux/Unix (pthreads):**

When the standard library needs to create a thread, it calls:

```c
pthread_t thread_id;
pthread_attr_t attributes;
pthread_attr_init(&attributes);
pthread_create(&thread_id, &attributes, thread_start_function, argument);
```

This layer:
- Validates thread attributes (stack size, scheduling policy, priority)
- Allocates a pthread control structure
- Sets up thread-local storage (TLS) pointers
- Allocates the thread's stack (typically 2-8 MB)
- Makes a system call to the kernel to create the thread

**On Windows:**

```c
HANDLE thread_handle = CreateThread(
    NULL,                   // Security attributes
    0,                      // Stack size (0 = default)
    thread_start_function,  // Function pointer
    argument,               // Parameter
    0,                      // Creation flags
    &thread_id             // Thread ID
);
```

This layer performs similar tasks but uses Windows-specific data structures.

**Flow**: Platform API call → Validate parameters → Allocate structures → System call to kernel

**LAYER 4: OPERATING SYSTEM KERNEL**

The kernel is the core of the operating system that has direct access to hardware. When the platform layer makes a system call (like `clone()` on Linux or `NtCreateThread()` on Windows), the kernel:

1. **Creates a Kernel Thread Structure**: The kernel maintains its own representation of threads (task_struct on Linux, ETHREAD on Windows) containing:
   - Thread ID (TID)
   - Parent process ID (PID)
   - CPU registers state
   - Scheduling information (priority, time slice)
   - Memory management information (page tables)
   - Signal handlers (on Unix)
   - Security context

2. **Allocates Kernel Stack**: Each thread needs a kernel-mode stack (separate from user-mode stack) for handling system calls and interrupts. This is typically 8-16 KB.

3. **Registers with Scheduler**: The thread is added to the scheduler's run queue. The scheduler decides when this thread gets CPU time based on:
   - Priority level
   - Scheduling policy (FIFO, round-robin, completely fair scheduler)
   - CPU affinity masks
   - Current system load

4. **Sets Up Memory Management**: If this is the first thread in a new process, sets up the virtual address space. For new threads in existing processes, shares the address space but gets unique stack regions.

5. **Initializes Synchronization**: Sets up kernel-level synchronization primitives:
   - **Linux**: Futexes (fast userspace mutexes) for efficient waiting
   - **Windows**: Kernel event objects, mutexes, semaphores

**Flow**: System call → Create kernel structure → Allocate resources → Add to scheduler → Return to user space

**LAYER 5: HARDWARE CPUs**

At the bottom, the actual CPU cores execute instructions. The OS kernel's scheduler decides which thread runs on which core at any given time. When a context switch occurs:

1. **Save Current State**: The CPU saves the current thread's registers to memory (program counter, stack pointer, general-purpose registers, floating-point registers, SIMD registers).

2. **Load New State**: The CPU loads the new thread's saved register state from memory.

3. **Switch Page Tables**: If switching between processes (not just threads), update the memory management unit (MMU) to use the new process's page tables.

4. **Resume Execution**: The CPU begins executing instructions from the new thread's program counter.

5. **Cache Effects**: Context switches flush CPU caches to some degree, which is why excessive thread switching hurts performance.

**Upward Flow (Thread Execution):**

Once the thread is scheduled on a CPU:
1. CPU executes thread's instructions
2. Thread makes library calls (future.get())
3. Library acquires mutex, waits on condition variable
4. If needs to wait, makes system call back to kernel
5. Kernel puts thread to sleep, schedules another thread
6. When signaled, kernel wakes thread, schedules it
7. Thread resumes in library code
8. Library returns result to application code

This layered architecture provides portability (same C++ code runs on different platforms) while allowing efficient use of OS-specific features.

---

## EXECUTION FLOW: std::async CALL - Detailed Description

```
User Code: auto fut = std::async(std::launch::async, task_func);
    │
    ▼
┌─────────────────────────────────────────────────────────────────┐
│ Step 1: C++ Standard Library (std::async implementation)        │
│                                                                 │
│  • Allocate shared state object on heap                         │
│  • Initialize mutex, condition variable, status flag            │
│  • Create std::future<T> wrapper around shared state            │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│ Step 2: Thread Creation Request                                 │
│                                                                 │
│  if (launch::async) {                                           │
│      std::thread thread([shared_state, task_func]() {           │
│          // Wrapper function                                    │
│      });                                                        │
│      thread.detach();                                           │
│  }                                                              │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│ Step 3: Platform Thread Layer                                   │
│                                                                 │
│  Linux/Unix:                    Windows:                        │
│  • pthread_create()             • _beginthreadex()              │
│  • Setup thread attributes      • CreateThread()                │
│  • Allocate thread stack        • Setup thread context          │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                            ▼
┌─────────────────────────────────────────────────────────────────┐
│ Step 4: OS Kernel                                               │
│                                                                 │
│  • Create kernel thread structure                               │
│  • Allocate kernel stack                                        │
│  • Add thread to scheduler queue                                │
│  • Assign thread ID (TID)                                       │
└───────────────────────────┬─────────────────────────────────────┘
                            │
                            ▼
                    Thread is now READY
```

**Step-by-Step Detailed Flow:**

**STEP 1: STANDARD LIBRARY - std::async Implementation**

When you call `std::async(std::launch::async, task_func)`, the library template is instantiated for your specific function signature. Here's what happens inside the library code:

**1.1 Shared State Allocation:**
```cpp
// Pseudocode of what the library does internally
template<typename F>
future<typename result_of<F()>::type> async(launch policy, F&& func) {
    using R = typename result_of<F()>::type;
    
    // Allocate shared state on heap
    auto state = new __shared_state<R>();
    
    // Initialize synchronization primitives
    state->mutex = new std::mutex();
    state->condition = new std::condition_variable();
    state->status = __state_not_ready;
    state->ref_count = 2; // One for promise side, one for future
```

The shared state is a heap-allocated object containing:
- **Storage**: An aligned buffer capable of holding a value of type R (or empty if void)
- **Mutex**: A `std::mutex` for protecting access to the shared state
- **Condition Variable**: A `std::condition_variable` for blocking/waking threads
- **Status Flag**: An enum or atomic indicating: not_ready, ready, or exception
- **Exception Storage**: A `std::exception_ptr` to hold any thrown exception
- **Reference Count**: Tracks how many promise/future objects point to this state

**1.2 Future Object Creation:**
```cpp
    // Create future object that wraps the shared state
    future<R> fut;
    fut.__state = state; // Internal pointer to shared state
    
    return fut; // Will be returned to caller
}
```

The `std::future<R>` is a lightweight wrapper around a pointer to the shared state. It provides the public API (`get()`, `wait()`, etc.) but the real data lives in the shared state.

**1.3 Time Taken**: This step is very fast - typically a few microseconds:
- Heap allocation: ~100-500 nanoseconds
- Mutex initialization: ~50-200 nanoseconds
- Future construction: ~10 nanoseconds (just pointer assignment)

**STEP 2: THREAD CREATION REQUEST**

Still inside the `std::async` implementation, the library now needs to create the actual worker thread:

**2.1 Launch Policy Check:**
```cpp
if (policy & launch::async) {
    // Must create a new thread
    
    // Create a wrapper lambda that handles the task execution
    auto wrapper = [state, func = std::forward<F>(func)]() mutable {
        try {
            // Execute the user's function
            R result = func();
            
            // Store result in shared state
            std::lock_guard<std::mutex> lock(state->mutex);
            new (&state->storage) R(std::move(result)); // Placement new
            state->status = __state_ready;
            state->condition->notify_all();
            
        } catch (...) {
            // Store exception in shared state
            std::lock_guard<std::mutex> lock(state->mutex);
            state->exception = std::current_exception();
            state->status = __state_exception;
            state->condition->notify_all();
        }
        
        // Decrement reference count (promise side done)
        if (--state->ref_count == 0) {
            delete state;
        }
    };
```

This wrapper lambda:
- Captures the shared state pointer and the user's function
- When executed in the worker thread, it calls the user's function
- Catches any exceptions
- Stores the result or exception in the shared state
- Notifies waiting threads via the condition variable
- Handles cleanup

**2.2 Thread Object Creation:**
```cpp
    // Create std::thread with the wrapper
    std::thread worker(std::move(wrapper));
    
    // Detach so the thread can run independently
    worker.detach();
```

Creating a `std::thread` doesn't immediately execute on a CPU - it just creates the thread object and registers it with the OS. The `detach()` call allows the thread to run independently without needing to be joined.

**2.3 Time Taken**: This step involves more overhead:
- Thread object creation: ~1-5 microseconds
- Preparing wrapper function: ~100-500 nanoseconds

**STEP 3: PLATFORM THREADING LAYER**

The `std::thread` constructor calls the platform-specific thread creation API. This is where cross-platform differences emerge:

**3.1 Linux/Unix - pthread_create:**

```c
// Inside std::thread's constructor, eventually calls:
pthread_attr_t attr;
pthread_attr_init(&attr);

// Set stack size (often 2MB or 8MB depending on system)
pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE);

// Set scheduling policy if needed
// pthread_attr_setschedpolicy(&attr, SCHED_OTHER);

pthread_t thread_handle;
int result = pthread_create(
    &thread_handle,           // Thread ID output
    &attr,                    // Thread attributes
    __thread_proxy_function,  // Internal start function
    wrapper_data              // Pointer to wrapper lambda
);

pthread_attr_destroy(&attr);
```

**What pthread_create does:**
1. **Allocates pthread Structure**: Creates a `pthread_t` structure in user-space containing thread metadata
2. **Allocates Stack**: Allocates a stack (usually 2-8 MB) using `mmap()` with guard pages to detect stack overflow
3. **Sets Up TLS**: Initializes thread-local storage (TLS) pointers for `thread_local` variables
4. **Prepares Arguments**: Packages the start function and arguments for the kernel
5. **Makes System Call**: Calls the `clone()` system call to ask the kernel to create the thread

**3.2 Windows - CreateThread/_beginthreadex:**

```c
// Inside std::thread's constructor on Windows:
HANDLE thread_handle = _beginthreadex(
    NULL,                          // Security attributes
    0,                             // Stack size (0 = default, usually 1MB)
    __thread_proxy_function,       // Internal start function
    wrapper_data,                  // Argument
    0,                             // Creation flags
    &thread_id                     // Thread ID output
);
```

**What CreateThread does:**
1. **Validates Parameters**: Checks stack size, security attributes
2. **Allocates Thread Environment Block (TEB)**: A Windows-specific structure for thread data
3. **Allocates Stack**: Reserves virtual memory for stack (commit-on-demand for efficiency)
4. **Sets Up TLS**: Initializes TLS slots for `__declspec(thread)` variables
5. **Makes System Call**: Calls `NtCreateThread()` to transition to kernel mode

**3.3 Time Taken**: Platform layer adds significant overhead:
- pthread structure allocation: ~1-2 microseconds
- Stack allocation (mmap): ~10-50 microseconds
- TLS setup: ~1-5 microseconds
- System call transition: ~1-3 microseconds
- **Total**: ~15-60 microseconds

**STEP 4: OPERATING SYSTEM KERNEL**

When the system call (clone/NtCreateThread) reaches the kernel, the OS creates a true kernel thread:

**4.1 Kernel Thread Structure Creation (Linux):**

```c
// Inside the kernel (simplified):
struct task_struct *new_thread = copy_process(
    CLONE_VM | CLONE_FS | CLONE_FILES | CLONE_SIGHAND | CLONE_THREAD,
    user_stack_pointer,
    user_registers,
    stack_size,
    parent_thread,
    NULL
);
```

The `task_struct` (Linux) or `ETHREAD` (Windows) contains:
- **Thread ID (TID)**: Unique identifier for this thread
- **Process ID (PID)**: ID of the parent process
- **CPU State**: Saved registers (general purpose, floating point, SIMD, program counter, stack pointer)
- **Scheduling Info**: Priority, scheduling class, CPU affinity, time slice
- **Memory Info**: Pointer to page tables (shared with other threads in the process)
- **Kernel Stack Pointer**: Points to this thread's kernel stack
- **Signal Handlers**: For Unix signals (on Linux)
- **Statistics**: CPU time used, context switch count, page faults

**4.2 Kernel Stack Allocation:**

```c
// Allocate kernel stack (typically 8KB-16KB)
void *kernel_stack = kmalloc(THREAD_SIZE, GFP_KERNEL);
new_thread->stack = kernel_stack;
```

The kernel stack is separate from the user stack and is used when the thread makes system calls or handles interrupts.

**4.3 Add to Scheduler:**

```c
// Add thread to the appropriate run queue
enqueue_task(cpu_rq(target_cpu), new_thread, flags);
new_thread->state = TASK_RUNNING;

// Wake up scheduler to consider this new thread
wake_up_new_task(new_thread);
```

The scheduler maintains multiple run queues (one per CPU, or more complex structures):
- **Real-time Queue**: For SCHED_FIFO/SCHED_RR threads
- **Normal Queue**: For SCHED_OTHER threads (default)
- **Idle Thread**: Runs when no other thread is ready

**4.4 Assign Thread ID:**

```c
// Assign a unique TID
new_thread->pid = allocate_pid();
```

On Linux, each thread gets a unique TID, even though they share a PID with their parent process. On Windows, each thread gets a unique thread ID.

**4.5 Time Taken**: Kernel operations are the most expensive:
- Task structure allocation: ~5-10 microseconds
- Kernel stack allocation: ~5-10 microseconds
- Scheduler registration: ~5-15 microseconds
- TID allocation: ~1-2 microseconds
- **Total**: ~15-40 microseconds

**4.6 Return to User Space:**

The system call returns, passing back:
- **Thread Handle**: An opaque handle to refer to the thread
- **Thread ID**: The TID for this thread
- **Return Code**: Success or error code

Control returns to the platform layer, then to the standard library, then to your code. The `std::async()` call completes, returning the `future` object. The total time from `async()` call to return is typically **30-150 microseconds**.

**STEP 5: THREAD BECOMES READY AND AWAITS SCHEDULING**

After creation, the thread is in the READY state. The OS scheduler decides when to actually run it:

**5.1 Scheduler Decision:**

The scheduler considers:
- **Priority**: Higher priority threads run first
- **CPU Affinity**: Thread may be pinned to specific CPUs
- **Load Balancing**: Try to distribute threads evenly across cores
- **Time Slice**: How long each thread runs before being preempted
- **Interactive vs Batch**: Interactive threads get priority for responsiveness

**5.2 Context Switch:**

When the scheduler decides to run the new thread:

1. **Save Current Thread State**: CPU registers are saved to the old thread's task_struct
2. **Load New Thread State**: New thread's registers are loaded from its task_struct
3. **Switch Page Tables (if needed)**: If switching processes, change the MMU's page table pointer
4. **Update CPU State**: Change current task pointer in CPU-local storage
5. **Resume Execution**: Jump to the new thread's program counter

**5.3 Thread Execution Begins:**

The new thread starts executing at the thread proxy function (the wrapper created in Step 2). This wrapper then calls your `task_func`, and the async operation is truly underway.

**5.4 Time Until Scheduled**: This varies dramatically:
- **Best case** (idle CPU): ~1-10 microseconds
- **Typical case** (moderate load): ~100 microseconds to 1 millisecond
- **Worst case** (heavy load, low priority): 10+ milliseconds

---

## PARALLEL EXECUTION MODEL - Flow Description

```
Time ──────────────────────────────────────────────────────────────────►

CPU Core 0          CPU Core 1        CPU Core 2
─────────           ─────────         ─────────

┌──────────┐        
│  Main    │        [idle]            [idle]
│  Thread  │
└────┬─────┘
     │ std::async()
     │ creates shared state
     │
     ├─────────────────►┌──────────┐
     │                  │ Worker   │  [idle]
     │                  │ Thread   │
┌────▼─────┐            │ Starts   │
│  Main    │            └────┬─────┘
│ continues│                 │
│          │                 │ task_func()
│          │                 │ executing
│ other    │            ┌────▼─────┐
│ work()   │            │ Worker   │  [idle]
│          │            │ running  │
│          │            │          │
│ fut.get()│            │          │
│ BLOCKS ──┼───────────►│          │
│ waiting  │            │ completes│
│          │            └────┬─────┘
│          │                 │ write result
│          │            ┌────▼─────┐
│          │◄───────────┤ notify   │
│ UNBLOCKS │            │ waiting  │
└────┬─────┘            └──────────┘
     │                       X (thread exits)
     │ process result
     ▼
```

**Detailed Timeline Analysis:**

**T=0: Initial State**
- **Core 0**: Executing the main thread
- **Core 1**: Idle or running other system threads
- **Core 2**: Idle or running other system threads
- Main thread is about to call `std::async()`

**T=1: std::async() Call (Core 0)**

Main thread executes `std::async(std::launch::async, task_func)`. During this call:

1. **Heap Allocation** (~500 ns): Allocate shared state on heap
2. **Initialization** (~300 ns): Initialize mutex, condition variable, status
3. **Thread Creation Request** (~5 µs): Call pthread_create/CreateThread
4. **System Call** (~20 µs): Kernel creates thread structure
5. **Return** (~1 µs): async() returns with future object

**Total time**: ~27 microseconds. The main thread spent this time on Core 0. The worker thread has been created but isn't running yet - it's in the OS scheduler's ready queue.

**T=2: Main Thread Continues (Core 0), Worker Thread Scheduled (Core 1)**

**On Core 0 - Main Thread:**
- Continues executing post-async code
- Calls `other_work()` function
- Performs computations, I/O, or other tasks
- Completely unaware of worker thread's state
- No blocking, no waiting

**On Core 1 - Worker Thread:**
- OS scheduler selects worker thread to run (after ~100 µs)
- Context switch occurs (~2 µs)
- Worker thread begins executing wrapper function
- Wrapper calls `task_func()`
- `task_func()` performs its computation

**On Core 2:**
- Idle or running other threads (not shown)

**Parallelism Achievement**: Both cores are now doing useful work simultaneously. If the task is CPU-bound, we're getting true 2x parallelism (assuming 2-core CPU).

**T=3: Worker Executing, Main Reaches get() (Core 0, Core 1)**

**On Core 0 - Main Thread:**
- Finishes `other_work()`
- Reaches `future.get()` call
- Calls get(), which internally does:

```cpp
// Inside future::get()
std::unique_lock<std::mutex> lock(shared_state->mutex);
while (shared_state->status == not_ready) {
    shared_state->condition.wait(lock); // BLOCKS HERE
}
// ... (won't reach here until worker signals)
```

- **Blocking Point**: The thread blocks on the condition variable
- **CPU Release**: Core 0's time slice for main thread ends
- **CPU Freed**: Core 0 becomes available for other threads

**On Core 1 - Worker Thread:**
- Still executing `task_func()`
- Making progress on the computation
- Unaware that main thread is waiting
- No performance impact from main thread blocking

**Context Switch on Core 0**:
When main thread blocks, the OS scheduler selects another thread to run on Core 0. This might be:
- Another application's thread
- A system daemon thread
- The idle thread (if no work available)

The main thread is now in **SLEEPING** state in the scheduler's wait queue.

**T=4: Worker Completes (Core 1)**

**On Core 1 - Worker Thread:**
- `task_func()` returns with result value
- Wrapper catch block didn't catch exception (none thrown)
- Wrapper now stores result:

```cpp
try {
    R result = func(); // Completed!
    
    // Store result in shared state
    {
        std::lock_guard<std::mutex> lock(shared_state->mutex);
        
        // Placement new to construct result in shared state storage
        new (&shared_state->storage) R(std::move(result));
        
        // Mark as ready
        shared_state->status = state_ready;
        
        // Wake waiting threads
        shared_state->condition.notify_all();
    }
```

**What happens inside notify_all():**

1. **Kernel Notification**: The condition variable's `notify_all()` makes a system call (futex on Linux, NtReleaseKeyedEvent on Windows)
2. **Wake Queue**: The kernel finds all threads waiting on this condition variable
3. **State Change**: Changes main thread's state from SLEEPING to READY
4. **Scheduler Queue**: Adds main thread to the scheduler's run queue
5. **Scheduling Decision**: Scheduler will decide when to actually run main thread

**T=5: Main Thread Scheduled and Unblocks (Core 0)**

**On Core 0:**
- Scheduler decides to run main thread (might take 10 µs - 1 ms depending on load)
- Context switch back to main thread (~2 µs)
- Main thread resumes execution inside `condition.wait()`
- Thread re-acquires the mutex (might block briefly if worker still holds it)
- Checks status flag: now "ready"
- Exits the while loop

```cpp
// Continuing inside future::get()
while (shared_state->status == not_ready) {
    shared_state->condition.wait(lock);
} // Loop exits!

// Extract result
R result = std::move(*reinterpret_cast<R*>(&shared_state->storage));

// Mark future as invalid (can't call get() again)
shared_state = nullptr;

lock.unlock();
return result; // Returns to your code
```

**On Core 1 - Worker Thread:**
- Wrapper function completes
- Decrements shared state reference count
- Thread termination sequence begins:
  1. Thread-local storage destructors run
  2. Stack is unwound
  3. pthread_exit() or thread's entry point returns
  4. Kernel deallocates thread structure
  5. Thread stack is unmapped
  6. Thread disappears from system

**T=6: Main Thread Continues with Result (Core 0)**

- Main thread now has the result value
- Continues executing subsequent code
- Can use the result in further computations
- Core 1 becomes available for other threads

**Performance Analysis:**

Let's say:
- `task_func()` takes 100 ms
- `other_work()` takes 50 ms
- Thread creation overhead: 0.1 ms
- Context switches: 0.01 ms total

**Sequential Execution Time** (no async):
```
other_work(): 50 ms
task_func():  100 ms
Total:        150 ms
```

**Parallel Execution Time** (with async on 2 cores):
```
Parallel execution of other_work() and task_func():
max(50 ms, 100 ms) = 100 ms

Plus overhead:
100 ms + 0.1 ms + 0.01 ms = 100.11 ms

Speedup: 150 / 100.11 = 1.5x (perfect speedup would be 2x)
```

The speedup isn't perfect 2x because:
1. `other_work()` finishes before `task_func()`, leaving 50 ms where only one core is busy
2. Thread creation and synchronization overhead
3. Cache effects (cores may contend for shared memory)

**Best Case for Speedup**:
If both tasks took 100 ms each:
- Sequential: 200 ms
- Parallel: 100 ms + overhead = ~100.1 ms
- Speedup: ~2x (near-perfect)

