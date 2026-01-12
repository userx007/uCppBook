# Mutex Fundamentals in C++

**Core Topics:**
- What mutexes are and why they're needed
- Basic `std::mutex` usage with lock/unlock
- Race condition examples showing problems without mutexes
- RAII-based locking with `std::lock_guard` and `std::unique_lock`
- Thread-safe data structure implementation
- Deadlock scenarios and prevention using `std::scoped_lock`

**Code Examples Include:**
- Safe vs unsafe counter incrementing
- Exception-safe locking patterns
- Flexible locking with deferred and try_lock
- Thread-safe logger class
- Performance optimization techniques
- Deadlock demonstration and solution

**Key Takeaways:**
- Always prefer RAII wrappers over manual locking
- Keep critical sections minimal
- Use `std::scoped_lock` for multiple mutexes
- Understand common pitfalls like deadlocks and race conditions

---

# Mutex Fundamentals in C++

## Introduction

A mutex (mutual exclusion) is a synchronization primitive used to protect shared data from being simultaneously accessed by multiple threads. When multiple threads need to read and write shared resources, mutexes ensure that only one thread can access the critical section at a time, preventing race conditions and data corruption.

The C++ Standard Library provides several mutex types in the `<mutex>` header, with `std::mutex` being the most fundamental.

## What is a Mutex?

A mutex acts like a lock on a door. When a thread wants to access shared data:
1. It attempts to **lock** the mutex
2. If the mutex is already locked by another thread, the requesting thread waits
3. Once it acquires the lock, it performs its operation
4. It **unlocks** the mutex when done, allowing other threads to proceed

## std::mutex Basics

### Basic Usage

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;
int shared_counter = 0;

void increment() {
    for (int i = 0; i < 100000; ++i) {
        mtx.lock();           // Acquire the lock
        ++shared_counter;     // Critical section
        mtx.unlock();         // Release the lock
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    
    t1.join();
    t2.join();
    
    std::cout << "Final counter: " << shared_counter << std::endl;
    // Output: Final counter: 200000 (correct result)
    return 0;
}
```

### Without Mutex (Race Condition)

```cpp
#include <iostream>
#include <thread>

int shared_counter = 0;

void increment_unsafe() {
    for (int i = 0; i < 100000; ++i) {
        ++shared_counter;  // Race condition!
    }
}

int main() {
    std::thread t1(increment_unsafe);
    std::thread t2(increment_unsafe);
    
    t1.join();
    t2.join();
    
    std::cout << "Final counter: " << shared_counter << std::endl;
    // Output: Unpredictable (likely less than 200000)
    return 0;
}
```

## RAII-Based Locking with std::lock_guard

Manually calling `lock()` and `unlock()` is error-prone. If an exception occurs between lock and unlock, the mutex remains locked forever (deadlock). The solution is RAII (Resource Acquisition Is Initialization) through `std::lock_guard`.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <stdexcept>

std::mutex mtx;
int shared_data = 0;

void safe_increment() {
    for (int i = 0; i < 100000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);  // Locks on construction
        ++shared_data;
        // Automatically unlocks when lock goes out of scope
        // Even if an exception is thrown!
    }
}

void risky_operation() {
    std::lock_guard<std::mutex> lock(mtx);
    ++shared_data;
    
    if (shared_data == 50) {
        throw std::runtime_error("Something went wrong");
    }
    // Mutex is still unlocked even though exception was thrown
}

int main() {
    std::thread t1(safe_increment);
    std::thread t2(safe_increment);
    
    t1.join();
    t2.join();
    
    std::cout << "Final value: " << shared_data << std::endl;
    return 0;
}
```

## std::unique_lock - Flexible Locking

`std::unique_lock` provides more flexibility than `std::lock_guard`. It allows deferred locking, manual unlocking, and is movable.

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void flexible_locking() {
    // Deferred locking - doesn't lock immediately
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    // Do some work that doesn't need the lock
    std::cout << "Doing work without lock..." << std::endl;
    
    // Now lock when needed
    lock.lock();
    std::cout << "Critical section" << std::endl;
    lock.unlock();
    
    // More work without lock
    std::cout << "More work..." << std::endl;
    
    // Lock again
    lock.lock();
    std::cout << "Another critical section" << std::endl;
    // Automatically unlocked when lock is destroyed
}

void try_lock_example() {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    if (lock.try_lock()) {
        std::cout << "Lock acquired successfully" << std::endl;
        // Do work
    } else {
        std::cout << "Could not acquire lock" << std::endl;
    }
}

int main() {
    std::thread t1(flexible_locking);
    t1.join();
    
    std::thread t2(try_lock_example);
    t2.join();
    
    return 0;
}
```

## Protecting Data Structures

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>
#include <string>

class ThreadSafeLogger {
private:
    std::mutex mtx;
    std::vector<std::string> logs;

public:
    void add_log(const std::string& message) {
        std::lock_guard<std::mutex> lock(mtx);
        logs.push_back(message);
    }
    
    size_t get_log_count() {
        std::lock_guard<std::mutex> lock(mtx);
        return logs.size();
    }
    
    void print_logs() {
        std::lock_guard<std::mutex> lock(mtx);
        for (const auto& log : logs) {
            std::cout << log << std::endl;
        }
    }
};

void worker(ThreadSafeLogger& logger, int id) {
    for (int i = 0; i < 5; ++i) {
        logger.add_log("Thread " + std::to_string(id) + 
                       " - Message " + std::to_string(i));
    }
}

int main() {
    ThreadSafeLogger logger;
    
    std::thread t1(worker, std::ref(logger), 1);
    std::thread t2(worker, std::ref(logger), 2);
    std::thread t3(worker, std::ref(logger), 3);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "Total logs: " << logger.get_log_count() << std::endl;
    logger.print_logs();
    
    return 0;
}
```

## Common Pitfalls

### Deadlock Example

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx1, mtx2;

void thread1_func() {
    std::lock_guard<std::mutex> lock1(mtx1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(mtx2);  // Waits for mtx2
    std::cout << "Thread 1" << std::endl;
}

void thread2_func() {
    std::lock_guard<std::mutex> lock2(mtx2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock1(mtx1);  // Waits for mtx1
    std::cout << "Thread 2" << std::endl;
}

// This will deadlock! Don't run this code.
```

### Avoiding Deadlock with std::scoped_lock (C++17)

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx1, mtx2;

void safe_thread1() {
    std::scoped_lock lock(mtx1, mtx2);  // Locks both atomically
    std::cout << "Thread 1 acquired both locks" << std::endl;
}

void safe_thread2() {
    std::scoped_lock lock(mtx2, mtx1);  // Order doesn't matter
    std::cout << "Thread 2 acquired both locks" << std::endl;
}

int main() {
    std::thread t1(safe_thread1);
    std::thread t2(safe_thread2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Performance Considerations

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
int counter = 0;

// Bad: Lock held too long
void inefficient() {
    std::lock_guard<std::mutex> lock(mtx);
    ++counter;
    // Expensive operation while holding lock
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
}

// Good: Minimize critical section
void efficient() {
    // Do expensive work outside lock
    std::this_thread::sleep_for(std::chrono::milliseconds(1));
    
    // Quick critical section
    {
        std::lock_guard<std::mutex> lock(mtx);
        ++counter;
    }
}
```

## std::try_lock - Non-Blocking Attempt

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void try_lock_example() {
    if (mtx.try_lock()) {
        std::cout << "Lock acquired by " 
                  << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        mtx.unlock();
    } else {
        std::cout << "Lock not available for " 
                  << std::this_thread::get_id() << std::endl;
    }
}

int main() {
    std::thread t1(try_lock_example);
    std::thread t2(try_lock_example);
    std::thread t3(try_lock_example);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

## Summary

### Key Concepts

- **std::mutex** is the fundamental mutual exclusion primitive for protecting shared data
- **Race conditions** occur when multiple threads access shared data without synchronization
- **Critical sections** are code regions that access shared resources and must be protected

### Locking Mechanisms

- **Manual locking**: `lock()` and `unlock()` (error-prone, avoid when possible)
- **std::lock_guard**: RAII-based automatic locking/unlocking (recommended for simple cases)
- **std::unique_lock**: Flexible locking with deferred and manual control
- **std::scoped_lock** (C++17): Deadlock-free locking of multiple mutexes

### Best Practices

1. Always use RAII wrappers (`lock_guard`, `unique_lock`, `scoped_lock`) instead of manual locking
2. Keep critical sections as small as possible to minimize contention
3. Never lock the same mutex twice in the same thread (unless using `std::recursive_mutex`)
4. Use `std::scoped_lock` when locking multiple mutexes to avoid deadlock
5. Ensure all access to shared data is protected by the same mutex
6. Consider the granularity of locking—too coarse reduces parallelism, too fine increases overhead

### Common Pitfalls

- Forgetting to unlock (solved by RAII)
- Deadlocks from circular lock dependencies
- Holding locks during expensive operations
- Unprotected access to shared data
- Accessing destroyed mutexes

Mutexes are the foundation of thread synchronization in C++. Understanding them deeply is essential for writing correct and efficient multithreaded programs.