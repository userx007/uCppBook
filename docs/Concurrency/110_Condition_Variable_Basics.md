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