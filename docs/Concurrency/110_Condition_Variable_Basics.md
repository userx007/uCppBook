# Condition Variable Basics in C++

## Introduction

A **condition variable** is a synchronization primitive that enables threads to wait until a particular condition is met. It provides an efficient mechanism for thread communication, allowing threads to block until they are notified by another thread that something of interest has occurred.

In C++, `std::condition_variable` (from `<condition_variable>`) works in conjunction with `std::mutex` and `std::unique_lock` to implement wait/notify patterns, avoiding busy-waiting and reducing CPU consumption.

## Core Concepts

### Why Condition Variables?

Without condition variables, you might implement waiting using a busy-wait loop:

```cpp
// Bad approach - wastes CPU cycles
while (!dataReady) {
    // Spin and check repeatedly
}
```

Condition variables allow threads to sleep until they're explicitly notified, making synchronization efficient:

```cpp
// Good approach - thread sleeps until notified
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock, []{ return dataReady; });
```

### Key Components

1. **std::condition_variable**: The synchronization object itself
2. **std::mutex**: Protects the shared state
3. **std::unique_lock**: A lockable wrapper that can be unlocked/relocked automatically
4. **Predicate**: A condition that must be true for the thread to proceed

## Basic Operations

### wait()

Blocks the current thread until notified and the predicate is satisfied:

```cpp
void wait(std::unique_lock<std::mutex>& lock);
void wait(std::unique_lock<std::mutex>& lock, Predicate pred);
```

The `wait()` function:
1. Atomically unlocks the mutex and blocks the thread
2. When notified, reacquires the lock and wakes up
3. If a predicate is provided, checks it and continues waiting if false

### notify_one()

Wakes up one waiting thread:

```cpp
void notify_one() noexcept;
```

### notify_all()

Wakes up all waiting threads:

```cpp
void notify_all() noexcept;
```

## Code Examples

### Example 1: Basic Producer-Consumer Pattern

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> dataQueue;
bool finished = false;

void producer() {
    for (int i = 1; i <= 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            dataQueue.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        
        cv.notify_one(); // Notify one waiting consumer
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_all(); // Notify all consumers to check finished flag
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait until data is available or production is finished
        cv.wait(lock, []{ return !dataQueue.empty() || finished; });
        
        if (!dataQueue.empty()) {
            int value = dataQueue.front();
            dataQueue.pop();
            lock.unlock(); // Unlock before processing
            
            std::cout << "Consumer " << id << " consumed: " << value << std::endl;
        } else if (finished) {
            break; // Exit if finished and no data
        }
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons1(consumer, 1);
    std::thread cons2(consumer, 2);
    
    prod.join();
    cons1.join();
    cons2.join();
    
    return 0;
}
```

### Example 2: Thread Synchronization for Task Completion

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool taskCompleted = false;

void performTask() {
    std::cout << "Task started..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        taskCompleted = true;
        std::cout << "Task completed!" << std::endl;
    }
    
    cv.notify_one(); // Notify the waiting thread
}

void waitForTask() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Waiting for task to complete..." << std::endl;
    
    // Wait until taskCompleted is true
    cv.wait(lock, []{ return taskCompleted; });
    
    std::cout << "Task completion detected, proceeding..." << std::endl;
}

int main() {
    std::thread worker(performTask);
    std::thread waiter(waitForTask);
    
    worker.join();
    waiter.join();
    
    return 0;
}
```

### Example 3: Using wait_for() with Timeout

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool dataReady = false;

void prepareData() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        dataReady = true;
    }
    
    cv.notify_one();
}

void waitWithTimeout() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Wait for up to 2 seconds
    if (cv.wait_for(lock, std::chrono::seconds(2), []{ return dataReady; })) {
        std::cout << "Data is ready!" << std::endl;
    } else {
        std::cout << "Timeout: Data not ready within 2 seconds" << std::endl;
    }
}

int main() {
    std::thread preparer(prepareData);
    std::thread waiter(waitWithTimeout);
    
    preparer.join();
    waiter.join();
    
    return 0;
}
```

### Example 4: Multiple Threads Waiting (Barrier Pattern)

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

std::mutex mtx;
std::condition_variable cv;
int threadsReady = 0;
const int TOTAL_THREADS = 3;

void workerThread(int id) {
    // Phase 1: Do some work
    std::cout << "Thread " << id << " doing initial work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    
    // Signal ready and wait for all threads
    {
        std::unique_lock<std::mutex> lock(mtx);
        threadsReady++;
        std::cout << "Thread " << id << " ready, waiting for others..." << std::endl;
        
        // Wait until all threads are ready
        cv.wait(lock, []{ return threadsReady == TOTAL_THREADS; });
    }
    
    // Notify others that we've checked the condition
    cv.notify_all();
    
    // Phase 2: Continue after synchronization
    std::cout << "Thread " << id << " proceeding to phase 2" << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 1; i <= TOTAL_THREADS; ++i) {
        threads.emplace_back(workerThread, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## Important Considerations

### Spurious Wakeups

Condition variables can experience spurious wakeups where `wait()` returns even when not notified. Always use a predicate or loop to verify the condition:

```cpp
// Without predicate - must manually check
std::unique_lock<std::mutex> lock(mtx);
while (!condition) {
    cv.wait(lock);
}

// With predicate - handles spurious wakeups automatically
cv.wait(lock, []{ return condition; });
```

### Lost Wakeup Problem

Always check the condition before waiting, as notifications sent before `wait()` is called are lost:

```cpp
{
    std::unique_lock<std::mutex> lock(mtx);
    // Check condition first
    if (!dataReady) {
        cv.wait(lock, []{ return dataReady; });
    }
}
```

### Why unique_lock Instead of lock_guard?

`std::condition_variable::wait()` needs to unlock and relock the mutex, which requires a lockable wrapper with `unlock()` and `lock()` methods. `std::unique_lock` provides this, while `std::lock_guard` does not.

### Thread Safety

The shared state accessed in the predicate must always be protected by the same mutex used with the condition variable. The predicate is evaluated while the lock is held.

## Summary

**Condition variables** are essential synchronization primitives in C++ for efficient thread communication. The `std::condition_variable` class enables threads to wait for specific conditions without wasting CPU cycles in busy-wait loops.

Key takeaways:
- Condition variables work with `std::mutex` and `std::unique_lock` to provide wait/notify patterns
- The `wait()` function atomically releases the lock and blocks until notified
- Always use predicates to guard against spurious wakeups and lost notifications
- `notify_one()` wakes one thread, while `notify_all()` wakes all waiting threads
- Common use cases include producer-consumer patterns, task synchronization, and barrier implementations
- The shared condition must always be protected by the associated mutex

Condition variables are particularly useful when implementing thread pools, event systems, and any scenario where threads need to coordinate based on shared state changes.

---

## Which Thread Gets Woken Up?

The C++ standard is deliberately vague on this — it simply states:

> *"If any threads are waiting, unblocks one of the waiting threads"*

**No specific thread is guaranteed.** The choice is:
- **Implementation-defined**
- **Unspecified** by the standard
- In practice, typically determined by the OS scheduler

---

## What Real Implementations Tend To Do

In practice on most platforms (Linux/pthreads, Windows):

- It's often **FIFO-like** — the thread that has been waiting the longest tends to get woken — but this is **not guaranteed**
- The OS scheduler has the final say and can factor in thread **priorities**, **CPU affinity**, **scheduling policy** (SCHED_FIFO, SCHED_RR, etc.)
- Under high contention it can appear essentially **random**

---

## Why This Matters — The Spurious Wakeup Problem

Because of this uncertainty, you should **always** use a predicate with `wait()`:

```cpp
std::mutex mtx;
std::condition_variable cv;
bool data_ready = false;

// WRONG — dangerous
void worker() {
    std::unique_lock lk(mtx);
    cv.wait(lk);              // Might wake spuriously, or be the "wrong" thread
    process();                // No guarantee the condition is actually true!
}

// CORRECT — always use a predicate
void worker() {
    std::unique_lock lk(mtx);
    cv.wait(lk, []{ return data_ready; });  // Re-checks condition on every wakeup
    process();                               // Guaranteed condition is true here
}
```

The predicate form is exactly equivalent to:

```cpp
while (!data_ready) {
    cv.wait(lk);
}
```

So even if the **wrong** thread wakes up, or a **spurious** wakeup occurs, it simply goes back to sleep.

---

## A Concrete Scenario

```cpp
// 3 threads all waiting on the same cv
// notify_one() is called — Thread B wakes up

// Thread A: still sleeping — condition checked → false → back to sleep
// Thread B: wakes up — condition checked → true  → proceeds ✅
// Thread C: still sleeping — condition checked → false → back to sleep
```

But the next call to `notify_one()` might wake Thread C, or Thread A — you cannot predict it.

---

## When You Need a Specific Thread

If you need to wake a **specific** thread, a condition variable is the wrong tool. Instead use:

| Mechanism | Use case |
|---|---|
| One `cv` **per thread** | Each thread has its own cv, you notify the exact one you want |
| `std::counting_semaphore` (C++20) | Signal specific slots |
| `std::binary_semaphore` (C++20) | One dedicated signal per thread |

```cpp
// Per-thread condition variable pattern
struct Worker {
    std::mutex mtx;
    std::condition_variable cv;
    bool ready = false;
};

std::vector<Worker> workers(3);

// Wake specifically worker[1]
{
    std::unique_lock lk(workers[1].mtx);
    workers[1].ready = true;
    workers[1].cv.notify_one();  // Only worker[1] is listening on this cv
}
```

Full example:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <chrono>
#include <string>

// -------------------------------------------------------
// Each worker owns its own synchronization primitives,
// so notify_one() on its cv wakes exactly that worker.
// -------------------------------------------------------

struct Worker {
    std::mutex              mtx;
    std::condition_variable cv;
    bool                    ready = false;
    int                     id    = 0;
};

// Global print mutex — prevents interleaved console output
std::mutex print_mtx;

void log(const std::string& msg) {
    std::unique_lock lk(print_mtx);
    std::cout << msg << "\n";
}

// -------------------------------------------------------
// Worker thread function
// Each thread blocks on its own cv, waiting for ready=true
// -------------------------------------------------------

void worker_func(Worker& w) {
    log("  [Worker " + std::to_string(w.id) + "] Started, waiting for signal...");

    std::unique_lock lk(w.mtx);
    w.cv.wait(lk, [&w] { return w.ready; });  // Predicate guards against spurious wakeups

    log("  [Worker " + std::to_string(w.id) + "] Woke up! Doing work...");

    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(200));

    log("  [Worker " + std::to_string(w.id) + "] Done.");
}

// -------------------------------------------------------
// Signal a specific worker by index
// -------------------------------------------------------

void signal_worker(std::vector<Worker>& workers, int index) {
    log("\n[Main] Signalling worker " + std::to_string(index) + " specifically...");

    {
        std::unique_lock lk(workers[index].mtx);
        workers[index].ready = true;
    } // Unlock BEFORE notify — best practice to avoid immediate re-block

    workers[index].cv.notify_one();  // Only worker[index] listens on this cv
}

// -------------------------------------------------------
// Main
// -------------------------------------------------------

int main() {
    constexpr int NUM_WORKERS = 4;

    // Workers can't be moved/copied (mutex/cv are not movable),
    // so we manage them via unique_ptr
    std::vector<std::unique_ptr<Worker>> workers;
    workers.reserve(NUM_WORKERS);

    for (int i = 0; i < NUM_WORKERS; ++i) {
        auto w = std::make_unique<Worker>();
        w->id  = i;
        workers.push_back(std::move(w));
    }

    // Launch all worker threads — they all immediately block on their own cv
    std::vector<std::thread> threads;
    threads.reserve(NUM_WORKERS);

    for (auto& w : workers) {
        threads.emplace_back(worker_func, std::ref(*w));
    }

    // Give threads time to reach their wait() call
    std::this_thread::sleep_for(std::chrono::milliseconds(100));

    // -------------------------------------------------------
    // Selectively wake specific workers — others stay asleep
    // -------------------------------------------------------

    signal_worker(workers, 2);   // Wake only worker 2
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    signal_worker(workers, 0);   // Wake only worker 0
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    signal_worker(workers, 3);   // Wake only worker 3
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    signal_worker(workers, 1);   // Wake only worker 1
    std::this_thread::sleep_for(std::chrono::milliseconds(300));

    // Wait for all threads to finish
    for (auto& t : threads) {
        t.join();
    }

    log("\n[Main] All workers done.");
    return 0;
}
```

---

## Expected Output

```
  [Worker 0] Started, waiting for signal...
  [Worker 1] Started, waiting for signal...
  [Worker 2] Started, waiting for signal...
  [Worker 3] Started, waiting for signal...

[Main] Signalling worker 2 specifically...
  [Worker 2] Woke up! Doing work...
  [Worker 2] Done.

[Main] Signalling worker 0 specifically...
  [Worker 0] Woke up! Doing work...
  [Worker 0] Done.

[Main] Signalling worker 3 specifically...
  [Worker 3] Woke up! Doing work...
  [Worker 3] Done.

[Main] Signalling worker 1 specifically...
  [Worker 1] Woke up! Doing work...
  [Worker 1] Done.

[Main] All workers done.
```

Workers 0, 1, and 3 stay fully asleep while worker 2 is running — demonstrating true targeted wakeup.

---

## Key Design Points

**`unique_ptr` for Workers** — `std::mutex` and `std::condition_variable` are neither copyable nor movable, so they can't live directly in a `std::vector` that resizes. Wrapping in `unique_ptr` solves this cleanly.

**Unlock before `notify_one()`** — The `unique_lock` is released before calling `notify_one()` by scoping it in `{}`. This avoids the woken thread immediately blocking again trying to re-acquire the mutex from inside `wait()`.

**Predicate in `wait()`** — `cv.wait(lk, [&w]{ return w.ready; })` guards against spurious wakeups. Without it, a worker could theoretically wake and proceed even though `ready` is still false.

---

## Clarification `threads.emplace_back(worker_func, std::ref(*w))`


`w` in the range loop is of type `std::unique_ptr<Worker>&` — a reference to a smart pointer:

```cpp
std::vector<std::unique_ptr<Worker>> workers;

for (auto& w : workers) {
    // w  is: std::unique_ptr<Worker>&   — a smart pointer
    // *w  is: Worker&                   — the actual Worker object
}
```

So the dereference layers are:

```cpp
w          // unique_ptr<Worker>  — the smart pointer itself
*w         // Worker              — the object the pointer points to
std::ref(*w) // reference_wrapper<Worker> — a copyable reference to the Worker
```

---

## Why `std::ref` at all?

`std::thread` **copies** its arguments by default. Since `Worker` contains a `std::mutex` and `std::condition_variable` — which are **not copyable** — passing it directly would fail to compile:

```cpp
threads.emplace_back(worker_func, *w);        // ❌ Tries to copy Worker — won't compile
threads.emplace_back(worker_func, std::ref(*w)); // ✅ Passes a reference — no copy
```

`std::ref(*w)` wraps the reference in a `std::reference_wrapper<Worker>`, which **is** copyable and which `std::thread` knows how to unwrap back into a `Worker&` when invoking `worker_func`.

---

## Why Not Just Pass the `unique_ptr`?

```cpp
threads.emplace_back(worker_func, w);   // ❌ unique_ptr is not copyable either
```

And even if you moved it, the `unique_ptr` would be consumed by the thread, leaving `workers` with a dangling entry — you'd lose ownership.

---

## The Full Chain Summarized

```cpp
//  w       →  unique_ptr<Worker>&    smart pointer (loop variable)
//  *w      →  Worker&                actual object on the heap
//  ref(*w) →  reference_wrapper<Worker>  copyable wrapper around Worker&
//                                        thread safely holds a reference,
//                                        no copy, no ownership transfer
```
---


## Summary

- The standard gives **no guarantee** about which thread wakes
- Treat it as arbitrary/random for correctness purposes
- **Always use a predicate** with `wait()` to guard against wrong-thread and spurious wakeups
- If you need to target a specific thread, use per-thread CVs or C++20 semaphores
