# Sequential Locks (SeqLock)

## Overview

Sequential locks (SeqLocks) are a synchronization mechanism optimized for scenarios where reads vastly outnumber writes. They provide a way for readers to access shared data without blocking writers, and without readers blocking each other. The key innovation is that readers operate optimistically—they assume the data won't change during their read operation and verify this assumption afterward.

SeqLocks are particularly effective when:
- Read operations are much more frequent than writes
- The protected data is small enough to be copied quickly
- Occasional read retries are acceptable
- Writers can tolerate brief blocking periods

The mechanism uses a sequence counter that writers increment before and after modifying data. Readers check this counter before and after reading to detect concurrent writes.

## How SeqLocks Work

The core principle behind SeqLocks is simple:

1. **Writers** increment a sequence counter before writing, perform their write operation, then increment the counter again
2. **Readers** note the sequence counter value, read the data, then check if the counter has changed
3. If the counter changed or was odd during the read, a writer was active, so the reader retries

The sequence counter being odd indicates a write is in progress. An even counter with the same value before and after reading indicates a successful read.

## Basic Implementation

Here's a foundational SeqLock implementation:

```cpp
#include <atomic>
#include <thread>
#include <iostream>
#include <vector>

template <typename T>
class SeqLock {
private:
    std::atomic<size_t> sequence_{0};
    T data_;
    mutable std::mutex write_mutex_;  // Protects writers from each other

public:
    SeqLock() = default;
    
    explicit SeqLock(const T& initial_value) : data_(initial_value) {}
    
    // Write operation - blocks other writers but not readers
    void write(const T& value) {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        // Increment sequence (making it odd) to signal write start
        sequence_.fetch_add(1, std::memory_order_release);
        
        // Perform the write
        data_ = value;
        
        // Increment sequence again (making it even) to signal write completion
        sequence_.fetch_add(1, std::memory_order_release);
    }
    
    // Read operation - never blocks
    T read() const {
        T result;
        size_t seq1, seq2;
        
        do {
            // Read sequence number (must be even for valid read)
            seq1 = sequence_.load(std::memory_order_acquire);
            
            // Wait if a write is in progress (odd sequence number)
            while (seq1 & 1) {
                std::this_thread::yield();
                seq1 = sequence_.load(std::memory_order_acquire);
            }
            
            // Read the data
            result = data_;
            
            // Ensure data read completes before reading seq2
            std::atomic_thread_fence(std::memory_order_acquire);
            
            // Read sequence again to check if write occurred during read
            seq2 = sequence_.load(std::memory_order_acquire);
            
            // Retry if sequence changed (write occurred during read)
        } while (seq1 != seq2);
        
        return result;
    }
};
```

## Practical Example: Statistics Tracking

SeqLocks excel at tracking frequently-read statistics that are occasionally updated:

```cpp
#include <atomic>
#include <mutex>
#include <thread>
#include <chrono>
#include <iomanip>

struct NetworkStats {
    uint64_t bytes_sent = 0;
    uint64_t bytes_received = 0;
    uint64_t packets_sent = 0;
    uint64_t packets_received = 0;
    uint64_t errors = 0;
};

class NetworkStatsTracker {
private:
    std::atomic<size_t> sequence_{0};
    NetworkStats stats_;
    mutable std::mutex write_mutex_;

public:
    // Update statistics (called by network thread)
    void update(uint64_t sent_bytes, uint64_t recv_bytes, 
                uint64_t sent_packets, uint64_t recv_packets) {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        sequence_.fetch_add(1, std::memory_order_release);
        
        stats_.bytes_sent += sent_bytes;
        stats_.bytes_received += recv_bytes;
        stats_.packets_sent += sent_packets;
        stats_.packets_received += recv_packets;
        
        sequence_.fetch_add(1, std::memory_order_release);
    }
    
    void record_error() {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        sequence_.fetch_add(1, std::memory_order_release);
        stats_.errors++;
        sequence_.fetch_add(1, std::memory_order_release);
    }
    
    // Read statistics (called by monitoring/UI threads frequently)
    NetworkStats read() const {
        NetworkStats result;
        size_t seq1, seq2;
        
        do {
            seq1 = sequence_.load(std::memory_order_acquire);
            
            while (seq1 & 1) {
                std::this_thread::yield();
                seq1 = sequence_.load(std::memory_order_acquire);
            }
            
            result = stats_;
            
            std::atomic_thread_fence(std::memory_order_acquire);
            seq2 = sequence_.load(std::memory_order_acquire);
            
        } while (seq1 != seq2);
        
        return result;
    }
};

// Demonstration
void network_simulator(NetworkStatsTracker& tracker) {
    for (int i = 0; i < 1000; ++i) {
        tracker.update(1024, 2048, 10, 15);
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

void stats_monitor(NetworkStatsTracker& tracker) {
    for (int i = 0; i < 100; ++i) {
        auto stats = tracker.read();  // Non-blocking read
        std::cout << "Bytes sent: " << stats.bytes_sent 
                  << ", Bytes received: " << stats.bytes_received << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

## Advanced Example: Lock-Free Position Tracking

Here's a more sophisticated example for tracking 3D positions in a game or simulation:

```cpp
#include <atomic>
#include <cmath>
#include <array>

struct Position3D {
    double x, y, z;
    double velocity_x, velocity_y, velocity_z;
    uint64_t timestamp;
    
    Position3D() : x(0), y(0), z(0), 
                   velocity_x(0), velocity_y(0), velocity_z(0),
                   timestamp(0) {}
};

class EntityTracker {
private:
    std::atomic<size_t> sequence_{0};
    Position3D position_;
    mutable std::mutex write_mutex_;

public:
    // Physics thread updates position frequently
    void update_position(double x, double y, double z,
                        double vx, double vy, double vz,
                        uint64_t time) {
        std::lock_guard<std::mutex> lock(write_mutex_);
        
        sequence_.fetch_add(1, std::memory_order_release);
        
        position_.x = x;
        position_.y = y;
        position_.z = z;
        position_.velocity_x = vx;
        position_.velocity_y = vy;
        position_.velocity_z = vz;
        position_.timestamp = time;
        
        sequence_.fetch_add(1, std::memory_order_release);
    }
    
    // Rendering/AI threads read position very frequently
    Position3D get_position() const {
        Position3D result;
        size_t seq1, seq2;
        int retry_count = 0;
        
        do {
            seq1 = sequence_.load(std::memory_order_acquire);
            
            while (seq1 & 1) {
                std::this_thread::yield();
                seq1 = sequence_.load(std::memory_order_acquire);
            }
            
            result = position_;
            
            std::atomic_thread_fence(std::memory_order_acquire);
            seq2 = sequence_.load(std::memory_order_acquire);
            
            if (seq1 != seq2) {
                retry_count++;
                if (retry_count > 10) {
                    // Excessive contention - brief pause
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
            
        } while (seq1 != seq2);
        
        return result;
    }
    
    // Helper to calculate distance without blocking
    double distance_to(double target_x, double target_y, double target_z) const {
        auto pos = get_position();
        double dx = pos.x - target_x;
        double dy = pos.y - target_y;
        double dz = pos.z - target_z;
        return std::sqrt(dx*dx + dy*dy + dz*dz);
    }
};
```

## Performance Comparison

Here's a benchmark comparing SeqLock with traditional mutex-based protection:

```cpp
#include <mutex>
#include <shared_mutex>
#include <chrono>
#include <vector>
#include <numeric>

// Traditional mutex approach
class MutexProtected {
private:
    mutable std::shared_mutex mutex_;
    int data_;
    
public:
    void write(int value) {
        std::unique_lock lock(mutex_);
        data_ = value;
    }
    
    int read() const {
        std::shared_lock lock(mutex_);
        return data_;
    }
};

void benchmark_comparison() {
    const int NUM_READERS = 8;
    const int NUM_OPERATIONS = 1000000;
    
    // Benchmark SeqLock
    {
        SeqLock<int> seqlock(0);
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> readers;
        for (int i = 0; i < NUM_READERS; ++i) {
            readers.emplace_back([&seqlock]() {
                for (int j = 0; j < NUM_OPERATIONS; ++j) {
                    volatile int val = seqlock.read();
                }
            });
        }
        
        std::thread writer([&seqlock]() {
            for (int j = 0; j < NUM_OPERATIONS / 100; ++j) {
                seqlock.write(j);
            }
        });
        
        for (auto& t : readers) t.join();
        writer.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "SeqLock: " << duration.count() << " ms\n";
    }
    
    // Benchmark shared_mutex
    {
        MutexProtected mutex_data;
        auto start = std::chrono::high_resolution_clock::now();
        
        std::vector<std::thread> readers;
        for (int i = 0; i < NUM_READERS; ++i) {
            readers.emplace_back([&mutex_data]() {
                for (int j = 0; j < NUM_OPERATIONS; ++j) {
                    volatile int val = mutex_data.read();
                }
            });
        }
        
        std::thread writer([&mutex_data]() {
            for (int j = 0; j < NUM_OPERATIONS / 100; ++j) {
                mutex_data.write(j);
            }
        });
        
        for (auto& t : readers) t.join();
        writer.join();
        
        auto end = std::chrono::high_resolution_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
        std::cout << "Shared Mutex: " << duration.count() << " ms\n";
    }
}
```

## Summary

**Sequential Locks (SeqLocks)** provide an optimistic concurrency control mechanism that excels when reads significantly outnumber writes. The approach allows readers to proceed without acquiring locks, checking for concurrent modifications via a sequence counter.

**Key advantages:**
- Readers never block writers or other readers
- Extremely fast read operations in the common case (no write contention)
- No reader-writer priority inversion
- Predictable worst-case behavior (readers retry on conflict)

**Limitations:**
- Only suitable for data that can be quickly copied
- Writers block each other (though not readers)
- Readers may need to retry, causing variable latency
- Not ideal when write frequency approaches read frequency

**Best use cases:**
- System statistics and metrics collection
- Configuration data that's read frequently but updated rarely
- Game entity states read by rendering/AI but updated by physics
- Caching frequently-accessed computed values

SeqLocks represent an important tool in the concurrent programming toolkit, offering a pragmatic solution for read-dominated workloads where traditional locking would create unacceptable overhead.