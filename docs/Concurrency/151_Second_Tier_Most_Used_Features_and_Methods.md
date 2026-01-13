# C++ Concurrency: Second Tier Most Used Features and Methods

## Overview

Beyond the fundamental concurrency features, C++ provides additional specialized tools for more advanced synchronization patterns, thread management, and parallel algorithms. These features are less commonly used but critical for specific scenarios requiring fine-grained control or specialized synchronization.

## 1. std::call_once and std::once_flag - Thread-Safe Initialization

Ensures a function is called exactly once, even when invoked from multiple threads simultaneously.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::once_flag initFlag;
std::once_flag resourceFlag;

class Singleton {
private:
    Singleton() {
        std::cout << "Singleton initialized" << std::endl;
    }
    
public:
    static Singleton& getInstance() {
        static Singleton instance;
        return instance;
    }
    
    void doWork() {
        std::cout << "Working..." << std::endl;
    }
};

void expensiveInitialization() {
    std::cout << "Performing expensive one-time initialization..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

void workerThread(int id) {
    // This will only execute once across all threads
    std::call_once(initFlag, expensiveInitialization);
    
    std::cout << "Thread " << id << " doing work" << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(workerThread, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

**Use Cases:**
- Lazy initialization of singletons
- One-time resource allocation
- Thread-safe static initialization

## 2. std::shared_lock (C++14) - Shared Ownership Locking

Works with `std::shared_mutex` to acquire shared (read) locks.

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <map>
#include <string>
#include <chrono>

class ThreadSafeCache {
private:
    mutable std::shared_mutex mutex;
    std::map<int, std::string> cache;
    
public:
    void insert(int key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(mutex);
        cache[key] = value;
        std::cout << "Inserted: " << key << " -> " << value << std::endl;
    }
    
    std::string read(int key) const {
        std::shared_lock<std::shared_mutex> lock(mutex);
        auto it = cache.find(key);
        if (it != cache.end()) {
            return it->second;
        }
        return "Not found";
    }
    
    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex);
        return cache.size();
    }
};

int main() {
    ThreadSafeCache cache;
    
    // Writer thread
    std::thread writer([&cache]() {
        for (int i = 0; i < 5; ++i) {
            cache.insert(i, "Value_" + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    // Multiple reader threads
    std::vector<std::thread> readers;
    for (int i = 0; i < 3; ++i) {
        readers.emplace_back([&cache, i]() {
            for (int j = 0; j < 5; ++j) {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
                std::string value = cache.read(j);
                std::cout << "Reader " << i << " read key " << j 
                          << ": " << value << std::endl;
            }
        });
    }
    
    writer.join();
    for (auto& reader : readers) {
        reader.join();
    }
    
    return 0;
}
```

## 3. std::recursive_mutex - Reentrant Mutex

Allows the same thread to acquire the mutex multiple times.

```cpp
#include <iostream>
#include <thread>
#include <mutex>

class RecursiveCounter {
private:
    std::recursive_mutex mtx;
    int count = 0;
    
public:
    void increment() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        ++count;
    }
    
    void incrementMultiple(int times) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        for (int i = 0; i < times; ++i) {
            increment(); // This would deadlock with std::mutex
        }
    }
    
    int getCount() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return count;
    }
    
    void printAndIncrement() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        std::cout << "Current count: " << getCount() << std::endl;
        increment();
    }
};

int main() {
    RecursiveCounter counter;
    
    std::thread t1([&counter]() {
        counter.incrementMultiple(5);
    });
    
    std::thread t2([&counter]() {
        for (int i = 0; i < 3; ++i) {
            counter.printAndIncrement();
        }
    });
    
    t1.join();
    t2.join();
    
    std::cout << "Final count: " << counter.getCount() << std::endl;
    return 0;
}
```

**Warning:** Recursive mutexes have performance overhead and may indicate poor design. Use sparingly.

## 4. std::timed_mutex - Mutex with Timeout

Provides `try_lock_for()` and `try_lock_until()` for timeout-based locking.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::timed_mutex timedMtx;

void tryLockWithTimeout(int id, int milliseconds) {
    auto timeout = std::chrono::milliseconds(milliseconds);
    
    std::cout << "Thread " << id << " attempting to acquire lock..." << std::endl;
    
    if (timedMtx.try_lock_for(timeout)) {
        std::cout << "Thread " << id << " acquired lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        timedMtx.unlock();
        std::cout << "Thread " << id << " released lock" << std::endl;
    } else {
        std::cout << "Thread " << id << " failed to acquire lock within timeout" 
                  << std::endl;
    }
}

void tryLockUntilTime(int id) {
    auto deadline = std::chrono::steady_clock::now() + std::chrono::milliseconds(150);
    
    std::cout << "Thread " << id << " attempting with deadline..." << std::endl;
    
    if (timedMtx.try_lock_until(deadline)) {
        std::cout << "Thread " << id << " acquired lock before deadline" << std::endl;
        timedMtx.unlock();
    } else {
        std::cout << "Thread " << id << " deadline expired" << std::endl;
    }
}

int main() {
    std::thread t1(tryLockWithTimeout, 1, 100);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::thread t2(tryLockWithTimeout, 2, 100);
    std::thread t3(tryLockUntilTime, 3);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

## 5. std::packaged_task - Wrapping Callable Objects

Wraps any callable target for asynchronous execution with a future.

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <functional>
#include <queue>

int calculate(int x, int y) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x * y + x - y;
}

class TaskQueue {
private:
    std::queue<std::packaged_task<int()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    
public:
    std::future<int> addTask(std::packaged_task<int()>&& task) {
        std::future<int> result = task.get_future();
        {
            std::lock_guard<std::mutex> lock(mtx);
            tasks.push(std::move(task));
        }
        cv.notify_one();
        return result;
    }
    
    void workerThread() {
        while (true) {
            std::packaged_task<int()> task;
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this]{ return !tasks.empty() || done; });
                
                if (done && tasks.empty()) {
                    return;
                }
                
                task = std::move(tasks.front());
                tasks.pop();
            }
            task();
        }
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            done = true;
        }
        cv.notify_all();
    }
};

int main() {
    TaskQueue queue;
    std::thread worker(&TaskQueue::workerThread, &queue);
    
    // Create packaged tasks
    std::packaged_task<int()> task1(std::bind(calculate, 10, 5));
    std::packaged_task<int()> task2(std::bind(calculate, 20, 3));
    std::packaged_task<int()> task3([]() { return calculate(15, 7); });
    
    // Add tasks and get futures
    std::future<int> result1 = queue.addTask(std::move(task1));
    std::future<int> result2 = queue.addTask(std::move(task2));
    std::future<int> result3 = queue.addTask(std::move(task3));
    
    // Get results
    std::cout << "Result 1: " << result1.get() << std::endl;
    std::cout << "Result 2: " << result2.get() << std::endl;
    std::cout << "Result 3: " << result3.get() << std::endl;
    
    queue.shutdown();
    worker.join();
    
    return 0;
}
```

## 6. std::atomic_flag - Lock-Free Boolean Flag

The only guaranteed lock-free atomic type, useful for spinlocks.

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

class Spinlock {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // Spin until we acquire the lock
        }
    }
    
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

Spinlock spinlock;
int sharedCounter = 0;

void incrementWithSpinlock(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        spinlock.lock();
        ++sharedCounter;
        spinlock.unlock();
    }
}

// Simple example using atomic_flag directly
std::atomic_flag ready = ATOMIC_FLAG_INIT;

void waitForSignal(int id) {
    std::cout << "Thread " << id << " waiting..." << std::endl;
    while (!ready.test(std::memory_order_acquire)) {
        std::this_thread::yield();
    }
    std::cout << "Thread " << id << " proceeding!" << std::endl;
}

int main() {
    // Spinlock example
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(incrementWithSpinlock, 1000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Counter with spinlock: " << sharedCounter << std::endl;
    
    // Signal example
    threads.clear();
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(waitForSignal, i);
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    ready.test_and_set(std::memory_order_release);
    std::cout << "Signal sent!" << std::endl;
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

**Key Methods:**
- `test_and_set()`: Atomically sets to true and returns previous value
- `clear()`: Atomically sets to false
- `test()`: Reads current value (C++20)

## 7. std::jthread (C++20) - Self-Joining Thread

Automatically joins on destruction and supports cooperative cancellation.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

// Note: This requires C++20
#if __cplusplus >= 202002L

void interruptibleTask(std::stop_token stoken, int id) {
    int count = 0;
    while (!stoken.stop_requested() && count < 10) {
        std::cout << "Thread " << id << " iteration " << count << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        ++count;
    }
    
    if (stoken.stop_requested()) {
        std::cout << "Thread " << id << " was interrupted" << std::endl;
    } else {
        std::cout << "Thread " << id << " completed normally" << std::endl;
    }
}

void demonstrateJThread() {
    std::cout << "=== jthread example ===" << std::endl;
    
    {
        std::jthread t1(interruptibleTask, 1);
        std::jthread t2(interruptibleTask, 2);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(600));
        
        // Request stop
        t1.request_stop();
        std::cout << "Requested stop for thread 1" << std::endl;
        
        // t1 and t2 automatically join when going out of scope
    }
    
    std::cout << "Both threads finished" << std::endl;
}

#endif

int main() {
#if __cplusplus >= 202002L
    demonstrateJThread();
#else
    std::cout << "jthread requires C++20 or later" << std::endl;
    
    // Fallback to regular thread
    std::atomic<bool> stopRequested{false};
    
    std::thread t([&stopRequested]() {
        int count = 0;
        while (!stopRequested && count < 10) {
            std::cout << "Thread iteration " << count << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
            ++count;
        }
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(600));
    stopRequested = true;
    t.join();
#endif
    
    return 0;
}
```

## 8. Memory Ordering for Atomics

Fine-grained control over memory synchronization and visibility.

```cpp
#include <iostream>
#include <thread>
#include <atomic>

std::atomic<int> data{0};
std::atomic<bool> ready{false};

// Producer thread
void producer() {
    data.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release); // Synchronizes with acquire
}

// Consumer thread
void consumer() {
    while (!ready.load(std::memory_order_acquire)) { // Synchronizes with release
        std::this_thread::yield();
    }
    std::cout << "Data: " << data.load(std::memory_order_relaxed) << std::endl;
}

// Lock-free counter with different orderings
std::atomic<long> counter{0};

void incrementRelaxed(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void incrementSeqCst(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.fetch_add(1, std::memory_order_seq_cst);
    }
}

int main() {
    // Producer-consumer example
    std::thread prod(producer);
    std::thread cons(consumer);
    
    prod.join();
    cons.join();
    
    // Counter example
    counter = 0;
    std::thread t1(incrementRelaxed, 100000);
    std::thread t2(incrementRelaxed, 100000);
    
    t1.join();
    t2.join();
    
    std::cout << "Final counter: " << counter.load() << std::endl;
    
    return 0;
}
```

**Memory Orderings:**
- `memory_order_relaxed`: No synchronization guarantees
- `memory_order_acquire`: Synchronizes with release operations
- `memory_order_release`: Synchronizes with acquire operations
- `memory_order_acq_rel`: Both acquire and release
- `memory_order_seq_cst`: Sequential consistency (default, strictest)

## 9. std::latch and std::barrier (C++20) - Thread Coordination

Single-use and reusable synchronization points for multiple threads.

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

#if __cplusplus >= 202002L
#include <latch>
#include <barrier>

void demonstrateLatch() {
    std::cout << "=== Latch example ===" << std::endl;
    const int numThreads = 5;
    std::latch workDone(numThreads);
    std::latch startSignal(1);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            startSignal.wait(); // Wait for start signal
            std::cout << "Thread " << i << " working..." << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100 * (i + 1)));
            std::cout << "Thread " << i << " done" << std::endl;
            workDone.count_down(); // Signal completion
        });
    }
    
    std::cout << "Starting all threads..." << std::endl;
    startSignal.count_down(); // Release all threads
    
    workDone.wait(); // Wait for all threads to complete
    std::cout << "All threads completed" << std::endl;
    
    for (auto& t : threads) {
        t.join();
    }
}

void demonstrateBarrier() {
    std::cout << "\n=== Barrier example ===" << std::endl;
    const int numThreads = 3;
    const int numPhases = 3;
    
    std::barrier sync_point(numThreads, []() noexcept {
        std::cout << "--- Phase completed, all threads synchronized ---" << std::endl;
    });
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            for (int phase = 0; phase < numPhases; ++phase) {
                std::cout << "Thread " << i << " phase " << phase << std::endl;
                std::this_thread::sleep_for(std::chrono::milliseconds(50 * (i + 1)));
                sync_point.arrive_and_wait(); // Wait for all threads
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}

#endif

int main() {
#if __cplusplus >= 202002L
    demonstrateLatch();
    demonstrateBarrier();
#else
    std::cout << "latch and barrier require C++20 or later" << std::endl;
    
    // Manual barrier implementation using mutex and condition_variable
    std::mutex mtx;
    std::condition_variable cv;
    int counter = 0;
    const int numThreads = 3;
    
    auto barrierWait = [&]() {
        std::unique_lock<std::mutex> lock(mtx);
        ++counter;
        if (counter == numThreads) {
            counter = 0;
            cv.notify_all();
        } else {
            cv.wait(lock, [&]{ return counter == 0; });
        }
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back([&, i]() {
            std::cout << "Thread " << i << " at barrier" << std::endl;
            barrierWait();
            std::cout << "Thread " << i << " passed barrier" << std::endl;
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
#endif
    
    return 0;
}
```

## 10. std::shared_future - Multiple Waiters

Allows multiple threads to wait for the same asynchronous result.

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <vector>
#include <chrono>

int expensiveComputation() {
    std::cout << "Performing expensive computation..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 42;
}

void waitForResult(std::shared_future<int> fut, int id) {
    std::cout << "Thread " << id << " waiting for result..." << std::endl;
    int result = fut.get(); // Multiple threads can call get()
    std::cout << "Thread " << id << " got result: " << result << std::endl;
}

int main() {
    // Create a shared_future from a regular future
    std::future<int> fut = std::async(std::launch::async, expensiveComputation);
    std::shared_future<int> sharedFut = fut.share();
    
    // Multiple threads can wait for the same result
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(waitForResult, sharedFut, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Alternatively, create shared_future directly
    std::shared_future<int> sharedFut2 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        return 100;
    }).share();
    
    std::cout << "\nSecond computation result: " << sharedFut2.get() << std::endl;
    std::cout << "Can call get() again: " << sharedFut2.get() << std::endl;
    
    return 0;
}
```

**Key Difference from std::future:**
- `std::future`: Can only be retrieved once, move-only
- `std::shared_future`: Can be retrieved multiple times, copyable

## Summary

These second-tier concurrency features provide specialized functionality for advanced use cases:

**Initialization & Lifetime Management:**
- `std::call_once` for thread-safe one-time initialization
- `std::jthread` for automatic resource cleanup and cancellation support

**Advanced Synchronization:**
- `std::recursive_mutex` for reentrant locking
- `std::timed_mutex` for timeout-based lock acquisition
- `std::shared_lock` for efficient reader-writer patterns
- `std::latch` and `std::barrier` for coordinating multiple threads

**Task Management:**
- `std::packaged_task` for wrapping callable objects with futures
- `std::shared_future` for broadcasting results to multiple threads

**Low-Level Control:**
- `std::atomic_flag` for lock-free flags and spinlocks
- Memory ordering options for fine-tuning atomic operations

These features are essential when standard mutexes and threads don't quite fit your needs, whether you require timeout capabilities, recursive locking, efficient read-heavy access patterns, or sophisticated thread coordination. While less common than the primary concurrency features, mastering these tools enables you to build more robust and efficient concurrent systems tailored to specific performance and synchronization requirements.