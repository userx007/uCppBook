# Performance Profiling of Concurrent Code

Performance profiling of concurrent code is essential for identifying bottlenecks, contention points, and scalability issues in multithreaded applications. Unlike sequential code profiling, concurrent profiling must account for synchronization overhead, lock contention, cache coherency issues, and thread scheduling effects.

## Key Concepts

**Lock Contention**: When multiple threads compete for the same lock, causing threads to wait unnecessarily. High contention indicates poor scalability.

**Lock Overhead**: The computational cost of acquiring and releasing locks, even when uncontended. This includes memory barriers and atomic operations.

**Scalability Metrics**: Measuring how performance changes as thread count increases. Ideal scaling means doubling threads doubles throughput.

**Critical Path Analysis**: Identifying the longest chain of dependent operations that determines minimum execution time.

## Profiling Techniques

### 1. Manual Instrumentation with Timing

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>
#include <vector>
#include <iomanip>

class PerformanceMonitor {
    std::mutex mutex_;
    size_t total_waits_ = 0;
    std::chrono::nanoseconds total_wait_time_{0};
    std::chrono::nanoseconds total_hold_time_{0};
    size_t total_acquisitions_ = 0;

public:
    class ScopedLockProfile {
        PerformanceMonitor& monitor_;
        std::mutex& target_mutex_;
        std::chrono::steady_clock::time_point start_;
        std::chrono::steady_clock::time_point acquired_;

    public:
        ScopedLockProfile(PerformanceMonitor& monitor, std::mutex& m)
            : monitor_(monitor), target_mutex_(m), start_(std::chrono::steady_clock::now()) {
            target_mutex_.lock();
            acquired_ = std::chrono::steady_clock::now();
        }

        ~ScopedLockProfile() {
            auto released = std::chrono::steady_clock::now();
            auto wait_time = acquired_ - start_;
            auto hold_time = released - acquired_;
            
            target_mutex_.unlock();
            monitor_.record_lock_stats(wait_time, hold_time);
        }
    };

    void record_lock_stats(std::chrono::nanoseconds wait_time, 
                          std::chrono::nanoseconds hold_time) {
        std::lock_guard<std::mutex> lock(mutex_);
        total_acquisitions_++;
        total_wait_time_ += wait_time;
        total_hold_time_ += hold_time;
        
        if (wait_time > std::chrono::microseconds(1)) {
            total_waits_++;
        }
    }

    void print_statistics() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::cout << "\n=== Lock Performance Statistics ===\n";
        std::cout << "Total acquisitions: " << total_acquisitions_ << "\n";
        std::cout << "Contended acquisitions: " << total_waits_ 
                  << " (" << (100.0 * total_waits_ / total_acquisitions_) << "%)\n";
        std::cout << "Average wait time: " 
                  << std::chrono::duration_cast<std::chrono::microseconds>(
                      total_wait_time_ / total_acquisitions_).count() << " μs\n";
        std::cout << "Average hold time: " 
                  << std::chrono::duration_cast<std::chrono::microseconds>(
                      total_hold_time_ / total_acquisitions_).count() << " μs\n";
    }
};

// Example usage with shared resource
class BankAccount {
    std::mutex mutex_;
    PerformanceMonitor& monitor_;
    double balance_ = 1000.0;

public:
    BankAccount(PerformanceMonitor& monitor) : monitor_(monitor) {}

    void transfer(double amount) {
        PerformanceMonitor::ScopedLockProfile profile(monitor_, mutex_);
        // Simulate some work
        balance_ += amount;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }

    double get_balance() {
        PerformanceMonitor::ScopedLockProfile profile(monitor_, mutex_);
        return balance_;
    }
};

void demonstrate_profiling() {
    PerformanceMonitor monitor;
    BankAccount account(monitor);
    
    std::vector<std::thread> threads;
    const int num_threads = 8;
    const int operations_per_thread = 1000;
    
    auto start = std::chrono::steady_clock::now();
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&account, operations_per_thread]() {
            for (int j = 0; j < operations_per_thread; ++j) {
                account.transfer(1.0);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Total execution time: " << duration.count() << " ms\n";
    std::cout << "Throughput: " << (num_threads * operations_per_thread * 1000.0 / duration.count()) 
              << " ops/sec\n";
    
    monitor.print_statistics();
}
```

### 2. Contention Detection

```cpp
#include <atomic>
#include <array>

template<typename Mutex>
class ContentionDetectingMutex {
    Mutex mutex_;
    std::atomic<size_t> contention_count_{0};
    std::atomic<size_t> try_lock_failures_{0};

public:
    void lock() {
        if (!mutex_.try_lock()) {
            contention_count_.fetch_add(1, std::memory_order_relaxed);
            mutex_.lock();
        }
    }

    void unlock() {
        mutex_.unlock();
    }

    bool try_lock() {
        bool success = mutex_.try_lock();
        if (!success) {
            try_lock_failures_.fetch_add(1, std::memory_order_relaxed);
        }
        return success;
    }

    size_t get_contention_count() const {
        return contention_count_.load(std::memory_order_relaxed);
    }

    size_t get_try_lock_failures() const {
        return try_lock_failures_.load(std::memory_order_relaxed);
    }
};

// Example: Compare different synchronization strategies
template<typename MutexType>
double benchmark_mutex(int num_threads, int iterations) {
    MutexType mutex;
    std::atomic<long long> counter{0};
    
    auto start = std::chrono::steady_clock::now();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back([&, iterations]() {
            for (int j = 0; j < iterations; ++j) {
                std::lock_guard<MutexType> lock(mutex);
                counter.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    auto end = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(end - start).count();
}

void compare_synchronization_methods() {
    const int threads = 4;
    const int iterations = 100000;
    
    std::cout << "\n=== Synchronization Method Comparison ===\n";
    
    auto time_mutex = benchmark_mutex<std::mutex>(threads, iterations);
    std::cout << "std::mutex: " << time_mutex << " ms\n";
    
    auto time_contention = benchmark_mutex<ContentionDetectingMutex<std::mutex>>(threads, iterations);
    std::cout << "Contention detecting mutex: " << time_contention << " ms\n";
}
```

### 3. Scalability Testing

```cpp
#include <map>

class ScalabilityBenchmark {
public:
    struct Result {
        int thread_count;
        double execution_time_ms;
        double throughput;
        double speedup;
        double efficiency;
    };

private:
    template<typename WorkFunction>
    Result run_benchmark(int num_threads, WorkFunction work, int total_operations) {
        auto start = std::chrono::steady_clock::now();
        
        std::vector<std::thread> threads;
        int ops_per_thread = total_operations / num_threads;
        
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&work, ops_per_thread]() {
                for (int j = 0; j < ops_per_thread; ++j) {
                    work();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::steady_clock::now();
        double time_ms = std::chrono::duration<double, std::milli>(end - start).count();
        
        return Result{
            num_threads,
            time_ms,
            total_operations / (time_ms / 1000.0),
            0.0,  // Will be calculated later
            0.0   // Will be calculated later
        };
    }

public:
    template<typename WorkFunction>
    std::vector<Result> analyze_scalability(WorkFunction work, 
                                           int max_threads,
                                           int total_operations) {
        std::vector<Result> results;
        
        for (int threads = 1; threads <= max_threads; threads *= 2) {
            results.push_back(run_benchmark(threads, work, total_operations));
        }
        
        // Calculate speedup and efficiency relative to single-threaded
        double baseline_time = results[0].execution_time_ms;
        
        for (auto& result : results) {
            result.speedup = baseline_time / result.execution_time_ms;
            result.efficiency = result.speedup / result.thread_count;
        }
        
        return results;
    }

    static void print_results(const std::vector<Result>& results) {
        std::cout << "\n=== Scalability Analysis ===\n";
        std::cout << std::setw(10) << "Threads" 
                  << std::setw(15) << "Time (ms)"
                  << std::setw(18) << "Throughput (ops/s)"
                  << std::setw(12) << "Speedup"
                  << std::setw(12) << "Efficiency\n";
        std::cout << std::string(67, '-') << "\n";
        
        for (const auto& r : results) {
            std::cout << std::setw(10) << r.thread_count
                      << std::setw(15) << std::fixed << std::setprecision(2) << r.execution_time_ms
                      << std::setw(18) << std::fixed << std::setprecision(0) << r.throughput
                      << std::setw(12) << std::fixed << std::setprecision(2) << r.speedup
                      << std::setw(11) << std::fixed << std::setprecision(1) << (r.efficiency * 100) << "%\n";
        }
    }
};

// Example: Test different data structures for scalability
void test_concurrent_data_structure_scalability() {
    const int total_ops = 1000000;
    
    // Test with fine-grained locking
    std::cout << "\nFine-grained locking (multiple mutexes):\n";
    {
        std::array<std::mutex, 16> mutexes;
        std::array<int, 16> counters{};
        
        auto work = [&]() {
            size_t index = std::hash<std::thread::id>{}(std::this_thread::get_id()) % 16;
            std::lock_guard<std::mutex> lock(mutexes[index]);
            counters[index]++;
        };
        
        ScalabilityBenchmark benchmark;
        auto results = benchmark.analyze_scalability(work, 8, total_ops);
        ScalabilityBenchmark::print_results(results);
    }
    
    // Test with coarse-grained locking
    std::cout << "\nCoarse-grained locking (single mutex):\n";
    {
        std::mutex mutex;
        int counter = 0;
        
        auto work = [&]() {
            std::lock_guard<std::mutex> lock(mutex);
            counter++;
        };
        
        ScalabilityBenchmark benchmark;
        auto results = benchmark.analyze_scalability(work, 8, total_ops);
        ScalabilityBenchmark::print_results(results);
    }
    
    // Test with atomic operations
    std::cout << "\nAtomic operations (lock-free):\n";
    {
        std::atomic<int> counter{0};
        
        auto work = [&]() {
            counter.fetch_add(1, std::memory_order_relaxed);
        };
        
        ScalabilityBenchmark benchmark;
        auto results = benchmark.analyze_scalability(work, 8, total_ops);
        ScalabilityBenchmark::print_results(results);
    }
}
```

### 4. Cache Line Contention Detection

```cpp
#include <new>

// Helper to detect false sharing
struct alignas(64) CacheAlignedCounter {
    std::atomic<long long> value{0};
    char padding[64 - sizeof(std::atomic<long long>)];
};

void demonstrate_false_sharing_impact() {
    const int num_threads = 4;
    const int iterations = 10000000;
    
    std::cout << "\n=== False Sharing Impact ===\n";
    
    // Test with false sharing
    {
        std::array<std::atomic<long long>, 4> counters{};
        
        auto start = std::chrono::steady_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&counters, i, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    counters[i].fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "With false sharing: " << duration << " ms\n";
    }
    
    // Test without false sharing (cache-aligned)
    {
        std::array<CacheAlignedCounter, 4> counters{};
        
        auto start = std::chrono::steady_clock::now();
        
        std::vector<std::thread> threads;
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back([&counters, i, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    counters[i].value.fetch_add(1, std::memory_order_relaxed);
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration<double, std::milli>(end - start).count();
        
        std::cout << "Without false sharing (cache-aligned): " << duration << " ms\n";
    }
}
```

### 5. Complete Profiling Example

```cpp
int main() {
    std::cout << "=== Concurrent Code Performance Profiling Demo ===\n";
    
    // 1. Basic lock profiling
    demonstrate_profiling();
    
    // 2. Compare synchronization methods
    compare_synchronization_methods();
    
    // 3. Scalability analysis
    test_concurrent_data_structure_scalability();
    
    // 4. False sharing detection
    demonstrate_false_sharing_impact();
    
    return 0;
}
```

## Tools for Profiling Concurrent Code

### Built-in Tools
- **perf (Linux)**: Hardware performance counters for cache misses, branch mispredictions
- **Intel VTune**: Advanced thread profiling with lock contention analysis
- **Visual Studio Profiler**: Concurrency visualizer for Windows
- **valgrind --tool=helgrind**: Thread error detection and performance analysis

### Metrics to Monitor
1. **Lock wait time**: Time threads spend waiting for locks
2. **Lock hold time**: Duration locks are held
3. **Contention rate**: Percentage of lock acquisitions that must wait
4. **Thread utilization**: CPU time vs. wait time per thread
5. **Cache miss rate**: L1/L2/L3 cache misses indicating false sharing
6. **Context switches**: High rates indicate excessive contention

## Summary

Performance profiling of concurrent code requires specialized techniques beyond sequential profiling. Key practices include:

- **Manual instrumentation** with timing wrappers to measure lock wait and hold times
- **Contention detection** to identify synchronization bottlenecks
- **Scalability testing** across different thread counts to verify linear scaling
- **False sharing detection** using cache-aligned data structures
- **Comparative analysis** of different synchronization strategies (mutexes vs. atomics vs. lock-free)

Effective profiling combines manual instrumentation with specialized tools, focusing on metrics like contention rate, lock overhead, and scalability efficiency. The goal is to identify whether performance issues stem from excessive synchronization, inadequate parallelism, or architectural problems like false sharing. Always profile with realistic workloads and measure both absolute performance and scalability characteristics.