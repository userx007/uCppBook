# C++ ASYNC COMPONENTS FLOW DESCRIPTIONS

## 1. BASIC ASYNC LAUNCH - Flow Description

### The promise/future pattern

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

// Thread A: Producer function
void producer(std::promise<int>&& prom) {
    std::cout << "[Producer] Starting work...\n";
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Do the actual computation
    int result = 42;
    std::cout << "[Producer] Work complete! Setting value: " << result << "\n";
    
    // Set the value in the promise (this unblocks the consumer)
    prom.set_value(result);
    
    std::cout << "[Producer] Value set, exiting\n";
}

// Thread B: Consumer function
void consumer(std::future<int> fut) {
    std::cout << "[Consumer] Waiting for result...\n";
    
    // This blocks until the promise sets a value
    int result = fut.get();
    
    std::cout << "[Consumer] Received result: " << result << "\n";
    std::cout << "[Consumer] Continuing with result...\n";
}

int main() {
    std::cout << "=== Promise/Future Example ===\n\n";
    
    // Create promise
    std::promise<int> prom;
    
    // Get future from promise
    std::future<int> fut = prom.get_future();
    
    // Launch Thread B (consumer) with the future
    std::thread threadB(consumer, std::move(fut));
    
    // Launch Thread A (producer) with the promise
    std::thread threadA(producer, std::move(prom));
    
    // Wait for both threads to complete
    threadA.join();
    threadB.join();
    
    std::cout << "\n=== Both threads completed ===\n";
    
    return 0;
}

/* Expected Output:
=== Promise/Future Example ===

[Consumer] Waiting for result...
[Producer] Starting work...
[Producer] Work complete! Setting value: 42
[Producer] Value set, exiting
[Consumer] Received result: 42
[Consumer] Continuing with result...

=== Both threads completed ===
*/
```

### Graphical diagram

```
Main Thread          C++ Runtime                Thread A                 Thread B 
                                               (Producer)               (Consumer)
    |                     |                        |                        |
    |  std::promise<int>  |                        |                        |
    |-------------------->|                        |                        |
    |                     |                        |                        |
    |                  [Allocate]                  |                        |
    |                  [promise]                   |                        |
    |                  [object]                    |                        |
    |                     |                        |                        |
    |  get_future()       |                        |                        |
    |-------------------->|                        |                        |
    |                     |                        |                        |
    |                  [Create]                    |                        |
    |                  [future]                    |                        |
    |                  [linked to]                 |                        |
    |                  [promise]                   |                        |
    |                     |                        |                        |
    |  future object      |                        |                        |
    |<--------------------|                        |                        |
    |                     |                        |                        |
    |  std::thread(consumer, std::move(fut))       |                        |
    |-------------------->|                        |                        |
    |                     |                        |                        |
    |                  [Spawn]                     |                        |
    |                  [Thread B]                  |                        |
    |                     |------------------------------------------------>|
    |                     |                        |                        |
    |                     |                        |                   [Consumer] Waiting...
    |                     |                        |                        |
    |                     |                        |                   fut.get()
    |                     |                        |                        |
    |                     |<------------------------------------------------|
    |                     |                        |                        |
    |                  [Check shared]              |                        |
    |                  [state: NOT READY]          |                        |
    |                  [BLOCK]                     |                        |
    |                     |                        |                        |
    |  std::thread(producer, std::move(prom))      |                        |
    |-------------------->|                        |                        |
    |                     |                        |                        |
    |                  [Spawn]                     |                        |
    |                  [Thread A]                  |                        |
    |                     |----------------------->|                        |
    |                     |                        |                        |
    |                     |                   [Producer] Starting...        |
    |                     |                        |                        |
    |                     |                   sleep(2s)                     |
    |                     |                        |                        |
    |                     |                   compute result = 42           |
    |                     |                        |                        |
    |                     |                   prom.set_value(42)            |
    |                     |                        |                        |
    |                     |<-----------------------|                        |
    |                     |                        |                        |
    |                  [Store value]               |                        |
    |                  [in shared]                 |                        |
    |                  [state]                     |                        |
    |                     |                        |                        |
    |                  [Mark READY]                |                        |
    |                     |                        |                        |
    |                  [Notify]                    |                        |
    |                  [waiting]                   |                        |
    |                  [thread]                    |                        |
    |                     |------------------------------------------------>|
    |                     |                        |                        |
    |                     |                        |                   UNBLOCKED!
    |                     |                        |                        |
    |                     |                   [Producer] Exiting            |
    |                     |                        |                        |
    |                     |                        |                   return value = 42
    |                     |                        |                        |
    |                     |                        |                   [Consumer] Received 42
    |                     |                        |                        |
    |                     |                        |                   [Consumer] Continuing...
    |                     |                        |                        |
    |  threadA.join()     |                        |                        |
    |-------------------->|                        |                        |
    |                     |                        |                        |
    |                  [Wait for]                  |                        |
    |                  [Thread A]                  |                        |
    |                     |<-----------------------|                        |
    |                     |                        X                        |
    |  thread joined      |                                                 |
    |<--------------------|                                                 |
    |                     |                                                 |
    |  threadB.join()     |                                                 |
    |-------------------->|                                                 |
    |                     |                                                 |
    |                  [Wait for]                                           |
    |                  [Thread B]                                           |
    |                     |<------------------------------------------------|
    |                     |                                                 X
    |  thread joined      |
    |<--------------------|
    |                     |
    | Both threads completed
    |                     |
    V                     V
```

**What happens:**
1. Main creates a promise and gets its future
2. Thread B starts and immediately blocks on `fut.get()`
3. Thread A does work (simulated with sleep)
4. Thread A sets the value with `prom.set_value(42)`
5. Thread B unblocks, receives the value, and continues

This is a common pattern for one-time value passing between threads where you want the consumer to wait for the producer to finish its work.

### Using std::async

```cpp
#include <iostream>
#include <future>
#include <chrono>

// Task that will run asynchronously
int producer_task() {
    std::cout << "[Async Task] Starting work...\n";
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Do the actual computation
    int result = 42;
    std::cout << "[Async Task] Work complete! Returning: " << result << "\n";
    
    return result;
}

int main() {
    std::cout << "=== std::async Example ===\n\n";
    
    // Launch async task - returns std::future<int>
    // std::async automatically creates promise/future internally
    std::future<int> fut = std::async(std::launch::async, producer_task);
    
    std::cout << "[Main Thread] Async task launched, doing other work...\n";
    
    // Main thread can do other work here
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "[Main Thread] Still working...\n";
    
    std::cout << "[Main Thread] Now waiting for result...\n";
    
    // This blocks until the async task completes
    int result = fut.get();
    
    std::cout << "[Main Thread] Received result: " << result << "\n";
    std::cout << "[Main Thread] Continuing with result...\n";
    
    std::cout << "\n=== Completed ===\n";
    
    return 0;
}

/* Expected Output:
=== std::async Example ===

[Main Thread] Async task launched, doing other work...
[Async Task] Starting work...
[Main Thread] Still working...
[Main Thread] Now waiting for result...
[Async Task] Work complete! Returning: 42
[Main Thread] Received result: 42
[Main Thread] Continuing with result...

=== Completed ===
*/
```

```
Main Thread          C++ Runtime              Async Thread 
                                             (producer_task)
    |                     |                        |
    |  std::async(        |                        |
    |    std::launch::async,                       |
    |    producer_task)   |                        |
    |-------------------->|                        |
    |                     |                        |
    |                  [Create]                    |
    |                  [promise]                   |
    |                  [internally]                |
    |                     |                        |
    |                  [Create]                    |
    |                  [future]                    |
    |                  [linked to]                 |
    |                  [promise]                   |
    |                     |                        |
    |                  [Spawn new]                 |
    |                  [thread for]                |
    |                  [producer_task]             |
    |                     |----------------------->|
    |                     |                        |
    |  future object      |                   [Async Task] Starting...
    |<--------------------|                        |
    |                     |                        |
    | [Main] Async task   |                   sleep(2s)
    | launched, doing     |                        |
    | other work...       |                        |
    |                     |                        |
    | sleep(500ms)        |                        |
    |                     |                        |
    | [Main] Still        |                        |
    | working...          |                        |
    |                     |                        |
    | [Main] Now waiting  |                        |
    | for result...       |                        |
    |                     |                        |
    |  fut.get()          |                        |
    |-------------------->|                        |
    |                     |                        |
    |                  [Check shared]              |
    |                  [state: NOT]                |
    |                  [ready - BLOCK]             |
    |                  [main thread]               |
    |                     |                        |
    |                     |                   compute result = 42
    |                     |                        |
    |                     |                   [Async Task] Work complete!
    |                     |                        |
    |                     |                   return 42
    |                     |                        |
    |                     |<-----------------------|
    |                     |                        |
    |                  [Implicit]                  |
    |                  [promise.set_value(42)]     |
    |                     |                        |
    |                  [Store value]               |
    |                  [in shared]                 |
    |                  [state]                     |
    |                     |                        |
    |                  [Mark READY]                |
    |                     |                        |
    |                  [Notify]                    |
    |                  [waiting]                   |
    |                  [main thread]               |
    |                     |                        |
    |  UNBLOCKED!         |                        |
    |                     |                        |
    |  return value = 42  |                        |
    |<--------------------|                        |
    |                     |                        |
    | [Main] Received     |                   [Thread exits]
    | result: 42          |                        |
    |                     |                        X
    | [Main] Continuing   |                        
    | with result...      |                        
    |                     |                        
    | [Completed]         |                        
    |                     |                        
    |                  [Cleanup]                   
    |                  [future/promise]            
    |                  [shared state]              
    |                     |                        
    V                     V
```

#### COMPARISON EXAMPLE: Both approaches

***Manual promise/future approach:***

```cpp
// Step 1: Create a promise object that will hold the result
// The promise is the "write end" of the communication channel
std::promise<int> prom;

// Step 2: Extract the future from the promise
// The future is the "read end" of the communication channel
// A promise can only create ONE future (calling get_future() twice throws exception)
std::future<int> fut = prom.get_future();

// Step 3: Launch a new thread with a lambda function
// IMPORTANT: Promise must be moved (not copied) into the thread
// Once moved, the original 'prom' in main thread is no longer valid
std::thread t([](std::promise<int> p) {
    // Inside the thread: set the value in the promise
    // This stores 42 in the shared state and unblocks any thread waiting on fut.get()
    // After set_value(), the promise has fulfilled its purpose
    p.set_value(42);
}, std::move(prom));  // std::move transfers ownership to the thread

// Step 4: Block and wait for the result
// fut.get() will:
//   - Block if the promise hasn't called set_value() yet
//   - Return immediately if set_value() was already called
//   - Return the value (42) and invalidate the future (can only call get() once)
int result = fut.get();

// Step 5: Wait for the thread to finish execution
// REQUIRED: Without join(), the thread destructor would call std::terminate()
// join() blocks until the thread completes
t.join();

// Manual approach requires:
// - Explicit promise creation
// - Explicit future extraction
// - Explicit thread management (launch + join)
// - Explicit set_value() call
// - Careful ownership management (std::move)
```


***std::async approach (simpler):***

```cpp
// Step 1: Launch async task - all-in-one operation
// std::async AUTOMATICALLY:
//   - Creates a promise internally (hidden from you)
//   - Creates and returns a future linked to that promise
//   - Spawns a new thread (with std::launch::async policy)
//   - Executes the lambda in that thread
//   - Calls promise.set_value() when lambda returns
//   - Manages thread lifetime
// The lambda returns 42, which becomes the future's value
std::future<int> fut = std::async(std::launch::async, []() {
    return 42;  // Return value automatically becomes promise value
});

// Step 2: Block and wait for the result
// fut.get() works the same as manual approach:
//   - Blocks if async task hasn't completed yet
//   - Returns the value (42) when ready
//   - Invalidates the future (can only call get() once)
int result = fut.get();

// Step 3: Cleanup is AUTOMATIC
// No need to join() - the future's destructor will:
//   - Wait for the async thread to complete (blocks if necessary)
//   - Clean up all internal resources (promise, shared state, thread)
// 
// IMPORTANT: If you don't call fut.get(), the future destructor will
// still block waiting for the task to complete when 'fut' goes out of scope
//
// Async approach benefits:
// - No manual promise/future creation
// - No explicit thread management
// - No need to call set_value() - return does it automatically
// - No need to join() - destructor handles it
// - Much less boilerplate code
// - Exception safety: exceptions from lambda are stored and re-thrown on get()
```

**MANUAL APPROACH - You control everything:**
- ✓ Full control over promise/future lifecycle
- ✓ Can separate promise and future creation
- ✓ Can pass promise to different threads/functions
- ✗ More verbose and error-prone
- ✗ Must manually manage thread lifetime (join/detach)
- ✗ Must manually call set_value()

**ASYNC APPROACH - Runtime controls everything:**
- ✓ Minimal boilerplate
- ✓ Automatic thread management
- - Can specify launch policy (async vs deferred)
- ✓ Automatic promise fulfillment (return = set_value) 
- - std::async creates promise/future internally
- ✓ Exception propagation is automatic
- ✓ Automatic cleanup
- ✗ Less control over when/how thread is created
- ✗ Cannot separate promise and future
- ✗ Future destructor blocks and waits for completion (can be surprising)
- What happens internally:
- - `std::async` creates a promise/future pair for you
- - Launches the task (in a thread or thread pool)
- - The task's return value automatically calls `set_value()` on the promise
- - You just call `get()` on the returned future

The `std::async` approach is generally preferred for simple asynchronous tasks because it's less error-prone and more concise. You'd use manual promise/future when you need more control over thread creation or when the producer needs to set values at arbitrary points (not just return values).

```
Main Thread                           Async Thread
───────────                           ────────────
    │                                       │
    │  std::async(...)                      │
    ├──────────────────────────────────────>│
    │         launches                      │
    │                                       │ executes
    │  returns std::future<T>               │ function
    │<──────────────────────┐               │
    │                       │               │
    │  continues execution  │               │
    │  (non-blocking)       │               │
    │                       │               │
    │  future.get()         │               │ completes
    ├───────────────────────┼───────────────┤
    │  BLOCKS until ready   │<──────────────┤
    │<──────────────────────┘   returns T   │
    │                                       │
    │  resumes with result                  X (thread ends)
    ▼
```

**Detailed Flow:**

1. **Async Launch (Line 1-2)**: When you call `std::async()` with a function or lambda, the runtime immediately begins the launch process. The function signature is `std::async(launch_policy, callable, args...)`.

2. **Thread Creation (Line 3)**: The runtime creates a new worker thread (if using `std::launch::async` policy). This involves allocating a new thread stack, setting up thread-local storage, and registering the thread with the operating system.

3. **Future Return (Line 4-5)**: Before the worker thread completes (or even starts meaningful work), `std::async()` returns immediately with a `std::future<T>` object. This future is a handle to the shared state where the result will eventually be stored. The main thread is NOT blocked at this point.

4. **Parallel Execution (Line 6-9)**: Now both threads run in parallel. The main thread continues executing subsequent code (non-blocking behavior). Meanwhile, the worker thread executes the callable function you provided. This is true parallelism if multiple CPU cores are available.

5. **Main Thread Continues (Line 7-8)**: The main thread can perform any other work while waiting. It might do computations, I/O operations, or start other async tasks. This is the key benefit of asynchronous programming - you don't waste CPU time waiting.

6. **Result Retrieval Request (Line 10-11)**: When the main thread calls `future.get()`, it's requesting the result. At this point, the behavior depends on whether the worker thread has finished.

7. **Blocking Wait (Line 11-12)**: If the worker thread hasn't completed yet, `future.get()` BLOCKS the main thread. The main thread enters a waiting state, yielding its CPU time slice. The blocking is implemented using condition variables internally.

8. **Worker Completion (Line 11-13)**: When the worker thread completes execution of the function, it stores the result in the shared state object (or stores an exception if one was thrown). It then signals any waiting threads through a condition variable.

9. **Result Transfer (Line 13)**: The result is transferred from the shared state to the main thread. The `future.get()` call returns with the computed value of type T.

10. **Thread Cleanup (Line 15)**: The worker thread terminates. Its stack is deallocated, thread-local storage is cleaned up, and the OS thread handle is released. The shared state remains alive until the future is destroyed.

11. **Continuation (Line 15-16)**: The main thread resumes execution with the result value and continues processing.

---

## 2. PROMISE-FUTURE PAIR - Flow Description

```
Thread A (Producer)              std::promise<T>           Thread B (Consumer)
────────────────                 ───────────────           ─────────────────
     │                                   │                          │
     │  create promise                   │                          │
     ├──────────────────────────────────>│                          │
     │                                   │                          │
     │  get_future()                     │                          │
     │<──────────────┐                   │                          │
     │               │                   │                          │
     │               └───────────────────┼─────────────────────────>│
     │                    pass future    │                          │
     │                                   │                          │
     │  do work...                       │         future.get()     │
     │                                   │         BLOCKS ──────────┤
     │                                   │                 waiting  │
     │  set_value(result)                │                          │
     ├──────────────────────────────────>│                          │
     │              stores value    ┌────┤                          │
     │                              │    │                          │
     │                              └────┼─────────────────────────>│
     │                         value ready        UNBLOCKS          │
     │                                   │                          │
     │                                   X         continues with   │
     ▼                                             result           ▼
```

**Detailed Flow:**

1. **Promise Creation (Line 1-3)**: Thread A (the producer) creates a `std::promise<T>` object. This allocates a shared state object on the heap. The promise acts as the "write end" of a communication channel. The shared state contains storage for a value of type T, a mutex for thread-safe access, a condition variable for signaling, and status flags.

2. **Future Extraction (Line 4-6)**: Thread A calls `promise.get_future()` to obtain the corresponding `std::future<T>` object. This is the "read end" of the channel. Both the promise and future now point to the same shared state. The reference count of the shared state is now 2.

3. **Future Transfer (Line 7-8)**: Thread A passes the future object to Thread B (the consumer). This could happen through a function parameter, by storing it in a shared data structure, or by moving it into a lambda capture for a new thread. The future is movable but not copyable, ensuring single ownership.

4. **Parallel Work Begins (Line 9-12)**: Both threads now work in parallel. Thread A performs whatever computation is needed to produce the result. Thread B immediately calls `future.get()` to request the value. Since the value isn't ready yet, Thread B blocks.

5. **Consumer Blocking (Line 11-12)**: Inside `future.get()`, Thread B acquires the shared state's mutex, checks the status flag, sees it's not ready, and calls `condition_variable.wait()`. This atomically releases the mutex and puts Thread B into a waiting state, freeing its CPU core for other work.

6. **Producer Completes Work (Line 13-15)**: Thread A finishes its computation and has a result value ready. It calls `promise.set_value(result)`. This is the critical synchronization point.

7. **Value Storage (Line 15-16)**: Inside `set_value()`, Thread A acquires the shared state's mutex, writes the result value into the shared state's storage area (using placement new or similar), sets the status flag to "ready", then calls `condition_variable.notify_all()` to wake any waiting threads.

8. **Consumer Wake-Up (Line 17-19)**: The condition variable notification causes Thread B to wake up. It re-acquires the mutex, checks the status (now "ready"), and extracts the value from the shared state. The mutex is released.

9. **Result Delivery (Line 19-20)**: `future.get()` returns the value to Thread B. Note that `get()` can only be called once - after this, the future becomes invalid and further calls to `get()` throw an exception.

10. **Continuation (Line 20-22)**: Thread B continues execution with the result. Thread A can continue with other work or terminate. The shared state persists until both the promise and future are destroyed, at which point its reference count reaches zero and it's deallocated.

**Key Difference from Async**: With promise-future pairs, YOU control thread creation, lifetime, and when the value is set. With `std::async`, the runtime handles all of this automatically.

---

## 3. SHARED STATE MECHANISM - Flow Description

```
    ┌─────────────────────────────────────────────┐
    │         SHARED STATE OBJECT                 │
    │  (Created by promise or async)              │
    │                                             │
    │  ┌───────────────────────────────────────┐  │
    │  │  Status: [not ready | ready | error]  │  │
    │  ├───────────────────────────────────────┤  │
    │  │  Storage: T value                     │  │
    │  ├───────────────────────────────────────┤  │
    │  │  Exception: std::exception_ptr        │  │
    │  └───────────────────────────────────────┘  │
    └─────────────────────────────────────────────┘
           ▲                           ▲
           │                           │
           │ writes                    │ reads
           │                           │
    ┌──────┴───────┐            ┌──────┴──────┐
    │ std::promise │            │ std::future │
    │              │            │             │
    │ set_value()  │            │ get()       │
    │ set_exception│            │ wait()      │
    └──────────────┘            │ valid()     │
                                └─────────────┘
```

**Detailed Flow:**

1. **Shared State Allocation**: When a promise is created or `std::async` is called, a shared state object is allocated on the heap. This object is the central coordination point between producer and consumer threads.

2. **State Components**:
   - **Status Flag**: An atomic or mutex-protected enum indicating the current state: `not_ready` (initial), `ready` (value set), or `exception` (error set). This flag is checked by consumers to know if they should wait.
   
   - **Storage Area**: A union or aligned storage buffer capable of holding either a value of type T or remaining empty. When `set_value()` is called, the value is constructed in-place using placement new. This avoids extra copies.
   
   - **Exception Pointer**: A `std::exception_ptr` that can hold a captured exception. If the producer throws an exception, it's caught and stored here using `std::current_exception()`. When the consumer calls `get()`, the exception is rethrown.
   
   - **Synchronization Primitives** (not shown): A mutex protects access to the status and storage. A condition variable allows consumers to wait efficiently without busy-waiting. An atomic reference count tracks how many promise/future objects point to this state.

3. **Promise Write Operations**:
   - **set_value(T val)**: Acquires mutex, checks state isn't already set, constructs value in storage, sets status to ready, notifies waiting threads, releases mutex.
   
   - **set_exception(std::exception_ptr p)**: Acquires mutex, checks state isn't already set, stores exception pointer, sets status to exception, notifies waiting threads, releases mutex.
   
   - **Important**: You can only call set_value() OR set_exception() once. Calling either a second time throws `std::future_error` with error code `promise_already_satisfied`.

4. **Future Read Operations**:
   - **get()**: Acquires mutex, checks status. If not_ready, waits on condition variable (releasing mutex). When woken, re-checks status. If ready, moves/copies value out and returns it. If exception, rethrows it. Can only be called once per future.
   
   - **wait()**: Similar to get() but doesn't retrieve the value. Just blocks until status is ready or exception. Can be called multiple times.
   
   - **wait_for(duration)**: Timed wait. Returns immediately with a status: `ready`, `timeout`, or `deferred`. Useful for polling or implementing timeout logic.
   
   - **wait_until(time_point)**: Like wait_for but with an absolute deadline rather than a duration.
   
   - **valid()**: Returns true if the future has a shared state (hasn't been moved from or get() hasn't been called). Always check this before calling get().

5. **Lifetime Management**: The shared state uses reference counting. When the last owner (promise or future) is destroyed, the reference count hits zero and the shared state is deleted. If a promise is destroyed without setting a value, the shared state automatically stores a `std::future_error` with code `broken_promise`, which will be thrown when a consumer calls get().

6. **Thread Safety**: All operations on the shared state are thread-safe. Multiple threads can call wait() simultaneously. The mutex and condition variable ensure proper synchronization without data races.

---

## 4. MULTIPLE FUTURES WITH SHARED_FUTURE - Flow Description

```
                        std::future<T>
                              │
                              │ .share()
                              ▼
                    std::shared_future<T>
                              │
           ┌──────────────────┼──────────────────┐
           │                  │                  │
           ▼                  ▼                  ▼
      Thread 1           Thread 2           Thread 3
      get() ─────┐        get() ─────┐       get() ─────┐
                 │                   │                  │
                 └───────────────────┴──────────────────┘
                              │
                     All can read the same
                     shared state value
```

**Detailed Flow:**

1. **Regular Future Limitation**: A `std::future<T>` is movable but not copyable. This means only ONE consumer can own it and call `get()` on it. This is by design for single-producer, single-consumer scenarios.

2. **Share Conversion (Line 1-4)**: When you call `future.share()`, it converts the `std::future<T>` into a `std::shared_future<T>`. This operation moves the shared state ownership from the future to the shared_future. The original future becomes invalid.

3. **Shared Future Properties**: Unlike regular futures, `std::shared_future<T>` is copyable. You can create multiple copies, each pointing to the same shared state. All copies are valid and can be used independently.

4. **Distribution to Threads (Line 5-8)**: You can copy the shared_future and pass copies to multiple threads. Each thread gets its own shared_future object, but they all reference the same underlying shared state.

5. **Concurrent Access (Line 9-13)**: All threads can call `get()` on their shared_future copies simultaneously. The shared state's mutex ensures thread-safe access. Unlike regular futures, `get()` on shared_future doesn't invalidate it - you can call it multiple times.

6. **Read-Only Access**: The value is read-only. All threads receive a const reference (or a copy, depending on T) to the stored value. No thread can modify the value through the shared_future.

7. **Synchronization**: All threads that call `get()` before the value is ready will block on the same condition variable. When the value is set, all waiting threads are woken simultaneously via `notify_all()`.

8. **Use Cases**:
   - **Broadcasting**: One producer creates a result that many consumers need (e.g., loading configuration data once for multiple worker threads).
   - **Fan-out**: A single async operation whose result is needed by multiple downstream operations.
   - **Coordination**: Multiple threads waiting for a signal/event represented as a shared_future<void>.

9. **Lifetime**: The shared state remains alive as long as at least one shared_future (or the original promise) exists. When all copies are destroyed, the shared state is deallocated.

---

## 5. EXCEPTION HANDLING - Flow Description

```
Producer Thread              Shared State              Consumer Thread
───────────────              ────────────              ───────────────
     │                            │                           │
     │  try {                     │                           │
     │    throw exception         │                           │
     │  }                         │                           │
     │  set_exception(            │                           │
     │    current_exception())    │                           │
     ├───────────────────────────>│                           │
     │        stores exception    │                           │
     │                            │         future.get()      │
     │                            │<──────────────────────────┤
     │                            │                           │
     │                            ├──────────────────────────>│
     │                            │   RETHROWS exception      │
     │                            │                           │
     │                            │                           │ try-catch
     ▼                            ▼                           ▼ handles it
```

**Detailed Flow:**

1. **Exception Occurrence (Line 1-4)**: While the producer thread executes its task, an exception is thrown. This could be an explicit `throw` statement, a standard library exception (like `std::bad_alloc`), or any exception propagating up.

2. **Exception Capture (Line 5-7)**: In the catch block, the producer captures the current exception using `std::current_exception()`. This creates a `std::exception_ptr`, which is a type-erased smart pointer that can hold any exception type without knowing its exact type at compile time.

3. **Exception Storage (Line 7-8)**: The producer calls `promise.set_exception(std::current_exception())`. Internally, this:
   - Acquires the shared state's mutex
   - Checks that no value or exception has already been set
   - Stores the exception_ptr in the shared state's exception storage
   - Sets the status flag to "exception"
   - Calls `condition_variable.notify_all()` to wake waiting threads
   - Releases the mutex

4. **Alternative Auto-Capture**: If you're using `std::async`, you don't need to manually catch and set exceptions. The runtime automatically wraps your callable in try-catch logic that captures any exception and stores it in the shared state.

5. **Consumer Blocks (Line 9-10)**: Meanwhile, the consumer thread has called `future.get()` and is waiting. When the exception is set and the condition variable is notified, the consumer wakes up.

6. **Exception Detection (Line 11-12)**: Inside `get()`, the consumer thread:
   - Re-acquires the mutex after waking from the condition variable wait
   - Checks the status flag
   - Sees the status is "exception" rather than "ready"
   - Retrieves the stored exception_ptr

7. **Exception Rethrow (Line 12-13)**: The consumer calls `std::rethrow_exception(exception_ptr)`. This reconstructs and throws the original exception with its exact type and value. It's as if the exception was thrown directly in the consumer's context.

8. **Exception Propagation (Line 15)**: The exception propagates up the consumer's call stack normally. The consumer can catch it with appropriate catch blocks just like any other exception.

9. **Type Preservation**: The beauty of this mechanism is that exception type information is preserved. If the producer threw a `std::runtime_error("file not found")`, the consumer can catch it as `catch(const std::runtime_error& e)` and access the exact error message.

10. **Special Cases**:
    - **Broken Promise**: If a promise is destroyed without setting a value or exception, the shared state automatically stores a `std::future_error` with error code `std::future_errc::broken_promise`. Consumers will receive this exception.
    - **Future Error**: Calling `get()` twice, or calling `set_value()` twice on a promise, throws `std::future_error` with error code `std::future_errc::promise_already_satisfied`.

---

## 6. LAUNCH POLICIES - Flow Description

```
std::launch::async
──────────────────
    Guarantees new thread creation
    
    main() ─────────────────────────────────> continues
              │
              └──> [New Thread] ──> executes function


std::launch::deferred
─────────────────────
    Lazy evaluation - runs in calling thread
    
    main() ─────────> future.get() ──> executes NOW ──> result
                      (blocks here)


std::launch::async | std::launch::deferred (default)
────────────────────────────────────────────────────
    Implementation decides (typically async)
```

**Detailed Flow:**

**ASYNC POLICY:**

1. **Guaranteed Parallelism**: When you specify `std::launch::async`, you're explicitly requesting that the function execute in a new thread, providing true parallelism (if CPU cores are available).

2. **Immediate Launch**: The thread is created immediately when `std::async()` is called, not lazily. The function begins execution right away, even before `async()` returns.

3. **Use Cases**: 
   - CPU-intensive computations where you want to ensure parallel execution
   - When you have work that must start immediately
   - When you have multiple cores and want to utilize them

4. **Performance Consideration**: Creating threads has overhead (stack allocation, OS thread structure). If you call `std::async` with async policy thousands of times, you'll create thousands of threads, which can overwhelm the system.

**DEFERRED POLICY:**

1. **Lazy Evaluation**: With `std::launch::deferred`, no new thread is created. The function doesn't execute immediately.

2. **Execution Trigger**: The function only executes when you call `get()` or `wait()` on the future. At that moment, it runs synchronously in the calling thread.

3. **Blocking Behavior**: When `get()` is called, it blocks the current thread while executing the deferred function. There's no parallelism - it's just delayed execution.

4. **Use Cases**:
   - When you want to defer expensive computation until you know you need it
   - When you're building a task graph and want to control execution order
   - When you want future/promise semantics without thread overhead
   - When you might not need the result at all (can check with `wait_for(0ms)` without triggering execution)

5. **Performance Consideration**: No thread creation overhead. Useful for fine-grained tasks where thread creation cost exceeds the task duration.

**DEFAULT POLICY (async | deferred):**

1. **Implementation Choice**: When you use `std::async()` without specifying a policy (or explicitly use `async | deferred`), the implementation chooses the strategy.

2. **Typical Behavior**: Most implementations (GCC, Clang, MSVC) default to `async` behavior - creating a new thread. But they're not required to.

3. **Unpredictability**: You can't rely on either behavior. The result might execute immediately in parallel, or might defer until `get()` is called.

4. **Use Cases**: Only use this when you don't care about the execution model. Generally, it's better to be explicit.

5. **Checking Execution**: You can check if a future is deferred by calling `future.wait_for(std::chrono::seconds(0))`. If it returns `std::future_status::deferred`, the task hasn't started and won't until you call `get()`.

**Comparison Example:**
```cpp
// Async: Function starts running NOW, in parallel
auto f1 = std::async(std::launch::async, heavy_computation);
do_other_work();  // Runs in parallel with heavy_computation
auto result1 = f1.get();  // Might return immediately if already done

// Deferred: Function doesn't run yet
auto f2 = std::async(std::launch::deferred, heavy_computation);
do_other_work();  // heavy_computation hasn't started yet
auto result2 = f2.get();  // NOW heavy_computation runs (blocks here)

// Default: Implementation decides
auto f3 = std::async(heavy_computation);
do_other_work();  // May or may not run in parallel
auto result3 = f3.get();  // Might block or might return immediately
```

---

## 7. COMPLETE WORKFLOW EXAMPLE - Flow Description

```
auto future = std::async(std::launch::async, []{ return compute(); });
│
├─ Creates shared state
├─ Launches thread
└─ Returns std::future<T>

Main Thread                              Worker Thread
───────────                              ─────────────
    │                                         │
    │ do_other_work()                         │ compute()
    │                                         │
    │                                         │ (working...)
    │ more_work()                             │
    │                                         │ (working...)
    │                                         │
    │ if (future.wait_for(0ms)                │
    │     == ready)                           │ COMPLETES
    │   // not ready yet                      │
    │                                         │ stores result
    │ result = future.get()                   │ in shared state
    ├─────────────────────────────────────────┤
    │          BLOCKS until ready             │
    │<────────────────────────────────────────┤
    │          receives result                X (thread ends)
    │
    │ use result
    ▼
```

**Complete Detailed Flow:**

**PHASE 1: INITIALIZATION (Line 1-3)**

1. **Code**: `auto future = std::async(std::launch::async, []{ return compute(); });`

2. **Shared State Creation**: The runtime allocates a new shared state object on the heap. This object contains:
   - Storage for the return value of `compute()`
   - A mutex for thread-safe access
   - A condition variable for blocking/waking threads
   - Status flags (initially "not ready")
   - Reference count (initially 1, will become 2)

3. **Lambda Capture**: The lambda `[]{ return compute(); }` is captured. Since we used `[]`, there are no captures - the lambda is stateless. If you had `[&x]` or `[=]`, those variables would be copied or captured by reference into the lambda's internal storage.

4. **Thread Launch**: The runtime calls the platform's thread creation function:
   - **Linux/Unix**: `pthread_create(&thread_handle, nullptr, thread_func, args)`
   - **Windows**: `_beginthreadex(nullptr, 0, thread_func, args, 0, &thread_id)`
   
   The thread function is a wrapper that will:
   - Execute the lambda
   - Catch any exceptions
   - Store the result or exception in the shared state
   - Notify waiting threads
   - Clean up and exit

5. **Future Return**: `std::async` returns a `std::future<T>` object (where T is the return type of `compute()`). This future holds a pointer to the shared state. The reference count is now 2 (promise-side held by the thread, future held by the caller).

6. **Asynchronous Start**: By the time the `async()` call returns, the worker thread may have already started executing `compute()`, or it might still be in the OS scheduler queue. Either way, the main thread continues immediately without waiting.

**PHASE 2: PARALLEL EXECUTION (Line 5-11)**

7. **Main Thread Work (Line 5-6)**: The main thread calls `do_other_work()` and `more_work()`. These execute while the worker thread is simultaneously running `compute()`. This is the key benefit - both CPU cores are doing useful work.

8. **Worker Thread Execution (Line 6-10)**: The worker thread executes the `compute()` function. This might involve:
   - Heavy numerical computation
   - I/O operations (file reading, network requests)
   - Database queries
   - Any long-running task
   
   While this happens, the main thread isn't blocked - it's running its own code.

9. **Polling Example (Line 11-13)**: The main thread can optionally check if the result is ready without blocking:
   ```cpp
   if (future.wait_for(std::chrono::milliseconds(0)) == std::future_status::ready) {
       // Result is ready, get() won't block
   } else {
       // Still computing, do more work or wait
   }
   ```
   This returns immediately with one of three statuses:
   - `ready`: Result is available
   - `timeout`: Result not ready yet (will always be this status with 0ms timeout if not ready)
   - `deferred`: Task hasn't started (not applicable with async policy)

**PHASE 3: SYNCHRONIZATION (Line 14-17)**

10. **Completion (Line 13-15)**: The worker thread finishes executing `compute()`. The return value (let's say an `int`) is ready.

11. **Result Storage (Line 15)**: Inside the thread wrapper function:
    ```cpp
    try {
        T result = compute();  // Execute user's function
        shared_state->set_value(std::move(result));  // Store result
    } catch (...) {
        shared_state->set_exception(std::current_exception());  // Store exception
    }
    ```
    The `set_value()` call:
    - Acquires the shared state's mutex
    - Constructs the result value in the shared state's storage (using move semantics if possible)
    - Sets the status flag to "ready"
    - Calls `condition_variable.notify_all()` to wake any waiting threads
    - Releases the mutex

12. **Get() Call (Line 16)**: The main thread calls `future.get()` to retrieve the result. At this point, one of two things happens:

    **Scenario A - Result Already Ready:**
    - `get()` acquires the mutex
    - Checks status, sees "ready"
    - Moves/copies the value from shared state
    - Marks the future as invalid (can't call get() again)
    - Releases the mutex
    - Returns the value immediately

    **Scenario B - Result Not Ready Yet:**
    - `get()` acquires the mutex
    - Checks status, sees "not ready"
    - Calls `condition_variable.wait(mutex)`
    - This atomically releases the mutex and blocks the thread
    - The OS scheduler removes this thread from the CPU
    - When the worker thread calls `notify_all()`, this thread wakes up
    - It re-acquires the mutex, checks status again (now "ready")
    - Proceeds as in Scenario A

13. **Blocking Behavior (Line 16-17)**: If the worker thread hasn't finished, the main thread is BLOCKED at this line. It's not consuming CPU time - it's in a waiting state. The CPU core is free to run other threads.

14. **Unblocking (Line 18)**: When the worker thread stores the result and calls `notify_all()`, the main thread wakes up, retrieves the value, and `get()` returns.

**PHASE 4: CLEANUP (Line 18-20)**

15. **Worker Thread Termination (Line 18)**: After storing the result, the worker thread's function returns. The thread wrapper function completes, and the thread is destroyed:
    - The thread's stack is deallocated
    - Thread-local storage is destroyed
    - The OS thread handle is closed
    - The thread disappears from the OS scheduler
    
    Note: The shared state is NOT destroyed yet because the future still holds a reference.

16. **Result Usage (Line 20)**: The main thread now has the result value and can use it in subsequent computations.

17. **Future Destruction**: When the `future` variable goes out of scope, its destructor:
    - Decrements the shared state's reference count
    - If the count reaches zero, deallocates the shared state
    - If the count is still positive (shouldn't happen in normal usage), the shared state persists

**Error Scenarios:**

- **Exception in compute()**: If `compute()` throws an exception, the worker thread catches it, stores it with `set_exception()`, and when the main thread calls `get()`, the exception is rethrown.

- **Forgotten get()**: If you never call `get()` or `wait()`, the future's destructor will block, waiting for the worker thread to complete. This ensures the worker thread's side effects complete even if you don't need the result.

- **Double get()**: Calling `get()` twice throws `std::future_error` because the first call invalidates the future.

This complete workflow demonstrates the full lifecycle of an async operation, from creation through parallel execution to synchronization and cleanup.

---

# The "channel" metaphor/relationship between promise and future:

```
═══════════════════════════════════════════════════════════════════════════
                    PROMISE/FUTURE CHANNEL ARCHITECTURE
═══════════════════════════════════════════════════════════════════════════

   PRODUCER SIDE                 SHARED STATE              CONSUMER SIDE
   (Write End)                  (The Channel)              (Read End)
        
   ┌─────────────┐             ┌─────────────┐           ┌─────────────┐
   │             │             │             │           │             │
   │  std::      │   writes    │   SHARED    │   reads   │   std::     │
   │  promise    │────────────>│   STATE     │<──────────│   future    │
   │  <int>      │   to        │             │   from    │   <int>     │
   │             │             │             │           │             │
   └─────────────┘             └─────────────┘           └─────────────┘
        │                             │                         │
        │                             │                         │
        │ set_value(42)               │                         │ get()
        │ set_exception(...)          │                         │ wait()
        │                             │                         │ valid()
        │                             │                         │
        └─────────────────────────────┴─────────────────────────┘
                                  ONE-TO-ONE
                      single producer - single consumer 


═══════════════════════════════════════════════════════════════════════════
                         SHARED STATE INTERNALS
═══════════════════════════════════════════════════════════════════════════

                          ┌────────────────────────┐
                          │    SHARED STATE        │
                          │  (Reference Counted)   │
                          ├────────────────────────┤
                          │                        │
                          │  State Flag:           │
                          │  ┌──────────────────┐  │
                          │  │ NOT_READY        │  │ ← Initial state
                          │  │ READY            │  │ ← After set_value()
                          │  │ EXCEPTION        │  │ ← After set_exception()
                          │  └──────────────────┘  │
                          │                        │
                          │  Storage:              │
                          │  ┌──────────────────┐  │
                          │  │ int value;       │  │ ← Holds 42
                          │  │     OR           │  │
                          │  │ exception_ptr    │  │ ← Holds exception
                          │  └──────────────────┘  │
                          │                        │
                          │  Synchronization:      │
                          │  ┌──────────────────┐  │
                          │  │ std::mutex       │  │ ← Protects state
                          │  │ condition_var    │  │ ← Wakes waiters
                          │  └──────────────────┘  │
                          │                        │
                          │  Reference Count:      │
                          │  ┌──────────────────┐  │
                          │  │ promise: 1       │  │ ← Counts owners
                          │  │ future:  1       │  │
                          │  └──────────────────┘  │
                          │                        │
                          └────────────────────────┘


═══════════════════════════════════════════════════════════════════════════
                      COMMUNICATION FLOW EXAMPLE
═══════════════════════════════════════════════════════════════════════════

TIMELINE:
───────────────────────────────────────────────────────────────────────────    
T0: Creation
───────────────────────────────────────────────────────────────────────────        
    promise<int> prom;           →    [SHARED STATE CREATED]
                                      State: NOT_READY
                                      Value: <empty>
                                      RefCount: 1

    future<int> fut = prom.get_future();
                                 →    RefCount: 2
                                      (promise + future)

───────────────────────────────────────────────────────────────────────────    
T1: Consumer waits (Thread B)
───────────────────────────────────────────────────────────────────────────    
    int result = fut.get();      →    [CHECK STATE]
          ↓                           State: NOT_READY
          ↓                           ↓
        BLOCKS ON                     [WAIT ON CONDITION_VAR]
    CONDITION VARIABLE                Thread B sleeps...
          ↓
          ↓
      [WAITING...]
          ↓
          ↓

───────────────────────────────────────────────────────────────────────────    
T2: Producer writes (Thread A)
───────────────────────────────────────────────────────────────────────────    
    prom.set_value(42);          →    [LOCK MUTEX]
                                      ↓
                                      [STORE VALUE]
                                      value = 42
                                      ↓
                                      [UPDATE STATE]
                                      State: NOT_READY → READY
                                      ↓
                                      [NOTIFY CONDITION_VAR]
                                      ↓
                                      [UNLOCK MUTEX]
                                      ↓
                                      Wake up Thread B!

───────────────────────────────────────────────────────────────────────────    
T3: Consumer unblocked (Thread B)
───────────────────────────────────────────────────────────────────────────    
    [WAKES UP]                   →    [LOCK MUTEX]
          ↓                           ↓
    fut.get() returns            ←    [READ VALUE]
          ↓                           value = 42
    result = 42                       ↓
                                      [UNLOCK MUTEX]

───────────────────────────────────────────────────────────────────────────    
T4: Cleanup
───────────────────────────────────────────────────────────────────────────    
    prom destroyed               →    RefCount: 2 → 1
    
    fut destroyed                →    RefCount: 1 → 0
                                      ↓
                                      [SHARED STATE DESTROYED]


═══════════════════════════════════════════════════════════════════════════
                         CHANNEL GUARANTEES
═══════════════════════════════════════════════════════════════════════════

┌─────────────────────────────────────────────────────────────────────────┐
│                                                                         │
│  ✓ THREAD-SAFE: Multiple threads can safely access promise/future       │
│                                                                         │
│  ✓ ONE-TIME USE: set_value() can only be called ONCE                    │
│                  get() can only be called ONCE                          │
│                                                                         │
│  ✓ BLOCKING: get() blocks until set_value() is called                   │
│                                                                         │
│  ✓ EXCEPTION PROPAGATION: Exceptions in producer thread are             │
│                            stored and re-thrown in consumer thread      │
│                                                                         │
│  ✓ MEMORY SAFETY: Shared state kept alive until both promise            │
│                   and future are destroyed                              │
│                                                                         │
└─────────────────────────────────────────────────────────────────────────┘


═══════════════════════════════════════════════════════════════════════════
                    VISUAL ANALOGY: MAIL SYSTEM
═══════════════════════════════════════════════════════════════════════════

    SENDER                      MAILBOX                    RECEIVER
    (Promise)                (Shared State)                (Future)
    
    ┌─────┐                                               ┌─────┐
    │ ✉️  │   Write letter                                │     │
    │     │  (set_value)       ┌───────────┐              │     │ Check mailbox
    │     │                    │           │<─────────────│     │ (get)
    │     │                    │  [EMPTY]  │              │     │  
    │     │  Put in mailbox    │           │              │     │  
    │     │───────────────────>│           │              │     │
    └─────┘                    └───────────┘              └─────┘
                                     │                         │
                                     │                         │
                               [Letter arrives!]               │
                                     │                    [Mailbox empty]
                                     ↓                         ↓
                               ┌───────────┐              [WAITING...]
                               │    [42]   │                   │
                               │           │                   │
                               │  [READY]  │                   │
                               │           │<──────────────────┘
                               └───────────┘         [Check again]
                                     │                         │
                                     │                         ↓
                                     │                    [Letter found!]
                                     │                         │
                                     └────────────────────> [Read: 42]


═══════════════════════════════════════════════════════════════════════════
                      MULTIPLE FUTURES (NOT ALLOWED)
═══════════════════════════════════════════════════════════════════════════

    promise<int> prom;
    
    future<int> fut1 = prom.get_future();  ✓ OK
    
    future<int> fut2 = prom.get_future();  ✗ THROWS std::future_error!
                                              (future_already_retrieved)
    
    
    WHY? The channel is ONE-TO-ONE:
    
         ┌─────────┐         ┌─────────┐
         │ promise │────────>│ future1 │  ✓ Valid
         └─────────┘         └─────────┘
              │                    
              │              ┌─────────┐
              └─────────────>│ future2 │  ✗ Not allowed!
                             └─────────┘
    
    Only ONE consumer can read from the channel.
    Use std::shared_future if you need multiple readers.


═══════════════════════════════════════════════════════════════════════════
```

This diagram shows:

1. **The channel metaphor** - Promise (write end) and Future (read end) connected by shared state
2. **Shared state internals** - What's actually stored (value, state flag, mutex, condition variable)
3. **Communication flow** - Step-by-step timeline of how data flows through the channel
4. **Thread safety mechanisms** - Mutex and condition variable for synchronization
5. **Mail system analogy** - A real-world parallel to help understand the concept
6. **One-to-one constraint** - Why you can't have multiple futures from one promise

The key insight is that promise and future are just **handles** to a shared state object that acts as a synchronized communication channel between threads!