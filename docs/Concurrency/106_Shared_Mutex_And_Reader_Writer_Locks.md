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
    mutable std::shared_mutex mutex_;   // mutable: allows locking in const methods without
                                        // breaking const correctness (locking is an
                                        // implementation detail, not a logical state change)
    std::string data_;

public:
    bool try_read(std::string& out) const {
        if (mutex_.try_lock_shared()) {     // non-blocking attempt to acquire shared lock
                                            // returns false immediately if a writer holds the lock
                                            // multiple readers can hold shared lock simultaneously
            std::shared_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
                                            // adopt_lock: mutex already locked above, don't lock again
                                            // RAII: lock destructor will release the shared lock
                                            //       automatically when scope exits
            out = data_;                    // safe to read: shared lock allows concurrent readers
            return true;                    // caller knows read succeeded
        }
        return false;                       // lock was busy (writer active), caller decides what to do
    }

    bool try_write(const std::string& value) {
        if (mutex_.try_lock()) {            // non-blocking attempt to acquire exclusive lock
                                            // returns false immediately if any reader or writer holds the lock
                                            // only one writer can hold exclusive lock at a time
            std::unique_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
                                            // adopt_lock: mutex already locked above, don't lock again
                                            // RAII: lock destructor will release the exclusive lock
                                            //       automatically when scope exits
            data_ = value;                  // safe to write: exclusive lock, no other readers or writers
            return true;                    // caller knows write succeeded
        }
        return false;                       // lock was busy (readers or writer active), caller decides what to do
    }
};

int main() {
    Resource res;
    std::string out;

    if (!res.try_write("hello")) {          // attempt write, don't block if busy
        std::cout << "Write failed, resource busy\n";
    }

    if (!res.try_read(out)) {              // attempt read, don't block if busy
        std::cout << "Read failed, resource busy\n";
    } else {
        std::cout << "Read: " << out << '\n';
    }
}
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

```
   shared_mutex
                        ┌─────────────┐
  ┌──────────┐   shared │  shared (3) │
  │ Reader 1 │──────────┤             ├──────────────────────┐
  └──────────┘          │             │                      │
                        │             │             ┌────────┴───────┐
  ┌──────────┐   shared │             │             │ Protected data │
  │ Reader 2 │──────────┤             │             └────────────────┘
  └──────────┘          │             │         
                        │             │                 (blocked)
  ┌──────────┐   shared │             │             ┌──────────────┐
  │ Reader 3 │──────────┤             │             │    Writer    │
  └──────────┘          └─────────────┘             └──────────────┘
                                               
                read-only access ×3 (concurrent)


  shared_mutex
                        ┌─────────────┐
  ┌──────────┐          │  exclusive  │  unique  ┌──────────┐
  │ Reader 1 │ (blocked)│             │◄─────────│  Writer  │
  └──────────┘          │             │          └──────────┘
                        │             │
  ┌──────────┐          │             │
  │ Reader 2 │ (blocked)│             ├──────────────────────┐
  └──────────┘          │             │                      │
                        │             │             ┌────────┴───────┐
  ┌──────────┐          │             │             │ Protected data │
  │ Reader 3 │ (blocked)│             │             └────────────────┘
  └──────────┘          └─────────────┘

                    exclusive write access ×1
```

**Read phase** (`std::shared_lock`) — multiple threads acquire the mutex concurrently. The mutex enters *shared* mode; any number of readers can hold it simultaneously. The writer attempts to acquire but blocks until all shared locks are released.

**Write phase** (`std::unique_lock`) — the writer acquires the mutex exclusively. The mutex enters *exclusive* mode; no reader can acquire a shared lock until the writer releases it.

A minimal C++ pattern looks like this:

```cpp
#include <shared_mutex>

std::shared_mutex mtx;
SomeData data;

// Readers — concurrent, non-blocking between themselves
void read() {
    std::shared_lock lock(mtx);   // shared ownership
    use(data);
}                                  // lock released here

// Writer — exclusive, blocks all readers
void write() {
    std::unique_lock lock(mtx);   // exclusive ownership
    modify(data);
}                                  // lock released here
```

A few things worth keeping in mind: `std::shared_mutex` is available since C++17. Writer starvation is a real concern — if readers continuously hold the lock, the writer may wait indefinitely. Some implementations give priority to pending writers once one is queued, but this is not guaranteed by the standard.


> `std::shared_mutex` maintains three distinct internal states:

**1. Unlocked**
- No thread holds any lock. 
- Both `shared_lock` and `unique_lock` acquisitions will succeed immediately.

**2. Shared (shared-locked)**
- One or more threads hold a `shared_lock`. 
- The mutex tracks an internal reader count (an integer). 
- Each new `shared_lock` increments it; each release decrements it. 
- A `unique_lock` attempt will block until the count drops to zero. 
- New `shared_lock` attempts continue to be granted freely.

**3. Exclusive (unique-locked)**
- Exactly one thread holds a `unique_lock`. 
- The reader count is zero and must stay zero for the duration. 
- All incoming `shared_lock` and `unique_lock` attempts block until the exclusive holder releases.

The typical internal representation looks roughly like this:

```cpp
// Conceptual internals — not actual stdlib code
struct shared_mutex_state {
    int        shared_count = 0;        // active reader count
    bool       exclusive    = false;    // writer holds the lock
    std::mutex internal_mtx;            // guards state transitions
    std::condition_variable no_readers; // writer waits here
    std::condition_variable no_writer;  // readers wait here
};
```

**State transition table**

```
  Current state     Request          Result
  ─────────────────────────────────────────────────────────────
  Unlocked          shared_lock      → Shared     (count = 1)
  Unlocked          unique_lock      → Exclusive
  Shared            shared_lock      → Shared     (count + 1)
  Shared            unique_lock      → BLOCKS until count == 0
  Exclusive         shared_lock      → BLOCKS
  Exclusive         unique_lock      → BLOCKS
  Exclusive         release          → Unlocked
  Shared            last release     → Unlocked   (count → 0)
```

**One important nuance — writer starvation**

The standard does not define fairness policy. A naive implementation that keeps granting incoming `shared_lock` requests while any readers are active can starve a waiting writer indefinitely. Most real implementations (libstdc++, libc++, MSVC) solve this by setting a *write-pending* flag the moment a `unique_lock` is requested, which causes all subsequent `shared_lock` attempts to block and queue behind the writer:

```
  Unlocked → [write pending] → readers drain → Exclusive → Unlocked
                  ↑
          new shared_lock requests block here
```

**Upgrade is not supported**

`std::shared_mutex` has no lock upgrade operation. A thread holding a `shared_lock` cannot promote it to a `unique_lock` in one atomic step — it must release the shared lock first, then acquire the unique lock, which opens a window where another writer could intervene. If you need atomic upgrade, you need a separate primitive (Boost's `upgrade_mutex`, or a manual condvar-based design).


## How does `std::shared_lock` work in C++?

`std::shared_lock` is a RAII wrapper that acquires a *shared* (read) lock on a `shared_mutex` at construction and releases it at destruction — the same relationship that `std::unique_lock` has with a regular `std::mutex`.

**Basic usage**

```cpp
#include <shared_mutex>

std::shared_mutex mtx;
SomeData data;

void read_data() {
    std::shared_lock lock(mtx);   // acquires shared lock
    use(data);                     // safe: multiple threads can be here
}                                  // lock released automatically (RAII)
```

**What happens internally at construction**

```
shared_lock(mtx)
    │
    ├─ if mutex state == Unlocked   → transition to Shared, count = 1 ✓
    ├─ if mutex state == Shared     → increment count ✓
    └─ if mutex state == Exclusive  → BLOCK until writer releases
```

The call ultimately reaches `mtx.lock_shared()`, which is the primitive the mutex exposes for shared acquisition.

**The class interface**

```cpp
// All the forms shared_lock supports
std::shared_lock lock(mtx);                        // lock immediately
std::shared_lock lock(mtx, std::defer_lock);       // don't lock yet
std::shared_lock lock(mtx, std::try_to_lock);      // try, don't block
std::shared_lock lock(mtx, std::adopt_lock);       // already locked, just own it
std::shared_lock lock(mtx, timeout);               // timed variants

lock.lock();          // manually acquire (if deferred)
lock.unlock();        // manually release early
lock.try_lock();      // non-blocking attempt → bool
lock.try_lock_for(duration);
lock.try_lock_until(time_point);

lock.release();       // detach from mutex without unlocking
lock.mutex();         // access underlying mutex pointer
lock.owns_lock();     // bool — do we currently hold the lock?
```

**Deferred locking pattern**

Useful when you want to set up multiple locks before acquiring them, or conditionally lock:

```cpp
std::shared_lock lock(mtx, std::defer_lock);  // no lock yet

if (some_condition) {
    lock.lock();     // acquire when ready
    read(data);
}                    // releases only if locked (owns_lock() check)
```

**Timed locking**

```cpp
std::shared_lock lock(mtx, std::defer_lock);

if (lock.try_lock_for(std::chrono::milliseconds(50))) {
    read(data);      // got the lock within 50ms
} else {
    // timed out — take fallback path
}
```

**Move semantics**

`shared_lock` is move-only (not copyable), which makes ownership transfer explicit and safe:

```cpp
std::shared_lock acquire_read_lock() {
    return std::shared_lock(mtx);   // transfers ownership to caller
}

void reader() {
    auto lock = acquire_read_lock();
    read(data);
}   // lock destructs here, releases shared ownership
```

**What it does NOT do**

```cpp
// 1. Cannot be used with plain std::mutex — only shared_mutex types
std::mutex m;
std::shared_lock lock(m);   // ✗ compile error

// 2. Cannot be upgraded to exclusive in-place
std::shared_lock slock(mtx);
// ... want to write now ...
slock.upgrade();            // ✗ doesn't exist
// Must do:
slock.unlock();
std::unique_lock ulock(mtx);   // window of vulnerability here

// 3. Not recursive
void foo() {
    std::shared_lock a(mtx);
    std::shared_lock b(mtx);   // ✗ undefined behaviour on most impls
}
```

**Relation to the other lock wrappers**

```
  Wrapper            Acquires        Use with           Copyable   Movable
  ──────────────────────────────────────────────────────────────────────────
  lock_guard         exclusive       mutex              no         no
  unique_lock        exclusive       mutex              no         yes
  shared_lock        shared          shared_mutex       no         yes
  scoped_lock        exclusive (N)   N mutexes          no         no
```

**Availability**

`std::shared_lock` and `std::shared_mutex` are both C++17. If you're on C++14 you can use `std::shared_timed_mutex` with a backported shared lock, or reach for `boost::shared_mutex`.


## How does `std::unique_lock` work with `std::shared_mutex` in C++?

`std::unique_lock` on a `std::shared_mutex` acquires *exclusive* ownership — the write side of the readers-writer pattern. It blocks until all current `shared_lock` holders have released, then locks out all new acquisitions (both shared and exclusive) for the duration.

**Basic usage**

```cpp
#include <shared_mutex>

std::shared_mutex mtx;
SomeData data;

void write_data(SomeData new_val) {
    std::unique_lock lock(mtx);    // acquires exclusive lock
    data = new_val;                // safe: no other thread can read or write
}                                  // lock released automatically (RAII)
```

**What happens internally at construction**

```
unique_lock(mtx)
    │
    ├─ if mutex state == Unlocked   → transition to Exclusive ✓
    ├─ if mutex state == Shared     → BLOCK until shared_count == 0
    └─ if mutex state == Exclusive  → BLOCK until other writer releases
```

Underneath it calls `mtx.lock()` — the exclusive primitive — not `lock_shared()`.

**The class interface**

```cpp
std::unique_lock lock(mtx);                      // lock immediately
std::unique_lock lock(mtx, std::defer_lock);     // don't lock yet
std::unique_lock lock(mtx, std::try_to_lock);    // try, don't block
std::unique_lock lock(mtx, std::adopt_lock);     // already locked externally

lock.lock();
lock.unlock();
lock.try_lock();
lock.try_lock_for(duration);
lock.try_lock_until(time_point);

lock.release();       // detach without unlocking
lock.mutex();         // raw pointer to underlying mutex
lock.owns_lock();     // bool
```

This interface is identical to `unique_lock<mutex>` — the mutex type is a template parameter, so swapping `mutex` for `shared_mutex` costs nothing at the call site.


**Common patterns with shared_mutex**

*Conditional write — check then update:*

```cpp
// ============================================================
// PATTERN: Conditional write with shared_mutex (check-then-update)
//
// shared_mutex supports two locking modes:
//   - shared_lock  (shared / read  lock): multiple readers can hold it simultaneously
//   - unique_lock  (exclusive / write lock): only one writer, blocks all readers too
//
// The challenge: you often need to READ a condition first, then WRITE only if needed.
// Naively taking two separate locks creates a race window between them.
// ============================================================


// ----------------------------------------------------------------
// ❌ WRONG: unsynchronised gap between the read-check and the write
// ----------------------------------------------------------------
{
    std::shared_lock rl(mtx);       // acquire shared lock — other readers can still run
                                    // concurrently, but no writer holds the mutex yet

    if (data.needs_update()) {      // read the condition while safely protected

        rl.unlock();                // 💥 shared lock released — mutex is now UNLOCKED.
                                    //    Any number of other threads can rush in here:
                                    //    a concurrent writer might complete its own update,
                                    //    leaving data.needs_update() == false by the time
                                    //    we take the write lock below. We'll update anyway,
                                    //    causing a redundant or even corrupted write.

        std::unique_lock wl(mtx);   // acquire exclusive lock — but the window above means
                                    // the state we checked is already stale

        data.update();              // ⚠️  written on a potentially outdated assumption
    }
    // shared_lock destructor runs here; rl is already unlocked so this is a no-op
}


// ----------------------------------------------------------------
// ✅ CORRECT: optimistic read, then re-validate after taking write lock
// ----------------------------------------------------------------
{
    std::shared_lock rl(mtx);       // acquire shared lock for the cheap read-only check.
                                    // Using shared here allows maximum read concurrency —
                                    // we only pay for exclusive access if a write is truly needed.

    if (!data.needs_update()) {
        return;                     // fast path: no write needed, exit while still holding
                                    // the shared lock (released automatically by destructor).
                                    // This is the common case, so we optimise for it.
    }

    rl.unlock();                    // on this path the data needs update, so we have to release the shared_lock
                                    // ⚠️  race window opens here, just as in the wrong version.
                                    //     Another thread *could* sneak in and perform the update
                                    //     before we re-acquire below. That's acceptable — the
                                    //     re-check below is precisely what makes this safe.

    std::unique_lock wl(mtx);       // acquire exclusive lock — all readers and writers are
                                    // now blocked. We own the mutex completely.

    if (data.needs_update()) {      // 🔑 RE-CHECK (a.k.a. "double-checked locking"):
                                    //     the state may have changed during the window above.
                                    //     Only proceed if the condition is still true now that
                                    //     we hold the exclusive lock and have a consistent view.
        data.update();              // safe write: we verified the need under exclusive lock,
                                    // so no other thread can observe or modify data concurrently
    }
    // unique_lock destructor releases the exclusive lock here
}
```

*Deferred lock — set up before acquiring:*

```cpp
std::unique_lock lock(mtx, std::defer_lock);

// ... do some preparation work without holding the lock ...

lock.lock();       // acquire when actually needed
data = new_val;
lock.unlock();     // release early if more non-critical work follows
```

*Try-lock — non-blocking write attempt:*

```cpp
std::unique_lock lock(mtx, std::try_to_lock);

if (lock.owns_lock()) {
    data = new_val;         // got exclusive access immediately
} else {
    queue_for_later();      // couldn't acquire — take alternate path
}
```

*Timed lock:*

```cpp
std::unique_lock lock(mtx, std::defer_lock);

if (lock.try_lock_for(std::chrono::milliseconds(100))) {
    data = new_val;
} else {
    // write timed out
}
```

**Using with condition variables**

`std::condition_variable_any` works with `unique_lock<shared_mutex>`, which lets you build producer-consumer patterns on top of a shared mutex:

```cpp
std::shared_mutex mtx;
std::condition_variable_any cv;
std::queue<Work> work_queue;

// Producer
void produce(Work w) {
    std::unique_lock lock(mtx);
    work_queue.push(w);
    cv.notify_one();
}

// Consumer
void consume() {
    std::unique_lock lock(mtx);
    cv.wait(lock, [] { return !work_queue.empty(); });
    auto w = work_queue.front();
    work_queue.pop();
    lock.unlock();
    process(w);
}
```

Note: `std::condition_variable` (without `_any`) only works with `unique_lock<std::mutex>` — the plain mutex type. You need `condition_variable_any` for `shared_mutex`.

**Key differences from `unique_lock<mutex>`**

```
  Property                  unique_lock<mutex>      unique_lock<shared_mutex>
  ───────────────────────────────────────────────────────────────────────────
  What it blocks            other unique_locks       shared_locks AND
                                                     unique_locks
  Coexists with             nothing                  nothing
  Underlying primitive      mtx.lock()               mtx.lock()  (same call)
  Condition variable        condition_variable        condition_variable_any
  Overhead                  lower                    slightly higher
                                                     (shared state tracking)
```

**What it does NOT do**

```cpp
// 1. Does not downgrade to a shared_lock in-place
std::unique_lock wl(mtx);
// done writing, want to keep reading...
wl.downgrade();            // ✗ doesn't exist
// Must fully release then re-acquire shared:
wl.unlock();
std::shared_lock rl(mtx);  // another writer could enter in this gap

// 2. Not recursive
std::unique_lock a(mtx);
std::unique_lock b(mtx);   // ✗ deadlock — same thread, same mutex

// 3. Cannot mix lock types on the same mutex in one thread
std::shared_lock rl(mtx);
std::unique_lock wl(mtx);  // ✗ deadlock — waiting for itself
```

**The full lock hierarchy on `shared_mutex`**

```
  std::shared_mutex
  │
  ├── shared_lock<shared_mutex>   →  mtx.lock_shared()    (many allowed)
  │
  └── unique_lock<shared_mutex>   →  mtx.lock()           (one, exclusive)
        │
        └── also usable as:
              scoped_lock<shared_mutex>                    (no manual unlock)
              lock_guard<shared_mutex>                     (minimal RAII, no flexibility)
```

For most write paths, `unique_lock` is the right tool — `lock_guard` works too but gives you no flexibility to unlock early or use condition variables, and `scoped_lock` is preferred when you need to lock multiple mutexes simultaneously without deadlock risk.


## Summary

```
  Mutex type                    shared_lock    unique_lock    unique_lock timed ops
  ──────────────────────────────────────────────────────────────────────────────────
  std::shared_mutex             ✓              ✓              ✗
  std::shared_timed_mutex       ✓              ✓              ✓
  std::mutex                    ✗              ✓              ✗
  std::timed_mutex              ✗              ✓              ✓
  std::recursive_mutex          ✗              ✓              ✗
  std::recursive_timed_mutex    ✗              ✓              ✓
```

And the concept requirements:

```
  Concept            Requires                              Satisfied by
  ──────────────────────────────────────────────────────────────────────────────
  Lockable           lock / unlock / try_lock              all six
  TimedLockable      + try_lock_for / try_lock_until       timed_mutex
                                                           recursive_timed_mutex
                                                           shared_timed_mutex
  SharedLockable     + lock_shared / unlock_shared         shared_mutex
                       / try_lock_shared                   shared_timed_mutex
```