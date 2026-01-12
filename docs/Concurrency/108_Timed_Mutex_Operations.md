# Timed Mutex Operations in C++

## Introduction

Timed mutex operations provide a mechanism for threads to attempt lock acquisition with a timeout, preventing indefinite blocking. Unlike `std::mutex`, which blocks indefinitely when trying to acquire a lock, `std::timed_mutex` allows threads to specify how long they're willing to wait. This is particularly useful in scenarios where you want to avoid deadlocks, implement fallback behavior, or maintain responsiveness in your application.

## Core Concepts

The C++ Standard Library provides `std::timed_mutex` (defined in `<mutex>`), which extends the basic mutex interface with two time-based locking methods:

- **`try_lock_for(duration)`**: Attempts to lock the mutex for a specified duration
- **`try_lock_until(time_point)`**: Attempts to lock the mutex until a specific point in time

Both methods return `true` if the lock was acquired successfully, and `false` if the timeout expired before acquisition.

## Basic Timed Mutex Usage

Here's a simple example demonstrating `try_lock_for`:

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::timed_mutex tmutex;
int shared_resource = 0;

void attempt_access(int thread_id, std::chrono::milliseconds timeout) {
    std::cout << "Thread " << thread_id << " attempting to acquire lock...\n";
    
    if (tmutex.try_lock_for(timeout)) {
        std::cout << "Thread " << thread_id << " acquired lock\n";
        
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        shared_resource++;
        
        std::cout << "Thread " << thread_id << " releasing lock\n";
        tmutex.unlock();
    } else {
        std::cout << "Thread " << thread_id << " timeout - couldn't acquire lock\n";
    }
}

int main() {
    std::thread t1(attempt_access, 1, std::chrono::milliseconds(50));
    std::thread t2(attempt_access, 2, std::chrono::milliseconds(200));
    
    t1.join();
    t2.join();
    
    std::cout << "Final resource value: " << shared_resource << "\n";
    return 0;
}
```

## Using try_lock_until

The `try_lock_until` method accepts an absolute time point rather than a duration:

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::timed_mutex tmutex;

void scheduled_task(int task_id) {
    using namespace std::chrono;
    
    // Set a deadline 2 seconds from now
    auto deadline = system_clock::now() + seconds(2);
    
    std::cout << "Task " << task_id << " waiting until deadline...\n";
    
    if (tmutex.try_lock_until(deadline)) {
        std::cout << "Task " << task_id << " acquired lock before deadline\n";
        
        // Do work
        std::this_thread::sleep_for(milliseconds(500));
        
        tmutex.unlock();
    } else {
        std::cout << "Task " << task_id << " failed - deadline expired\n";
    }
}

int main() {
    std::thread t1(scheduled_task, 1);
    std::thread t2(scheduled_task, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Practical Example: Resource Pool with Timeout

Here's a more realistic example implementing a connection pool with timeout handling:

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>
#include <chrono>
#include <optional>

class ConnectionPool {
private:
    std::timed_mutex pool_mutex;
    std::vector<int> available_connections;
    
public:
    ConnectionPool(int size) {
        for (int i = 0; i < size; ++i) {
            available_connections.push_back(i);
        }
    }
    
    std::optional<int> acquire_connection(std::chrono::milliseconds timeout) {
        if (pool_mutex.try_lock_for(timeout)) {
            std::optional<int> conn;
            
            if (!available_connections.empty()) {
                conn = available_connections.back();
                available_connections.pop_back();
                std::cout << "Acquired connection " << *conn << "\n";
            }
            
            pool_mutex.unlock();
            return conn;
        }
        
        std::cout << "Timeout waiting for connection pool access\n";
        return std::nullopt;
    }
    
    void release_connection(int conn_id) {
        std::lock_guard<std::timed_mutex> lock(pool_mutex);
        available_connections.push_back(conn_id);
        std::cout << "Released connection " << conn_id << "\n";
    }
};

void worker(ConnectionPool& pool, int worker_id) {
    using namespace std::chrono_literals;
    
    auto conn = pool.acquire_connection(500ms);
    
    if (conn.has_value()) {
        // Simulate using the connection
        std::cout << "Worker " << worker_id << " using connection " << *conn << "\n";
        std::this_thread::sleep_for(200ms);
        
        pool.release_connection(*conn);
    } else {
        std::cout << "Worker " << worker_id << " couldn't get connection\n";
    }
}

int main() {
    ConnectionPool pool(2); // Pool with 2 connections
    
    std::vector<std::thread> workers;
    for (int i = 0; i < 5; ++i) {
        workers.emplace_back(worker, std::ref(pool), i);
    }
    
    for (auto& t : workers) {
        t.join();
    }
    
    return 0;
}
```

## Recursive Timed Mutex

Similar to `std::recursive_mutex`, there's also `std::recursive_timed_mutex` for cases where the same thread needs to lock multiple times:

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::recursive_timed_mutex rtmutex;

void recursive_function(int depth) {
    using namespace std::chrono_literals;
    
    if (rtmutex.try_lock_for(100ms)) {
        std::cout << "Locked at depth " << depth << "\n";
        
        if (depth > 0) {
            recursive_function(depth - 1);
        }
        
        rtmutex.unlock();
    } else {
        std::cout << "Timeout at depth " << depth << "\n";
    }
}

int main() {
    recursive_function(3);
    return 0;
}
```

## Error Handling and Best Practices

Here's an example showing proper error handling with timed mutexes:

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>
#include <stdexcept>

class SafeCounter {
private:
    std::timed_mutex mutex;
    int counter = 0;
    
public:
    bool increment_with_timeout(std::chrono::milliseconds timeout) {
        if (mutex.try_lock_for(timeout)) {
            try {
                counter++;
                
                // Simulate potential exception
                if (counter == 5) {
                    mutex.unlock();
                    throw std::runtime_error("Counter reached critical value");
                }
                
                mutex.unlock();
                return true;
            } catch (...) {
                // Ensure unlock even on exception
                mutex.unlock();
                throw;
            }
        }
        return false; // Timeout
    }
    
    int get_value() {
        std::lock_guard<std::timed_mutex> lock(mutex);
        return counter;
    }
};

int main() {
    using namespace std::chrono_literals;
    SafeCounter counter;
    
    for (int i = 0; i < 10; ++i) {
        try {
            if (counter.increment_with_timeout(100ms)) {
                std::cout << "Incremented successfully\n";
            } else {
                std::cout << "Timeout occurred\n";
            }
        } catch (const std::runtime_error& e) {
            std::cout << "Exception: " << e.what() << "\n";
        }
    }
    
    std::cout << "Final counter value: " << counter.get_value() << "\n";
    return 0;
}
```

## Performance Considerations

Timed mutexes have slightly more overhead than regular mutexes. Here's a comparison:

```cpp
#include <iostream>
#include <mutex>
#include <chrono>

void benchmark_regular_mutex() {
    std::mutex m;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100000; ++i) {
        m.lock();
        m.unlock();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Regular mutex: " << duration.count() << " μs\n";
}

void benchmark_timed_mutex() {
    using namespace std::chrono_literals;
    std::timed_mutex tm;
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 100000; ++i) {
        tm.try_lock_for(1s); // Always succeeds immediately
        tm.unlock();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Timed mutex: " << duration.count() << " μs\n";
}

int main() {
    benchmark_regular_mutex();
    benchmark_timed_mutex();
    return 0;
}
```

## Summary

Timed mutex operations in C++ provide a flexible approach to thread synchronization with timeout capabilities. The key points to remember are:

**Core Features**: `std::timed_mutex` extends `std::mutex` with `try_lock_for()` (duration-based) and `try_lock_until()` (time-point-based) methods that attempt lock acquisition with timeouts.

**Use Cases**: Timed mutexes are ideal for scenarios requiring deadlock prevention, implementing fallback behavior when locks can't be acquired, maintaining application responsiveness, and handling resource pools with timeout policies.

**Variants**: The standard library provides both `std::timed_mutex` and `std::recursive_timed_mutex` for different locking patterns.

**Best Practices**: Always check the return value of timed lock attempts, ensure proper unlock in all code paths (especially exception scenarios), use RAII wrappers like `std::unique_lock` when possible, and be mindful of the performance overhead compared to regular mutexes.

**Common Pitfalls**: Avoid forgetting to unlock after successful acquisition, setting unrealistic timeout values, and using timed mutexes when regular mutexes suffice, as they have slightly higher overhead.

Timed mutex operations strike a balance between the blocking behavior of regular mutexes and the non-blocking `try_lock()`, making them an essential tool for building robust, responsive concurrent applications.