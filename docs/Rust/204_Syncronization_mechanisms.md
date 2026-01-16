# Synchronization Mechanisms in Concurrent Programming: C++ vs. Rust

## Introduction

Concurrent programming requires careful coordination between threads to prevent race conditions, data corruption, and deadlocks. Both C++ and Rust provide synchronization primitives, but they differ fundamentally in their approach: C++ relies on programmer discipline and runtime checks, while Rust enforces thread safety at compile time through its ownership system and type system.

## Fundamental Differences in Approach

### C++ Philosophy
C++ provides low-level synchronization primitives with minimal compile-time enforcement. The programmer is responsible for ensuring correct usage, and mistakes often manifest as runtime errors that are difficult to debug.

### Rust Philosophy
Rust's ownership system and type system prevent data races at compile time. The compiler enforces that shared mutable state is properly synchronized, making entire classes of concurrency bugs impossible. Rust's mantra: "fearless concurrency."

## Core Synchronization Mechanisms

### 1. Mutexes (Mutual Exclusion Locks)

**C++ Implementation:**

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mtx;
int shared_counter = 0;

void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(mtx);  // RAII-style locking
        ++shared_counter;
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(increment_counter, 1000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter: " << shared_counter << std::endl;
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::{Arc, Mutex};  // Arc = Atomic Reference Counting for shared ownership
use std::thread;

fn main() {
    // Mutex<T> wraps the data - compile-time guarantee of safe access
    let counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];
    
    // Spawn 5 threads
    for _ in 0..5 {
        
        // Clone the Arc (reference count++), not the data
        let counter_clone = Arc::clone(&counter);  // / new Arc clone each iteration
        //  ^^^^^^^^^^^^^ Lives in this scope

        // counter_clone is MOVED here, now owned by this thread
        // Safe because Arc allows shared ownership across threads            
        let handle = thread::spawn(move || {
        //                         ^^^^ Transfers ownership into this closure
            for _ in 0..1000 {
                //         Lock acquired here ↓ uses the moved value
                let mut num = counter_clone.lock().unwrap();
                //    ^^^                          ^^^^^^^^
                //    |                            Extract MutexGuard from Result
                //    MutexGuard<i32> smart pointer
                
                *num += 1;  // Dereference to access the i32 inside
                //  ^
                //  Access the actual data through the guard

            }  // IMPORTANT: MutexGuard automatically unlocks when dropped

        }); // Thread owns counter_clone until it finishes

        // IMPORTANT: counter_clone is no longer accessible here - it's been moved

        // state handles for being joined later on
        handles.push(handle);
    }
    
    // Wait for all threads to complete
    for handle in handles {
        handle.join().unwrap();
    }
    
    println!("Final counter: {}", *counter.lock().unwrap());  // Expected: 5000
}
```
#### Why Arc Makes This Work:

- `Arc::clone(&counter)` creates a new reference-counted pointer
- Each thread gets its **own Arc** (not the data itself)
- All Arcs point to the **same underlying data**
- When moved into the thread, the Arc's reference count keeps the data alive
- When the thread finishes and drops its Arc, the reference count decreases

#### Summary:

- `move` transfers ownership of `counter_clone` into the thread
- This is safe because `Arc` allows **shared ownership across threads**
- Each thread owns its own `Arc` instance
- All `Arc` instances share access to the same `Mutex<i32>`


**Key Differences:**
- **C++**: The mutex and data are separate entities. Nothing prevents accessing `shared_counter` without locking `mtx`.
- **Rust**: `Mutex<T>` wraps the data itself. The only way to access the data is through the lock, enforced at compile time.

### 2. Read-Write Locks

**C++ Implementation:**

```cpp
#include <shared_mutex>
#include <thread>
#include <vector>
#include <iostream>

std::shared_mutex rw_mutex;
std::vector<int> shared_data = {1, 2, 3, 4, 5};

void reader(int id) {
    std::shared_lock<std::shared_mutex> lock(rw_mutex);
    std::cout << "Reader " << id << " sees " << shared_data.size() 
              << " elements\n";
}

void writer(int value) {
    std::unique_lock<std::shared_mutex> lock(rw_mutex);
    shared_data.push_back(value);
    std::cout << "Writer added value: " << value << "\n";
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(reader, i);
    }
    threads.emplace_back(writer, 42);
    
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::{Arc, RwLock};
use std::thread;

fn main() {
    let data = Arc::new(RwLock::new(vec![1, 2, 3, 4, 5]));
    let mut handles = vec![];
    
    // Spawn reader threads
    for i in 0..3 {
        let data_clone = Arc::clone(&data);
        let handle = thread::spawn(move || {
            let read_guard = data_clone.read().unwrap();
            println!("Reader {} sees {} elements", i, read_guard.len());
        });
        handles.push(handle);
    }
    
    // Spawn writer thread
    let data_clone = Arc::clone(&data);
    let handle = thread::spawn(move || {
        let mut write_guard = data_clone.write().unwrap();
        write_guard.push(42);
        println!("Writer added value: 42");
    });
    handles.push(handle);
    
    for handle in handles {
        handle.join().unwrap();
    }
}
```

**Key Differences:**
- **C++**: Uses `shared_lock` for readers and `unique_lock` for writers. Data and lock are separate.
- **Rust**: `RwLock<T>` wraps data. Returns `RwLockReadGuard` or `RwLockWriteGuard` that provide access only while held.

### 3. Condition Variables

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> data_queue;
bool finished = false;

void producer() {
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(i);
            std::cout << "Produced: " << i << "\n";
        }
        cv.notify_one();
    }
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_all();
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !data_queue.empty() || finished; });
        
        if (data_queue.empty() && finished) break;
        
        if (!data_queue.empty()) {
            int value = data_queue.front();
            data_queue.pop();
            lock.unlock();
            std::cout << "Consumer " << id << " consumed: " << value << "\n";
        }
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons1(consumer, 1);
    std::thread cons2(consumer, 2);
    
    prod.join();
    cons1.join();
    cons2.join();
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::{Arc, Mutex, Condvar};
use std::thread;
use std::collections::VecDeque;
use std::time::Duration;

fn main() {
    let pair = Arc::new((Mutex::new(VecDeque::new()), Condvar::new()));
    let finished = Arc::new(Mutex::new(false));
    
    let pair_clone = Arc::clone(&pair);
    let finished_clone = Arc::clone(&finished);
    
    // Producer thread
    let producer = thread::spawn(move || {
        let (lock, cvar) = &*pair_clone;
        for i in 0..10 {
            thread::sleep(Duration::from_millis(100));
            {
                let mut queue = lock.lock().unwrap();
                queue.push_back(i);
                println!("Produced: {}", i);
            }
            cvar.notify_one();
        }
        *finished_clone.lock().unwrap() = true;
        cvar.notify_all();
    });
    
    // Consumer threads
    let mut consumers = vec![];
    for id in 1..=2 {
        let pair_clone = Arc::clone(&pair);
        let finished_clone = Arc::clone(&finished);
        
        let consumer = thread::spawn(move || {
            let (lock, cvar) = &*pair_clone;
            loop {
                let mut queue = lock.lock().unwrap();
                
                while queue.is_empty() && !*finished_clone.lock().unwrap() {
                    queue = cvar.wait(queue).unwrap();
                }
                
                if let Some(value) = queue.pop_front() {
                    drop(queue);
                    println!("Consumer {} consumed: {}", id, value);
                } else if *finished_clone.lock().unwrap() {
                    break;
                }
            }
        });
        consumers.push(consumer);
    }
    
    producer.join().unwrap();
    for consumer in consumers {
        consumer.join().unwrap();
    }
}
```

### 4. Atomic Operations

**C++ Implementation:**

```cpp
#include <atomic>
#include <thread>
#include <vector>
#include <iostream>

std::atomic<int> atomic_counter(0);
std::atomic<bool> flag(false);

void increment_atomic(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

void spin_wait() {
    while (!flag.load(std::memory_order_acquire)) {
        // Busy wait
    }
    std::cout << "Flag is now true!\n";
}

void set_flag() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    flag.store(true, std::memory_order_release);
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(increment_atomic, 1000);
    }
    
    std::thread waiter(spin_wait);
    std::thread setter(set_flag);
    
    for (auto& t : threads) {
        t.join();
    }
    waiter.join();
    setter.join();
    
    std::cout << "Final atomic counter: " << atomic_counter.load() << "\n";
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::atomic::{AtomicBool, AtomicI32, Ordering};
use std::sync::Arc;
use std::thread;
use std::time::Duration;

fn main() {
    let counter = Arc::new(AtomicI32::new(0));
    let flag = Arc::new(AtomicBool::new(false));
    
    let mut handles = vec![];
    
    // Increment threads
    for _ in 0..5 {
        let counter_clone = Arc::clone(&counter);
        let handle = thread::spawn(move || {
            for _ in 0..1000 {
                counter_clone.fetch_add(1, Ordering::Relaxed);
            }
        });
        handles.push(handle);
    }
    
    // Spin wait thread
    let flag_clone = Arc::clone(&flag);
    let waiter = thread::spawn(move || {
        while !flag_clone.load(Ordering::Acquire) {
            // Busy wait
        }
        println!("Flag is now true!");
    });
    
    // Flag setter thread
    let flag_clone = Arc::clone(&flag);
    let setter = thread::spawn(move || {
        thread::sleep(Duration::from_millis(100));
        flag_clone.store(true, Ordering::Release);
    });
    
    for handle in handles {
        handle.join().unwrap();
    }
    waiter.join().unwrap();
    setter.join().unwrap();
    
    println!("Final atomic counter: {}", counter.load(Ordering::SeqCst));
}
```

**Key Differences:**
- Both languages provide similar atomic types and memory ordering options
- Rust's `Arc<Atomic<T>>` pattern is idiomatic for sharing atomics across threads
- C++ allows direct atomic variables; Rust enforces explicit sharing through `Arc`

### 5. Channels (Message Passing)

**C++ Implementation (using a simple approach):**

```cpp
#include <iostream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>

template<typename T>
class Channel {
private:
    std::queue<T> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool closed = false;

public:
    void send(T value) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            if (closed) throw std::runtime_error("Channel closed");
            queue.push(std::move(value));
        }
        cv.notify_one();
    }
    
    std::optional<T> receive() {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this]{ return !queue.empty() || closed; });
        
        if (queue.empty()) return std::nullopt;
        
        T value = std::move(queue.front());
        queue.pop();
        return value;
    }
    
    void close() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            closed = true;
        }
        cv.notify_all();
    }
};

int main() {
    Channel<int> channel;
    
    std::thread sender([&channel]() {
        for (int i = 0; i < 5; ++i) {
            channel.send(i);
            std::cout << "Sent: " << i << "\n";
        }
        channel.close();
    });
    
    std::thread receiver([&channel]() {
        while (auto value = channel.receive()) {
            std::cout << "Received: " << *value << "\n";
        }
    });
    
    sender.join();
    receiver.join();
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::mpsc;
use std::thread;

fn main() {
    let (tx, rx) = mpsc::channel();
    
    let sender = thread::spawn(move || {
        for i in 0..5 {
            tx.send(i).unwrap();
            println!("Sent: {}", i);
        }
        // Channel automatically closes when tx is dropped
    });
    
    let receiver = thread::spawn(move || {
        while let Ok(value) = rx.recv() {
            println!("Received: {}", value);
        }
    });
    
    sender.join().unwrap();
    receiver.join().unwrap();
}

// Multiple producer example
fn multiple_producers_example() {
    let (tx, rx) = mpsc::channel();
    
    let mut senders = vec![];
    for id in 0..3 {
        let tx_clone = tx.clone();
        let sender = thread::spawn(move || {
            for i in 0..3 {
                tx_clone.send((id, i)).unwrap();
                println!("Producer {} sent: {}", id, i);
            }
        });
        senders.push(sender);
    }
    drop(tx); // Drop original sender
    
    let receiver = thread::spawn(move || {
        while let Ok((id, value)) = rx.recv() {
            println!("Received from producer {}: {}", id, value);
        }
    });
    
    for sender in senders {
        sender.join().unwrap();
    }
    receiver.join().unwrap();
}
```

**Key Differences:**
- **C++**: No built-in channel support; must implement or use third-party libraries
- **Rust**: Built-in `mpsc` (multiple producer, single consumer) channels in standard library
- Rust channels enforce ownership transfer, preventing data races by design

### 6. Barriers

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <barrier>
#include <vector>

void parallel_phase(std::barrier<>& sync_point, int id, int phase) {
    std::cout << "Thread " << id << " working on phase " << phase << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    std::cout << "Thread " << id << " finished phase " << phase << "\n";
    sync_point.arrive_and_wait();
}

int main() {
    const int num_threads = 4;
    std::barrier sync_point(num_threads);
    std::vector<std::thread> threads;
    
    for (int id = 0; id < num_threads; ++id) {
        threads.emplace_back([&sync_point, id]() {
            for (int phase = 0; phase < 3; ++phase) {
                parallel_phase(sync_point, id, phase);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::{Arc, Barrier};
use std::thread;
use std::time::Duration;

fn parallel_phase(barrier: &Barrier, id: usize, phase: usize) {
    println!("Thread {} working on phase {}", id, phase);
    thread::sleep(Duration::from_millis(100 * id as u64));
    println!("Thread {} finished phase {}", id, phase);
    barrier.wait();
}

fn main() {
    let num_threads = 4;
    let barrier = Arc::new(Barrier::new(num_threads));
    let mut handles = vec![];
    
    for id in 0..num_threads {
        let barrier_clone = Arc::clone(&barrier);
        let handle = thread::spawn(move || {
            for phase in 0..3 {
                parallel_phase(&barrier_clone, id, phase);
            }
        });
        handles.push(handle);
    }
    
    for handle in handles {
        handle.join().unwrap();
    }
}
```

### 7. Scoped Threads and Thread Safety

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <vector>

void process_data(std::vector<int>& data, int start, int end) {
    for (int i = start; i < end; ++i) {
        data[i] *= 2;
    }
}

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
    const int num_threads = 4;
    std::vector<std::thread> threads;
    
    int chunk_size = data.size() / num_threads;
    
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? data.size() : (i + 1) * chunk_size;
        
        // Reference to data is captured, programmer must ensure safety
        threads.emplace_back(process_data, std::ref(data), start, end);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (int val : data) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    return 0;
}
```

**Rust Implementation:**

```rust
use std::thread;

fn main() {
    let mut data = vec![1, 2, 3, 4, 5, 6, 7, 8];
    let num_threads = 4;
    let chunk_size = data.len() / num_threads;
    
    // Scoped threads allow borrowing local data
    thread::scope(|s| {
        let chunks: Vec<&mut [i32]> = data.chunks_mut(chunk_size).collect();
        
        for chunk in chunks {
            s.spawn(move || {
                for item in chunk.iter_mut() {
                    *item *= 2;
                }
            });
        }
        // All threads automatically joined at end of scope
    });
    
    for val in &data {
        print!("{} ", val);
    }
    println!();
}
```

**Key Differences:**
- **C++**: Programmer must manually ensure references remain valid and threads are joined
- **Rust**: Scoped threads (`thread::scope`) guarantee all threads finish before local data goes out of scope, enforced at compile time

### 8. Semaphores

Semaphores are synchronization primitives that maintain a count, allowing a limited number of threads to access a resource simultaneously.

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <semaphore>
#include <chrono>

// C++20 introduces counting_semaphore
std::counting_semaphore<5> connection_pool(5); // Max 5 concurrent connections

void simulate_connection(int id) {
    std::cout << "Thread " << id << " waiting for connection...\n";
    
    connection_pool.acquire(); // Wait for available slot
    
    std::cout << "Thread " << id << " got connection, working...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Thread " << id << " releasing connection\n";
    
    connection_pool.release(); // Release slot
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(simulate_connection, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}

// Binary semaphore example (mutex-like)
void binary_semaphore_example() {
    std::binary_semaphore sem(1); // Acts like a mutex
    int shared_resource = 0;
    
    auto worker = [&sem, &shared_resource](int id) {
        for (int i = 0; i < 3; ++i) {
            sem.acquire();
            ++shared_resource;
            std::cout << "Thread " << id << " incremented to " 
                      << shared_resource << "\n";
            sem.release();
        }
    };
    
    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    
    t1.join();
    t2.join();
}
```

**Rust Implementation:**

```rust
use std::sync::{Arc, Semaphore};
use std::thread;
use std::time::Duration;

// Note: As of Rust 1.x, semaphores are not in std, but available in tokio
// Here's a custom implementation for demonstration:

use std::sync::{Mutex, Condvar};

struct Semaphore {
    count: Mutex<usize>,
    cvar: Condvar,
}

impl Semaphore {
    fn new(count: usize) -> Self {
        Semaphore {
            count: Mutex::new(count),
            cvar: Condvar::new(),
        }
    }
    
    fn acquire(&self) {
        let mut count = self.count.lock().unwrap();
        while *count == 0 {
            count = self.cvar.wait(count).unwrap();
        }
        *count -= 1;
    }
    
    fn release(&self) {
        let mut count = self.count.lock().unwrap();
        *count += 1;
        self.cvar.notify_one();
    }
}

fn main() {
    let connection_pool = Arc::new(Semaphore::new(5));
    let mut handles = vec![];
    
    for id in 0..10 {
        let pool = Arc::clone(&connection_pool);
        let handle = thread::spawn(move || {
            println!("Thread {} waiting for connection...", id);
            
            pool.acquire();
            
            println!("Thread {} got connection, working...", id);
            thread::sleep(Duration::from_millis(500));
            println!("Thread {} releasing connection", id);
            
            pool.release();
        });
        handles.push(handle);
    }
    
    for handle in handles {
        handle.join().unwrap();
    }
}
```

**Key Differences:**
- **C++**: `counting_semaphore` and `binary_semaphore` in C++20 standard library
- **Rust**: No built-in semaphore in std library; use external crates like `tokio` or implement using mutex + condvar
- Both provide similar semantics for resource counting

### 9. Once/Call Once Initialization

For thread-safe lazy initialization that happens exactly once.

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::once_flag init_flag;
int expensive_resource = 0;

void initialize_resource() {
    std::cout << "Initializing expensive resource...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    expensive_resource = 42;
    std::cout << "Resource initialized!\n";
}

void use_resource(int id) {
    std::call_once(init_flag, initialize_resource);
    std::cout << "Thread " << id << " using resource: " 
              << expensive_resource << "\n";
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(use_resource, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::Once;
use std::thread;
use std::time::Duration;

static INIT: Once = Once::new();
static mut EXPENSIVE_RESOURCE: i32 = 0;

fn initialize_resource() {
    println!("Initializing expensive resource...");
    thread::sleep(Duration::from_millis(100));
    unsafe {
        EXPENSIVE_RESOURCE = 42;
    }
    println!("Resource initialized!");
}

fn use_resource(id: usize) {
    INIT.call_once(|| {
        initialize_resource();
    });
    
    unsafe {
        println!("Thread {} using resource: {}", id, EXPENSIVE_RESOURCE);
    }
}

fn main() {
    let mut handles = vec![];
    
    for id in 0..5 {
        let handle = thread::spawn(move || {
            use_resource(id);
        });
        handles.push(handle);
    }
    
    for handle in handles {
        handle.join().unwrap();
    }
}

// Better approach with OnceLock (Rust 1.70+)
use std::sync::OnceLock;

static RESOURCE: OnceLock<i32> = OnceLock::new();

fn use_resource_safe(id: usize) {
    let value = RESOURCE.get_or_init(|| {
        println!("Initializing expensive resource (safe)...");
        thread::sleep(Duration::from_millis(100));
        42
    });
    
    println!("Thread {} using resource: {}", id, value);
}
```

**Key Differences:**
- **C++**: `std::once_flag` with `std::call_once`
- **Rust**: `Once` for basic cases, `OnceLock`/`LazyLock` for safer typed initialization
- Rust's `OnceLock` provides type-safe lazy initialization without `unsafe`

### 10. Spin Locks

Low-level locks that busy-wait instead of yielding to the OS scheduler.

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <vector>

class SpinLock {
private:
    std::atomic_flag flag = ATOMIC_FLAG_INIT;
    
public:
    void lock() {
        while (flag.test_and_set(std::memory_order_acquire)) {
            // Busy wait
            #ifdef __cpp_lib_atomic_flag_test
            while (flag.test(std::memory_order_relaxed)) {
                // Reduce contention
            }
            #endif
        }
    }
    
    void unlock() {
        flag.clear(std::memory_order_release);
    }
};

SpinLock spin_lock;
int counter = 0;

void increment_with_spinlock(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        spin_lock.lock();
        ++counter;
        spin_lock.unlock();
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(increment_with_spinlock, 10000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter: " << counter << "\n";
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::atomic::{AtomicBool, Ordering};
use std::sync::Arc;
use std::thread;
use std::hint;

struct SpinLock {
    locked: AtomicBool,
}

impl SpinLock {
    fn new() -> Self {
        SpinLock {
            locked: AtomicBool::new(false),
        }
    }
    
    fn lock(&self) {
        while self.locked.compare_exchange_weak(
            false,
            true,
            Ordering::Acquire,
            Ordering::Relaxed
        ).is_err() {
            // Reduce contention
            while self.locked.load(Ordering::Relaxed) {
                hint::spin_loop();
            }
        }
    }
    
    fn unlock(&self) {
        self.locked.store(false, Ordering::Release);
    }
}

fn main() {
    let spin_lock = Arc::new(SpinLock::new());
    let counter = Arc::new(std::sync::atomic::AtomicI32::new(0));
    let mut handles = vec![];
    
    for _ in 0..4 {
        let lock = Arc::clone(&spin_lock);
        let counter_clone = Arc::clone(&counter);
        
        let handle = thread::spawn(move || {
            for _ in 0..10000 {
                lock.lock();
                counter_clone.fetch_add(1, Ordering::Relaxed);
                lock.unlock();
            }
        });
        handles.push(handle);
    }
    
    for handle in handles {
        handle.join().unwrap();
    }
    
    println!("Final counter: {}", counter.load(Ordering::SeqCst));
}

// Using the spin crate (popular in Rust ecosystem)
use spin::Mutex as SpinMutex;

fn with_spin_crate() {
    let counter = Arc::new(SpinMutex::new(0));
    let mut handles = vec![];
    
    for _ in 0..4 {
        let counter_clone = Arc::clone(&counter);
        let handle = thread::spawn(move || {
            for _ in 0..10000 {
                *counter_clone.lock() += 1;
            }
        });
        handles.push(handle);
    }
    
    for handle in handles {
        handle.join().unwrap();
    }
    
    println!("Final counter: {}", *counter.lock());
}
```

**Key Differences:**
- **C++**: Manual implementation using `atomic_flag`
- **Rust**: Manual implementation or use `spin` crate for production-ready spinlocks
- Both suitable for very short critical sections where context switching overhead exceeds spin time

### 11. Latches and Countdown Latches

Single-use synchronization point where threads wait until a counter reaches zero.

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <latch>
#include <vector>

void worker_with_latch(std::latch& done_signal, int id) {
    std::cout << "Worker " << id << " starting work...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    std::cout << "Worker " << id << " finished work\n";
    
    done_signal.count_down(); // Decrement counter
}

void coordinator_with_latch() {
    const int num_workers = 5;
    std::latch all_done(num_workers); // Initialize with count
    std::vector<std::thread> threads;
    
    std::cout << "Starting workers...\n";
    
    for (int i = 0; i < num_workers; ++i) {
        threads.emplace_back(worker_with_latch, std::ref(all_done), i);
    }
    
    std::cout << "Coordinator waiting for all workers...\n";
    all_done.wait(); // Wait for count to reach zero
    std::cout << "All workers done! Proceeding...\n";
    
    for (auto& t : threads) {
        t.join();
    }
}

// Start gate pattern
void start_gate_example() {
    const int num_threads = 5;
    std::latch start_signal(1); // All threads wait for this
    std::latch done_signal(num_threads); // Main waits for these
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&start_signal, &done_signal, i]() {
            std::cout << "Thread " << i << " ready, waiting for start...\n";
            start_signal.wait(); // Wait for start signal
            
            std::cout << "Thread " << i << " running!\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            done_signal.count_down();
        });
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "GO!\n";
    start_signal.count_down(); // Release all threads at once
    
    done_signal.wait();
    std::cout << "All threads finished\n";
    
    for (auto& t : threads) {
        t.join();
    }
}

int main() {
    coordinator_with_latch();
    std::cout << "\n--- Start Gate Pattern ---\n\n";
    start_gate_example();
    return 0;
}
```

**Rust Implementation:**

```rust
use std::sync::{Arc, Mutex, Condvar};
use std::thread;
use std::time::Duration;

// Custom countdown latch implementation
struct CountDownLatch {
    count: Mutex<usize>,
    cvar: Condvar,
}

impl CountDownLatch {
    fn new(count: usize) -> Self {
        CountDownLatch {
            count: Mutex::new(count),
            cvar: Condvar::new(),
        }
    }
    
    fn count_down(&self) {
        let mut count = self.count.lock().unwrap();
        *count -= 1;
        if *count == 0 {
            self.cvar.notify_all();
        }
    }
    
    fn wait(&self) {
        let mut count = self.count.lock().unwrap();
        while *count > 0 {
            count = self.cvar.wait(count).unwrap();
        }
    }
}

fn worker_with_latch(done_signal: Arc<CountDownLatch>, id: usize) {
    println!("Worker {} starting work...", id);
    thread::sleep(Duration::from_millis(100 * id as u64));
    println!("Worker {} finished work", id);
    
    done_signal.count_down();
}

fn coordinator_with_latch() {
    let num_workers = 5;
    let all_done = Arc::new(CountDownLatch::new(num_workers));
    let mut handles = vec![];
    
    println!("Starting workers...");
    
    for i in 0..num_workers {
        let latch = Arc::clone(&all_done);
        let handle = thread::spawn(move || {
            worker_with_latch(latch, i);
        });
        handles.push(handle);
    }
    
    println!("Coordinator waiting for all workers...");
    all_done.wait();
    println!("All workers done! Proceeding...");
    
    for handle in handles {
        handle.join().unwrap();
    }
}

// Start gate pattern
fn start_gate_example() {
    let num_threads = 5;
    let start_signal = Arc::new(CountDownLatch::new(1));
    let done_signal = Arc::new(CountDownLatch::new(num_threads));
    
    let mut handles = vec![];
    
    for i in 0..num_threads {
        let start = Arc::clone(&start_signal);
        let done = Arc::clone(&done_signal);
        
        let handle = thread::spawn(move || {
            println!("Thread {} ready, waiting for start...", i);
            start.wait();
            
            println!("Thread {} running!", i);
            thread::sleep(Duration::from_millis(100));
            
            done.count_down();
        });
        handles.push(handle);
    }
    
    thread::sleep(Duration::from_millis(200));
    println!("GO!");
    start_signal.count_down();
    
    done_signal.wait();
    println!("All threads finished");
    
    for handle in handles {
        handle.join().unwrap();
    }
}

fn main() {
    coordinator_with_latch();
    println!("\n--- Start Gate Pattern ---\n");
    start_gate_example();
}
```

**Key Differences:**
- **C++**: Built-in `std::latch` in C++20
- **Rust**: No built-in latch; implement using mutex + condvar or use crates like `crossbeam`
- Both are single-use synchronization primitives (unlike barriers which can be reused)

### 12. Scoped Threads and Thread Safety

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <vector>

void process_data(std::vector<int>& data, int start, int end) {
    for (int i = start; i < end; ++i) {
        data[i] *= 2;
    }
}

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8};
    const int num_threads = 4;
    std::vector<std::thread> threads;
    
    int chunk_size = data.size() / num_threads;
    
    for (int i = 0; i < num_threads; ++i) {
        int start = i * chunk_size;
        int end = (i == num_threads - 1) ? data.size() : (i + 1) * chunk_size;
        
        // Reference to data is captured, programmer must ensure safety
        threads.emplace_back(process_data, std::ref(data), start, end);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    for (int val : data) {
        std::cout << val << " ";
    }
    std::cout << "\n";
    return 0;
}
```

**Rust Implementation:**

```rust
use std::thread;

fn main() {
    let mut data = vec![1, 2, 3, 4, 5, 6, 7, 8];
    let num_threads = 4;
    let chunk_size = data.len() / num_threads;
    
    // Scoped threads allow borrowing local data
    thread::scope(|s| {
        let chunks: Vec<&mut [i32]> = data.chunks_mut(chunk_size).collect();
        
        for chunk in chunks {
            s.spawn(move || {
                for item in chunk.iter_mut() {
                    *item *= 2;
                }
            });
        }
        // All threads automatically joined at end of scope
    });
    
    for val in &data {
        print!("{} ", val);
    }
    println!();
}
```

**Key Differences:**
- **C++**: Programmer must manually ensure references remain valid and threads are joined
- **Rust**: Scoped threads (`thread::scope`) guarantee all threads finish before local data goes out of scope, enforced at compile time

### 13. Futures and Promises

Mechanism for passing values between threads asynchronously.

**C++ Implementation:**

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

int expensive_computation(int x) {
    std::cout << "Computing " << x << "...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return x * x;
}

void future_basic_example() {
    // Launch async task
    std::future<int> result = std::async(std::launch::async, 
                                         expensive_computation, 10);
    
    std::cout << "Doing other work while computation runs...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Get result (blocks if not ready)
    int value = result.get();
    std::cout << "Result: " << value << "\n";
}

void promise_example() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread worker([](std::promise<int> p) {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        p.set_value(42); // Set the value
    }, std::move(prom));
    
    std::cout << "Waiting for result...\n";
    int result = fut.get();
    std::cout << "Got: " << result << "\n";
    
    worker.join();
}

// Shared future - multiple threads can wait on same result
void shared_future_example() {
    std::promise<int> prom;
    std::shared_future<int> fut = prom.get_future().share();
    
    auto waiter = [](std::shared_future<int> f, int id) {
        std::cout << "Thread " << id << " waiting...\n";
        int value = f.get();
        std::cout << "Thread " << id << " got: " << value << "\n";
    };
    
    std::thread t1(waiter, fut, 1);
    std::thread t2(waiter, fut, 2);
    std::thread t3(waiter, fut, 3);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    prom.set_value(100); // All threads get the same value
    
    t1.join();
    t2.join();
    t3.join();
}

int main() {
    future_basic_example();
    std::cout << "\n--- Promise Example ---\n";
    promise_example();
    std::cout << "\n--- Shared Future Example ---\n";
    shared_future_example();
    return 0;
}
```

**Rust Implementation:**

```rust
use std::thread;
use std::sync::mpsc;
use std::time::Duration;

fn expensive_computation(x: i32) -> i32 {
    println!("Computing {}...", x);
    thread::sleep(Duration::from_millis(500));
    x * x
}

// Rust doesn't have built-in futures in std for threads
// (async/await is for async runtime, not threads)
// Here's a channel-based approach:

fn future_like_example() {
    let (tx, rx) = mpsc::channel();
    
    // Spawn computation
    thread::spawn(move || {
        let result = expensive_computation(10);
        tx.send(result).unwrap();
    });
    
    println!("Doing other work while computation runs...");
    thread::sleep(Duration::from_millis(100));
    
    // Get result (blocks until ready)
    let value = rx.recv().unwrap();
    println!("Result: {}", value);
}

// Promise-like pattern
fn promise_example() {
    let (tx, rx) = mpsc::channel();
    
    let worker = thread::spawn(move || {
        thread::sleep(Duration::from_millis(300));
        tx.send(42).unwrap(); // "Set" the value
    });
    
    println!("Waiting for result...");
    let result = rx.recv().unwrap();
    println!("Got: {}", result);
    
    worker.join().unwrap();
}

// Shared future - using Arc to share receiver (requires sync_channel)
use std::sync::{Arc, Mutex};

fn shared_future_example() {
    let (tx, rx) = mpsc::channel();
    let rx = Arc::new(Mutex::new(Some(rx)));
    
    let mut handles = vec![];
    
    for id in 1..=3 {
        let rx_clone = Arc::clone(&rx);
        let handle = thread::spawn(move || {
            println!("Thread {} waiting...", id);
            
            // First thread to lock gets the value
            let mut rx_guard = rx_clone.lock().unwrap();
            if let Some(receiver) = rx_guard.take() {
                let value = receiver.recv().unwrap();
                println!("Thread {} got: {}", id, value);
                
                // Share value with others via channel
                // (This is simplified; proper impl would use broadcast)
            }
        });
        handles.push(handle);
    }
    
    thread::sleep(Duration::from_millis(200));
    tx.send(100).unwrap();
    
    for handle in handles {
        handle.join().unwrap();
    }
}

// Better approach: using crossbeam for broadcast
// Or use async Rust with tokio for true future semantics

fn main() {
    future_like_example();
    println!("\n--- Promise Example ---");
    promise_example();
}
```

**Key Differences:**
- **C++**: Built-in `std::future`, `std::promise`, `std::async` for thread-based async operations
- **Rust**: No built-in thread-based futures; use channels or async/await with runtimes like `tokio`
- Rust's async model is fundamentally different and more sophisticated for I/O-bound tasks


## Compile-Time Safety Guarantees

### Data Race Prevention

**C++ - Compiles but has data race:**
```cpp
int counter = 0;

void increment() {
    for (int i = 0; i < 1000; ++i) {
        ++counter;  // Data race! But compiles fine
    }
}
```

**Rust - Won't compile:**
```rust
let mut counter = 0;

thread::spawn(|| {
    for _ in 0..1000 {
        counter += 1;  // Compile error: cannot move `counter` into closure
    }
});
```

Rust forces you to use proper synchronization:
```rust
let counter = Arc::new(Mutex::new(0));
// Now it compiles and is thread-safe
```

## Comprehensive Summary Comparison Table

| Feature | C++ | Rust | Notes |
|---------|-----|------|-------|
| **Mutex** | `std::mutex` with separate data | `Mutex<T>` wraps protected data | Rust enforces lock-before-access |
| **Lock Guards** | `lock_guard`, `unique_lock`, manual | RAII guards (`MutexGuard`), automatic | Both use RAII, Rust stricter |
| **Data Race Prevention** | Runtime detection only (e.g., TSan) | Compile-time prevention via ownership | Rust's killer feature |
| **RW Locks** | `shared_mutex` with `shared_lock`/`unique_lock` | `RwLock<T>` with read/write guards | Similar semantics, different APIs |
| **Condition Variables** | `condition_variable` with manual lock management | `Condvar` with mutex pairs | C++ more flexible, Rust safer |
| **Atomics** | `std::atomic<T>` with memory ordering | `Atomic*` types with `Ordering` enum | Similar power, similar complexity |
| **Channels** | No standard support (third-party like Boost) | Built-in `mpsc` channels | Rust encourages message passing |
| **Barriers** | `std::barrier` (C++20) | `Barrier` in std library | Both allow reusable sync points |
| **Semaphores** | `counting_semaphore`, `binary_semaphore` (C++20) | Custom impl or crates (`tokio`, `async-std`) | C++ added recently, Rust via ecosystem |
| **Once Initialization** | `std::once_flag` + `std::call_once` | `Once`, `OnceLock`, `LazyLock` | Rust has more ergonomic options |
| **Spin Locks** | Manual via `atomic_flag` or libraries | Manual or `spin` crate | Both require careful use |
| **Latches** | `std::latch` (C++20) - single use countdown | Custom or `crossbeam::sync::WaitGroup` | C++ built-in, Rust via crates |
| **Futures/Promises** | `std::future`, `std::promise`, `std::async` | Channels or async/await with `tokio`/`async-std` | Different paradigms entirely |
| **Scoped Threads** | Manual lifetime management (C++20 jthread helps) | `thread::scope` with compile-time guarantees | Rust prevents dangling references |
| **Send/Sync Traits** | No equivalent concept | Compile-time thread safety markers | Unique to Rust's type system |
| **Memory Ordering** | Full control via `memory_order_*` | Full control via `Ordering::*` | Same semantics, equal power |
| **Error Handling** | Exceptions, undefined behavior possible | `Result<T, E>` for explicit errors | Rust forces error handling |
| **Poisoning** | No built-in concept | Mutexes poison on panic | Rust detects thread crashes |
| **Thread Local Storage** | `thread_local` keyword | `thread_local!` macro | Similar functionality |
| **Panic Safety** | No formal concept | Compile-time panic safety checks | Rust prevents broken invariants |
| **Lock-Free Data Structures** | Manual implementation challenging | Manual implementation equally challenging | Both require deep expertise |
| **Learning Curve** | Lower initial, higher for correctness | Steeper initial, enforces correctness | Rust front-loads complexity |
| **Performance** | Zero-overhead abstractions possible | Zero-cost abstractions guaranteed | Comparable in practice |
| **Deadlock Prevention** | Programmer responsibility | Programmer responsibility (but no data races) | Neither solves deadlock |
| **Default Safety** | Unsafe by default, opt-in to safety | Safe by default, `unsafe` explicit | Fundamental philosophy difference |
| **Ecosystem Maturity** | Very mature, decades of libraries | Growing rapidly, modern design | C++ has more legacy code |


## Key Philosophical Differences

1. **Trust Model**: C++ trusts the programmer to use synchronization correctly; Rust assumes programmers make mistakes and prevents them at compile time.

2. **Type System**: Rust's `Send` and `Sync` traits automatically determine if types can be safely transferred or shared between threads. C++ has no equivalent compile-time checking.

3. **Ownership**: Rust's ownership system naturally prevents sharing mutable state without synchronization. C++ requires manual discipline.

4. **Error Discovery**: C++ finds concurrency bugs at runtime (if you're lucky); Rust finds them at compile time.

5. **Abstraction Cost**: Both languages achieve zero-cost abstractions for synchronization primitives, but Rust's compile-time checks add no runtime overhead.

## Conclusion

C++ provides powerful, low-level synchronization primitives with maximum flexibility but minimal compile-time safety. This makes it suitable for systems where performance is critical and the team has deep expertise in concurrent programming.

Rust takes a fundamentally different approach by making thread safety part of the type system. While this creates a steeper learning curve initially, it eliminates entire classes of concurrency bugs at compile time, making it easier to write correct concurrent code. The ownership system and `Send`/`Sync` traits provide "fearless concurrency" where the compiler guides you toward correct patterns.

For new concurrent systems, Rust's approach prevents costly bugs before they reach production. For existing C++ codebases, understanding these differences helps appreciate both the flexibility and the risks inherent in C++'s model.