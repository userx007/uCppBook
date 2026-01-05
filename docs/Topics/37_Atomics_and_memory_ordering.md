# Guide on C++ atomics and memory ordering 

**Key Topics:**
- What atomics are and why they're needed for thread-safe programming
- All six memory ordering options (`relaxed`, `acquire`, `release`, `acq_rel`, `seq_cst`, `consume`)
- Practical examples including producer-consumer patterns, lock-free data structures, and spinlocks
- A memory ordering cheat sheet for quick reference
- Best practices and performance considerations

**Notable Examples Included:**
1. **Basic atomic counter** - thread-safe increment operations
2. **Acquire-release synchronization** - the most common pattern
3. **Lock-free stack** - demonstrating compare-and-swap
4. **Spinlock implementation** - practical synchronization primitive
5. **Double-checked locking** - optimized singleton pattern

The guide emphasizes that while atomics enable lock-free programming, they're complex and should be used judiciously. For most applications, starting with `seq_cst` (sequential consistency) is the safest approach, with optimization to weaker memory orders only when profiling shows a need.

# C++ Atomics & Memory Ordering

## Introduction

Atomics in C++ provide a mechanism for lock-free thread-safe operations on shared data. They guarantee that operations complete without interruption and provide control over how memory operations are ordered across threads.

## Why Atomics?

In multithreaded programs, regular variables can suffer from:
- **Data races**: Multiple threads accessing the same memory simultaneously
- **Torn reads/writes**: Operations that aren't atomic may be partially complete when interrupted
- **Visibility issues**: Changes made by one thread may not be visible to others

```cpp
// NOT thread-safe
int counter = 0;

void increment() {
    counter++;  // Actually: load, add, store - can be interrupted!
}
```

## Basic Atomic Operations

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> counter(0);

void increment() {
    counter++;  // Atomic increment - thread-safe
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    t1.join();
    t2.join();
    std::cout << counter << std::endl;  // Always prints 2
}
```

## Memory Ordering

Memory ordering controls how memory operations are reordered by the compiler and CPU. C++ provides six memory order options:

### 1. `memory_order_relaxed`
No synchronization or ordering constraints, only atomicity is guaranteed.

```cpp
std::atomic<int> x(0);
std::atomic<int> y(0);

// Thread 1
x.store(1, std::memory_order_relaxed);
y.store(1, std::memory_order_relaxed);

// Thread 2
int r1 = y.load(std::memory_order_relaxed);  // May see 1
int r2 = x.load(std::memory_order_relaxed);  // May see 0!
// Operations can be reordered
```

**Use case**: Simple counters where only the final value matters.

```cpp
std::atomic<long> page_views(0);

void record_view() {
    page_views.fetch_add(1, std::memory_order_relaxed);
}
```

### 2. `memory_order_acquire`
Used with **load** operations. Ensures that subsequent reads/writes cannot be reordered before this load.

### 3. `memory_order_release`
Used with **store** operations. Ensures that previous reads/writes cannot be reordered after this store.

### 4. `memory_order_acq_rel`
Combines acquire and release semantics for read-modify-write operations.

### 5. `memory_order_seq_cst` (Sequential Consistency)
Strongest guarantee: all operations have a single total order visible to all threads. **This is the default**.

```cpp
std::atomic<bool> ready(false);
int data = 0;

// Thread 1 (Producer)
data = 42;
ready.store(true, std::memory_order_seq_cst);

// Thread 2 (Consumer)
if (ready.load(std::memory_order_seq_cst)) {
    std::cout << data;  // Guaranteed to see 42
}
```

### 6. `memory_order_consume`
Specialized acquire for dependent operations (rarely used in practice).

## Acquire-Release Semantics Example

This is the most common pattern for synchronization:

```cpp
#include <atomic>
#include <thread>
#include <cassert>

std::atomic<bool> ready(false);
int data = 0;

void producer() {
    data = 100;  // Non-atomic write
    ready.store(true, std::memory_order_release);  // Release
}

void consumer() {
    while (!ready.load(std::memory_order_acquire)) {  // Acquire
        // Spin wait
    }
    assert(data == 100);  // Guaranteed to pass
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer);
    t1.join();
    t2.join();
}
```

**Key insight**: Release-store synchronizes with acquire-load, creating a happens-before relationship.

## Compare and Swap (CAS)

```cpp
std::atomic<int> value(0);

bool compare_and_swap_example() {
    int expected = 0;
    int desired = 42;
    
    // Atomically: if value == expected, set value = desired
    bool success = value.compare_exchange_strong(
        expected,  // Updated to actual value if CAS fails
        desired,
        std::memory_order_acq_rel
    );
    
    return success;
}
```

## Lock-Free Stack Example

```cpp
template<typename T>
class LockFreeStack {
private:
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
        new_node->next = head.load(std::memory_order_relaxed);
        
        // Keep trying until CAS succeeds
        while (!head.compare_exchange_weak(
            new_node->next,
            new_node,
            std::memory_order_release,
            std::memory_order_relaxed
        ));
    }
    
    bool pop(T& result) {
        Node* old_head = head.load(std::memory_order_acquire);
        
        while (old_head != nullptr) {
            if (head.compare_exchange_weak(
                old_head,
                old_head->next,
                std::memory_order_release,
                std::memory_order_acquire
            )) {
                result = old_head->data;
                delete old_head;
                return true;
            }
        }
        return false;  // Stack was empty
    }
};
```

## Spinlock Implementation

```cpp
class Spinlock {
    std::atomic<bool> flag{false};

public:
    void lock() {
        while (flag.exchange(true, std::memory_order_acquire)) {
            // Spin until we acquire the lock
            while (flag.load(std::memory_order_relaxed)) {
                // Reduce contention by just reading
            }
        }
    }
    
    void unlock() {
        flag.store(false, std::memory_order_release);
    }
};
```

## Memory Ordering Cheat Sheet

| Memory Order | Load | Store | Read-Modify-Write | Use Case |
|--------------|------|-------|-------------------|----------|
| `relaxed` | ✓ | ✓ | ✓ | Counters, statistics |
| `acquire` | ✓ | - | ✓ | Lock acquisition |
| `release` | - | ✓ | ✓ | Lock release |
| `acq_rel` | - | - | ✓ | RMW operations |
| `seq_cst` | ✓ | ✓ | ✓ | Default, full ordering |

## Common Patterns

### 1. Flag Pattern (Relaxed)
```cpp
std::atomic<bool> shutdown_flag(false);

void worker() {
    while (!shutdown_flag.load(std::memory_order_relaxed)) {
        do_work();
    }
}
```

### 2. Producer-Consumer (Acquire-Release)
```cpp
std::atomic<Data*> shared_data(nullptr);

// Producer
Data* d = new Data();
shared_data.store(d, std::memory_order_release);

// Consumer
Data* d = shared_data.load(std::memory_order_acquire);
if (d) process(d);
```

### 3. Double-Checked Locking (Acquire-Release + Relaxed)
```cpp
std::atomic<Widget*> instance(nullptr);
std::mutex m;

Widget* get_instance() {
    Widget* tmp = instance.load(std::memory_order_acquire);
    if (tmp == nullptr) {
        std::lock_guard<std::mutex> lock(m);
        tmp = instance.load(std::memory_order_relaxed);
        if (tmp == nullptr) {
            tmp = new Widget();
            instance.store(tmp, std::memory_order_release);
        }
    }
    return tmp;
}
```

## Best Practices

1. **Start with `seq_cst`**: It's the safest default. Optimize later if needed.
2. **Use acquire-release pairs**: Most common pattern for synchronization.
3. **Relaxed for counters**: When only atomicity matters, not ordering.
4. **Avoid lock-free unless necessary**: It's complex and error-prone.
5. **Test thoroughly**: Race conditions are hard to reproduce.
6. **Profile before optimizing**: Memory ordering optimization is premature in most cases.

## Performance Considerations

- `seq_cst`: Slowest, full memory barriers
- `acq_rel`: Moderate, one-way barriers
- `relaxed`: Fastest, no synchronization overhead

On x86/x64, acquire-release and sequential consistency have similar performance due to strong hardware memory model. On ARM/PowerPC, the differences are more significant.

## Conclusion

Atomics and memory ordering provide fine-grained control over concurrent access to shared data. While they enable lock-free programming, they require careful reasoning about memory visibility and ordering. For most applications, higher-level primitives like mutexes are safer and sufficient.