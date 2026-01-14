# Shared Mutex and Reader-Writer Locks in C++

## Introduction

In multithreaded applications, there's a common scenario where data is read frequently but modified infrequently. Traditional mutexes (`std::mutex`) provide exclusive access, meaning only one thread can access the protected resource at a time, even if multiple threads just want to read the data. This can create unnecessary bottlenecks.

**Shared mutexes** solve this problem by implementing a reader-writer lock pattern. They allow multiple threads to hold shared (read) access simultaneously while ensuring exclusive access for writers. This significantly improves performance in read-heavy scenarios.

C++17 introduced `std::shared_mutex` along with `std::shared_lock` to facilitate this pattern.

## Core Concepts

### std::shared_mutex

`std::shared_mutex` supports two types of ownership:

1. **Exclusive ownership** (for writers): Only one thread can hold exclusive ownership at a time
2. **Shared ownership** (for readers): Multiple threads can hold shared ownership simultaneously, but only when no thread holds exclusive ownership

### Lock Types

- **std::unique_lock** or **std::lock_guard**: Used with `lock()` or `try_lock()` for exclusive (write) access
- **std::shared_lock**: Used with `lock_shared()` or `try_lock_shared()` for shared (read) access

## Basic Usage

Here's a simple example demonstrating the reader-writer pattern:

```cpp
/*
 * Reader-Writer Pattern Demo using std::shared_mutex
 * Compile: g++ -pthread --std=c++20 reader_writer_pattern.cpp -o app
 * 
 * Demonstrates:
 * - Multiple concurrent readers with shared_lock
 * - Exclusive writer access with unique_lock
 * - Thread-safe cache implementation
 */

#include <iostream>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>

/**
 * Thread-safe cache using the Reader-Writer pattern
 * 
 * Uses std::shared_mutex to allow:
 * - Multiple simultaneous readers (shared access)
 * - Exclusive writer access (blocks all readers and other writers)
 */
class ThreadSafeCache {
private:
    // Mutable allows locking in const methods (read operations)
    mutable std::shared_mutex mutex_;
    
    // Underlying data structure protected by mutex
    std::unordered_map<std::string, std::string> cache_;

public:
    /**
     * Read operation - allows multiple concurrent readers
     * 
     * Uses shared_lock: multiple threads can hold this lock simultaneously
     * as long as no writer holds unique_lock
     * 
     * @param key The key to look up
     * @return The value if found, otherwise "Not found"
     */
    std::string read(const std::string& key) const {
        // Shared lock - multiple readers can acquire this simultaneously
        std::shared_lock<std::shared_mutex> lock(mutex_);
        
        auto it = cache_.find(key);
        return (it != cache_.end()) ? it->second : "Not found";
        
        // Lock automatically released when 'lock' goes out of scope (RAII)
    }

    /**
     * Write operation - exclusive access required
     * 
     * Uses unique_lock: only one thread can hold this lock
     * Blocks all readers and other writers until complete
     * 
     * @param key The key to insert/update
     * @param value The value to store
     */
    void write(const std::string& key, const std::string& value) {
        // Unique lock - exclusive access, blocks all other threads
        std::unique_lock<std::shared_mutex> lock(mutex_);
        
        cache_[key] = value;
        
        // Lock automatically released when 'lock' goes out of scope (RAII)
    }

    /**
     * Size query - read operation with shared access
     * 
     * @return Current number of entries in cache
     */
    size_t size() const {
        // Shared lock - can be called concurrently with other reads
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return cache_.size();
    }
};

// Separate mutex to protect std::cout from interleaved output
// (std::cout is not thread-safe for concurrent writes)
std::mutex cout_mutex;

int main() {
    ThreadSafeCache cache;
    
    /**
     * Writer thread - populates cache with 5 key-value pairs
     * Writes every 100ms
     */
    std::thread writer([&cache]() {
        for (int i = 0; i < 5; ++i) {
            // Exclusive write - blocks all readers during this operation
            cache.write("key" + std::to_string(i), "value" + std::to_string(i));
            
            // Protect console output to prevent garbled text
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Writer " << i << ": " << i << "\n";
            } // cout_mutex released here
            
            // Sleep to simulate real work and allow readers to interleave
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    /**
     * Reader threads - 3 concurrent readers
     * Each attempts to read keys 0-4 (cycling), every 50ms
     * 
     * Note: Readers run faster (50ms) than writer (100ms),
     * so early reads may find "Not found" for keys not yet written
     */
    std::vector<std::thread> readers;
    for (int i = 0; i < 3; ++i) {
        readers.emplace_back([&cache, i]() {
            for (int j = 0; j < 10; ++j) {
                // Shared read - can run concurrently with other reads
                // but will block if writer holds unique_lock
                std::string value = cache.read("key" + std::to_string(j % 5));
                
                // Protect console output
                {
                    std::lock_guard<std::mutex> lock(cout_mutex);
                    std::cout << "Reader " << i << ": " << value << "\n";
                } // cout_mutex released here
                
                // Sleep shorter than writer, demonstrating race conditions
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
    
    // Wait for writer to complete
    writer.join();
    
    // Wait for all readers to complete
    for (auto& reader : readers) {
        reader.join();
    }
    
    return 0;
}
```

### Rust equivalent code

**C++ → Rust Mappings:**
- `std::shared_mutex` → `RwLock` (reader-writer lock)
- `std::shared_lock` → `RwLock::read()` (shared read access)
- `std::unique_lock` → `RwLock::write()` (exclusive write access)
- `std::mutex` → `Mutex` (for stdout protection)
- `std::thread` → `std::thread`
- Raw pointers/references → `Arc` (Atomic Reference Counting for shared ownership)

#### Important Rust Differences

1. **Ownership with Arc**: Rust requires explicit shared ownership using `Arc` (like `shared_ptr` in C++), since multiple threads need access to the same cache

2. **No manual locking syntax**: Rust's RAII is even more explicit - the lock guard variable (`cache`) must exist to access the data

3. **Poisoning**: Rust locks can be "poisoned" if a thread panics while holding a lock, which is why we use `.unwrap()` (or could handle errors explicitly)

4. **Move semantics**: Variables are moved into closures with `move`, and we clone `Arc` pointers with `Arc::clone()`

5. **Type safety**: Rust's type system ensures thread safety at compile time - you literally cannot compile code that has data races

The behavior and output will be identical to the C++ version, including the "Not found" messages due to readers racing ahead of the writer!

```rust
/*
 * Reader-Writer Pattern Demo using RwLock
 * Compile: rustc reader_writer_pattern.rs
 * Or with Cargo: cargo run
 * 
 * Demonstrates:
 * - Multiple concurrent readers with read()
 * - Exclusive writer access with write()
 * - Thread-safe cache implementation using Arc and RwLock
 */

use std::collections::HashMap;
use std::sync::{Arc, RwLock, Mutex};
use std::thread;
use std::time::Duration;

/**
 * Thread-safe cache using the Reader-Writer pattern
 * 
 * Uses RwLock to allow:
 * - Multiple simultaneous readers (shared access)
 * - Exclusive writer access (blocks all readers and other writers)
 * 
 * Wrapped in Arc for shared ownership across threads
 */
struct ThreadSafeCache {
    // RwLock allows multiple readers or one writer
    cache: RwLock<HashMap<String, String>>,
}

impl ThreadSafeCache {
    /**
     * Create a new empty cache
     */
    fn new() -> Self {
        ThreadSafeCache {
            cache: RwLock::new(HashMap::new()),
        }
    }

    /**
     * Read operation - allows multiple concurrent readers
     * 
     * Uses read(): multiple threads can hold read locks simultaneously
     * as long as no writer holds a write lock
     * 
     * @param key The key to look up
     * @return The value if found, otherwise "Not found"
     */
    fn read(&self, key: &str) -> String {
        // Acquire read lock - multiple readers can hold this simultaneously
        // .unwrap() panics if lock is poisoned (a thread panicked while holding lock)
        let cache = self.cache.read().unwrap();
        
        // Look up the key and return cloned value or "Not found"
        cache.get(key)
            .map(|v| v.clone())
            .unwrap_or_else(|| "Not found".to_string())
        
        // Read lock automatically released when 'cache' goes out of scope (RAII)
    }

    /**
     * Write operation - exclusive access required
     * 
     * Uses write(): only one thread can hold this lock
     * Blocks all readers and other writers until complete
     * 
     * @param key The key to insert/update
     * @param value The value to store
     */
    fn write(&self, key: String, value: String) {
        // Acquire write lock - exclusive access, blocks all other threads
        // .unwrap() panics if lock is poisoned
        let mut cache = self.cache.write().unwrap();
        
        cache.insert(key, value);
        
        // Write lock automatically released when 'cache' goes out of scope (RAII)
    }

    /**
     * Size query - read operation with shared access
     * 
     * @return Current number of entries in cache
     */
    fn size(&self) -> usize {
        // Acquire read lock - can be called concurrently with other reads
        let cache = self.cache.read().unwrap();
        cache.len()
    }
}

fn main() {
    // Arc (Atomic Reference Counting) allows shared ownership across threads
    let cache = Arc::new(ThreadSafeCache::new());
    
    // Mutex to protect stdout from interleaved output
    let cout_mutex = Arc::new(Mutex::new(()));

    /**
     * Writer thread - populates cache with 5 key-value pairs
     * Writes every 100ms
     */
    let cache_writer = Arc::clone(&cache);
    let cout_writer = Arc::clone(&cout_mutex);
    let writer = thread::spawn(move || {
        for i in 0..5 {
            // Exclusive write - blocks all readers during this operation
            cache_writer.write(
                format!("key{}", i),
                format!("value{}", i)
            );
            
            // Protect console output to prevent garbled text
            {
                let _lock = cout_writer.lock().unwrap();
                println!("Writer {}: {}", i, i);
            } // cout_mutex released here
            
            // Sleep to simulate real work and allow readers to interleave
            thread::sleep(Duration::from_millis(100));
        }
    });

    /**
     * Reader threads - 3 concurrent readers
     * Each attempts to read keys 0-4 (cycling), every 50ms
     * 
     * Note: Readers run faster (50ms) than writer (100ms),
     * so early reads may find "Not found" for keys not yet written
     */
    let mut readers = vec![];
    for i in 0..3 {
        let cache_reader = Arc::clone(&cache);
        let cout_reader = Arc::clone(&cout_mutex);
        
        let reader = thread::spawn(move || {
            for j in 0..10 {
                // Shared read - can run concurrently with other reads
                // but will block if writer holds write lock
                let value = cache_reader.read(&format!("key{}", j % 5));
                
                // Protect console output
                {
                    let _lock = cout_reader.lock().unwrap();
                    println!("Reader {}: {}", i, value);
                } // cout_mutex released here
                
                // Sleep shorter than writer, demonstrating race conditions
                thread::sleep(Duration::from_millis(50));
            }
        });
        
        readers.push(reader);
    }

    // Wait for writer to complete
    writer.join().unwrap();
    
    // Wait for all readers to complete
    for reader in readers {
        reader.join().unwrap();
    }
}
```

## Advanced Example: Configuration Manager

Here's a more realistic example of a configuration manager where reads vastly outnumber writes:

```cpp
#include <iostream>
#include <shared_mutex>
#include <unordered_map>
#include <string>
#include <thread>
#include <chrono>
#include <optional>

class ConfigManager {
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> config_;
    std::chrono::system_clock::time_point last_update_;

public:
    ConfigManager() : last_update_(std::chrono::system_clock::now()) {}

    // Read operation - allows concurrent reads
    std::optional<std::string> get(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = config_.find(key);
        if (it != config_.end()) {
            return it->second;
        }
        return std::nullopt;
    }

    // Write operation - exclusive access
    void set(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        config_[key] = value;
        last_update_ = std::chrono::system_clock::now();
        std::cout << "Config updated: " << key << " = " << value << "\n";
    }

    // Batch update - exclusive access
    void bulk_update(const std::unordered_map<std::string, std::string>& updates) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        for (const auto& [key, value] : updates) {
            config_[key] = value;
        }
        last_update_ = std::chrono::system_clock::now();
        std::cout << "Bulk update completed\n";
    }

    // Read-only snapshot
    std::unordered_map<std::string, std::string> get_snapshot() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return config_;  // Returns a copy while holding shared lock
    }

    // Check if config is fresh
    bool is_fresh(std::chrono::seconds threshold) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto now = std::chrono::system_clock::now();
        auto age = std::chrono::duration_cast<std::chrono::seconds>(now - last_update_);
        return age < threshold;
    }
};
```

## Upgrade/Downgrade Patterns

Sometimes you need to upgrade from shared to exclusive access (or downgrade). Standard library doesn't provide atomic upgrade, so you need to release and reacquire:

```cpp
#include <shared_mutex>
#include <unordered_map>
#include <string>

class SmartCache {
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, int> data_;

public:
    // Check if exists (shared), then insert if missing (exclusive)
    int get_or_create(const std::string& key, int default_value) {
        // First, try with shared lock
        {
            std::shared_lock<std::shared_mutex> read_lock(mutex_);
            auto it = data_.find(key);
            if (it != data_.end()) {
                return it->second;  // Found it, return value
            }
        }  // Release shared lock

        // Not found, acquire exclusive lock to insert
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        
        // Double-check (another thread might have inserted while we waited)
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }

        // Still not there, insert it
        data_[key] = default_value;
        return default_value;
    }
};
```

## Performance Comparison

Here's an example demonstrating the performance benefit:

```cpp
#include <iostream>
#include <mutex>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <chrono>

class DataWithMutex {
    mutable std::mutex mutex_;
    int data_ = 0;
public:
    int read() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return data_;
    }
    void write(int val) {
        std::lock_guard<std::mutex> lock(mutex_);
        data_ = val;
    }
};

class DataWithSharedMutex {
    mutable std::shared_mutex mutex_;
    int data_ = 0;
public:
    int read() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return data_;
    }
    void write(int val) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        data_ = val;
    }
};

template<typename T>
void benchmark(T& data, const std::string& name) {
    auto start = std::chrono::high_resolution_clock::now();
    
    std::vector<std::thread> threads;
    
    // 10 reader threads
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&data]() {
            for (int j = 0; j < 100000; ++j) {
                volatile int val = data.read();
                (void)val;
            }
        });
    }
    
    // 1 writer thread
    threads.emplace_back([&data]() {
        for (int j = 0; j < 10000; ++j) {
            data.write(j);
        }
    });
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << name << " took: " << duration.count() << "ms\n";
}

int main() {
    DataWithMutex data1;
    DataWithSharedMutex data2;
    
    benchmark(data1, "std::mutex");
    benchmark(data2, "std::shared_mutex");
    
    return 0;
}
```

## Try-Lock Variants

Both shared and exclusive locks support non-blocking variants:

```cpp
#include <shared_mutex>
#include <iostream>

class Resource {
    mutable std::shared_mutex mutex_;
    std::string data_;

public:
    bool try_read(std::string& out) const {
        if (mutex_.try_lock_shared()) {
            std::shared_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
            out = data_;
            return true;
        }
        return false;
    }

    bool try_write(const std::string& value) {
        if (mutex_.try_lock()) {
            std::unique_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
            data_ = value;
            return true;
        }
        return false;
    }
};
```

## Timed Locks

You can also use timed variants for timeout scenarios:

```cpp
#include <shared_mutex>
#include <chrono>
#include <string>

class TimedResource {
    mutable std::shared_mutex mutex_;
    std::string data_;

public:
    bool timed_read(std::string& out, std::chrono::milliseconds timeout) const {
        if (mutex_.try_lock_shared_for(timeout)) {
            std::shared_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
            out = data_;
            return true;
        }
        return false;
    }

    bool timed_write(const std::string& value, std::chrono::milliseconds timeout) {
        if (mutex_.try_lock_for(timeout)) {
            std::unique_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
            data_ = value;
            return true;
        }
        return false;
    }
};
```

## Common Pitfalls

### 1. Writer Starvation

With many readers, writers might wait indefinitely. Some implementations provide priority to writers, but this isn't guaranteed by the standard:

```cpp
// Be aware: continuous readers might starve writers
void potential_starvation() {
    std::shared_mutex mutex;
    
    // Many readers continuously acquiring shared locks
    // might prevent a writer from ever acquiring exclusive lock
}
```

### 2. Deadlock with Multiple Shared Mutexes

Lock multiple shared mutexes in consistent order:

```cpp
class Account {
    mutable std::shared_mutex mutex_;
    double balance_;
public:
    // Always lock in order of memory address to prevent deadlock
    static void transfer(Account& from, Account& to, double amount) {
        std::shared_mutex& first = (&from < &to) ? from.mutex_ : to.mutex_;
        std::shared_mutex& second = (&from < &to) ? to.mutex_ : from.mutex_;
        
        std::unique_lock<std::shared_mutex> lock1(first);
        std::unique_lock<std::shared_mutex> lock2(second);
        
        from.balance_ -= amount;
        to.balance_ += amount;
    }
};
```

### 3. Forgetting const for Read Operations

Read methods should be `const` to allow shared locking on const objects:

```cpp
class Data {
    mutable std::shared_mutex mutex_;  // Note: mutable
    int value_;
public:
    int read() const {  // const allows this to work on const Data&
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return value_;
    }
};
```

## When to Use Shared Mutexes

**Use shared_mutex when:**
- Reads greatly outnumber writes (typically 10:1 ratio or higher)
- Read operations are relatively quick
- You need maximum concurrency for read-heavy workloads

**Use regular mutex when:**
- Writes are frequent
- The protected section is very short
- Simplicity is more important than maximum performance
- You're working with C++14 or earlier (use `std::shared_timed_mutex` in C++14)

## Summary

`std::shared_mutex` and `std::shared_lock` provide an elegant solution for the reader-writer problem in C++. By allowing multiple concurrent readers while ensuring exclusive access for writers, they significantly improve performance in read-heavy scenarios.

**Key points:**
- Use `std::shared_lock` for read operations (allows multiple simultaneous readers)
- Use `std::unique_lock` or `std::lock_guard` for write operations (exclusive access)
- Mark read methods as `const` and the mutex as `mutable`
- Be aware of potential writer starvation in read-heavy workloads
- Consider the overhead: shared mutexes have slightly more overhead than regular mutexes, so they're only beneficial when reads significantly outnumber writes
- `std::shared_mutex` was introduced in C++17; use `std::shared_timed_mutex` for C++14

The reader-writer pattern with shared mutexes is a powerful tool for optimizing multithreaded applications where data is frequently read but infrequently modified, such as caches, configuration managers, and lookup tables.