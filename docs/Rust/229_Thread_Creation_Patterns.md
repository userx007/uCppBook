# Thread Creation Patterns: C++ and Rust

## C++ Thread Pattern with `emplace_back`

Using `threads.emplace_back()` is a standard and idiomatic pattern in C++ for managing multiple threads. This pattern involves storing thread objects in a `std::vector<std::thread>` container.

**Why this pattern exists:**

The fundamental reason this pattern emerged is that `std::thread` objects are **non-copyable** - they can only be moved. When you create a thread, you get a unique handle to that running thread, and C++ enforces that only one `std::thread` object can own that handle at a time. This design prevents accidental duplication of thread handles, which could lead to undefined behavior.

**Why `emplace_back` specifically:**

`emplace_back()` constructs the thread object directly in the vector's memory location, avoiding any unnecessary move operations. It forwards its arguments to the `std::thread` constructor, creating the thread in-place. While `push_back()` with move semantics also works in modern C++, `emplace_back()` is slightly more efficient and has become the conventional choice.

Here's the typical pattern:

```cpp
std::vector<std::thread> threads;

// Launch multiple threads
for (int i = 0; i < num_threads; ++i) {
    threads.emplace_back(worker_function, arg1, arg2);
}

// Wait for all threads to complete
for (auto& t : threads) {
    t.join();
}
```

**Advantages over alternatives:**

Compared to managing threads manually with raw pointers or individual named variables, this vector-based approach offers several benefits: automatic lifetime management through RAII (the vector destructor will terminate the program if threads aren't joined), easy scalability for dynamic thread counts, straightforward iteration for joining or detaching threads, and cleaner code organization.

The alternative of creating individually named thread variables (`thread1`, `thread2`, etc.) becomes unwieldy quickly and doesn't scale. Using raw pointers or manual memory management adds unnecessary complexity and error potential.

## Rust's Idiomatic Approach

Rust takes a fundamentally different approach to thread management that emphasizes safety and ownership from the ground up.

**The standard pattern:**

Rust typically stores thread handles in a `Vec<JoinHandle<T>>`, where `T` is the return type of the thread. The idiomatic pattern looks like this:

```rust
use std::thread;

let handles: Vec<_> = (0..num_threads)
    .map(|i| {
        thread::spawn(move || {
            // Thread work here
            worker_function(i)
        })
    })
    .collect();

// Wait for all threads and collect results
let results: Vec<_> = handles
    .into_iter()
    .map(|h| h.join().unwrap())
    .collect();
```

**Key differences from C++:**

Rust's approach is more functional and composable. The `spawn` function returns a `JoinHandle<T>` that represents both the running thread and its eventual return value. Rust's ownership system ensures memory safety across thread boundaries through the `Send` and `Sync` traits, which are checked at compile time.

The `move` keyword in the closure explicitly captures variables by moving them into the thread's scope, preventing data races through Rust's ownership rules rather than through runtime checks. This means many threading bugs that are possible in C++ simply won't compile in Rust.

**Alternative Rust patterns:**

For more complex scenarios, Rust developers often use thread pools from the `rayon` crate for parallel iteration, or scoped threads for borrowing data without moving it. Scoped threads are particularly elegant:

```rust
use std::thread;

let data = vec![1, 2, 3, 4];

thread::scope(|s| {
    for item in &data {
        s.spawn(move || {
            process(*item);
        });
    }
    // All threads automatically joined at scope end
});
```

The scoped thread pattern allows threads to borrow data from the parent scope safely because the compiler guarantees all spawned threads complete before the scope exits.

**Why Rust's approach is considered better for safety:**

Rust prevents data races at compile time, makes thread return values explicit through the type system, and eliminates entire classes of bugs like use-after-free or iterator invalidation across threads. The trade-off is a steeper learning curve, but the result is concurrent code that's both safe and performant without runtime overhead.

Both languages provide effective threading primitives, but C++ requires more programmer discipline to avoid undefined behavior, while Rust enforces safety through its type system and ownership model.

## Complete, runnable examples that demonstrate the thread patterns in both languages.

### C++ example

```cpp
/*
g++ -pthread --std=c++14 thread_creation.cpp -o app
*/

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <mutex>

std::mutex mtx;

// Simple worker function that takes parameters
void worker(int id, const std::string& message) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Thread " << id << ": " << message << std::endl;
}

// Worker function that returns a value (simulated with shared state)
int calculate_sum(int start, int end) {
    int sum = 0;
    for (int i = start; i <= end; ++i) {
        sum += i;
    }
    return sum;
}

int main() {
    std::cout << "=== Basic Pattern with emplace_back ===" << std::endl;
    
    // Pattern 1: Simple thread creation and joining
    {
        std::vector<std::thread> threads;
        const int num_threads = 5;
        
        // Launch threads using emplace_back
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker, i, "Hello from thread");
        }
        
        // Join all threads
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "\n=== Pattern with Return Values (using promises) ===" << std::endl;
    
    // Pattern 2: Getting return values using promise/future
    {
        std::vector<std::thread> threads;
        std::vector<std::promise<int>> promises(3);
        std::vector<std::future<int>> futures;
        
        // Create futures from promises
        for (auto& p : promises) {
            futures.push_back(p.get_future());
        }
        
        // Launch threads with lambda to capture promise
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([](std::promise<int> p, int start, int end) {
                p.set_value(calculate_sum(start, end));
            }, std::move(promises[i]), i * 10, (i + 1) * 10);
        }
        
        // Get results
        for (int i = 0; i < 3; ++i) {
            std::cout << "Sum " << i << ": " << futures[i].get() << std::endl;
        }
        
        // Join threads
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "\n=== Pattern with Lambda Capture ===" << std::endl;
    
    // Pattern 3: Using lambdas with capture
    {
        std::vector<std::thread> threads;
        std::vector<int> data = {10, 20, 30, 40, 50};
        
        for (size_t i = 0; i < data.size(); ++i) {
            threads.emplace_back([i, val = data[i]]() {
                std::cout << "Processing index " << i 
                          << " with value " << val * 2 << std::endl;
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    return 0;
}
```

### Rust example

```rust
use std::thread;
use std::time::Duration;

// Simple worker function
fn worker(id: usize, message: &str) {
    thread::sleep(Duration::from_millis(100));
    println!("Thread {}: {}", id, message);
}

// Worker function that returns a value
fn calculate_sum(start: i32, end: i32) -> i32 {
    (start..=end).sum()
}

fn main() {
    println!("=== Basic Pattern with JoinHandle ===");
    
    // Pattern 1: Simple thread creation and joining
    {
        let handles: Vec<_> = (0..5)
            .map(|i| {
                thread::spawn(move || {
                    worker(i, "Hello from thread");
                })
            })
            .collect();
        
        // Join all threads
        for handle in handles {
            handle.join().unwrap();
        }
    }
    
    println!("\n=== Pattern with Return Values ===");
    
    // Pattern 2: Getting return values (built into JoinHandle)
    {
        let handles: Vec<_> = (0..3)
            .map(|i| {
                thread::spawn(move || {
                    calculate_sum(i * 10, (i + 1) * 10)
                })
            })
            .collect();
        
        // Collect results
        let results: Vec<i32> = handles
            .into_iter()
            .map(|h| h.join().unwrap())
            .collect();
        
        for (i, sum) in results.iter().enumerate() {
            println!("Sum {}: {}", i, sum);
        }
    }
    
    println!("\n=== Pattern with Move Closure ===");
    
    // Pattern 3: Using move to capture variables
    {
        let data = vec![10, 20, 30, 40, 50];
        
        let handles: Vec<_> = data
            .into_iter()
            .enumerate()
            .map(|(i, val)| {
                thread::spawn(move || {
                    println!("Processing index {} with value {}", i, val * 2);
                })
            })
            .collect();
        
        for handle in handles {
            handle.join().unwrap();
        }
    }
    
    println!("\n=== Scoped Threads (Borrowing Data) ===");
    
    // Pattern 4: Scoped threads for borrowing without moving
    {
        let data = vec![1, 2, 3, 4, 5];
        
        thread::scope(|s| {
            for (i, item) in data.iter().enumerate() {
                s.spawn(move || {
                    println!("Thread {} processing: {}", i, item * 2);
                });
            }
            // All threads automatically joined at scope end
        });
        
        // data is still available here!
        println!("Original data still accessible: {:?}", data);
    }
    
    println!("\n=== Shared State with Mutex ===");
    
    // Pattern 5: Sharing mutable state safely
    {
        use std::sync::{Arc, Mutex};
        
        let counter = Arc::new(Mutex::new(0));
        let handles: Vec<_> = (0..5)
            .map(|_| {
                let counter = Arc::clone(&counter);
                thread::spawn(move || {
                    let mut num = counter.lock().unwrap();
                    *num += 1;
                })
            })
            .collect();
        
        for handle in handles {
            handle.join().unwrap();
        }
        
        println!("Final counter value: {}", *counter.lock().unwrap());
    }
}
```

## Key Differences Highlighted

**C++ Example** shows:
- The `emplace_back` pattern with `std::vector<std::thread>`
- Using `promise/future` for return values (more verbose)
- Lambda captures for thread-local data
- Manual join management required

**Rust Example** shows:
- The `Vec<JoinHandle<T>>` pattern with built-in return values
- More concise functional style with `map` and `collect`
- The `move` keyword for explicit ownership transfer
- **Scoped threads** - a unique Rust feature that allows borrowing data safely
- Shared mutable state with `Arc<Mutex<T>>` (compile-time safety)

## Why Rust's Approach is More Ergonomic

Notice how Rust's `JoinHandle` naturally returns values through its type parameter `T`, while C++ requires the promise/future machinery. Rust's scoped threads (Pattern 4) demonstrate something impossible in standard C++ - safely borrowing data across threads without moving ownership.

Both examples are complete and ready to compile. The C++ version needs C++11 or later, and the Rust version works with stable Rust.