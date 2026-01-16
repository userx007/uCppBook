### Read-Write Locks

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