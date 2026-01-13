# Atomic Wait and Notify in C++

## Overview

Atomic wait and notify operations, introduced in C++20, provide an efficient mechanism for threads to wait for changes to atomic variables without busy-waiting (spinning). These operations allow threads to block until an atomic variable reaches a specific value, significantly reducing CPU usage compared to traditional polling approaches.

## The Problem with Busy-Waiting

Before C++20, developers often used busy-waiting loops to wait for atomic variables to change:

```cpp
#include <atomic>
#include <thread>

std::atomic<bool> ready{false};

void busy_wait_example() {
    // Bad: Wastes CPU cycles
    while (!ready.load()) {
        // Spinning - burns CPU
    }
    // Proceed when ready becomes true
}
```

This approach is inefficient because the waiting thread continuously consumes CPU resources checking the variable.

## Wait and Notify Operations

C++20 introduces four key operations for atomic types:

- **`wait(T old_value)`**: Blocks the current thread until the atomic value changes from `old_value`
- **`notify_one()`**: Wakes up one waiting thread
- **`notify_all()`**: Wakes up all waiting threads
- These operations work with `std::atomic` and `std::atomic_flag`

## Basic Usage Example

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

std::atomic<int> data{0};

void waiter() {
    std::cout << "Waiter: Waiting for data to become 42...\n";
    
    // Wait until data is no longer 0
    data.wait(0);
    
    std::cout << "Waiter: Data changed to " << data.load() << "\n";
}

void notifier() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "Notifier: Setting data to 42\n";
    data.store(42);
    
    // Wake up waiting threads
    data.notify_one();
}

int main() {
    std::thread t1(waiter);
    std::thread t2(notifier);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Producer-Consumer Pattern

A practical example using atomic wait/notify for synchronization:

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

class AtomicQueue {
private:
    std::atomic<int> count{0};
    std::vector<int> buffer;
    static constexpr int CAPACITY = 10;

public:
    void produce(int value) {
        // Wait if buffer is full
        while (count.load() >= CAPACITY) {
            count.wait(CAPACITY);
        }
        
        buffer.push_back(value);
        count.fetch_add(1);
        
        std::cout << "Produced: " << value 
                  << " (count: " << count.load() << ")\n";
        
        // Notify consumers that data is available
        count.notify_one();
    }
    
    int consume() {
        // Wait if buffer is empty
        while (count.load() == 0) {
            count.wait(0);
        }
        
        int value = buffer.back();
        buffer.pop_back();
        count.fetch_sub(1);
        
        std::cout << "Consumed: " << value 
                  << " (count: " << count.load() << ")\n";
        
        // Notify producers that space is available
        count.notify_one();
        
        return value;
    }
};

int main() {
    AtomicQueue queue;
    
    std::thread producer([&queue]() {
        for (int i = 0; i < 20; ++i) {
            queue.produce(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    std::thread consumer([&queue]() {
        for (int i = 0; i < 20; ++i) {
            queue.consume();
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    });
    
    producer.join();
    consumer.join();
    
    return 0;
}
```

## State Machine Synchronization

Using atomic wait/notify for coordinating state transitions:

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

enum class State : int {
    INIT = 0,
    READY = 1,
    PROCESSING = 2,
    DONE = 3
};

std::atomic<State> state{State::INIT};

void worker() {
    std::cout << "Worker: Waiting for READY state...\n";
    
    // Wait until state changes from INIT
    state.wait(State::INIT);
    
    std::cout << "Worker: Starting processing\n";
    state.store(State::PROCESSING);
    state.notify_all();
    
    // Simulate work
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Worker: Done\n";
    state.store(State::DONE);
    state.notify_all();
}

void coordinator() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Coordinator: Setting READY state\n";
    state.store(State::READY);
    state.notify_all();
    
    // Wait for processing to complete
    state.wait(State::PROCESSING);
    
    std::cout << "Coordinator: Work completed!\n";
}

int main() {
    std::thread t1(worker);
    std::thread t2(coordinator);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Multiple Waiters Example

Demonstrating `notify_one()` vs `notify_all()`:

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

std::atomic<int> signal{0};

void waiter(int id) {
    std::cout << "Thread " << id << ": Waiting...\n";
    signal.wait(0);
    std::cout << "Thread " << id << ": Woken up! Signal = " 
              << signal.load() << "\n";
}

int main() {
    std::vector<std::thread> threads;
    
    // Create multiple waiting threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(waiter, i);
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Wake all threads
    std::cout << "\nMain: Broadcasting signal\n";
    signal.store(1);
    signal.notify_all();  // Wake ALL waiting threads
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## Spinlock vs Atomic Wait Comparison

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <chrono>

// Traditional spinlock approach
class SpinLock {
    std::atomic<bool> locked{false};
public:
    void lock() {
        // Busy-wait - wastes CPU
        while (locked.exchange(true, std::memory_order_acquire)) {
            // Spinning
        }
    }
    
    void unlock() {
        locked.store(false, std::memory_order_release);
    }
};

// Atomic wait-based approach
class WaitLock {
    std::atomic<bool> locked{false};
public:
    void lock() {
        // Efficient waiting - thread sleeps
        while (locked.exchange(true, std::memory_order_acquire)) {
            locked.wait(true);  // Block until notified
        }
    }
    
    void unlock() {
        locked.store(false, std::memory_order_release);
        locked.notify_one();  // Wake one waiting thread
    }
};

template<typename Lock>
void benchmark(const char* name) {
    Lock lock;
    std::atomic<int> counter{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    auto worker = [&]() {
        for (int i = 0; i < 1000; ++i) {
            lock.lock();
            ++counter;
            lock.unlock();
        }
    };
    
    std::thread t1(worker);
    std::thread t2(worker);
    
    t1.join();
    t2.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << name << " took " << duration.count() << "ms\n";
}

int main() {
    benchmark<SpinLock>("SpinLock");
    benchmark<WaitLock>("WaitLock");
    return 0;
}
```

## Key Considerations

### Memory Ordering
Atomic wait and notify operations respect memory ordering constraints. The `wait()` operation uses `memory_order_seq_cst` by default, ensuring proper synchronization.

### Spurious Wakeups
Like condition variables, atomic `wait()` can experience spurious wakeups. Always use it in a loop that checks the condition:

```cpp
while (data.load() != expected_value) {
    data.wait(old_value);
    old_value = data.load();
}
```

### Performance Benefits
- **Reduced CPU usage**: Waiting threads don't consume CPU cycles
- **Better power efficiency**: Allows CPU to enter low-power states
- **Scalability**: Better performance with many waiting threads

### Limitations
- Only works with `std::atomic` types
- Cannot wait on complex conditions (use condition variables for that)
- Requires C++20 or later

## Summary

Atomic wait and notify operations provide an efficient, low-level synchronization mechanism for C++20 and beyond. They eliminate the CPU waste of busy-waiting while offering simpler semantics than condition variables for basic atomic value monitoring. These operations are particularly useful for implementing lock-free data structures, coordinating thread states, and building higher-level synchronization primitives. By allowing threads to sleep until atomic variables change, they improve both performance and power efficiency in concurrent applications. Use `wait()` to efficiently block until a value changes, `notify_one()` to wake a single waiter, and `notify_all()` to wake all waiting threads.