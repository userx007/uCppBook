# Compare and Swap Operations in C++

## Overview

Compare-and-swap (CAS) operations are fundamental atomic primitives that enable lock-free programming in C++. These operations atomically compare a memory location's value with an expected value and, if they match, update it to a new value. C++ provides two variants: `compare_exchange_weak` and `compare_exchange_strong`, both essential for building efficient concurrent data structures without traditional locks.

## Understanding Compare-and-Swap

The CAS operation follows this logical pattern:

```
if (current_value == expected_value) {
    current_value = new_value;
    return true;
} else {
    expected_value = current_value;
    return false;
}
```

The critical feature is that this entire operation executes atomically—no other thread can observe or modify the value during the operation.

## compare_exchange_strong vs compare_exchange_weak

### compare_exchange_strong

This variant guarantees that if the comparison fails, it's because the values genuinely differ. It never produces spurious failures.

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

class LockFreeCounter {
private:
    std::atomic<int> counter{0};

public:
    void increment() {
        int expected = counter.load();
        int desired;
        
        // Keep trying until successful
        while (!counter.compare_exchange_strong(expected, expected + 1)) {
            // If CAS fails, expected is automatically updated
            // to the current value, so we can retry
        }
    }
    
    int get() const {
        return counter.load();
    }
};

int main() {
    LockFreeCounter counter;
    std::vector<std::thread> threads;
    
    // Launch 10 threads, each incrementing 1000 times
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
    // Output: Final count: 10000
    
    return 0;
}
```

### compare_exchange_weak

This variant may spuriously fail even when the comparison would succeed. It's typically faster on some architectures but requires a loop structure.

```cpp
#include <atomic>
#include <iostream>

class LockFreeStack {
private:
    struct Node {
        int value;
        Node* next;
        Node(int v) : value(v), next(nullptr) {}
    };
    
    std::atomic<Node*> head{nullptr};

public:
    void push(int value) {
        Node* new_node = new Node(value);
        Node* old_head = head.load();
        
        do {
            new_node->next = old_head;
            // Use weak version in a loop - more efficient
        } while (!head.compare_exchange_weak(old_head, new_node));
    }
    
    bool pop(int& result) {
        Node* old_head = head.load();
        
        // Keep trying until successful or stack is empty
        while (old_head != nullptr) {
            if (head.compare_exchange_weak(old_head, old_head->next)) {
                result = old_head->value;
                delete old_head;
                return true;
            }
            // old_head automatically updated on failure
        }
        
        return false; // Stack was empty
    }
    
    ~LockFreeStack() {
        int value;
        while (pop(value)) {}
    }
};

int main() {
    LockFreeStack stack;
    
    stack.push(1);
    stack.push(2);
    stack.push(3);
    
    int value;
    while (stack.pop(value)) {
        std::cout << "Popped: " << value << std::endl;
    }
    // Output: Popped: 3, Popped: 2, Popped: 1
    
    return 0;
}
```

## Memory Ordering Considerations

CAS operations support custom memory ordering, which affects performance and synchronization guarantees.

```cpp
#include <atomic>
#include <thread>

class LockFreeFlagWithOrdering {
private:
    std::atomic<bool> flag{false};
    int data{0};

public:
    void set_data(int value) {
        data = value;
        
        bool expected = false;
        // Release semantics: all prior writes visible when flag is set
        while (!flag.compare_exchange_weak(
            expected, 
            true,
            std::memory_order_release,  // Success ordering
            std::memory_order_relaxed   // Failure ordering
        )) {
            expected = false;
        }
    }
    
    int get_data() {
        bool expected = true;
        // Acquire semantics: see all writes before the flag was set
        if (flag.compare_exchange_strong(
            expected,
            false,
            std::memory_order_acquire,
            std::memory_order_relaxed
        )) {
            return data;
        }
        return -1;
    }
};
```

## Advanced Example: Lock-Free Queue

Here's a more complex example implementing a single-producer, single-consumer lock-free queue:

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

template<typename T, size_t Size>
class LockFreeQueue {
private:
    struct Node {
        T data;
        std::atomic<bool> ready{false};
    };
    
    std::vector<Node> buffer;
    std::atomic<size_t> write_pos{0};
    std::atomic<size_t> read_pos{0};

public:
    LockFreeQueue() : buffer(Size) {}
    
    bool enqueue(const T& value) {
        size_t current_write = write_pos.load(std::memory_order_relaxed);
        size_t next_write = (current_write + 1) % Size;
        
        // Check if queue is full
        if (next_write == read_pos.load(std::memory_order_acquire)) {
            return false;
        }
        
        buffer[current_write].data = value;
        buffer[current_write].ready.store(true, std::memory_order_release);
        
        // Update write position
        size_t expected = current_write;
        while (!write_pos.compare_exchange_weak(
            expected,
            next_write,
            std::memory_order_release,
            std::memory_order_relaxed
        )) {
            expected = current_write;
        }
        
        return true;
    }
    
    bool dequeue(T& value) {
        size_t current_read = read_pos.load(std::memory_order_relaxed);
        
        // Check if data is ready
        if (!buffer[current_read].ready.load(std::memory_order_acquire)) {
            return false;
        }
        
        value = buffer[current_read].data;
        buffer[current_read].ready.store(false, std::memory_order_release);
        
        // Update read position
        size_t next_read = (current_read + 1) % Size;
        size_t expected = current_read;
        while (!read_pos.compare_exchange_weak(
            expected,
            next_read,
            std::memory_order_release,
            std::memory_order_relaxed
        )) {
            expected = current_read;
        }
        
        return true;
    }
};

int main() {
    LockFreeQueue<int, 100> queue;
    
    std::thread producer([&queue]() {
        for (int i = 0; i < 50; ++i) {
            while (!queue.enqueue(i)) {
                std::this_thread::yield();
            }
        }
    });
    
    std::thread consumer([&queue]() {
        int value;
        int count = 0;
        while (count < 50) {
            if (queue.dequeue(value)) {
                std::cout << "Consumed: " << value << std::endl;
                ++count;
            } else {
                std::this_thread::yield();
            }
        }
    });
    
    producer.join();
    consumer.join();
    
    return 0;
}
```

## Performance Considerations

**When to use compare_exchange_weak:**
- Inside loops where spurious failures are acceptable
- On architectures where weak is significantly faster
- When you're already looping for correctness reasons

**When to use compare_exchange_strong:**
- Outside loops or in complex control flow
- When spurious failures would cause unnecessary work
- When code clarity is more important than minor performance gains

**Practical tip:** Start with `compare_exchange_weak` in loop-based algorithms, as it's typically more efficient and the spurious failure behavior is masked by the retry logic.

## Common Pitfalls

1. **ABA Problem:** A value changes from A to B and back to A, causing CAS to succeed incorrectly. Solution: use tagged pointers or generation counters.

2. **Memory Management:** Deleted nodes may be accessed by other threads. Solution: use hazard pointers or reference counting.

3. **Incorrect Memory Ordering:** Using overly relaxed orderings can cause data races. Solution: understand and apply appropriate memory orders.

## Summary

Compare-and-swap operations are the cornerstone of lock-free programming in C++. The `compare_exchange_weak` and `compare_exchange_strong` functions provide atomic read-modify-write semantics that enable building efficient concurrent data structures without locks. While `compare_exchange_weak` may spuriously fail but is faster in loops, `compare_exchange_strong` never fails spuriously but may be slightly slower. Both variants support custom memory ordering for fine-tuned performance. Mastering these primitives is essential for writing high-performance, scalable concurrent code, though they require careful consideration of issues like the ABA problem and memory reclamation strategies.