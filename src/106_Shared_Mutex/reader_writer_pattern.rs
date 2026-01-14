/*
## Key Rust Equivalents

**C++ → Rust Mappings:**
- `std::shared_mutex` → `RwLock` (reader-writer lock)
- `std::shared_lock` → `RwLock::read()` (shared read access)
- `std::unique_lock` → `RwLock::write()` (exclusive write access)
- `std::mutex` → `Mutex` (for stdout protection)
- `std::thread` → `std::thread`
- Raw pointers/references → `Arc` (Atomic Reference Counting for shared ownership)

## Important Rust Differences

1. **Ownership with Arc**: Rust requires explicit shared ownership using `Arc` (like `shared_ptr` in C++), since multiple threads need access to the same cache

2. **No manual locking syntax**: Rust's RAII is even more explicit - the lock guard variable (`cache`) must exist to access the data

3. **Poisoning**: Rust locks can be "poisoned" if a thread panics while holding a lock, which is why we use `.unwrap()` (or could handle errors explicitly)

4. **Move semantics**: Variables are moved into closures with `move`, and we clone `Arc` pointers with `Arc::clone()`

5. **Type safety**: Rust's type system ensures thread safety at compile time - you literally cannot compile code that has data races

The behavior and output will be identical to the C++ version, including the "Not found" messages due to readers racing ahead of the writer!
*/

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