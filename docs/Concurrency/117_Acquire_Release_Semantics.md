# Acquire-Release Semantics in C++

## Overview

Acquire-release semantics is a fundamental memory ordering model in C++ concurrency that provides a lightweight synchronization mechanism between threads. It establishes a **happens-before relationship** without requiring the heavy overhead of sequential consistency or full memory fences.

The core idea is simple: when one thread performs a **release** operation on an atomic variable, and another thread performs an **acquire** operation on the same variable, all memory writes by the first thread that happened before the release become visible to the second thread after the acquire.

## Memory Ordering Basics

In C++11 and later, atomic operations can specify memory ordering through `std::memory_order` enumerators:

- **`memory_order_release`**: A store operation with release semantics. Prevents prior memory operations from being reordered after this store.
- **`memory_order_acquire`**: A load operation with acquire semantics. Prevents subsequent memory operations from being reordered before this load.
- **`memory_order_acq_rel`**: Combined acquire-release for read-modify-write operations.
- **`memory_order_relaxed`**: No synchronization or ordering constraints (only atomicity).
- **`memory_order_seq_cst`**: Sequential consistency (default, strongest ordering).

## How Acquire-Release Works

Think of acquire-release as a one-way gate:

1. **Release operation** acts as a gate that ensures all previous writes are completed before the atomic store
2. **Acquire operation** acts as a gate that ensures all subsequent reads happen after the atomic load
3. When paired together on the same atomic variable, they create a synchronization point

```
Thread 1:                          Thread 2:
---------                          ---------
x = 42;                           
y = 17;                           
flag.store(true, release);  ----> while (!flag.load(acquire));
                                  // x and y are guaranteed visible here
                                  assert(x == 42);
                                  assert(y == 17);
```

## Code Example 1: Producer-Consumer with Acquire-Release

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <cassert>

class Message {
public:
    int data1;
    int data2;
    std::string text;
};

class Channel {
private:
    Message message;
    std::atomic<bool> ready{false};

public:
    // Producer: writes message and signals with release
    void send(int d1, int d2, const std::string& txt) {
        message.data1 = d1;
        message.data2 = d2;
        message.text = txt;
        
        // Release: all previous writes are visible to acquirer
        ready.store(true, std::memory_order_release);
    }

    // Consumer: waits with acquire and reads message
    Message receive() {
        // Acquire: spin until ready, synchronizes with release
        while (!ready.load(std::memory_order_acquire)) {
            std::this_thread::yield();
        }
        
        // All writes from send() are now visible
        return message;
    }
};

int main() {
    Channel channel;
    
    std::thread producer([&channel]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        channel.send(42, 17, "Hello from producer!");
    });
    
    std::thread consumer([&channel]() {
        Message msg = channel.receive();
        std::cout << "Received: " << msg.data1 << ", " 
                  << msg.data2 << ", " << msg.text << std::endl;
        
        assert(msg.data1 == 42);
        assert(msg.data2 == 17);
    });
    
    producer.join();
    consumer.join();
    
    return 0;
}
```

## Code Example 2: Lock-Free Flag with Data Synchronization

```cpp
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>

class DataPublisher {
private:
    int value1;
    int value2;
    int value3;
    std::atomic<int> version{0};

public:
    // Publish data with release semantics
    void publish(int v1, int v2, int v3) {
        value1 = v1;
        value2 = v2;
        value3 = v3;
        
        // Release: makes all previous writes visible
        version.fetch_add(1, std::memory_order_release);
    }

    // Read data with acquire semantics
    bool try_read(int expected_version, int& v1, int& v2, int& v3) {
        // Acquire: synchronizes with release in publish()
        int current = version.load(std::memory_order_acquire);
        
        if (current >= expected_version) {
            // All writes from publish() are now visible
            v1 = value1;
            v2 = value2;
            v3 = value3;
            return true;
        }
        return false;
    }
};

int main() {
    DataPublisher publisher;
    std::atomic<bool> stop{false};
    
    std::thread writer([&]() {
        for (int i = 0; i < 5; ++i) {
            publisher.publish(i * 10, i * 20, i * 30);
            std::cout << "Published version " << i + 1 << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        stop.store(true, std::memory_order_release);
    });
    
    std::thread reader([&]() {
        int version = 1;
        while (!stop.load(std::memory_order_acquire)) {
            int v1, v2, v3;
            if (publisher.try_read(version, v1, v2, v3)) {
                std::cout << "Read version " << version 
                          << ": " << v1 << ", " << v2 << ", " << v3 << std::endl;
                version++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    
    writer.join();
    reader.join();
    
    return 0;
}
```

## Code Example 3: Spinlock Implementation

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

class Spinlock {
private:
    std::atomic<bool> flag{false};

public:
    void lock() {
        // Try to acquire lock with exchange
        while (flag.exchange(true, std::memory_order_acquire)) {
            // Spin with relaxed loads to reduce cache traffic
            while (flag.load(std::memory_order_relaxed)) {
                std::this_thread::yield();
            }
        }
        // Lock acquired with acquire semantics
    }

    void unlock() {
        // Release lock with release semantics
        flag.store(false, std::memory_order_release);
    }
};

class Counter {
private:
    int count = 0;
    Spinlock lock;

public:
    void increment() {
        lock.lock();
        ++count;  // Protected by acquire-release
        lock.unlock();
    }

    int get() {
        lock.lock();
        int result = count;
        lock.unlock();
        return result;
    }
};

int main() {
    Counter counter;
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 1000; ++j) {
                counter.increment();
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final count: " << counter.get() << std::endl;
    std::cout << "Expected: 10000" << std::endl;
    
    return 0;
}
```

## Code Example 4: Read-Modify-Write with Acq_Rel

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

class SharedResource {
private:
    int data[100];
    std::atomic<int> write_count{0};

public:
    SharedResource() {
        for (int i = 0; i < 100; ++i) {
            data[i] = 0;
        }
    }

    // Writer: modify data and update counter
    void write(int thread_id) {
        // Get current position with acq_rel
        int pos = write_count.fetch_add(1, std::memory_order_acq_rel);
        
        if (pos < 100) {
            data[pos] = thread_id;
            std::cout << "Thread " << thread_id 
                      << " wrote at position " << pos << std::endl;
        }
    }

    // Reader: safely read final count
    void display() {
        // Acquire: synchronize with all previous acq_rel operations
        int count = write_count.load(std::memory_order_acquire);
        
        std::cout << "Total writes: " << count << std::endl;
        std::cout << "Data: ";
        for (int i = 0; i < std::min(count, 100); ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << std::endl;
    }
};

int main() {
    SharedResource resource;
    std::vector<std::thread> threads;
    
    // Launch 10 threads, each writing 3 times
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&resource, i]() {
            for (int j = 0; j < 3; ++j) {
                resource.write(i);
                std::this_thread::sleep_for(std::chrono::milliseconds(10));
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    resource.display();
    
    return 0;
}
```

## When to Use Acquire-Release

**Use acquire-release when:**
- You need to synchronize data between threads
- You want better performance than sequential consistency
- You have a clear producer-consumer relationship
- You're implementing custom synchronization primitives

**Don't use when:**
- You need total ordering across all threads (use `seq_cst`)
- You only need atomicity without synchronization (use `relaxed`)
- You're unsure—default to `seq_cst` for correctness

## Performance Considerations

Acquire-release semantics typically map to efficient CPU instructions:
- **x86/x64**: Almost free (stores are naturally release, loads are naturally acquire)
- **ARM/ARM64**: Requires memory barrier instructions (DMB)
- **Other architectures**: Varies, but generally lighter than full barriers

The performance benefit over `memory_order_seq_cst` comes from avoiding expensive sequentially consistent ordering between unrelated atomic operations.

## Common Pitfalls

1. **Forgetting the pairing**: Acquire without release (or vice versa) doesn't provide synchronization
2. **Wrong variable**: Acquire and release must operate on the **same** atomic variable
3. **Race conditions on non-atomic data**: Only data written before release is synchronized, not concurrent writes
4. **Relaxed loads in spin loops**: Can cause infinite loops on some architectures if not careful

## Summary

Acquire-release semantics provides a middle ground between relaxed atomics and sequential consistency. By establishing happens-before relationships through paired atomic operations, it enables efficient inter-thread communication and data synchronization. The **release store** publishes changes, and the **acquire load** consumes them, creating a synchronization point that guarantees visibility of all prior writes. This pattern is fundamental to building lock-free data structures and custom synchronization primitives, offering both correctness guarantees and performance benefits over stronger memory orderings. Understanding acquire-release is essential for writing correct, efficient concurrent code in modern C++.