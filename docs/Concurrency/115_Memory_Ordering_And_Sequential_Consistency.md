# Memory Ordering and Sequential Consistency in C++

1. **Introduction to memory ordering** - explaining why it exists and the problems it solves
2. **Sequential consistency details** - the strongest memory ordering with its guarantees
3. **Six practical code examples**:
   - Basic sequential consistency demonstration
   - Default atomic operations
   - Producer-consumer pattern
   - Comparing sequential vs relaxed ordering
   - Multi-threaded flag synchronization
   - Performance benchmarking

4. **Guidance on when to use** sequential consistency vs weaker orderings
5. **Common pitfalls** with real examples
6. **Performance considerations** across different architectures
7. **Comprehensive summary** with best practices

The examples progress from simple to complex, showing both correct usage and common mistakes. Each example includes analysis explaining why the code behaves the way it does. The guide emphasizes that sequential consistency is the safe default and should only be optimized away when profiling demonstrates the need and you fully understand the implications.

# Memory Ordering and Sequential Consistency in C++

## Introduction

Memory ordering is one of the most complex aspects of C++ concurrency. When multiple threads access shared memory, the order in which operations appear to execute can differ from what the source code suggests due to compiler optimizations and CPU reordering. C++ provides several memory ordering options to control this behavior, with **sequential consistency** (`memory_order_seq_cst`) being the strongest and most intuitive guarantee.

## What is Memory Ordering?

Memory ordering defines the rules for how memory operations (reads and writes) from different threads become visible to each other. Without proper memory ordering:

- Compilers may reorder instructions for optimization
- CPUs may execute instructions out of order
- CPU caches may delay visibility of writes to other cores
- Different threads may observe operations in different orders

The C++ memory model provides six memory ordering options:
- `memory_order_relaxed`
- `memory_order_consume` (deprecated)
- `memory_order_acquire`
- `memory_order_release`
- `memory_order_acq_rel`
- `memory_order_seq_cst` (default)

## Sequential Consistency (`memory_order_seq_cst`)

Sequential consistency is the **default and strongest** memory ordering in C++. It provides two critical guarantees:

1. **Single Total Order**: All sequentially consistent operations across all threads appear to execute in a single global order
2. **Program Order**: Operations within each thread appear in the order specified in the source code

This makes reasoning about concurrent programs much easier because the behavior matches our intuitive understanding of how programs should work.

### Key Properties

- **Synchronization**: Provides both acquire and release semantics
- **Visibility**: Ensures all threads eventually see a consistent view of memory
- **Ordering**: No reordering of any atomic operations marked as seq_cst
- **Performance**: Most expensive memory ordering due to strong guarantees

## Code Examples

### Example 1: Basic Sequential Consistency

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<bool> x{false};
std::atomic<bool> y{false};
std::atomic<int> z{0};

void write_x() {
    x.store(true, std::memory_order_seq_cst);
}

void write_y() {
    y.store(true, std::memory_order_seq_cst);
}

void read_x_then_y() {
    while (!x.load(std::memory_order_seq_cst));
    if (y.load(std::memory_order_seq_cst)) {
        ++z;
    }
}

void read_y_then_x() {
    while (!y.load(std::memory_order_seq_cst));
    if (x.load(std::memory_order_seq_cst)) {
        ++z;
    }
}

int main() {
    std::thread t1(write_x);
    std::thread t2(write_y);
    std::thread t3(read_x_then_y);
    std::thread t4(read_y_then_x);
    
    t1.join(); t2.join(); t3.join(); t4.join();
    
    // With seq_cst, z must be 1 or 2, never 0
    std::cout << "z = " << z << std::endl;
    
    return 0;
}
```

**Analysis**: With sequential consistency, there's a global order to all operations. At least one of the reading threads must see both `x` and `y` as true, so `z` cannot be 0. With weaker orderings (like relaxed), `z` could be 0.

### Example 2: Default Atomic Operations

```cpp
#include <atomic>
#include <thread>
#include <iostream>

std::atomic<int> counter{0};

void increment() {
    for (int i = 0; i < 1000; ++i) {
        // Uses memory_order_seq_cst by default
        counter.fetch_add(1);
        
        // Equivalent to:
        // counter.fetch_add(1, std::memory_order_seq_cst);
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    std::thread t3(increment);
    
    t1.join();
    t2.join();
    t3.join();
    
    std::cout << "Counter: " << counter << std::endl; // Always 3000
    
    return 0;
}
```

### Example 3: Producer-Consumer with Sequential Consistency

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

class Message {
public:
    int data;
    Message(int d) : data(d) {}
};

std::atomic<Message*> message_ptr{nullptr};
std::atomic<bool> ready{false};

void producer() {
    Message* msg = new Message(42);
    
    // Write data first
    message_ptr.store(msg, std::memory_order_seq_cst);
    
    // Signal ready second
    ready.store(true, std::memory_order_seq_cst);
}

void consumer() {
    // Wait for ready signal
    while (!ready.load(std::memory_order_seq_cst));
    
    // Read data
    Message* msg = message_ptr.load(std::memory_order_seq_cst);
    
    if (msg) {
        std::cout << "Received: " << msg->data << std::endl;
        delete msg;
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons(consumer);
    
    prod.join();
    cons.join();
    
    return 0;
}
```

**Analysis**: Sequential consistency ensures the consumer sees the message pointer write before the ready flag, maintaining the happens-before relationship.

### Example 4: Comparing Sequential vs Relaxed Ordering

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <cassert>

// Sequential consistency version
void sequential_example() {
    std::atomic<int> x{0}, y{0};
    int r1 = 0, r2 = 0;
    
    std::thread t1([&]() {
        x.store(1, std::memory_order_seq_cst);
        r1 = y.load(std::memory_order_seq_cst);
    });
    
    std::thread t2([&]() {
        y.store(1, std::memory_order_seq_cst);
        r2 = x.load(std::memory_order_seq_cst);
    });
    
    t1.join();
    t2.join();
    
    // With seq_cst: r1 == 0 && r2 == 0 is IMPOSSIBLE
    // At least one thread must see the other's write
    std::cout << "Sequential: r1=" << r1 << ", r2=" << r2 << std::endl;
}

// Relaxed ordering version
void relaxed_example() {
    std::atomic<int> x{0}, y{0};
    int r1 = 0, r2 = 0;
    
    std::thread t1([&]() {
        x.store(1, std::memory_order_relaxed);
        r1 = y.load(std::memory_order_relaxed);
    });
    
    std::thread t2([&]() {
        y.store(1, std::memory_order_relaxed);
        r2 = x.load(std::memory_order_relaxed);
    });
    
    t1.join();
    t2.join();
    
    // With relaxed: r1 == 0 && r2 == 0 is POSSIBLE!
    // No ordering guarantees between threads
    std::cout << "Relaxed: r1=" << r1 << ", r2=" << r2 << std::endl;
}

int main() {
    std::cout << "Running examples multiple times:\n";
    for (int i = 0; i < 5; ++i) {
        sequential_example();
        relaxed_example();
    }
    return 0;
}
```

### Example 5: Multi-threaded Flag Synchronization

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

std::atomic<bool> flag1{false};
std::atomic<bool> flag2{false};
int shared_data = 0;

void thread1() {
    shared_data = 100;
    flag1.store(true, std::memory_order_seq_cst);
}

void thread2() {
    // Wait for thread1's signal
    while (!flag1.load(std::memory_order_seq_cst));
    
    shared_data += 50;
    flag2.store(true, std::memory_order_seq_cst);
}

void thread3() {
    // Wait for thread2's signal
    while (!flag2.load(std::memory_order_seq_cst));
    
    // Guaranteed to see shared_data == 150
    std::cout << "Final value: " << shared_data << std::endl;
}

int main() {
    std::thread t1(thread1);
    std::thread t2(thread2);
    std::thread t3(thread3);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

### Example 6: Understanding the Cost of Sequential Consistency

```cpp
#include <atomic>
#include <thread>
#include <chrono>
#include <iostream>

const int ITERATIONS = 10'000'000;

void benchmark_seq_cst() {
    std::atomic<int> counter{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1([&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            counter.fetch_add(1, std::memory_order_seq_cst);
        }
    });
    
    std::thread t2([&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            counter.fetch_add(1, std::memory_order_seq_cst);
        }
    });
    
    t1.join();
    t2.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential Consistency: " << duration.count() << "ms" << std::endl;
}

void benchmark_relaxed() {
    std::atomic<int> counter{0};
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1([&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    });
    
    std::thread t2([&]() {
        for (int i = 0; i < ITERATIONS; ++i) {
            counter.fetch_add(1, std::memory_order_relaxed);
        }
    });
    
    t1.join();
    t2.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Relaxed Ordering: " << duration.count() << "ms" << std::endl;
}

int main() {
    benchmark_seq_cst();
    benchmark_relaxed();
    return 0;
}
```

## When to Use Sequential Consistency

### Use `memory_order_seq_cst` when:

1. **Correctness is paramount** and performance isn't critical
2. You need **intuitive reasoning** about thread interactions
3. You're implementing **complex synchronization** protocols
4. You're unsure which memory ordering to use (safe default)
5. Multiple threads coordinate through **multiple atomic variables**

### Consider weaker orderings when:

1. Performance profiling shows atomic operations are a bottleneck
2. You have simple patterns (like acquire-release for locks)
3. You thoroughly understand the memory model
4. You have extensive testing for data races

## Common Pitfalls

### Pitfall 1: Mixing Orderings Incorrectly

```cpp
std::atomic<bool> ready{false};
int data = 0;

// Thread 1
data = 42;
ready.store(true, std::memory_order_relaxed); // WRONG!

// Thread 2
if (ready.load(std::memory_order_seq_cst)) {
    std::cout << data; // May not see 42!
}
```

**Fix**: Both sides need appropriate ordering (seq_cst or release-acquire pair).

### Pitfall 2: Assuming Non-atomic Variables Are Protected

```cpp
std::atomic<bool> flag{false};
int value = 0; // Non-atomic!

// Thread 1
value = 100; // Data race if concurrent access!
flag.store(true, std::memory_order_seq_cst);

// Thread 2
if (flag.load(std::memory_order_seq_cst)) {
    std::cout << value; // Safe only because flag synchronizes
}
```

**Note**: The atomic flag creates synchronization, making the non-atomic access safe in this pattern.

## Performance Considerations

Sequential consistency typically requires:
- **Memory barriers** on most architectures
- **Cache coherency** protocols to enforce global order
- **Prevention of compiler reordering**

On x86/x64, the performance difference between seq_cst and acquire-release is often minimal because the hardware provides strong guarantees. On ARM and other weakly-ordered architectures, the difference can be significant.

## Summary

**Sequential consistency** (`memory_order_seq_cst`) is C++'s strongest and default memory ordering model. It ensures:

- All threads observe a single, consistent global ordering of atomic operations
- Operations within each thread execute in program order
- Complete synchronization between threads

**Key Takeaways**:
- Sequential consistency is the **safest choice** when in doubt
- It provides **intuitive behavior** matching single-threaded reasoning
- It's the **default** for all atomic operations (can be omitted)
- It has **performance costs** on some architectures but ensures correctness
- For simple patterns (locks, flags), **acquire-release** may suffice
- For performance-critical code, **profile first** before optimizing memory ordering

**Best Practice**: Start with sequential consistency for correctness, then optimize to weaker orderings only when profiling shows it's necessary and you fully understand the implications.

The golden rule: **Prefer correctness over premature optimization**. Sequential consistency makes concurrent code easier to reason about, maintain, and verify—these benefits often outweigh the modest performance costs.