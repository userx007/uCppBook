# C++ Concurrency: Most Used Features and Methods

## Overview

C++ concurrency support, introduced in C++11 and enhanced in subsequent standards, provides powerful tools for writing multi-threaded applications. This guide covers the most frequently used features and methods that form the foundation of concurrent programming in modern C++.

## 1. std::thread - Basic Thread Management

The `std::thread` class is the fundamental building block for creating and managing threads.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void printNumbers(int count) {
    for (int i = 0; i < count; ++i) {
        std::cout << "Thread ID: " << std::this_thread::get_id() 
                  << " - Number: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    // Creating threads
    std::thread t1(printNumbers, 5);
    std::thread t2(printNumbers, 5);
    
    // Wait for threads to complete
    t1.join();
    t2.join();
    
    std::cout << "All threads completed" << std::endl;
    return 0;
}
```

**Key Methods:**
- `join()`: Blocks until the thread completes execution
- `detach()`: Separates the thread from the thread object, allowing independent execution
- `joinable()`: Checks if the thread is joinable
- `get_id()`: Returns the thread identifier

## 2. std::mutex - Mutual Exclusion

Mutexes prevent race conditions by ensuring only one thread accesses shared resources at a time.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
int sharedCounter = 0;

void incrementCounter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        mtx.lock();
        ++sharedCounter;
        mtx.unlock();
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(incrementCounter, 1000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter value: " << sharedCounter << std::endl;
    return 0;
}
```

**Key Methods:**
- `lock()`: Acquires the mutex, blocking if necessary
- `unlock()`: Releases the mutex
- `try_lock()`: Attempts to acquire the mutex without blocking

## 3. std::lock_guard - RAII Mutex Wrapper

Provides exception-safe mutex locking using RAII (Resource Acquisition Is Initialization).

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
std::vector<int> sharedData;

void addData(int value) {
    std::lock_guard<std::mutex> lock(mtx);
    // Mutex automatically released when lock goes out of scope
    sharedData.push_back(value);
    std::cout << "Added: " << value << std::endl;
}

int main() {
    std::thread t1(addData, 10);
    std::thread t2(addData, 20);
    std::thread t3(addData, 30);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

## 4. std::unique_lock - Flexible Mutex Wrapper

Offers more flexibility than `lock_guard`, including deferred locking and manual unlocking.

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void flexibleLocking(int id) {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    // Do some work without holding the lock
    std::cout << "Thread " << id << " doing preliminary work" << std::endl;
    
    // Now acquire the lock
    lock.lock();
    std::cout << "Thread " << id << " has the lock" << std::endl;
    
    // Can unlock manually if needed
    lock.unlock();
    
    // Do more work without lock
    std::cout << "Thread " << id << " released lock" << std::endl;
}

int main() {
    std::thread t1(flexibleLocking, 1);
    std::thread t2(flexibleLocking, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## 5. std::condition_variable - Thread Synchronization

Enables threads to wait for specific conditions and notify other threads.

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
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lock(mtx);
            dataQueue.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        cv.notify_one();
    }
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_all();
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !dataQueue.empty() || finished; });
        
        if (!dataQueue.empty()) {
            int value = dataQueue.front();
            dataQueue.pop();
            lock.unlock();
            std::cout << "Consumer " << id << " consumed: " << value << std::endl;
        } else if (finished) {
            break;
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

**Key Methods:**
- `wait()`: Blocks until notified
- `notify_one()`: Wakes up one waiting thread
- `notify_all()`: Wakes up all waiting threads
- `wait_for()`: Waits with a timeout
- `wait_until()`: Waits until a specific time point

## 6. std::atomic - Lock-Free Operations

Provides atomic operations on shared variables without explicit locking.

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

std::atomic<int> atomicCounter(0);

void incrementAtomic(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        atomicCounter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(incrementAtomic, 1000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Atomic counter value: " << atomicCounter.load() << std::endl;
    return 0;
}
```

**Key Methods:**
- `load()`: Atomically reads the value
- `store()`: Atomically writes a value
- `fetch_add()`: Atomically adds and returns previous value
- `fetch_sub()`: Atomically subtracts and returns previous value
- `exchange()`: Atomically replaces and returns old value
- `compare_exchange_weak/strong()`: Atomic compare-and-swap

## 7. std::future and std::promise - Asynchronous Results

Enables communication between threads for returning results from asynchronous operations.

```cpp
#include <iostream>
#include <thread>
#include <future>

int calculateSum(int a, int b) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return a + b;
}

void promiseExample() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread t([&prom]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        prom.set_value(42);
    });
    
    std::cout << "Waiting for result..." << std::endl;
    std::cout << "Result: " << fut.get() << std::endl;
    
    t.join();
}

int main() {
    // Using std::async
    std::future<int> result = std::async(std::launch::async, calculateSum, 10, 20);
    std::cout << "Doing other work..." << std::endl;
    std::cout << "Sum: " << result.get() << std::endl;
    
    promiseExample();
    
    return 0;
}
```

**Key Methods:**
- `get()`: Blocks and retrieves the result
- `wait()`: Waits for result to be ready
- `wait_for()`: Waits with timeout
- `valid()`: Checks if future has a shared state
- `set_value()`: Sets the promise value (promise)
- `set_exception()`: Sets an exception (promise)

## 8. std::async - High-Level Async Execution

Provides a simple way to run functions asynchronously.

```cpp
#include <iostream>
#include <future>
#include <vector>

int processData(int data) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return data * 2;
}

int main() {
    std::vector<std::future<int>> futures;
    
    // Launch async tasks
    for (int i = 0; i < 5; ++i) {
        futures.push_back(std::async(std::launch::async, processData, i));
    }
    
    // Collect results
    for (auto& fut : futures) {
        std::cout << "Result: " << fut.get() << std::endl;
    }
    
    return 0;
}
```

**Launch Policies:**
- `std::launch::async`: Guarantees asynchronous execution
- `std::launch::deferred`: Lazy evaluation on first access
- `std::launch::async | std::launch::deferred`: Implementation decides

## 9. std::shared_mutex (C++17) - Reader-Writer Lock

Allows multiple readers or a single writer.

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <vector>

std::shared_mutex sharedMtx;
int sharedResource = 0;

void reader(int id) {
    std::shared_lock<std::shared_mutex> lock(sharedMtx);
    std::cout << "Reader " << id << " reads: " << sharedResource << std::endl;
}

void writer(int id, int value) {
    std::unique_lock<std::shared_mutex> lock(sharedMtx);
    sharedResource = value;
    std::cout << "Writer " << id << " wrote: " << value << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    
    threads.emplace_back(writer, 1, 100);
    threads.emplace_back(reader, 1);
    threads.emplace_back(reader, 2);
    threads.emplace_back(writer, 2, 200);
    threads.emplace_back(reader, 3);
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## 10. std::scoped_lock (C++17) - Multiple Mutex Locking

Acquires multiple mutexes simultaneously, avoiding deadlocks.

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx1, mtx2;
int resource1 = 0, resource2 = 0;

void transferResources(int amount) {
    std::scoped_lock lock(mtx1, mtx2);
    resource1 -= amount;
    resource2 += amount;
    std::cout << "Transferred " << amount << std::endl;
}

int main() {
    resource1 = 100;
    resource2 = 0;
    
    std::thread t1(transferResources, 30);
    std::thread t2(transferResources, 50);
    
    t1.join();
    t2.join();
    
    std::cout << "Resource1: " << resource1 << ", Resource2: " << resource2 << std::endl;
    return 0;
}
```

## Summary

C++ concurrency features provide a comprehensive toolkit for multi-threaded programming:

**Basic Threading:**
- `std::thread` for creating and managing threads
- Thread lifecycle management with `join()` and `detach()`

**Synchronization Primitives:**
- `std::mutex` for mutual exclusion
- `std::lock_guard` and `std::unique_lock` for RAII-style locking
- `std::condition_variable` for thread coordination
- `std::shared_mutex` for reader-writer scenarios

**Atomic Operations:**
- `std::atomic` for lock-free operations on simple types
- Memory ordering options for fine-grained control

**Asynchronous Programming:**
- `std::async` for simple asynchronous task execution
- `std::future` and `std::promise` for result communication
- High-level abstractions that hide thread management complexity

**Advanced Features:**
- `std::scoped_lock` for deadlock-free multiple mutex acquisition
- Various timeout and timed waiting mechanisms

The choice of which feature to use depends on your specific needs: use atomics for simple counters, mutexes for protecting critical sections, condition variables for producer-consumer patterns, and async/future for task-based parallelism. Modern C++ encourages using higher-level abstractions like `std::async` when possible, falling back to lower-level primitives only when necessary for performance or specific synchronization requirements.