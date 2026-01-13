# C++ Concurrency: Third Tier Most Used Features and Methods

## Thread Management & Synchronization

### `std::thread::hardware_concurrency()`
- Returns the number of concurrent threads supported by the implementation
- Useful for optimizing thread pool sizes
- Returns 0 if the value is not computable

### `std::thread::get_id()`
- Returns the thread ID of the current thread
- Useful for debugging and logging
- Can be used to identify threads in concurrent operations

### `std::call_once()` with `std::once_flag`
- Ensures a function is called exactly once across multiple threads
- Common for lazy initialization in multithreaded contexts
- Thread-safe alternative to double-checked locking

### `std::lock()` (multi-mutex locking)
- Locks multiple mutexes simultaneously without deadlock
- Uses a deadlock avoidance algorithm
- More efficient than locking mutexes sequentially

## Atomic Operations

### `std::atomic::compare_exchange_weak()`
- Lock-free compare-and-swap operation
- May fail spuriously (useful in loops)
- More efficient than `compare_exchange_strong()` on some platforms

### `std::atomic::fetch_add()` / `fetch_sub()`
- Atomically adds/subtracts and returns the previous value
- Useful for counters and accumulators
- Lock-free on most platforms

### Memory Order Parameters
- `std::memory_order_acquire` / `release`
- `std::memory_order_acq_rel`
- Fine-grained control over memory synchronization
- Enables optimizations beyond sequential consistency

## Condition Variables

### `std::condition_variable::wait_for()`
- Waits with a timeout duration
- Returns `std::cv_status::timeout` or `std::cv_status::no_timeout`
- Useful for implementing polling with backoff

### `std::condition_variable::wait_until()`
- Waits until a specific time point
- Useful for deadline-based synchronization
- Works with chrono time points

### `std::condition_variable::notify_all()`
- Wakes all waiting threads (vs `notify_one()`)
- Necessary when multiple threads might satisfy the condition
- Can cause thundering herd if not carefully used

## Futures & Promises

### `std::packaged_task`
- Wraps any callable target for asynchronous execution
- Links a function to a future
- More flexible than `std::async` for custom thread management

### `std::promise::set_exception()`
- Sets an exception in the shared state
- Allows propagating errors across thread boundaries
- Retrieved via `future::get()`

### `std::future::wait_for()` / `wait_until()`
- Non-blocking checks for future readiness with timeout
- Returns `std::future_status`: ready, timeout, or deferred
- Enables polling-based async patterns

## Synchronization Primitives

### `std::shared_mutex` (C++17)
- Multiple readers, single writer lock
- `lock_shared()` for read access, `lock()` for write access
- Better performance than exclusive locks for read-heavy workloads

### `std::scoped_lock` (C++17)
- RAII wrapper that can lock multiple mutexes
- Deadlock avoidance built-in
- Replaces manual `std::lock()` + multiple `std::lock_guard`

### `std::shared_lock` (C++14)
- RAII wrapper for shared ownership locking
- Used with `std::shared_mutex`
- Automatically releases shared lock on scope exit

## Thread-Local Storage

### `thread_local` keyword
- Declares variables with thread storage duration
- Each thread gets its own instance
- Useful for per-thread caching and state

## Atomic Flags & Spin Locks

### `std::atomic_flag`
- Guaranteed lock-free atomic boolean
- `test_and_set()` and `clear()` operations
- Building block for spin locks

### Spin Lock Implementation
- Custom synchronization using `atomic_flag` or `atomic<bool>`
- Busy-waiting instead of blocking
- Efficient for very short critical sections

## Advanced Patterns

### `std::shared_future`
- Multiple threads can wait on the same shared state
- Created from `std::future` via `share()`
- Copyable, unlike regular futures

### `std::launch::deferred` policy
- Lazy evaluation for `std::async`
- Function runs when `get()` or `wait()` is called
- Useful for optional asynchronous operations

### Exception Handling Across Threads
- Using `std::current_exception()` and `std::rethrow_exception()`
- Capturing exceptions in one thread, rethrowing in another
- Essential for proper error propagation in concurrent code

These third-tier features are used less frequently than basics like `std::thread`, `std::mutex`, and `std::atomic` but are crucial for building robust, efficient concurrent systems. They address specific use cases like deadlock prevention, fine-grained memory ordering, timeout handling, and reader-writer scenarios.