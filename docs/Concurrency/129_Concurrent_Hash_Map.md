# Concurrent Hash Map

## Overview

A concurrent hash map is a thread-safe data structure that allows multiple threads to simultaneously read from and write to a hash map without data corruption or race conditions. Unlike simply wrapping a standard hash map with a single global lock (which would serialize all access and eliminate concurrency benefits), a well-designed concurrent hash map uses **fine-grained locking** strategies to allow multiple operations to proceed in parallel when they don't conflict.

## Key Concepts

### Why Fine-Grained Locking?

**Coarse-grained locking** (one lock for entire map):
- Simple to implement
- Severely limits concurrency - only one thread can access the map at a time
- Performance bottleneck under high contention

**Fine-grained locking** (multiple locks for different sections):
- More complex to implement
- Allows multiple threads to access different parts of the map simultaneously
- Better scalability and throughput

### Design Strategies

1. **Bucket-level locking**: Each bucket (or group of buckets) has its own lock
2. **Striped locking**: A fixed number of locks protect different regions of buckets
3. **Lock-free techniques**: Using atomic operations and compare-and-swap (advanced)

## Implementation Example

Here's a concurrent hash map implementation using bucket-level locking with separate chaining:

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <mutex>
#include <shared_mutex>
#include <functional>
#include <optional>
#include <memory>

template<typename Key, typename Value, typename Hash = std::hash<Key>>
class ConcurrentHashMap {
private:
    struct Entry {
        Key key;
        Value value;
        
        Entry(const Key& k, const Value& v) : key(k), value(v) {}
    };
    
    struct Bucket {
        std::list<Entry> entries;
        mutable std::shared_mutex mutex;  // Allows multiple readers or single writer
    };
    
    std::vector<Bucket> buckets;
    Hash hash_function;
    
    size_t get_bucket_index(const Key& key) const {
        return hash_function(key) % buckets.size();
    }
    
    // Find entry in bucket (assumes bucket is locked)
    typename std::list<Entry>::iterator find_entry(
        std::list<Entry>& entries, const Key& key) {
        return std::find_if(entries.begin(), entries.end(),
            [&key](const Entry& e) { return e.key == key; });
    }

public:
    explicit ConcurrentHashMap(size_t bucket_count = 19)
        : buckets(bucket_count) {}
    
    // Insert or update a key-value pair
    void insert(const Key& key, const Value& value) {
        size_t bucket_idx = get_bucket_index(key);
        Bucket& bucket = buckets[bucket_idx];
        
        std::unique_lock<std::shared_mutex> lock(bucket.mutex);
        
        auto it = find_entry(bucket.entries, key);
        if (it != bucket.entries.end()) {
            it->value = value;  // Update existing
        } else {
            bucket.entries.emplace_back(key, value);  // Insert new
        }
    }
    
    // Retrieve a value by key
    std::optional<Value> get(const Key& key) const {
        size_t bucket_idx = get_bucket_index(key);
        const Bucket& bucket = buckets[bucket_idx];
        
        std::shared_lock<std::shared_mutex> lock(bucket.mutex);
        
        auto it = std::find_if(bucket.entries.begin(), bucket.entries.end(),
            [&key](const Entry& e) { return e.key == key; });
        
        if (it != bucket.entries.end()) {
            return it->value;
        }
        return std::nullopt;
    }
    
    // Remove a key-value pair
    bool erase(const Key& key) {
        size_t bucket_idx = get_bucket_index(key);
        Bucket& bucket = buckets[bucket_idx];
        
        std::unique_lock<std::shared_mutex> lock(bucket.mutex);
        
        auto it = find_entry(bucket.entries, key);
        if (it != bucket.entries.end()) {
            bucket.entries.erase(it);
            return true;
        }
        return false;
    }
    
    // Check if key exists
    bool contains(const Key& key) const {
        size_t bucket_idx = get_bucket_index(key);
        const Bucket& bucket = buckets[bucket_idx];
        
        std::shared_lock<std::shared_mutex> lock(bucket.mutex);
        
        auto it = std::find_if(bucket.entries.begin(), bucket.entries.end(),
            [&key](const Entry& e) { return e.key == key; });
        
        return it != bucket.entries.end();
    }
    
    // Get approximate size (may be inaccurate during concurrent modifications)
    size_t size() const {
        size_t total = 0;
        for (const auto& bucket : buckets) {
            std::shared_lock<std::shared_mutex> lock(bucket.mutex);
            total += bucket.entries.size();
        }
        return total;
    }
};
```

## Usage Example

Here's how to use the concurrent hash map with multiple threads:

```cpp
#include <thread>
#include <chrono>
#include <random>

void test_concurrent_hash_map() {
    ConcurrentHashMap<int, std::string> map(50);  // 50 buckets
    
    // Writer threads
    auto writer = [&map](int thread_id, int start, int end) {
        for (int i = start; i < end; ++i) {
            map.insert(i, "Value_" + std::to_string(thread_id) + "_" + std::to_string(i));
            std::this_thread::sleep_for(std::chrono::microseconds(10));
        }
    };
    
    // Reader threads
    auto reader = [&map](int start, int end) {
        int successful_reads = 0;
        for (int i = start; i < end; ++i) {
            auto value = map.get(i);
            if (value.has_value()) {
                successful_reads++;
            }
            std::this_thread::sleep_for(std::chrono::microseconds(5));
        }
        std::cout << "Reader found " << successful_reads << " entries\n";
    };
    
    // Launch multiple threads
    std::vector<std::thread> threads;
    
    // 3 writer threads
    threads.emplace_back(writer, 1, 0, 100);
    threads.emplace_back(writer, 2, 100, 200);
    threads.emplace_back(writer, 3, 200, 300);
    
    // 2 reader threads
    threads.emplace_back(reader, 0, 300);
    threads.emplace_back(reader, 50, 250);
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final map size: " << map.size() << std::endl;
}

int main() {
    test_concurrent_hash_map();
    return 0;
}
```

## Advanced: Striped Locking Implementation

For very large hash maps, having one lock per bucket may consume too much memory. Striped locking uses a fixed number of locks:

```cpp
template<typename Key, typename Value, typename Hash = std::hash<Key>>
class StripedHashMap {
private:
    static constexpr size_t NUM_STRIPES = 16;
    
    struct Entry {
        Key key;
        Value value;
        Entry(const Key& k, const Value& v) : key(k), value(v) {}
    };
    
    std::vector<std::list<Entry>> buckets;
    std::array<std::shared_mutex, NUM_STRIPES> stripe_mutexes;
    Hash hash_function;
    
    size_t get_bucket_index(const Key& key) const {
        return hash_function(key) % buckets.size();
    }
    
    size_t get_stripe_index(const Key& key) const {
        return hash_function(key) % NUM_STRIPES;
    }

public:
    explicit StripedHashMap(size_t bucket_count = 100)
        : buckets(bucket_count) {}
    
    void insert(const Key& key, const Value& value) {
        size_t bucket_idx = get_bucket_index(key);
        size_t stripe_idx = get_stripe_index(key);
        
        std::unique_lock<std::shared_mutex> lock(stripe_mutexes[stripe_idx]);
        
        auto& entries = buckets[bucket_idx];
        auto it = std::find_if(entries.begin(), entries.end(),
            [&key](const Entry& e) { return e.key == key; });
        
        if (it != entries.end()) {
            it->value = value;
        } else {
            entries.emplace_back(key, value);
        }
    }
    
    std::optional<Value> get(const Key& key) const {
        size_t bucket_idx = get_bucket_index(key);
        size_t stripe_idx = get_stripe_index(key);
        
        std::shared_lock<std::shared_mutex> lock(stripe_mutexes[stripe_idx]);
        
        const auto& entries = buckets[bucket_idx];
        auto it = std::find_if(entries.begin(), entries.end(),
            [&key](const Entry& e) { return e.key == key; });
        
        if (it != entries.end()) {
            return it->value;
        }
        return std::nullopt;
    }
};
```

## Performance Considerations

### Choosing Bucket Count

```cpp
// Rule of thumb: bucket_count should be a prime number
// roughly equal to expected number of elements
ConcurrentHashMap<int, std::string> small_map(19);    // ~20 elements
ConcurrentHashMap<int, std::string> medium_map(97);   // ~100 elements
ConcurrentHashMap<int, std::string> large_map(1009);  // ~1000 elements
```

### Lock Granularity Trade-offs

- **More buckets/locks**: Better concurrency, higher memory overhead
- **Fewer buckets/locks**: Lower memory overhead, more contention
- **Optimal**: Balance based on expected workload and contention patterns

### Read vs Write Operations

Using `std::shared_mutex` allows:
- Multiple readers simultaneously (shared locks)
- Exclusive writer access (unique locks)
- Particularly beneficial for read-heavy workloads

## Summary

A concurrent hash map with fine-grained locking provides thread-safe access to hash map operations while maintaining good performance under concurrent access. The key insights are:

1. **Bucket-level locking** divides the map into independently lockable sections, allowing non-conflicting operations to proceed in parallel

2. **Shared mutexes** (`std::shared_mutex`) enable multiple concurrent readers while ensuring exclusive writer access, optimizing for read-heavy workloads

3. **Striped locking** offers a middle ground between full per-bucket locking and coarse-grained locking, using a fixed number of locks to protect multiple buckets

4. **Design trade-offs** include memory overhead (more locks), contention patterns (fewer locks), and implementation complexity versus performance gains

5. **Practical considerations** involve choosing appropriate bucket counts (typically prime numbers), handling hash collisions through separate chaining, and ensuring proper lock acquisition ordering to prevent deadlocks

This approach scales significantly better than a single-lock solution and is foundational to building high-performance concurrent data structures in multi-threaded applications.