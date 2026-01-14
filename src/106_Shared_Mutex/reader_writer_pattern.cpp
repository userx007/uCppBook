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