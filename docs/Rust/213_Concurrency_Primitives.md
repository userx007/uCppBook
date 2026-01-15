# Concurrency Primitives: C++ vs Rust

## Overview

Concurrency is a critical aspect of modern programming, enabling programs to perform multiple tasks simultaneously. Both C++ and Rust provide robust concurrency primitives, but they differ fundamentally in their approach to thread safety. C++ offers flexibility with manual synchronization mechanisms, while Rust enforces thread safety at compile time through its type system and ownership model.

## C++ Concurrency Primitives

C++ introduced comprehensive threading support in C++11, providing a standard library for concurrent programming. The model is based on shared memory with explicit synchronization.

### std::thread

The `std::thread` class represents a single thread of execution. Threads begin execution immediately upon construction.

```cpp
#include <iostream>
#include <thread>
#include <vector>

void print_numbers(int start, int end) {
    for (int i = start; i < end; ++i) {
        std::cout << "Thread " << std::this_thread::get_id() 
                  << ": " << i << std::endl;
    }
}

int main() {
    std::vector<std::thread> threads;
    
    // Spawn multiple threads
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(print_numbers, i * 10, (i + 1) * 10);
    }
    
    // Join all threads
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### std::mutex

Mutexes (mutual exclusion objects) protect shared data from concurrent access. A thread must lock a mutex before accessing shared data.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
int shared_counter = 0;

void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(mtx);  // RAII-style lock
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
    
    std::cout << "Final counter value: " << shared_counter << std::endl;
    // Output: Final counter value: 5000
    
    return 0;
}
```

### std::condition_variable

Condition variables enable threads to wait for specific conditions and be notified when those conditions are met, facilitating coordination between threads.

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

void producer(int items) {
    for (int i = 0; i < items; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        std::lock_guard<std::mutex> lock(mtx);
        data_queue.push(i);
        std::cout << "Produced: " << i << std::endl;
        cv.notify_one();  // Notify waiting consumer
    }
    
    std::lock_guard<std::mutex> lock(mtx);
    finished = true;
    cv.notify_all();
}

void consumer() {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, []{ return !data_queue.empty() || finished; });
        
        if (!data_queue.empty()) {
            int value = data_queue.front();
            data_queue.pop();
            lock.unlock();
            std::cout << "Consumed: " << value << std::endl;
        } else if (finished) {
            break;
        }
    }
}

int main() {
    std::thread prod(producer, 10);
    std::thread cons(consumer);
    
    prod.join();
    cons.join();
    
    return 0;
}
```

## Rust Concurrency Primitives

Rust's concurrency model is built on the principle of "fearless concurrency," where the compiler prevents data races at compile time using the ownership system and type-system enforced thread safety.

### thread::spawn

Rust's `thread::spawn` creates a new thread and returns a `JoinHandle` for synchronization.

```rust
use std::thread;
use std::time::Duration;

fn print_numbers(start: i32, end: i32) {
    for i in start..end {
        println!("Thread {:?}: {}", thread::current().id(), i);
        thread::sleep(Duration::from_millis(10));
    }
}

fn main() {
    let mut handles = vec![];
    
    // Spawn multiple threads
    for i in 0..3 {
        let handle = thread::spawn(move || {
            print_numbers(i * 10, (i + 1) * 10);
        });
        handles.push(handle);
    }
    
    // Join all threads
    for handle in handles {
        handle.join().unwrap();
    }
}
```

### Mutex<T> and Arc

Rust's `Mutex<T>` wraps data of type `T`, ensuring exclusive access. `Arc` (Atomic Reference Counted) enables safe sharing of ownership across threads.

```rust
use std::sync::{Arc, Mutex};
use std::thread;

fn main() {
    // Arc allows multiple ownership, Mutex provides interior mutability
    let shared_counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];
    
    for _ in 0..5 {
        let counter = Arc::clone(&shared_counter);
        let handle = thread::spawn(move || {
            for _ in 0..1000 {
                let mut num = counter.lock().unwrap();
                *num += 1;
            }  // Lock automatically released when `num` goes out of scope
        });
        handles.push(handle);
    }
    
    for handle in handles {
        handle.join().unwrap();
    }
    
    println!("Final counter value: {}", *shared_counter.lock().unwrap());
    // Output: Final counter value: 5000
}
```

### Send and Sync Traits

Rust's type system uses marker traits to enforce thread safety at compile time:

- **Send**: Types that can be transferred across thread boundaries
- **Sync**: Types that can be referenced from multiple threads simultaneously

```rust
use std::sync::Arc;
use std::thread;
use std::rc::Rc;

fn main() {
    // Arc<T> is Send + Sync (works with threads)
    let data = Arc::new(vec![1, 2, 3, 4, 5]);
    let data_clone = Arc::clone(&data);
    
    let handle = thread::spawn(move || {
        println!("Thread data: {:?}", data_clone);
    });
    
    handle.join().unwrap();
    
    // Rc<T> is NOT Send (won't compile if moved to thread)
    // let rc_data = Rc::new(vec![1, 2, 3]);
    // thread::spawn(move || {
    //     println!("{:?}", rc_data);  // Compile error!
    // });
}
```

### Channel Communication

Rust provides message-passing concurrency through channels, following the philosophy "Do not communicate by sharing memory; instead, share memory by communicating."

```rust
use std::sync::mpsc;
use std::thread;
use std::time::Duration;

fn main() {
    let (tx, rx) = mpsc::channel();
    
    // Producer thread
    let producer = thread::spawn(move || {
        for i in 0..10 {
            thread::sleep(Duration::from_millis(100));
            tx.send(i).unwrap();
            println!("Produced: {}", i);
        }
    });
    
    // Consumer (main thread)
    for received in rx {
        println!("Consumed: {}", received);
    }
    
    producer.join().unwrap();
}
```

## Type-System Enforced Thread Safety

Rust's most significant advantage in concurrency is compile-time verification of thread safety. The borrow checker prevents data races by ensuring:

1. **No mutable aliasing**: Cannot have mutable references to data accessible from multiple threads
2. **Ownership transfer**: Moving data to a thread transfers ownership
3. **Lifetime tracking**: References cannot outlive the data they point to

```rust
use std::thread;

fn main() {
    let mut data = vec![1, 2, 3];
    
    // This would NOT compile - data race detected at compile time
    /*
    thread::spawn(|| {
        data.push(4);  // Error: cannot capture mutable reference
    });
    
    data.push(5);  // Error: data used after move
    */
    
    // Correct approach: move ownership
    let handle = thread::spawn(move || {
        data.push(4);
        println!("Thread data: {:?}", data);
    });
    
    handle.join().unwrap();
    // data is no longer accessible here (ownership moved)
}
```

In C++, similar code would compile but cause undefined behavior:

```cpp
#include <thread>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {1, 2, 3};
    
    // Compiles but causes data race (undefined behavior)
    std::thread t([&data]() {
        data.push_back(4);  // Race condition!
    });
    
    data.push_back(5);  // Race condition!
    
    t.join();
    
    // Undefined behavior - may crash or produce wrong results
    return 0;
}
```

## Summary Comparison Table

| Feature | C++ | Rust | Key Difference |
|---------|-----|------|----------------|
| **Thread Creation** | `std::thread` with function/lambda | `thread::spawn` with closure | Rust requires `move` for capturing, enforcing ownership transfer |
| **Mutex** | `std::mutex` with separate data | `Mutex<T>` wraps data | Rust's mutex owns the data it protects, preventing unsynchronized access |
| **Lock Management** | `lock_guard`/`unique_lock` (RAII) | Automatic via `MutexGuard` | Both use RAII, but Rust enforces it at type level |
| **Shared Ownership** | `std::shared_ptr` (not thread-safe by default) | `Arc<T>` (atomic, always thread-safe) | Rust's `Arc` is explicitly for threading; C++ requires `atomic` operations |
| **Condition Variables** | `std::condition_variable` | `Condvar` | Similar functionality, both require mutex |
| **Message Passing** | No standard channels (use libraries) | Built-in `mpsc::channel` | Rust emphasizes message passing as primary pattern |
| **Thread Safety** | Runtime checks, undefined behavior on errors | Compile-time checks via Send/Sync | **Rust prevents data races at compile time** |
| **Data Race Prevention** | Developer responsibility, tools help | **Compiler guarantees** (via borrow checker) | Rust's killer feature: impossible to compile code with data races |
| **Performance Overhead** | Minimal when used correctly | Minimal, zero-cost abstractions | Comparable performance, Rust adds no runtime overhead for safety |
| **Learning Curve** | Moderate, easy to misuse | Steep initially, but prevents common errors | Rust's compiler teaches safe concurrency patterns |
| **Debugging** | Race conditions appear at runtime | Most concurrency bugs caught at compile time | Significantly fewer debugging hours in Rust |
| **Flexibility** | High, allows unsafe patterns | Restricted to safe patterns (unless `unsafe` used) | C++ trusts programmer, Rust requires proof of safety |

## Key Takeaways

**C++ Concurrency** provides powerful primitives with maximum flexibility but places the burden of correctness on the developer. Data races, deadlocks, and other concurrency bugs manifest as runtime undefined behavior that can be difficult to reproduce and debug.

**Rust Concurrency** leverages the type system and ownership model to make data races impossible to compile. While this creates a steeper learning curve initially, it eliminates entire categories of bugs and makes concurrent code more maintainable. The Send and Sync traits provide compiler-verified thread safety guarantees that C++ cannot match without extensive testing and code review.

The fundamental philosophical difference is that C++ assumes developers will use concurrency primitives correctly, while Rust's compiler enforces correct usage. This makes Rust particularly valuable for large codebases, concurrent systems programming, and situations where correctness is critical.