# Atomic synchronization with acquire-release semantics

## C++ implementation

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
        // Busy-wait (spin). In production, consider yielding:
        // std::this_thread::yield();
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

## Rust has very similar atomic memory ordering primitives. 

Here's a comparison and Rust examples:

```rust
use std::sync::atomic::{AtomicBool, AtomicI32, Ordering};
use std::thread;
use std::sync::Arc;

// ============================================================================
// BASIC EXAMPLE: Same as C++ (Release-Acquire)
// ============================================================================

fn basic_example() {
    // Arc = Atomic Reference Counted (for sharing between threads)
    let ready = Arc::new(AtomicBool::new(false));
    let data = Arc::new(AtomicI32::new(0));
    
    let ready_clone = ready.clone();
    let data_clone = data.clone();
    
    // Producer thread
    let producer = thread::spawn(move || {
        data_clone.store(100, Ordering::Relaxed); // Non-atomic write to atomic
        ready_clone.store(true, Ordering::Release); // Release
    });
    
    // Consumer thread
    let consumer = thread::spawn(move || {
        while !ready.load(Ordering::Acquire) { // Acquire
            // Spin wait
            thread::yield_now(); // More polite than pure spinning
        }
        assert_eq!(data.load(Ordering::Relaxed), 100);
        println!("Data received: {}", data.load(Ordering::Relaxed));
    });
    
    producer.join().unwrap();
    consumer.join().unwrap();
}

// ============================================================================
// RUST'S MEMORY ORDERINGS (similar to C++)
// ============================================================================

/*
Ordering::Relaxed   -> std::memory_order_relaxed
Ordering::Acquire   -> std::memory_order_acquire
Ordering::Release   -> std::memory_order_release
Ordering::AcqRel    -> std::memory_order_acq_rel (acquire + release)
Ordering::SeqCst    -> std::memory_order_seq_cst
*/

// ============================================================================
// EXTENSION 1: Multiple Data with UnsafeCell (like C++ non-atomic data)
// ============================================================================

use std::cell::UnsafeCell;

struct SharedData {
    ready: AtomicBool,
    // UnsafeCell allows interior mutability without atomics
    // This is what you use when you want C++-style non-atomic data
    data: UnsafeCell<Vec<i32>>,
}

// UnsafeCell is not Sync by default, we must promise it's safe
unsafe impl Sync for SharedData {}

fn multiple_data_example() {
    let shared = Arc::new(SharedData {
        ready: AtomicBool::new(false),
        data: UnsafeCell::new(Vec::new()),
    });
    
    let shared_producer = shared.clone();
    let producer = thread::spawn(move || {
        unsafe {
            // Write to non-atomic data (requires unsafe)
            (*shared_producer.data.get()).push(1);
            (*shared_producer.data.get()).push(2);
            (*shared_producer.data.get()).push(3);
        }
        // Release: makes all above writes visible
        shared_producer.ready.store(true, Ordering::Release);
    });
    
    let consumer = thread::spawn(move || {
        // Acquire: synchronizes with release
        while !shared.ready.load(Ordering::Acquire) {
            thread::yield_now();
        }
        unsafe {
            // Now safe to read - guaranteed to see producer's writes
            let data = &*shared.data.get();
            println!("Received data: {:?}", data);
            assert_eq!(data.len(), 3);
        }
    });
    
    producer.join().unwrap();
    consumer.join().unwrap();
}

// ============================================================================
// EXTENSION 2: Compare-and-Swap (Lock-Free Data Structures)
// ============================================================================

fn compare_and_swap_example() {
    let value = Arc::new(AtomicI32::new(0));
    let mut threads = vec![];
    
    for i in 0..10 {
        let value_clone = value.clone();
        threads.push(thread::spawn(move || {
            // Try to increment atomically
            loop {
                let current = value_clone.load(Ordering::Acquire);
                // compare_exchange: if value == current, set to current+1
                match value_clone.compare_exchange(
                    current,
                    current + 1,
                    Ordering::Release,  // Success ordering
                    Ordering::Acquire   // Failure ordering
                ) {
                    Ok(_) => {
                        println!("Thread {} incremented to {}", i, current + 1);
                        break;
                    }
                    Err(_) => continue, // Retry if someone else changed it
                }
            }
        }));
    }
    
    for t in threads {
        t.join().unwrap();
    }
    
    println!("Final value: {}", value.load(Ordering::SeqCst));
}

// ============================================================================
// EXTENSION 3: Fetch Operations (Atomic Read-Modify-Write)
// ============================================================================

fn fetch_operations() {
    let counter = Arc::new(AtomicI32::new(0));
    let mut threads = vec![];
    
    for _ in 0..10 {
        let counter_clone = counter.clone();
        threads.push(thread::spawn(move || {
            for _ in 0..1000 {
                // Atomic increment - returns OLD value
                counter_clone.fetch_add(1, Ordering::Relaxed);
                
                // Other operations:
                // fetch_sub, fetch_and, fetch_or, fetch_xor, etc.
            }
        }));
    }
    
    for t in threads {
        t.join().unwrap();
    }
    
    println!("Counter: {}", counter.load(Ordering::SeqCst));
}

// ============================================================================
// EXTENSION 4: Rust's Safer Alternative - Mutex (most common)
// ============================================================================

use std::sync::Mutex;

fn mutex_example() {
    // Mutex is usually preferred in Rust - safer than atomics + UnsafeCell
    let data = Arc::new(Mutex::new(vec![0, 1, 2]));
    
    let data_clone = data.clone();
    let producer = thread::spawn(move || {
        let mut d = data_clone.lock().unwrap();
        d.push(3);
        d.push(4);
        // Lock automatically released when 'd' goes out of scope
    });
    
    producer.join().unwrap();
    
    let d = data.lock().unwrap();
    println!("Data: {:?}", *d);
}

// ============================================================================
// EXTENSION 5: Once (Thread-Safe Lazy Initialization)
// ============================================================================

use std::sync::Once;

static INIT: Once = Once::new();
static mut VALUE: i32 = 0;

fn lazy_init_example() {
    let mut threads = vec![];
    
    for i in 0..5 {
        threads.push(thread::spawn(move || {
            INIT.call_once(|| {
                unsafe {
                    VALUE = 42;
                }
                println!("Initialized by thread {}", i);
            });
            
            unsafe {
                println!("Thread {} sees value: {}", i, VALUE);
            }
        }));
    }
    
    for t in threads {
        t.join().unwrap();
    }
}

// ============================================================================
// EXTENSION 6: Atomic vs Mutex - When to Use What
// ============================================================================

fn performance_comparison() {
    use std::time::Instant;
    
    // Atomic counter
    let atomic_counter = Arc::new(AtomicI32::new(0));
    let start = Instant::now();
    let mut threads = vec![];
    
    for _ in 0..4 {
        let counter = atomic_counter.clone();
        threads.push(thread::spawn(move || {
            for _ in 0..1_000_000 {
                counter.fetch_add(1, Ordering::Relaxed);
            }
        }));
    }
    
    for t in threads {
        t.join().unwrap();
    }
    println!("Atomic time: {:?}", start.elapsed());
    
    // Mutex counter
    let mutex_counter = Arc::new(Mutex::new(0));
    let start = Instant::now();
    let mut threads = vec![];
    
    for _ in 0..4 {
        let counter = mutex_counter.clone();
        threads.push(thread::spawn(move || {
            for _ in 0..1_000_000 {
                let mut c = counter.lock().unwrap();
                *c += 1;
            }
        }));
    }
    
    for t in threads {
        t.join().unwrap();
    }
    println!("Mutex time: {:?}", start.elapsed());
}

// ============================================================================
// MAIN
// ============================================================================

fn main() {
    println!("=== Basic Release-Acquire ===");
    basic_example();
    
    println!("\n=== Multiple Data ===");
    multiple_data_example();
    
    println!("\n=== Compare and Swap ===");
    compare_and_swap_example();
    
    println!("\n=== Fetch Operations ===");
    fetch_operations();
    
    println!("\n=== Mutex (Safer Alternative) ===");
    mutex_example();
    
    println!("\n=== Lazy Initialization ===");
    lazy_init_example();
    
    println!("\n=== Performance Comparison ===");
    performance_comparison();
}
```

**Key Differences Between Rust and C++:**

| Feature | C++ | Rust |
|---------|-----|------|
| **Memory orderings** | Same names | Same semantics, different syntax |
| **Non-atomic data** | Direct access | Requires `UnsafeCell` + `unsafe` |
| **Default choice** | Often use atomics | Prefer `Mutex` for safety |
| **Thread safety** | Manual via docs | Enforced by compiler (`Send`/`Sync`) |
| **Sharing data** | Raw pointers/refs | `Arc` (atomic ref count) |

**Rust Advantages:**
- **Compile-time safety**: Can't accidentally share non-`Sync` types
- **No data races**: Type system prevents them entirely
- **Memory safety**: No use-after-free with atomics

**When to use Atomics in Rust:**
- Simple flags/counters (`AtomicBool`, `AtomicI32`)
- Lock-free algorithms (advanced)
- High-performance scenarios where mutex overhead matters

**When to use Mutex in Rust:**
- Default choice for shared mutable data
- Complex data structures
- When you need to protect multiple values together
- When code clarity > raw performance

Rust's philosophy: make the safe thing easy (`Mutex`) and the fast thing explicit (`Atomic` + `unsafe`).