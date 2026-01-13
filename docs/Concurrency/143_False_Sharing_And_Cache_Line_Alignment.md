# False Sharing and Cache Line Alignment in C++

## Overview

False sharing is a performance problem that occurs in multi-threaded programs when multiple threads access different variables that happen to reside on the same cache line. Even though the threads are accessing logically independent data, the cache coherence protocol treats the entire cache line as a single unit, causing unnecessary synchronization overhead and severe performance degradation.

## Understanding Cache Lines

Modern CPUs use cache lines as the fundamental unit of cache coherence. A cache line is typically 64 bytes on x86/x64 architectures. When one CPU core modifies data in its cache, the entire cache line containing that data must be invalidated in other cores' caches, even if those cores are working with different variables on the same line.

## The False Sharing Problem

False sharing occurs when:
1. Two or more threads frequently modify different variables
2. These variables are located on the same cache line
3. The cache coherence protocol causes constant cache line bouncing between CPU cores
4. Performance degrades dramatically despite no actual data sharing

## Code Examples

### Example 1: Demonstrating False Sharing

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

// BAD: False sharing - counters share cache lines
struct CounterWithFalseSharing {
    alignas(64) std::atomic<long> counter1{0};
    std::atomic<long> counter2{0};  // Likely on same cache line as counter1
    std::atomic<long> counter3{0};
    std::atomic<long> counter4{0};
};

// GOOD: Properly aligned to prevent false sharing
struct CounterWithoutFalseSharing {
    alignas(64) std::atomic<long> counter1{0};
    alignas(64) std::atomic<long> counter2{0};  // Each on its own cache line
    alignas(64) std::atomic<long> counter3{0};
    alignas(64) std::atomic<long> counter4{0};
};

void incrementCounter(std::atomic<long>& counter, long iterations) {
    for (long i = 0; i < iterations; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

template<typename CounterType>
void benchmarkCounters(const std::string& name, long iterations) {
    CounterType counters;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1(incrementCounter, std::ref(counters.counter1), iterations);
    std::thread t2(incrementCounter, std::ref(counters.counter2), iterations);
    std::thread t3(incrementCounter, std::ref(counters.counter3), iterations);
    std::thread t4(incrementCounter, std::ref(counters.counter4), iterations);
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << name << " took: " << duration.count() << "ms\n";
    std::cout << "  Counter values: " << counters.counter1 << ", " 
              << counters.counter2 << ", " << counters.counter3 << ", " 
              << counters.counter4 << "\n";
}

int main() {
    const long iterations = 10'000'000;
    
    std::cout << "Cache line size: " << std::hardware_destructive_interference_size 
              << " bytes\n\n";
    
    benchmarkCounters<CounterWithFalseSharing>("WITH false sharing", iterations);
    benchmarkCounters<CounterWithoutFalseSharing>("WITHOUT false sharing", iterations);
    
    return 0;
}
```

### Example 2: Thread-Local Data Array

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

// BAD: Array elements will cause false sharing
struct DataWithFalseSharing {
    long data[8] = {0};  // Elements likely share cache lines
};

// GOOD: Each element aligned to cache line
struct DataWithoutFalseSharing {
    struct alignas(64) Element {
        long value = 0;
    };
    Element data[8];
};

template<typename DataType>
void workerThread(DataType& sharedData, int index, long iterations) {
    if constexpr (std::is_same_v<DataType, DataWithFalseSharing>) {
        for (long i = 0; i < iterations; ++i) {
            sharedData.data[index]++;
        }
    } else {
        for (long i = 0; i < iterations; ++i) {
            sharedData.data[index].value++;
        }
    }
}

template<typename DataType>
void benchmarkArray(const std::string& name, int numThreads, long iterations) {
    DataType sharedData;
    std::vector<std::thread> threads;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < numThreads; ++i) {
        threads.emplace_back(workerThread<DataType>, 
                           std::ref(sharedData), i, iterations);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << name << " took: " << duration.count() << "ms\n";
}

int main() {
    const int numThreads = 4;
    const long iterations = 50'000'000;
    
    benchmarkArray<DataWithFalseSharing>("Array WITH false sharing", 
                                        numThreads, iterations);
    benchmarkArray<DataWithoutFalseSharing>("Array WITHOUT false sharing", 
                                           numThreads, iterations);
    
    return 0;
}
```

### Example 3: Padding Techniques

```cpp
#include <iostream>
#include <thread>
#include <atomic>
#include <array>

// C++17 provides constants for cache line sizes
constexpr size_t CACHE_LINE_SIZE = 
    std::hardware_destructive_interference_size; // Typically 64

// Technique 1: Manual padding
struct PaddedCounter1 {
    std::atomic<long> counter{0};
    char padding[CACHE_LINE_SIZE - sizeof(std::atomic<long>)];
};

// Technique 2: Using alignas
struct alignas(CACHE_LINE_SIZE) PaddedCounter2 {
    std::atomic<long> counter{0};
};

// Technique 3: Template-based padding helper
template<typename T>
struct CacheLineAligned {
    alignas(CACHE_LINE_SIZE) T value;
};

// Example usage in a thread pool scenario
class ThreadPool {
private:
    struct alignas(std::hardware_destructive_interference_size) WorkerStats {
        std::atomic<size_t> tasksCompleted{0};
        std::atomic<size_t> tasksInProgress{0};
        // Each worker's stats on separate cache line
    };
    
    std::array<WorkerStats, 16> workerStats;
    
public:
    void recordTaskStart(size_t workerId) {
        workerStats[workerId].tasksInProgress.fetch_add(1, 
            std::memory_order_relaxed);
    }
    
    void recordTaskComplete(size_t workerId) {
        workerStats[workerId].tasksInProgress.fetch_sub(1, 
            std::memory_order_relaxed);
        workerStats[workerId].tasksCompleted.fetch_add(1, 
            std::memory_order_relaxed);
    }
    
    void printStats() const {
        for (size_t i = 0; i < workerStats.size(); ++i) {
            std::cout << "Worker " << i << ": " 
                      << workerStats[i].tasksCompleted.load() 
                      << " tasks completed\n";
        }
    }
};

int main() {
    std::cout << "Destructive interference size: " 
              << std::hardware_destructive_interference_size << "\n";
    std::cout << "Constructive interference size: " 
              << std::hardware_constructive_interference_size << "\n";
    
    std::cout << "\nSizes with padding:\n";
    std::cout << "PaddedCounter1: " << sizeof(PaddedCounter1) << " bytes\n";
    std::cout << "PaddedCounter2: " << sizeof(PaddedCounter2) << " bytes\n";
    std::cout << "CacheLineAligned<long>: " 
              << sizeof(CacheLineAligned<long>) << " bytes\n";
    std::cout << "ThreadPool::WorkerStats: " 
              << sizeof(ThreadPool) / 16 << " bytes\n";
    
    return 0;
}
```

### Example 4: Real-World Lock-Free Queue

```cpp
#include <iostream>
#include <atomic>
#include <array>
#include <thread>
#include <optional>

template<typename T, size_t Size>
class LockFreeQueue {
private:
    struct Node {
        std::atomic<T> data;
        std::atomic<bool> ready{false};
    };
    
    // Separate head and tail to different cache lines to prevent false sharing
    alignas(std::hardware_destructive_interference_size) 
        std::atomic<size_t> head_{0};
    
    alignas(std::hardware_destructive_interference_size) 
        std::atomic<size_t> tail_{0};
    
    alignas(std::hardware_destructive_interference_size) 
        std::array<Node, Size> buffer_;
    
public:
    bool push(T value) {
        size_t tail = tail_.load(std::memory_order_relaxed);
        size_t nextTail = (tail + 1) % Size;
        
        if (nextTail == head_.load(std::memory_order_acquire)) {
            return false; // Queue full
        }
        
        buffer_[tail].data.store(value, std::memory_order_relaxed);
        buffer_[tail].ready.store(true, std::memory_order_release);
        tail_.store(nextTail, std::memory_order_release);
        
        return true;
    }
    
    std::optional<T> pop() {
        size_t head = head_.load(std::memory_order_relaxed);
        
        if (!buffer_[head].ready.load(std::memory_order_acquire)) {
            return std::nullopt; // Queue empty or not ready
        }
        
        T value = buffer_[head].data.load(std::memory_order_relaxed);
        buffer_[head].ready.store(false, std::memory_order_release);
        head_.store((head + 1) % Size, std::memory_order_release);
        
        return value;
    }
};

int main() {
    LockFreeQueue<int, 1024> queue;
    
    std::thread producer([&queue]() {
        for (int i = 0; i < 1000; ++i) {
            while (!queue.push(i)) {
                std::this_thread::yield();
            }
        }
    });
    
    std::thread consumer([&queue]() {
        int count = 0;
        while (count < 1000) {
            auto value = queue.pop();
            if (value) {
                count++;
            } else {
                std::this_thread::yield();
            }
        }
        std::cout << "Consumed 1000 items\n";
    });
    
    producer.join();
    consumer.join();
    
    return 0;
}
```

## Detection Techniques

### Using Performance Counters

On Linux, you can use `perf` to detect false sharing:

```bash
perf c2c record ./your_program
perf c2c report
```

### Profiling Tools

- Intel VTune Amplifier
- AMD μProf
- Linux perf with cache-related events

### Code Review Checklist

1. Look for adjacent atomic variables or frequently modified variables
2. Check array access patterns where different threads access nearby indices
3. Examine struct layouts with multiple frequently-accessed members
4. Review thread-local storage implementations

## Best Practices

1. **Use `alignas` for hot variables**: Align frequently modified variables to cache line boundaries
2. **Separate read-mostly from write-frequently data**: Keep them on different cache lines
3. **Pad structures**: Add padding between members accessed by different threads
4. **Use C++17 interference sizes**: Leverage `std::hardware_destructive_interference_size`
5. **Profile before optimizing**: False sharing may not always be the bottleneck
6. **Consider memory overhead**: Cache line alignment increases memory usage
7. **Group related data**: Data accessed together can share cache lines (use `std::hardware_constructive_interference_size`)

## Summary

False sharing is a subtle but serious performance problem in concurrent programs where independent variables sharing a cache line cause excessive cache coherence traffic. The primary solution is cache line alignment using `alignas(64)` or the C++17 constants `std::hardware_destructive_interference_size` and `std::hardware_constructive_interference_size`. While alignment solves false sharing, it increases memory usage, so profiling is essential to identify actual bottlenecks. Key indicators include high cache miss rates, poor scaling with additional threads, and significant time spent in memory synchronization. Proper cache line alignment can improve performance by 2-10x in false sharing scenarios, making it a critical optimization for high-performance concurrent code.