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

// ============================================================================
// MEMORY ORDERING DEMO: Sequential Consistency vs Relaxed
//
// This file illustrates the most fundamental difference between the two
// extremes of the C++ memory model:
//
//   memory_order_seq_cst  — all threads agree on a single global order
//                           of all atomic operations. The strongest and
//                           most expensive ordering.
//
//   memory_order_relaxed  — the only guarantee is atomicity (no torn reads
//                           or writes). Operations may be reordered freely
//                           by the compiler and CPU. No inter-thread
//                           synchronisation is implied.
//
// The classic test for this is the "store-buffering" litmus pattern below:
//
//   Thread 1:  x = 1  ;  r1 = y      Thread 2:  y = 1  ;  r2 = x
//
// The question: can both r1 and r2 read 0?  That would mean each thread
// read before the other's write was visible — i.e. neither thread's store
// was seen by the other.  Under seq_cst this outcome is forbidden; under
// relaxed it is a legal result.
// ============================================================================


// ----------------------------------------------------------------------------
// sequential_example()
//
// Demonstrates the "store-buffer" litmus test under seq_cst ordering.
//
// seq_cst imposes a single total order over ALL seq_cst operations across
// ALL threads.  Every thread observes atomic operations in exactly that order.
// This is equivalent to interleaving the two thread bodies in some serial
// sequence.
//
// The two possible interleavings that produce r1==0 or r2==0:
//
//   t1 runs fully first:  x=1, r1=y(→0), y=1, r2=x(→1)   → r1=0, r2=1  ✓
//   t2 runs fully first:  y=1, r2=x(→0), x=1, r1=y(→1)   → r1=1, r2=0  ✓
//   stores interleaved:   x=1, y=1, r1=y(→1), r2=x(→1)   → r1=1, r2=1  ✓
//
// Every valid interleaving has either x=1 visible to t2 OR y=1 visible to
// t1 (or both).  There is NO valid seq_cst interleaving that hides BOTH
// stores, so r1==0 && r2==0 is impossible.
// ----------------------------------------------------------------------------
void sequential_example() {
    std::atomic<int> x{0}, y{0};
    int r1 = 0, r2 = 0;

    std::thread t1([&]() {
        // Store 1 into x with seq_cst.
        // This is placed into the single global total order before any
        // seq_cst operation that observes x == 0.  The store is immediately
        // visible to all threads once it appears in that total order.
        x.store(1, std::memory_order_seq_cst);

        // Load y with seq_cst.
        // Because the store to x already appears in the global order, and
        // t2's store to y must also appear in that same order, exactly one
        // of these must come first.  If y.store(1) precedes this load in
        // the total order, we read 1; otherwise we read 0.  Either way, at
        // least one of r1 or r2 will be 1 — both cannot be 0.
        r1 = y.load(std::memory_order_seq_cst);
    });

    std::thread t2([&]() {
        // Store 1 into y with seq_cst — symmetric counterpart to x.store above.
        y.store(1, std::memory_order_seq_cst);

        // Load x with seq_cst.
        // By the same total-order argument: if x.store(1) from t1 precedes
        // this load in the global order, r2 == 1.  If this load precedes
        // x.store(1), then r2 == 0 — but then t1's y.load must come *after*
        // y.store(1) in the total order, forcing r1 == 1.
        r2 = x.load(std::memory_order_seq_cst);
    });

    t1.join();
    t2.join();

    // INVARIANT: r1 == 0 && r2 == 0 is IMPOSSIBLE under seq_cst.
    //
    // The total order constraint means we cannot have:
    //   r1 == 0  (y not yet stored when t1 loaded it)  AND
    //   r2 == 0  (x not yet stored when t2 loaded it)
    // simultaneously — that would imply each thread's store happened *after*
    // the other's load, which is a cycle in the total order: a contradiction.
    //
    // On x86 this is enforced "for free" by the TSO (Total Store Order)
    // memory model.  On ARM/POWER, seq_cst requires explicit barrier
    // instructions (e.g. DMB ISH) to drain store buffers before loads.
    std::cout << "Sequential: r1=" << r1 << ", r2=" << r2 << std::endl;
}


// ----------------------------------------------------------------------------
// relaxed_example()
//
// Same litmus test, but with memory_order_relaxed on every operation.
//
// Relaxed gives only ONE guarantee: each individual atomic read/write is
// indivisible (no tearing, no word-shredding).  Beyond that:
//
//   - The compiler may reorder relaxed operations past each other.
//   - The CPU may reorder them via out-of-order execution or store buffers.
//   - Other threads have NO obligation to observe stores in any particular
//     order, or even at all, until a synchronisation point is reached.
//
// Crucially, relaxed does NOT form a happens-before edge between threads.
// Without happens-before, there is no requirement that t2 ever sees t1's
// store (or vice versa) before it performs its own load.
// ----------------------------------------------------------------------------
void relaxed_example() {
    std::atomic<int> x{0}, y{0};
    int r1 = 0, r2 = 0;

    std::thread t1([&]() {
        // Store 1 into x — relaxed.
        // The compiler/CPU is free to reorder this store AFTER the load below.
        // On ARM/POWER, the store may sit in a per-core store buffer and
        // not reach the coherence point before the thread issues its load.
        x.store(1, std::memory_order_relaxed);

        // Load y — relaxed.
        // No barrier separates this load from the store above.  The CPU can
        // speculatively execute this load first, observe y == 0 from its
        // local cache, and only later flush x = 1 to the cache bus.
        r1 = y.load(std::memory_order_relaxed);
    });

    std::thread t2([&]() {
        // Store 1 into y — relaxed.
        // Same situation as t1: the store may be delayed in the store buffer.
        y.store(1, std::memory_order_relaxed);

        // Load x — relaxed.
        // t2 may read x == 0 from its own cache while its y = 1 store is
        // still buffered and not yet visible to t1.
        r2 = x.load(std::memory_order_relaxed);
    });

    t1.join();
    t2.join();

    // POSSIBLE OUTCOME: r1 == 0 && r2 == 0 ("the forbidden result" for seq_cst)
    //
    // Both threads may have read before the other's store propagated:
    //
    //   Execution trace (conceptual):
    //     t1 load y  → 0   (y store from t2 not yet visible)
    //     t2 load x  → 0   (x store from t1 not yet visible)
    //     t1 store x → 1   (propagates to memory after the load)
    //     t2 store y → 1   (propagates to memory after the load)
    //
    // This is architecturally legal on ARM, POWER, and RISC-V.
    // Even on x86 (which has TSO), the compiler alone may reorder
    // the loads before the stores since there is no barrier obligation.
    //
    // How often does r1==0 && r2==0 appear in practice?
    //   - With optimisation disabled: rarely, but non-zero.
    //   - With -O2 on ARM: observable within a handful of iterations.
    //   - On x86 with -O2: the compiler reordering makes it possible;
    //     TSO hardware alone would not produce it, but LLVM/GCC may.
    std::cout << "Relaxed: r1=" << r1 << ", r2=" << r2 << std::endl;
}


int main() {
    // Run both examples 5 times.
    //
    // The sequential version will always print one of:
    //   r1=0, r2=1  |  r1=1, r2=0  |  r1=1, r2=1
    //
    // The relaxed version may additionally print:
    //   r1=0, r2=0
    //
    // If you want to reliably observe the "both zero" relaxed outcome,
    // increase the iteration count to ~10,000 and compile with -O2 on
    // an ARM or POWER machine.  On x86 you may need to look at the
    // generated assembly to confirm the reordering is happening.
    std::cout << "Running examples multiple times:\n";
    for (int i = 0; i < 5; ++i) {
        sequential_example();
        relaxed_example();
    }
    return 0;
}
```

A few things worth highlighting beyond the comments:

The `r1==0 && r2==0` outcome is sometimes called the "IRIW" (Independent Reads of Independent Writes) or "store-buffering" forbidden result. It's the canonical example used in academic memory model papers to distinguish SC from weaker models.

On x86 specifically, the hardware TSO model alone would not produce this result — x86 stores are always observed in program order. But the C++ abstract machine allows the *compiler* to reorder relaxed operations, so the forbidden outcome becomes reachable once `std::memory_order_relaxed` is in the source, even on TSO hardware. Inspecting the assembly with `-O2 -S` will often show the loads hoisted above the stores.

The fix, if you needed cross-thread visibility without the full cost of `seq_cst`, would be `release`/`acquire` pairs — but that only synchronises *one direction* per pair, not the bidirectional guarantee `seq_cst` provides here.


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

// Thread 1 — publisher (BROKEN)
//
// The intent here is the classic "publication idiom": write some data,
// then raise a flag so another thread knows the data is ready to read.
// That idiom requires a happens-before edge from the write to 'data'
// into the read of 'data' in thread 2.  The only way to establish that
// edge across threads is through a release/acquire pair on the atomic flag.
//
// Using relaxed on the store destroys that edge entirely.
// memory_order_relaxed makes NO synchronisation promise whatsoever —
// it only guarantees the store to 'ready' is atomic (not torn).
// It does NOT prevent the compiler or CPU from reordering the non-atomic
// write to 'data' to appear AFTER the store to 'ready'.
data = 42;
ready.store(true, std::memory_order_relaxed); // WRONG!
//
// What the hardware/compiler is allowed to do:
//
//   Compiler: 'data = 42' and 'ready.store(true)' have no ordering
//             constraint between them (relaxed imposes none), so the
//             compiler may schedule the store to 'ready' first.
//
//   CPU (ARM/POWER): the store to 'data' may sit in a write buffer
//             and not reach the cache coherence bus before the store
//             to 'ready' does.  Thread 2 can observe ready==true while
//             the cache line for 'data' still holds 0.
//
//   CPU (x86): TSO means stores are observed in program order by *other*
//             cores, so the hardware alone won't reorder them here.
//             But the compiler does not know the target is x86 when
//             lowering relaxed — it may still reorder at the IR level.
//
// The result: thread 2 can see ready == true and then read data == 0.

// ---

// Thread 2 — consumer (also broken, even though it uses seq_cst)
//
// memory_order_seq_cst on the load side is irrelevant here because
// synchronisation is ALWAYS established by the PAIR of operations:
//
//   a release-store on the writer side  +  an acquire-load on the reader side
//
// seq_cst implies release on stores and acquire on loads, so a seq_cst
// load CAN form a happens-before edge — but only with a store that has
// at least release semantics on the other end.  A relaxed store provides
// no release, so there is nothing for this seq_cst load to synchronise with.
//
// Upgrading one side of the pair without the other is a common mistake.
// The synchronisation contract is bilateral: both ends must opt in.
if (ready.load(std::memory_order_seq_cst)) {
    std::cout << data; // May not see 42! (no happens-before from the write)
}

// THE FIX — use a release/acquire pair:
//
//   Thread 1:  data = 42;
//              ready.store(true, std::memory_order_release);
//              //  ↑ "release" means: all writes (to any variable) that
//              //  appear before this store in program order are guaranteed
//              //  to be visible to any thread that acquires this store.
//
//   Thread 2:  if (ready.load(std::memory_order_acquire)) {
//              //  ↑ "acquire" means: if this load observes the value
//              //  written by a release-store, then all writes the
//              //  releasing thread performed before its store are now
//              //  visible to this thread.
//                  std::cout << data; // guaranteed to see 42
//              }
//
// The release store "publishes" every preceding write.
// The acquire load "subscribes" to everything that was published.
// Together they form a happens-before edge:
//
//   data = 42  →hb→  ready.store(release)  →hb→  ready.load(acquire)  →hb→  read data
//
// seq_cst would also fix it (seq_cst implies release+acquire), but it
// carries an extra cost — a full memory barrier on ARM/POWER — that is
// not needed here.  release/acquire is the minimal correct ordering.
```

The root issue is a widespread misconception: people assume that using a stronger ordering on one side compensates for a weaker ordering on the other. It doesn't — `memory_order_release` and `memory_order_acquire` are matched keywords in the C++ memory model, not independent knobs. The standard is explicit: a release operation synchronises-with an acquire operation that reads the value it wrote. A relaxed store cannot synchronise-with anything, regardless of what the other side uses.

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