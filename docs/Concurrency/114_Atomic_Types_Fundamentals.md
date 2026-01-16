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

// Shared atomic counter - safe for concurrent access from multiple threads
// Initialized to 0
std::atomic<int> shared_counter(0);

// Function executed by each thread to increment the shared counter
void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // Atomic increment operation - thread-safe without locks
        // fetch_add atomically adds 1 and returns the previous value
        // memory_order_relaxed: provides atomicity but no synchronization/ordering guarantees
        // - Fastest memory order for simple counters where order doesn't matter
        // - Only ensures this operation itself is atomic
        // - Does not prevent reordering with other operations
        shared_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    // Configuration:  
    const int num_threads = 10;     // number of threads
    const int iterations = 1000;    // iterations per thread
    
    // Container to hold all thread objects
    std::vector<std::thread> threads;
    
    // Launch multiple threads that will run concurrently
    // Each thread will increment the counter 'iterations' times
    for (int i = 0; i < num_threads; ++i) {
        // emplace_back constructs thread in-place, passing 
        // increment_counter function and iterations parameter
        threads.emplace_back(increment_counter, iterations);
    }
    
    // Wait for all threads to complete execution
    // join() blocks until the thread finishes
    for (auto& t : threads) {
        t.join();
    }
    
    // Display results
    // load() atomically reads the counter value
    // Result is always exactly num_threads * iterations because atomic operations
    // guarantee that no increments are lost due to race conditions
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

// Shared atomic value - will be safely incremented up to a maximum by multiple threads
std::atomic<int> atomic_value(0);

// Atomic compare-and-swap (CAS) implementation
// Safely increments the atomic value without exceeding max_value
// Multiple threads can call this concurrently without data races
void safe_increment_to_max(int max_value) {
    // Load the current value - this is our initial "expected" value
    int current = atomic_value.load();
    
    // Loop until we've either successfully incremented or reached the max
    while (current < max_value) {
        // compare_exchange_weak attempts to atomically:
        // 1. Compare atomic_value with 'current' (expected value)
        // 2. If equal: set atomic_value to 'current + 1' and return true
        // 3. If not equal: update 'current' to actual atomic_value and return false
        //
        // Why "weak"? 
        // - Can spuriously fail (return false even when values match)
        // - Faster on some architectures (e.g., ARM, PowerPC with LL/SC instructions)
        // - Acceptable here because we're in a loop that will retry
        // - Use compare_exchange_strong if spurious failures are problematic
        //
        // This is a lock-free algorithm: no thread can block another
        if (atomic_value.compare_exchange_weak(current, current + 1)) {
            break;  // Success - we incremented the value, our work is done
        }
        // Failure case (another thread changed the value first):
        // - 'current' is automatically updated to the actual current value
        // - Loop continues with the updated 'current' value
        // - Will retry if still below max_value
        // - This is the CAS retry loop pattern
    }
    // Note: If current >= max_value when we enter or after update, 
    // the loop exits without incrementing (preventing overflow)
}

int main() {
    // Maximum value the atomic counter should reach
    const int max_val = 100;
    
    std::vector<std::thread> threads;
    
    // Create 200 threads all trying to increment to max_val
    // Since max_val is 100, only the first 100 successful increments will occur
    // The remaining 100+ threads will find the value already at max and exit
    for (int i = 0; i < 200; ++i) {
        threads.emplace_back(safe_increment_to_max, max_val);
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Display final value
    std::cout << "Final value: " << atomic_value.load() << std::endl;
    // Output: Final value: 100 (never exceeds max)
    // Guaranteed: despite 200 threads, the value stops at exactly max_val
    // because compare_exchange ensures no thread can increment past the limit
    
    return 0;
}
```

```
Main Thread    Thread 1         Thread 2         Thread 3             atomic_value = 0
    |              |                |                |                        |
    |--create----->|                |                |                        | 
    |--create---------------------->|                |                        |
    |--create--------------------------------------->|                        |
    |              |                |                |                        |
    |              | load()         |                |                        |
    |              |--------------->|--------------->|----------------------->|
    |              | current=0      | current=0      | current=0              |
    |              |                |                |                        |
    |       [while: 0 < 100]        |                |                        |
    |              |                |                |                        |
    |              | compare_exchange_weak(0, 1)     |                        |
    |              |--------------------------------------------------------->|         
    |              |                |                | [CAS: 0==0?]           |
    |              |                |                | YES, set to 1          |
    |              |<---------------------------------------------------------| true 
    |              | break          |                |                        |
    |              |                |                |                        |
    |              |             [while: 0 < 100]    |                        |
    |              |                |                |                        |
    |              |                | compare_exchange_weak(0, 1)             |
    |              |                |---------------------------------------->|
    |              |                |                |         [CAS: 1==0?]   |
    |              |                |                |         NO, current=1  |
    |              |                |<----------------------------------------| false
    |              |                | current=1      |                        | = 1
    |              |                |                |                        |
    |              |                |         [while: 1 < 100]                |
    |              |                |                |                        |
    |              |                | compare_exchange_weak(1, 2)             |
    |              |                |---------------------------------------->|
    |              |                |                | [CAS: 1==1?]           |
    |              |                |                | YES, set to 2          |
    |              |                |<----------------------------------------| true
    |              |                | break          |                        | = 2
    |              |                |                |                        |
    |              |                |           [while: 0 < 100]              |
    |              |                |                |                        |
    |              |                |                | compare_exchange_weak(0, 1)
    |              |                |                |----------------------->|
    |              |                |                | [CAS: 2==0?]           |
    |              |                |                | NO, current=2          |
    |              |                |                |<-----------------------| false
    |              |                |                | current=2              | = 2
    |              |                |                |                        |
    |              |                |          [while: 2 < 100]               |
    |              |                |                |                        |
    |              |                |                | compare_exchange_weak(2, 3)
    |              |                |                |----------------------->|
    |              |                |                | [CAS: 2==2?]           |
    |              |                |                | YES, set to 3          |
    |              |                |                |<-----------------------| true
    |              |                |                | break                  | = 3
    |              |                |                |                        |
    |              |                |                |                        |
    |         ... (continues until atomic_value reaches 100) ...              |
    |              |                |                |                        |
    |              |                |                |                        | = 99
    |              |                |                |                        |
    |              | load()         |                |                        |
    |              |--------------------------------------------------------->|
    |              | current=99     |                |                        |
    |              |                |                |                        |
    |        [while: 99 < 100]      |                |                        |
    |              |                |                |                        |
    |              | compare_exchange_weak(99, 100)  |                        |
    |              |--------------------------------------------------------->|
    |              |                |                | [CAS: 99==99?]         |
    |              |                |                | YES, set to 100        |
    |              |<---------------------------------------------------------| true     
    |              | break          |                |                        | = 100
    |              X                |                |                        |
    |              |                |                |                        |
    |              |             load()              |                        |
    |              |                |---------------------------------------->|
    |              |                | current=100    |                        |
    |              |                |                |                        |
    |              |     [while: 100 < 100 = FALSE]  |                        |
    |              |                | exit loop      |                        |
    |              |                X                |                        |
    |              |                |                |                        |
    |              |                |             load()                      |
    |              |                |                |----------------------->|
    |              |                |                | current=100            |
    |              |                |                |                        |
    |              |                |     [while: 100 < 100 = FALSE]          |
    |              |                |                | exit loop              |
    |              |                |                X                        |
    |              |                |                |                        |
    | join()------>|                |                |                        |
    |<-------------|                |                |                        |
    |              |                |                |                        |
    | join()----------------------->|                |                        |
    |<------------------------------|                |                        |
    |              |                |                |                        |
    | join()---------------------------------------->|                        |
    |<-----------------------------------------------|                        |
    |              |                |                |                        |
    | load() final value                             |                        |
    |<------------------------------------------------------------------------|  
    | 100          |                |                |                        |
    |              |                |                |                        |
    | "Final value: 100"            |                |                        |
    |              |                |                |                        |
    V

Key CAS Scenarios Shown:
------------------------
1. Thread 1: CAS(0,1) succeeds - first increment (value: 0→1)
2. Thread 2: CAS(0,1) fails because Thread 1 changed it to 1
   - 'current' auto-updates to 1, retries with CAS(1,2)
3. Thread 3: CAS(0,1) fails, updates to current=2, retries with CAS(2,3)
4. Eventually reaches 100, remaining threads exit without incrementing
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