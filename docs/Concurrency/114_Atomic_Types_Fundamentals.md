# Atomic Types Fundamentals in C++

## Key Topics Covered:

1. **Introduction to Atomics** - What they are and why they matter
2. **Basic Usage** - Declaration, initialization, and common operations
3. **Lock-Free Operations** - Understanding and verifying lock-free behavior
4. **Compare-and-Swap (CAS)** - The foundation of lock-free algorithms
5. **Memory Ordering** - The six memory order options and when to use them
6. **Practical Examples**:
   - Thread-safe counter
   - Lock-free stack implementation
   - Producer-consumer synchronization
7. **Best Practices** - Choosing the right memory ordering and avoiding pitfalls
8. **Performance Considerations** - Atomic vs mutex comparison, false sharing
9. **Common Pitfalls** - What to watch out for

Each section includes working code examples that demonstrate the concepts in practice. The examples progress from simple atomic operations to more complex lock-free data structures, giving you a solid foundation in atomic programming.

The guide emphasizes that atomic types are excellent for simple shared state (flags, counters) but require careful consideration of memory ordering semantics for correct synchronization between threads.

# Atomic Types Fundamentals in C++

## Introduction

Atomic types in C++ provide thread-safe operations on shared data without the overhead of explicit locking mechanisms. The `std::atomic` template class, introduced in C++11, enables lock-free programming for primitive types and guarantees that operations on atomic variables are indivisible—they either complete entirely or not at all, with no possibility of another thread observing a partially completed operation.

## What Are Atomic Operations?

An atomic operation is one that appears to occur instantaneously from the perspective of other threads. When multiple threads access an atomic variable:

- **No data races occur**: The operation is inherently thread-safe
- **No torn reads/writes**: You never observe partial state changes
- **Memory ordering guarantees**: The operation provides specific synchronization semantics

## Basic std::atomic Usage

### Declaration and Initialization

```cpp
#include <atomic>
#include <iostream>
#include <thread>

// Declare atomic variables
std::atomic<int> counter(0);           // Initialize to 0
std::atomic<bool> flag{false};         // Brace initialization
std::atomic<double> value;             // Default initialized

int main() {
    // Atomic operations are thread-safe
    counter.store(10);                 // Atomic write
    int current = counter.load();      // Atomic read
    
    std::cout << "Counter: " << current << std::endl;
    return 0;
}
```

### Common Atomic Operations

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::atomic<int> shared_counter(0);

void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // Atomic increment - thread-safe
        shared_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    const int num_threads = 10;
    const int iterations = 1000;
    
    std::vector<std::thread> threads;
    
    // Launch multiple threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(increment_counter, iterations);
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    // Result is always exactly num_threads * iterations
    std::cout << "Final counter: " << shared_counter.load() << std::endl;
    std::cout << "Expected: " << (num_threads * iterations) << std::endl;
    
    return 0;
}
```

## Key Atomic Operations

### Read-Modify-Write Operations

```cpp
#include <atomic>
#include <iostream>
#include <thread>

void demonstrate_atomic_operations() {
    std::atomic<int> value(100);
    
    // fetch_add: returns old value, adds to atomic
    int old = value.fetch_add(5);
    std::cout << "Old: " << old << ", New: " << value.load() << std::endl;
    // Output: Old: 100, New: 105
    
    // fetch_sub: returns old value, subtracts from atomic
    old = value.fetch_sub(3);
    std::cout << "Old: " << old << ", New: " << value.load() << std::endl;
    // Output: Old: 105, New: 102
    
    // fetch_and, fetch_or, fetch_xor for bitwise operations
    std::atomic<unsigned int> flags(0b1010);
    old = flags.fetch_or(0b0101);
    std::cout << "Flags after OR: " << flags.load() << std::endl;
    // Output: Flags after OR: 15 (0b1111)
}

int main() {
    demonstrate_atomic_operations();
    return 0;
}
```

### Compare-and-Swap (CAS)

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

std::atomic<int> atomic_value(0);

// Atomic compare-and-swap implementation
void safe_increment_to_max(int max_value) {
    int current = atomic_value.load();
    
    while (current < max_value) {
        // Try to increment: only succeeds if value hasn't changed
        if (atomic_value.compare_exchange_weak(current, current + 1)) {
            break;  // Success
        }
        // Failure: 'current' is updated to actual value, retry
    }
}

int main() {
    const int max_val = 100;
    std::vector<std::thread> threads;
    
    // Many threads trying to increment
    for (int i = 0; i < 200; ++i) {
        threads.emplace_back(safe_increment_to_max, max_val);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final value: " << atomic_value.load() << std::endl;
    // Output: Final value: 100 (never exceeds max)
    
    return 0;
}
```

## Lock-Free Operations

### Checking Lock-Free Capability

```cpp
#include <atomic>
#include <iostream>

int main() {
    std::atomic<int> atomic_int;
    std::atomic<double> atomic_double;
    std::atomic<long long> atomic_longlong;
    
    // Check if operations are lock-free
    std::cout << "int is lock-free: " 
              << atomic_int.is_lock_free() << std::endl;
    std::cout << "double is lock-free: " 
              << atomic_double.is_lock_free() << std::endl;
    std::cout << "long long is lock-free: " 
              << atomic_longlong.is_lock_free() << std::endl;
    
    // Compile-time check (C++17)
    if constexpr (std::atomic<int>::is_always_lock_free) {
        std::cout << "int is ALWAYS lock-free on this platform" << std::endl;
    }
    
    return 0;
}
```

### Lock-Free Stack Example

```cpp
#include <atomic>
#include <iostream>
#include <memory>
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
    
public:
    LockFreeStack() : head(nullptr) {}
    
    void push(const T& value) {
        Node* new_node = new Node(value);
        new_node->next = head.load();
        
        // Keep trying until we successfully update head
        while (!head.compare_exchange_weak(new_node->next, new_node)) {
            // compare_exchange_weak updates new_node->next on failure
        }
    }
    
    bool pop(T& result) {
        Node* old_head = head.load();
        
        while (old_head && 
               !head.compare_exchange_weak(old_head, old_head->next)) {
            // Retry if another thread modified head
        }
        
        if (old_head) {
            result = old_head->data;
            delete old_head;
            return true;
        }
        return false;
    }
    
    ~LockFreeStack() {
        T dummy;
        while (pop(dummy)) {}
    }
};

int main() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;
    
    // Multiple threads pushing
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&stack, i]() {
            for (int j = 0; j < 10; ++j) {
                stack.push(i * 10 + j);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    // Pop all values
    int value;
    std::cout << "Stack contents: ";
    while (stack.pop(value)) {
        std::cout << value << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

## Memory Ordering

Atomic operations support different memory ordering semantics:

```cpp
#include <atomic>
#include <thread>
#include <cassert>

std::atomic<bool> ready(false);
std::atomic<int> data(0);

void producer() {
    data.store(42, std::memory_order_relaxed);
    // Release: all previous writes are visible to acquirer
    ready.store(true, std::memory_order_release);
}

void consumer() {
    // Acquire: synchronizes with release, sees all previous writes
    while (!ready.load(std::memory_order_acquire)) {
        // Spin until ready
    }
    assert(data.load(std::memory_order_relaxed) == 42);
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Memory Order Options

- **memory_order_relaxed**: No synchronization, only atomicity guaranteed
- **memory_order_acquire**: Synchronizes with a release operation
- **memory_order_release**: Synchronizes with an acquire operation
- **memory_order_acq_rel**: Both acquire and release semantics
- **memory_order_seq_cst**: Sequential consistency (default, strongest)

## Practical Example: Thread-Safe Counter

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

class ThreadSafeCounter {
private:
    std::atomic<long long> value;
    
public:
    ThreadSafeCounter() : value(0) {}
    
    void increment() {
        value.fetch_add(1, std::memory_order_relaxed);
    }
    
    void decrement() {
        value.fetch_sub(1, std::memory_order_relaxed);
    }
    
    long long get() const {
        return value.load(std::memory_order_relaxed);
    }
    
    // Atomic exchange
    long long reset() {
        return value.exchange(0, std::memory_order_relaxed);
    }
};

int main() {
    ThreadSafeCounter counter;
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Spawn threads that increment
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter]() {
            for (int j = 0; j < 100000; ++j) {
                counter.increment();
            }
        });
    }
    
    // Wait for completion
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Final count: " << counter.get() << std::endl;
    std::cout << "Expected: 1000000" << std::endl;
    std::cout << "Time: " << duration.count() << "ms" << std::endl;
    
    return 0;
}
```

## Best Practices

### 1. Use Atomic Types for Simple Shared State

```cpp
// Good: Simple flag
std::atomic<bool> shutdown_requested(false);

// Avoid: Complex types (may not be lock-free)
struct BigStruct { int data[100]; };
std::atomic<BigStruct> big_atomic;  // Likely uses locks internally
```

### 2. Choose Appropriate Memory Ordering

```cpp
// For independent operations: relaxed
std::atomic<int> counter(0);
counter.fetch_add(1, std::memory_order_relaxed);

// For synchronization: acquire-release
std::atomic<bool> data_ready(false);
data_ready.store(true, std::memory_order_release);
while (!data_ready.load(std::memory_order_acquire)) {}

// When in doubt: sequential consistency (default)
counter.store(value);  // Uses memory_order_seq_cst
```

### 3. Verify Lock-Free Behavior

```cpp
// Check at runtime for critical paths
if (!my_atomic.is_lock_free()) {
    std::cerr << "Warning: atomic operations are not lock-free!" << std::endl;
}
```

## Performance Considerations

### Atomic vs Mutex Comparison

```cpp
#include <atomic>
#include <mutex>
#include <chrono>
#include <iostream>

// Atomic version
std::atomic<long> atomic_counter(0);

void atomic_increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

// Mutex version
long mutex_counter = 0;
std::mutex counter_mutex;

void mutex_increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(counter_mutex);
        ++mutex_counter;
    }
}

// Atomics are typically 2-10x faster for simple operations
```

## Common Pitfalls

### 1. False Sharing

```cpp
// Bad: Atomic variables on same cache line
struct Counters {
    std::atomic<int> counter1;  // Same cache line
    std::atomic<int> counter2;  // Causes false sharing
};

// Good: Pad to separate cache lines
struct PaddedCounters {
    alignas(64) std::atomic<int> counter1;
    alignas(64) std::atomic<int> counter2;
};
```

### 2. Not Understanding Memory Ordering

```cpp
// Problematic code
std::atomic<int> x(0), y(0);

// Thread 1
x.store(1, std::memory_order_relaxed);
y.store(1, std::memory_order_relaxed);

// Thread 2
while (y.load(std::memory_order_relaxed) == 0) {}
assert(x.load(std::memory_order_relaxed) == 1);  // May fail!
```

## Summary

**Atomic types** provide thread-safe operations without explicit locks, enabling efficient concurrent programming:

- **std::atomic<T>** guarantees indivisible operations on primitive types
- **Lock-free operations** avoid kernel-level synchronization overhead when supported
- **Read-modify-write operations** like `fetch_add`, `compare_exchange` enable complex atomic updates
- **Memory ordering** controls visibility and ordering guarantees between threads
- **Performance benefit** significant for high-contention scenarios with simple operations

Atomic types are ideal for flags, counters, and simple synchronization primitives. For complex shared data structures, consider higher-level synchronization primitives or lock-free data structures built on atomic operations.

**Key Takeaway**: Use atomic types for lock-free coordination of simple shared state between threads, but understand their memory ordering semantics and verify lock-free behavior for performance-critical code.