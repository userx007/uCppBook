# Lock-Free Stack

## Overview

A **lock-free stack** is a concurrent data structure that allows multiple threads to push and pop elements without using traditional mutex locks. Instead, it relies on **atomic operations** and **Compare-And-Swap (CAS)** loops to ensure thread-safety while avoiding the overhead and potential deadlock issues associated with locks.

Lock-free data structures guarantee that at least one thread makes progress in a finite number of steps, even if other threads are suspended. This makes them particularly valuable in high-performance concurrent systems where contention is high and blocking would severely impact throughput.

## Key Concepts

### Compare-And-Swap (CAS)
CAS is an atomic operation that compares the value at a memory location with an expected value and, if they match, updates it to a new value. This operation is fundamental to lock-free programming:

```cpp
bool compare_exchange_weak(T& expected, T desired);
// Returns true if *this == expected and sets *this = desired
// Returns false otherwise and updates expected to current value
```

### ABA Problem
A significant challenge in lock-free programming where a value changes from A to B and back to A, making CAS incorrectly assume nothing changed. Solutions include:
- Tagged pointers (combining pointer with version counter)
- Hazard pointers
- Reference counting

### Memory Ordering
Atomic operations require careful consideration of memory ordering to ensure correctness across different CPU architectures.

## Implementation

Here's a comprehensive implementation of a lock-free stack:

```cpp
#include <atomic>
#include <memory>
#include <iostream>
#include <thread>
#include <vector>

template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    std::atomic<size_t> size_count;
    
public:
    LockFreeStack() : head(nullptr), size_count(0) {}
    
    ~LockFreeStack() {
        while (Node* node = head.load()) {
            head.store(node->next);
            delete node;
        }
    }
    
    // Push operation using CAS loop
    void push(const T& value) {
        Node* new_node = new Node(value);
        new_node->next = head.load(std::memory_order_relaxed);
        
        // CAS loop: retry until successful
        while (!head.compare_exchange_weak(
            new_node->next,
            new_node,
            std::memory_order_release,
            std::memory_order_relaxed
        )) {
            // If CAS fails, new_node->next is updated to current head
            // Loop continues until we successfully update head
        }
        
        size_count.fetch_add(1, std::memory_order_relaxed);
    }
    
    // Pop operation using CAS loop
    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_relaxed);
        
        // CAS loop: retry until successful or stack is empty
        while (old_head != nullptr) {
            if (head.compare_exchange_weak(
                old_head,
                old_head->next,
                std::memory_order_acquire,
                std::memory_order_relaxed
            )) {
                // Successfully updated head to old_head->next
                result = old_head->data;
                delete old_head;
                size_count.fetch_sub(1, std::memory_order_relaxed);
                return true;
            }
            // If CAS fails, old_head is updated to current head
            // Loop continues
        }
        
        return false; // Stack was empty
    }
    
    bool empty() const {
        return head.load(std::memory_order_relaxed) == nullptr;
    }
    
    size_t size() const {
        return size_count.load(std::memory_order_relaxed);
    }
};
```

## Advanced Implementation with ABA Protection

Here's an implementation using tagged pointers to prevent the ABA problem:

```cpp
template<typename T>
class LockFreeStackABASafe {
private:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    // Tagged pointer combining pointer and version counter
    struct TaggedPointer {
        Node* ptr;
        uintptr_t tag;
        
        TaggedPointer(Node* p = nullptr, uintptr_t t = 0) 
            : ptr(p), tag(t) {}
    };
    
    std::atomic<TaggedPointer> head;
    
public:
    LockFreeStackABASafe() : head(TaggedPointer()) {}
    
    ~LockFreeStackABASafe() {
        TaggedPointer current = head.load();
        while (current.ptr != nullptr) {
            Node* next = current.ptr->next;
            delete current.ptr;
            current.ptr = next;
        }
    }
    
    void push(const T& value) {
        Node* new_node = new Node(value);
        TaggedPointer old_head = head.load(std::memory_order_relaxed);
        TaggedPointer new_head;
        
        do {
            new_node->next = old_head.ptr;
            new_head.ptr = new_node;
            new_head.tag = old_head.tag + 1; // Increment version
            
        } while (!head.compare_exchange_weak(
            old_head,
            new_head,
            std::memory_order_release,
            std::memory_order_relaxed
        ));
    }
    
    bool pop(T& result) {
        TaggedPointer old_head = head.load(std::memory_order_relaxed);
        TaggedPointer new_head;
        
        do {
            if (old_head.ptr == nullptr) {
                return false; // Stack is empty
            }
            
            new_head.ptr = old_head.ptr->next;
            new_head.tag = old_head.tag + 1; // Increment version
            
        } while (!head.compare_exchange_weak(
            old_head,
            new_head,
            std::memory_order_acquire,
            std::memory_order_relaxed
        ));
        
        result = old_head.ptr->data;
        delete old_head.ptr;
        return true;
    }
    
    bool empty() const {
        return head.load(std::memory_order_relaxed).ptr == nullptr;
    }
};
```

## Practical Usage Example

```cpp
void producer(LockFreeStack<int>& stack, int thread_id, int count) {
    for (int i = 0; i < count; ++i) {
        int value = thread_id * 1000 + i;
        stack.push(value);
        std::cout << "Thread " << thread_id << " pushed: " << value << "\n";
    }
}

void consumer(LockFreeStack<int>& stack, int thread_id, int count) {
    for (int i = 0; i < count; ++i) {
        int value;
        while (!stack.pop(value)) {
            // Spin until we get a value
            std::this_thread::yield();
        }
        std::cout << "Thread " << thread_id << " popped: " << value << "\n";
    }
}

int main() {
    LockFreeStack<int> stack;
    
    constexpr int NUM_THREADS = 4;
    constexpr int OPS_PER_THREAD = 100;
    
    std::vector<std::thread> threads;
    
    // Start producer threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(producer, std::ref(stack), i, OPS_PER_THREAD);
    }
    
    // Start consumer threads
    for (int i = 0; i < NUM_THREADS; ++i) {
        threads.emplace_back(consumer, std::ref(stack), i + NUM_THREADS, OPS_PER_THREAD);
    }
    
    // Wait for all threads
    for (auto& thread : threads) {
        thread.join();
    }
    
    std::cout << "Final stack size: " << stack.size() << "\n";
    std::cout << "Stack empty: " << (stack.empty() ? "yes" : "no") << "\n";
    
    return 0;
}
```

## Memory Ordering Explained

The implementation uses specific memory orderings for optimal performance:

- **`memory_order_relaxed`**: No synchronization or ordering constraints, only atomicity guaranteed. Used when loading/storing values that don't require synchronization.

- **`memory_order_release`**: Ensures all writes before this operation are visible to threads that acquire. Used when publishing new data (push operation).

- **`memory_order_acquire`**: Ensures all reads after this operation see writes from release operations. Used when consuming data (pop operation).

## Performance Considerations

### Advantages
- **No blocking**: Threads never wait for locks, reducing contention
- **No deadlock**: Impossible to deadlock without locks
- **Better scalability**: Performance degrades more gracefully under high contention
- **Progress guarantee**: At least one thread always makes progress

### Disadvantages
- **Memory reclamation complexity**: Safely deleting nodes is challenging (ABA problem)
- **CPU cache effects**: CAS operations can cause cache line bouncing
- **Retry overhead**: Failed CAS operations waste CPU cycles
- **Complexity**: Much harder to implement correctly than lock-based versions

## Common Pitfalls

1. **Memory leaks**: Deleted nodes might still be accessed by other threads
2. **ABA problem**: Value changes and changes back, fooling CAS
3. **Memory ordering bugs**: Incorrect ordering can cause subtle race conditions
4. **Starvation**: While lock-free guarantees system progress, individual threads may starve

## Summary

A **lock-free stack** provides thread-safe stack operations without using locks, relying instead on atomic operations and CAS loops. The key mechanism is the **compare-and-swap loop**: repeatedly attempt to update the head pointer until successful, with the CAS operation automatically handling concurrent modifications by updating the expected value on failure.

While lock-free stacks offer excellent performance characteristics and avoid deadlock, they come with significant complexity. The **ABA problem** requires solutions like tagged pointers or hazard pointers. Proper **memory ordering** is crucial for correctness across different architectures. Memory reclamation remains challenging, often requiring sophisticated techniques like epoch-based reclamation or hazard pointers in production systems.

Lock-free data structures are best suited for scenarios with high contention where the benefits of non-blocking operations outweigh the implementation complexity. For many applications, simpler lock-based alternatives with careful design may provide adequate performance with significantly less complexity.