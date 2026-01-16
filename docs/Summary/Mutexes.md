### Mutexes (Mutual Exclusion Locks)

**C++ Implementation:**

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mtx;
int shared_counter = 0;  // WARNING: Not encapsulated - can be accessed without locking

void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // RAII-style locking - auto-unlocks when scope ends
        std::lock_guard<std::mutex> lock(mtx);  

        ++shared_counter;
    }  // lock is released here automatically
}

int main() {
    std::vector<std::thread> threads;
    
    // Spawn 5 threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(increment_counter, 1000);
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter: " << shared_counter << std::endl;  // Expected: 5000
    return 0;
}
```
- The mutex and data are separate entities. 
- Nothing prevents accessing `shared_counter` without locking `mtx`.


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


`Mutex<T>` wraps the data itself. 
The only way to access the data is through the lock, enforced at compile time.
