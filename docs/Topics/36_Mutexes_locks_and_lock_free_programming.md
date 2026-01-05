# Guide on C++ concurrency mechanisms covering mutexes, locks, and lock-free programming

**Key Topics Covered:**

1. **Mutexes** - Different types (`std::mutex`, `std::recursive_mutex`, `std::timed_mutex`, `std::shared_mutex`) with practical examples showing how to prevent race conditions

2. **Lock Types** - RAII-based lock wrappers:
   - `std::lock_guard` - Simple automatic locking
   - `std::unique_lock` - Flexible with deferred/manual locking
   - `std::shared_lock` - For reader-writer scenarios
   - `std::scoped_lock` - Deadlock-free multi-mutex locking

3. **Lock-Free Programming** - Using `std::atomic` for high-performance synchronization without blocking, including:
   - Atomic operations and compare-exchange
   - Memory ordering models
   - Lock-free data structure example (stack)
   - Wait-free vs lock-free concepts

4. **Best Practices** - Including when to use each approach, how to avoid deadlocks, and performance considerations

Each section includes working code examples that demonstrate both incorrect approaches (to show the problems) and correct solutions. The guide emphasizes that while lock-free programming can offer performance benefits, it's more complex and should only be used when profiling shows mutexes are a bottleneck.

# C++ Mutexes, Locks, and Lock-Free Programming

## Table of Contents
1. [Introduction](#introduction)
2. [Mutexes](#mutexes)
3. [Lock Types](#lock-types)
4. [Lock-Free Programming](#lock-free-programming)
5. [Best Practices](#best-practices)

---

## Introduction

When multiple threads access shared data concurrently, **race conditions** can occur, leading to unpredictable behavior. C++ provides several mechanisms to synchronize thread access and prevent data races.

### The Problem: Race Conditions

```cpp
#include <thread>
#include <iostream>

int counter = 0;

void increment() {
    for (int i = 0; i < 100000; ++i) {
        ++counter;  // NOT thread-safe!
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();
    std::cout << "Counter: " << counter << std::endl;
    // Expected: 200000, Actual: varies (less than 200000)
}
```

---

## Mutexes

A **mutex** (mutual exclusion) is a synchronization primitive that prevents multiple threads from simultaneously accessing shared resources.

### std::mutex

The basic mutex provides exclusive ownership.

```cpp
#include <mutex>
#include <thread>
#include <iostream>

std::mutex mtx;
int counter = 0;

void safe_increment() {
    for (int i = 0; i < 100000; ++i) {
        mtx.lock();
        ++counter;
        mtx.unlock();
    }
}

int main() {
    std::thread t1(safe_increment);
    std::thread t2(safe_increment);
    t1.join();
    t2.join();
    std::cout << "Counter: " << counter << std::endl;
    // Output: 200000 (guaranteed)
}
```

### std::recursive_mutex

Allows the same thread to lock the mutex multiple times.

```cpp
#include <mutex>
#include <thread>

std::recursive_mutex rec_mtx;

void recursive_function(int depth) {
    rec_mtx.lock();
    if (depth > 0) {
        std::cout << "Depth: " << depth << std::endl;
        recursive_function(depth - 1);  // Can lock again
    }
    rec_mtx.unlock();
}
```

### std::timed_mutex

Allows attempting to acquire a lock with a timeout.

```cpp
#include <mutex>
#include <chrono>
#include <thread>

std::timed_mutex t_mtx;

void try_lock_with_timeout() {
    if (t_mtx.try_lock_for(std::chrono::milliseconds(100))) {
        std::cout << "Lock acquired!" << std::endl;
        // Do work...
        t_mtx.unlock();
    } else {
        std::cout << "Failed to acquire lock" << std::endl;
    }
}
```

### std::shared_mutex (C++17)

Allows multiple readers or one writer (readers-writer lock).

```cpp
#include <shared_mutex>
#include <thread>
#include <vector>

std::shared_mutex sh_mtx;
int shared_data = 0;

void reader() {
    sh_mtx.lock_shared();  // Multiple readers can lock simultaneously
    std::cout << "Read: " << shared_data << std::endl;
    sh_mtx.unlock_shared();
}

void writer(int value) {
    sh_mtx.lock();  // Exclusive access
    shared_data = value;
    std::cout << "Wrote: " << value << std::endl;
    sh_mtx.unlock();
}
```

---

## Lock Types

**RAII-based locks** automatically manage mutex lifecycle, preventing forgotten unlocks.

### std::lock_guard

The simplest RAII lock wrapper. Locks on construction, unlocks on destruction.

```cpp
#include <mutex>
#include <thread>

std::mutex mtx;
int counter = 0;

void increment_safe() {
    std::lock_guard<std::mutex> lock(mtx);
    ++counter;
    // Automatically unlocked when lock goes out of scope
}

void process_data() {
    std::lock_guard<std::mutex> lock(mtx);
    // Do work...
    if (some_condition) {
        return;  // Safe! Lock is automatically released
    }
    // More work...
}  // Lock released here too
```

### std::unique_lock

More flexible than `lock_guard`. Supports deferred locking, unlocking, and relocking.

```cpp
#include <mutex>
#include <thread>

std::mutex mtx;

void deferred_locking() {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    // Mutex not locked yet
    
    // Do some work without lock...
    
    lock.lock();  // Lock when needed
    // Critical section
    lock.unlock();  // Can manually unlock
    
    // Do more work...
    
    lock.lock();  // Can relock
}  // Automatically unlocks if still locked

void try_lock_example() {
    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
    if (lock.owns_lock()) {
        // Successfully acquired lock
    } else {
        // Failed to acquire lock
    }
}
```

### std::shared_lock (C++14)

RAII wrapper for shared (read) access with `shared_mutex`.

```cpp
#include <shared_mutex>
#include <thread>
#include <string>

std::shared_mutex sh_mtx;
std::string shared_string = "initial";

void read_data() {
    std::shared_lock<std::shared_mutex> lock(sh_mtx);
    std::cout << "Reading: " << shared_string << std::endl;
    // Multiple threads can hold shared_lock simultaneously
}

void write_data(const std::string& new_value) {
    std::unique_lock<std::shared_mutex> lock(sh_mtx);
    shared_string = new_value;
    std::cout << "Writing: " << new_value << std::endl;
    // Exclusive access - no other readers or writers
}
```

### std::scoped_lock (C++17)

Locks multiple mutexes simultaneously, avoiding deadlocks.

```cpp
#include <mutex>
#include <thread>

std::mutex mtx1, mtx2;
int data1 = 0, data2 = 0;

void transfer_old_way() {
    // DEADLOCK RISK if another thread locks in opposite order!
    mtx1.lock();
    mtx2.lock();
    // Transfer data...
    mtx2.unlock();
    mtx1.unlock();
}

void transfer_safe() {
    std::scoped_lock lock(mtx1, mtx2);  // Deadlock-free!
    // Transfer data...
    ++data1;
    --data2;
}  // Both mutexes automatically unlocked

// Alternative with unique_lock
void transfer_with_std_lock() {
    std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);
    std::lock(lock1, lock2);  // Locks both without deadlock
    // Transfer data...
}
```

---

## Lock-Free Programming

Lock-free programming uses atomic operations to avoid mutex overhead and blocking.

### std::atomic

Provides lock-free operations for simple types.

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> atomic_counter(0);

void atomic_increment() {
    for (int i = 0; i < 100000; ++i) {
        ++atomic_counter;  // Thread-safe without mutex!
    }
}

int main() {
    std::thread t1(atomic_increment);
    std::thread t2(atomic_increment);
    t1.join();
    t2.join();
    std::cout << "Counter: " << atomic_counter << std::endl;
    // Output: 200000 (guaranteed, no locks!)
}
```

### Atomic Operations

```cpp
#include <atomic>

std::atomic<int> value(0);

void atomic_operations() {
    // Basic operations
    value.store(42);
    int current = value.load();
    
    // Atomic arithmetic
    value.fetch_add(10);     // Returns old value
    value.fetch_sub(5);
    value.fetch_and(0xFF);
    value.fetch_or(0x100);
    
    // Compare-and-swap
    int expected = 42;
    bool success = value.compare_exchange_strong(expected, 100);
    // If value == 42, set to 100 and return true
    // If value != 42, set expected to actual value and return false
}
```

### Lock-Free Stack Example

```cpp
#include <atomic>
#include <memory>

template<typename T>
class LockFreeStack {
    struct Node {
        T data;
        Node* next;
        Node(const T& d) : data(d), next(nullptr) {}
    };
    
    std::atomic<Node*> head;
    
public:
    LockFreeStack() : head(nullptr) {}
    
    void push(const T& data) {
        Node* new_node = new Node(data);
        new_node->next = head.load();
        
        // Keep trying until we successfully update head
        while (!head.compare_exchange_weak(new_node->next, new_node)) {
            // If CAS fails, new_node->next is updated to current head
        }
    }
    
    bool pop(T& result) {
        Node* old_head = head.load();
        
        while (old_head && 
               !head.compare_exchange_weak(old_head, old_head->next)) {
            // Keep trying if another thread modified head
        }
        
        if (old_head) {
            result = old_head->data;
            delete old_head;  // Note: ABA problem exists here!
            return true;
        }
        return false;
    }
};
```

### Memory Ordering

Control the synchronization guarantees of atomic operations.

```cpp
#include <atomic>
#include <thread>

std::atomic<bool> ready(false);
int data = 0;

// Producer thread
void producer() {
    data = 42;  // Non-atomic write
    ready.store(true, std::memory_order_release);  // Synchronize
}

// Consumer thread
void consumer() {
    while (!ready.load(std::memory_order_acquire)) {
        // Wait
    }
    // Guaranteed to see data == 42
    assert(data == 42);
}

// Memory orders:
// - memory_order_relaxed: No synchronization
// - memory_order_acquire: Synchronize with release
// - memory_order_release: Synchronize with acquire
// - memory_order_acq_rel: Both acquire and release
// - memory_order_seq_cst: Sequential consistency (default, strongest)
```

### Lock-Free vs Wait-Free

```cpp
// LOCK-FREE: At least one thread makes progress
// Example: compare_exchange in a loop
void lock_free_increment(std::atomic<int>& counter) {
    int old_value = counter.load();
    while (!counter.compare_exchange_weak(old_value, old_value + 1)) {
        // Some thread will succeed, but individual threads may retry
    }
}

// WAIT-FREE: Every thread makes progress in bounded steps
// Example: Simple fetch_add
void wait_free_increment(std::atomic<int>& counter) {
    counter.fetch_add(1);  // Always completes in finite steps
}
```

---

## Best Practices

### 1. Prefer RAII Locks Over Manual Lock/Unlock

```cpp
// BAD: Manual management
mtx.lock();
// If exception thrown here, mutex never unlocked!
mtx.unlock();

// GOOD: RAII
std::lock_guard<std::mutex> lock(mtx);
// Exception-safe
```

### 2. Keep Critical Sections Small

```cpp
// BAD: Large critical section
void process() {
    std::lock_guard<std::mutex> lock(mtx);
    slow_computation();  // Unnecessary locking
    shared_data = result;
}

// GOOD: Minimal locking
void process() {
    auto result = slow_computation();  // Do work outside lock
    std::lock_guard<std::mutex> lock(mtx);
    shared_data = result;  // Only lock for shared access
}
```

### 3. Lock Multiple Mutexes in Consistent Order

```cpp
// Use std::scoped_lock or std::lock to avoid deadlocks
void safe_transfer() {
    std::scoped_lock lock(account1_mtx, account2_mtx);
    // Deadlock-free
}
```

### 4. Use Lock-Free When Appropriate

```cpp
// For simple counters and flags, atomics are faster
std::atomic<bool> is_ready(false);  // Better than mutex + bool

// For complex data structures, mutexes may be simpler and sufficient
```

### 5. Measure Performance

Lock-free isn't always faster! Profile your specific use case:
- Low contention: Mutexes may be fine
- High contention with simple operations: Atomics excel
- Complex data structures: Mutexes may be simpler and fast enough

### 6. Avoid Recursive Locks When Possible

Recursive mutexes are slower and may indicate design issues. Refactor to avoid recursion or restructure code.

---

## Summary

| Mechanism | Use Case | Pros | Cons |
|-----------|----------|------|------|
| `std::mutex` | Basic exclusion | Simple, reliable | Blocking |
| `std::shared_mutex` | Many readers, few writers | Efficient reads | More complex |
| `std::lock_guard` | Automatic mutex management | Exception-safe | Inflexible |
| `std::unique_lock` | Flexible locking | Conditional/deferred locking | Slight overhead |
| `std::scoped_lock` | Multiple mutexes | Deadlock-free | C++17 only |
| `std::atomic` | Simple lock-free ops | No blocking | Limited types |
| Lock-free structures | High-performance needs | No blocking | Complex, error-prone |

Choose the right tool for your concurrency needs, prioritizing correctness over performance optimization.