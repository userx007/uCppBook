# Lock Guards and Unique Locks in C++

## Introduction

Lock guards and unique locks are RAII (Resource Acquisition Is Initialization) wrappers around mutexes in C++ that provide automatic, exception-safe locking and unlocking. They eliminate the need for manual mutex management and prevent common bugs like forgetting to unlock a mutex or failing to unlock when an exception occurs.

## Why RAII-Based Locking?

Traditional manual locking is error-prone:

```cpp
std::mutex mtx;

void riskyFunction() {
    mtx.lock();
    // If an exception occurs here, mutex never gets unlocked!
    performOperation();
    mtx.unlock(); // May never be reached
}
```

RAII-based locks solve this by automatically unlocking when they go out of scope, even during exceptions.

## std::lock_guard - Simple Scoped Locking

`std::lock_guard` is the simplest and most efficient lock wrapper. It locks the mutex on construction and unlocks on destruction. It cannot be unlocked manually or moved.

### Basic Usage

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mtx;
int shared_counter = 0;

void incrementCounter(int id) {
    for (int i = 0; i < 1000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        // Mutex is locked here
        ++shared_counter;
        // Mutex automatically unlocks when 'lock' goes out of scope
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(incrementCounter, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter: " << shared_counter << std::endl;
    // Output: Final counter: 5000
    
    return 0;
}
```

### Exception Safety with lock_guard

```cpp
#include <iostream>
#include <mutex>
#include <stdexcept>

std::mutex mtx;
int balance = 1000;

void withdraw(int amount) {
    std::lock_guard<std::mutex> lock(mtx);
    
    std::cout << "Current balance: " << balance << std::endl;
    
    if (amount > balance) {
        // Exception thrown, but mutex still unlocks automatically!
        throw std::runtime_error("Insufficient funds");
    }
    
    balance -= amount;
    std::cout << "Withdrew " << amount << ", new balance: " << balance << std::endl;
}

int main() {
    try {
        withdraw(500);  // Success
        withdraw(700);  // Throws exception
    } catch (const std::exception& e) {
        std::cout << "Error: " << e.what() << std::endl;
    }
    
    // Mutex is properly unlocked despite exception
    withdraw(300);  // Works fine
    
    return 0;
}
```

## std::unique_lock - Flexible Locking

`std::unique_lock` provides more flexibility than `std::lock_guard`. It supports deferred locking, timed locking, manual unlocking/relocking, and can be moved (but not copied).

### Basic Usage and Manual Control

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::mutex mtx;

void flexibleLocking() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Lock acquired" << std::endl;
    
    // Do some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Manually unlock
    lock.unlock();
    std::cout << "Lock released, doing non-critical work" << std::endl;
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Re-lock
    lock.lock();
    std::cout << "Lock re-acquired" << std::endl;
    
    // Automatically unlocks when lock goes out of scope
}

int main() {
    std::thread t1(flexibleLocking);
    std::thread t2(flexibleLocking);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Deferred Locking

```cpp
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx1, mtx2;

void deferredLocking() {
    // Create locks without locking immediately
    std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);
    
    // Lock both mutexes simultaneously (deadlock-free)
    std::lock(lock1, lock2);
    
    std::cout << "Both locks acquired safely" << std::endl;
    
    // Both unlock automatically at scope end
}

int main() {
    std::thread t1(deferredLocking);
    std::thread t2(deferredLocking);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Try Lock

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::mutex mtx;

void tryLockExample(int id) {
    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
    
    if (lock.owns_lock()) {
        std::cout << "Thread " << id << " acquired lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else {
        std::cout << "Thread " << id << " failed to acquire lock" << std::endl;
    }
}

int main() {
    std::thread t1(tryLockExample, 1);
    std::thread t2(tryLockExample, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Timed Locking

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::timed_mutex tmtx;

void timedLockExample(int id) {
    std::unique_lock<std::timed_mutex> lock(tmtx, std::defer_lock);
    
    // Try to lock for 50 milliseconds
    // - Waits up to 50 milliseconds for the mutex to become available
    // - Returns true if lock acquired within 50ms
    // - Returns false if timeout expires without getting the lock    
    if (lock.try_lock_for(std::chrono::milliseconds(50))) {
        std::cout << "Thread " << id << " acquired lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else { // couldn't acquire lock within 50ms
        std::cout << "Thread " << id << " try_lock_for() timeout" << std::endl;
    }
}

int main() {
    std::thread t1(timedLockExample, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread t2(timedLockExample, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Transferring Lock Ownership

```cpp
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;

std::unique_lock<std::mutex> getLock() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Lock acquired in function" << std::endl;
    return lock; // Ownership transferred
}

void transferOwnership() {
    std::unique_lock<std::mutex> lock = getLock();
    std::cout << "Lock ownership transferred" << std::endl;
    
    // Lock is still held and will be released at scope end
}

int main() {
    std::thread t(transferOwnership);
    t.join();
    
    return 0;
}
```

## Comparison: lock_guard vs unique_lock

```cpp
#include <iostream>
#include <mutex>
#include <chrono>

std::mutex mtx;

void demonstrateComparison() {
    // lock_guard: Simple, efficient, non-movable
    {
        std::lock_guard<std::mutex> lg(mtx);
        // Cannot unlock manually
        // Cannot be moved
        // Smallest overhead
    }
    
    // unique_lock: Flexible, movable, more features
    {
        std::unique_lock<std::mutex> ul(mtx);
        ul.unlock(); // Can unlock manually
        // Can try_lock, try_lock_for, try_lock_until
        // Can be moved
        ul.lock(); // Can relock
        // Slightly more overhead
    }
}

int main() {
    demonstrateComparison();
    return 0;
}
```

## Practical Example: Thread-Safe Queue

```cpp
#include <iostream>
#include <mutex>
#include <queue>
#include <thread>
#include <chrono>
#include <optional>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue;
    mutable std::mutex mtx;

public:
    void push(T value) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(std::move(value));
    }
    
    std::optional<T> pop() {
        std::lock_guard<std::mutex> lock(mtx);
        if (queue.empty()) {
            return std::nullopt;
        }
        T value = std::move(queue.front());
        queue.pop();
        return value;
    }
    
    bool empty() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.empty();
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mtx);
        return queue.size();
    }
};

int main() {
    ThreadSafeQueue<int> queue;
    
    // Producer thread
    std::thread producer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            queue.push(i);
            std::cout << "Produced: " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    
    // Consumer thread
    std::thread consumer([&queue]() {
        for (int i = 0; i < 10; ++i) {
            while (true) {
                auto value = queue.pop();
                if (value) {
                    std::cout << "Consumed: " << *value << std::endl;
                    break;
                }
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    return 0;
}
```

## Advanced: Adopting Pre-Locked Mutexes

```cpp
#include <iostream>
#include <mutex>

std::mutex mtx;

void adoptLock() {
    mtx.lock(); // Manual lock
    
    // Transfer ownership to lock_guard
    std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
    
    std::cout << "Lock adopted by lock_guard" << std::endl;
    
    // lock_guard will unlock automatically
}

int main() {
    adoptLock();
    return 0;
}
```

## Summary

**std::lock_guard** is ideal when you need simple, scoped locking with minimal overhead. It locks on construction and unlocks on destruction, providing perfect exception safety. Use it when you don't need manual control over the lock.

**std::unique_lock** provides maximum flexibility with features like deferred locking, manual unlock/relock, timed locking, and lock ownership transfer. Use it with condition variables, when you need to unlock before scope end, or when working with multiple mutexes simultaneously.

Both lock types implement RAII, ensuring that mutexes are always properly released even when exceptions occur. This makes concurrent code safer, cleaner, and less error-prone than manual mutex management. Choose `std::lock_guard` for simplicity and performance, and `std::unique_lock` when you need its additional flexibility.