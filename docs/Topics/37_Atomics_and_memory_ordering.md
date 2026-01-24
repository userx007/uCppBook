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

These are the most common patterns for synchronization:

```cpp
#include <atomic>
#include <thread>
#include <cassert>
#include <iostream>
#include <vector>
#include <chrono>

// ============================================================================
// BASIC EXAMPLE: Memory Order Release-Acquire
// ============================================================================

std::atomic<bool> ready(false);
int data = 0;

void producer() {
    // Step 1: Write to non-atomic variable
    // This write happens-before the release store below
    data = 100;  
    
    // Step 2: Release operation - publishes all prior writes
    // Any thread that performs an acquire load and sees 'true' will also
    // see data == 100 due to the synchronizes-with relationship
    ready.store(true, std::memory_order_release);
}

void consumer() {
    // Acquire operation - waits until producer publishes data
    // The acquire load synchronizes-with the release store in producer()
    while (!ready.load(std::memory_order_acquire)) {
        std::this_thread::yield();  // More CPU-friendly than pure spin
    }
    
    // Guaranteed to pass: acquire-release ensures we see all writes
    // that happened-before the release store
    assert(data == 100);
}

// ============================================================================
// EXTENSION 1: Multiple Data Items (Producer-Consumer Pattern)
// ============================================================================

struct SharedData {
    int value1 = 0;
    int value2 = 0;
    double value3 = 0.0;
};

std::atomic<bool> multi_ready(false);
SharedData shared;

void producer_multi() {
    // Write multiple non-atomic variables
    shared.value1 = 42;
    shared.value2 = 84;
    shared.value3 = 3.14159;
    
    // Single release ensures all above writes are visible
    multi_ready.store(true, std::memory_order_release);
}

void consumer_multi() {
    while (!multi_ready.load(std::memory_order_acquire)) {
        std::this_thread::yield();  // More CPU-friendly than pure spin
    }
    
    // All values are guaranteed to be visible
    std::cout << "Value1: " << shared.value1 << "\n";
    std::cout << "Value2: " << shared.value2 << "\n";
    std::cout << "Value3: " << shared.value3 << "\n";
}

// ============================================================================
// EXTENSION 2:Sequentially Consistent (Strongest Guarantee)
// ============================================================================

std::atomic<bool> seq_ready(false);
int seq_data = 0;

void producer_seq_cst() {
    seq_data = 200;
    // memory_order_seq_cst provides total ordering across all threads
    // Slower but easier to reason about - good starting point
    seq_ready.store(true, std::memory_order_seq_cst);
}

void consumer_seq_cst() {
    while (!seq_ready.load(std::memory_order_seq_cst)) {}
    assert(seq_data == 200);
}

// ============================================================================
// EXTENSION 3: Relaxed Ordering (Flag/Counter Example)
// ============================================================================

std::atomic<int> counter(0);
std::atomic<bool> done(false);

void increment_worker() {
    for (int i = 0; i < 1000; ++i) {
        // Relaxed: no synchronization, but atomic increment is safe
        // Use for counters where you only care about final value
        counter.fetch_add(1, std::memory_order_relaxed);
    }
    // Release: publish that this thread is done
    done.store(true, std::memory_order_release);
}

// ============================================================================
// EXTENSION 4: Double-Checked Locking Pattern
// ============================================================================

class Singleton {
    static std::atomic<Singleton*> instance;
    static std::mutex mtx;
    
    Singleton() = default;
    
public:
    static Singleton* get_instance() {
        // First check (relaxed) - fast path for already-initialized case
        Singleton* tmp = instance.load(std::memory_order_acquire);
        
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(mtx);
            // Second check - only one thread initializes
            tmp = instance.load(std::memory_order_relaxed);
            if (tmp == nullptr) {
                tmp = new Singleton();
                // Release ensures initialization is visible before pointer
                instance.store(tmp, std::memory_order_release);
            }
        }
        return tmp;
    }
};

std::atomic<Singleton*> Singleton::instance{nullptr};
std::mutex Singleton::mtx;

// ============================================================================
// EXTENSION 5: Wait/Notify (C++20) - Better than Spin-Wait
// ============================================================================
#if __cplusplus >= 202002L
void producer_wait_notify() {
    data = 300;
    ready.store(true, std::memory_order_release);
    ready.notify_one();  // Wake up waiting thread
}

void consumer_wait_notify() {
    // Block until ready becomes true (much better than spinning!)
    ready.wait(false, std::memory_order_acquire);
    assert(data == 300);
}
#endif

// ============================================================================
// MAIN: Run Different Examples
// ============================================================================

int main() {
    std::cout << "=== Basic Release-Acquire Example ===\n";
    {
        std::thread t1(producer);
        std::thread t2(consumer);
        t1.join();
        t2.join();
        std::cout << "Basic test passed!\n\n";
    }
    
    std::cout << "=== Multiple Data Items Example ===\n";
    {
        std::thread t1(producer_multi);
        std::thread t2(consumer_multi);
        t1.join();
        t2.join();
        std::cout << "\n";
    }
    
    std::cout << "=== Sequential Consistency Example ===\n";
    {
        std::thread t1(producer_seq_cst);
        std::thread t2(consumer_seq_cst);
        t1.join();
        t2.join();
        std::cout << "Seq-cst test passed!\n\n";
    }
    
    std::cout << "=== Relaxed Ordering with Counter ===\n";
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < 10; ++i) {
            threads.emplace_back(increment_worker);
        }
        for (auto& t : threads) {
            t.join();
        }
        std::cout << "Final counter: " << counter.load() << "\n\n";
    }
    
    std::cout << "=== Singleton Pattern ===\n";
    {
        std::vector<std::thread> threads;
        for (int i = 0; i < 5; ++i) {
            threads.emplace_back([]() {
                Singleton* s = Singleton::get_instance();
                std::cout << "Instance address: " << s << "\n";
            });
        }
        for (auto& t : threads) {
            t.join();
        }
    }
    
    return 0;
}
```

**Key Memory Ordering Concepts:**

1. **`memory_order_relaxed`**: No synchronization, only atomicity
2. **`memory_order_acquire`**: Prevents reads/writes from moving before this operation
3. **`memory_order_release`**: Prevents reads/writes from moving after this operation
4. **`memory_order_seq_cst`**: Total global ordering (default, slowest)

**When to use what:**
- Release-Acquire: Producer-consumer patterns, publishing data
- Relaxed: Independent counters, flags where order doesn't matter
- Seq-cst: When you need strongest guarantees and performance isn't critical

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