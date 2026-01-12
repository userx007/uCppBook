# Thread Identification and Hardware

## Overview

Thread identification and hardware-related features in C++ provide crucial capabilities for managing concurrent programs effectively. Understanding how to identify threads, determine hardware capabilities, and optimize thread placement can significantly impact application performance and debugging capabilities.

## Thread Identification with std::thread::id

Every thread in C++ has a unique identifier represented by `std::thread::id`. This identifier allows you to distinguish between different threads, which is essential for debugging, logging, and coordination between threads.

### Basic Thread ID Usage

```cpp
#include <iostream>
#include <thread>
#include <map>
#include <mutex>

std::mutex cout_mutex;

void worker_function(int task_id) {
    std::thread::id this_id = std::this_thread::get_id();
    
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Task " << task_id 
                  << " running on thread: " << this_id << std::endl;
    }
    
    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main() {
    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
    
    std::thread t1(worker_function, 1);
    std::thread t2(worker_function, 2);
    std::thread t3(worker_function, 3);
    
    // Store thread IDs for later reference
    std::thread::id t1_id = t1.get_id();
    std::thread::id t2_id = t2.get_id();
    
    std::cout << "Launched thread 1 with ID: " << t1_id << std::endl;
    std::cout << "Launched thread 2 with ID: " << t2_id << std::endl;
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

### Using Thread IDs as Map Keys

Thread IDs can be used as keys in associative containers, making them useful for per-thread data storage:

```cpp
#include <iostream>
#include <thread>
#include <map>
#include <mutex>
#include <vector>

class ThreadRegistry {
private:
    std::map<std::thread::id, std::string> thread_names;
    std::map<std::thread::id, int> operation_counts;
    std::mutex registry_mutex;

public:
    void register_thread(const std::string& name) {
        std::lock_guard<std::mutex> lock(registry_mutex);
        thread_names[std::this_thread::get_id()] = name;
        operation_counts[std::this_thread::get_id()] = 0;
    }
    
    void increment_operations() {
        std::lock_guard<std::mutex> lock(registry_mutex);
        operation_counts[std::this_thread::get_id()]++;
    }
    
    void print_statistics() {
        std::lock_guard<std::mutex> lock(registry_mutex);
        std::cout << "\nThread Statistics:\n";
        for (const auto& [id, name] : thread_names) {
            std::cout << "Thread " << name << " (ID: " << id 
                      << ") performed " << operation_counts[id] 
                      << " operations\n";
        }
    }
};

ThreadRegistry registry;

void worker(const std::string& name, int iterations) {
    registry.register_thread(name);
    
    for (int i = 0; i < iterations; ++i) {
        registry.increment_operations();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
}

int main() {
    std::vector<std::thread> threads;
    
    threads.emplace_back(worker, "Worker-A", 50);
    threads.emplace_back(worker, "Worker-B", 75);
    threads.emplace_back(worker, "Worker-C", 60);
    
    for (auto& t : threads) {
        t.join();
    }
    
    registry.print_statistics();
    
    return 0;
}
```

## Hardware Concurrency

The `std::thread::hardware_concurrency()` function returns the number of concurrent threads supported by the hardware implementation. This is typically the number of logical CPU cores available.

### Using hardware_concurrency for Thread Pool Sizing

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class SimpleThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    SimpleThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { 
                            return stop || !tasks.empty(); 
                        });
                        
                        if (stop && tasks.empty()) return;
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }
    
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
    
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};

int main() {
    unsigned int num_cores = std::thread::hardware_concurrency();
    
    std::cout << "Hardware concurrency: " << num_cores << " cores\n";
    
    // Create thread pool with optimal size
    size_t pool_size = (num_cores > 0) ? num_cores : 2;
    std::cout << "Creating thread pool with " << pool_size << " threads\n";
    
    SimpleThreadPool pool(pool_size);
    
    // Enqueue tasks
    for (int i = 0; i < 20; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " executed on thread " 
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Pool destructor will wait for all tasks to complete
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    return 0;
}
```

### Adaptive Workload Distribution

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <numeric>
#include <algorithm>

// Function to process a portion of data
void process_chunk(const std::vector<int>& data, 
                   size_t start, size_t end, 
                   long long& partial_sum) {
    partial_sum = 0;
    for (size_t i = start; i < end; ++i) {
        partial_sum += data[i] * data[i]; // Some computation
    }
}

long long parallel_process(const std::vector<int>& data) {
    unsigned int num_threads = std::thread::hardware_concurrency();
    
    // Handle case where hardware_concurrency returns 0
    if (num_threads == 0) {
        num_threads = 2;
    }
    
    // Ensure we don't create more threads than data elements
    num_threads = std::min(num_threads, 
                           static_cast<unsigned int>(data.size()));
    
    std::cout << "Using " << num_threads << " threads for processing\n";
    
    std::vector<std::thread> threads;
    std::vector<long long> partial_results(num_threads);
    
    size_t chunk_size = data.size() / num_threads;
    
    for (unsigned int i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? data.size() : (i + 1) * chunk_size;
        
        threads.emplace_back(process_chunk, 
                           std::cref(data), 
                           start, end, 
                           std::ref(partial_results[i]));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return std::accumulate(partial_results.begin(), 
                          partial_results.end(), 
                          0LL);
}

int main() {
    std::vector<int> large_dataset(1000000);
    std::iota(large_dataset.begin(), large_dataset.end(), 1);
    
    auto start = std::chrono::high_resolution_clock::now();
    long long result = parallel_process(large_dataset);
    auto end = std::chrono::high_resolution_clock::now();
    
    std::cout << "Result: " << result << std::endl;
    std::cout << "Time: " 
              << std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count()
              << " ms" << std::endl;
    
    return 0;
}
```

## Thread Affinity Concepts

Thread affinity refers to binding threads to specific CPU cores. While standard C++ doesn't provide built-in thread affinity control, it's a crucial concept for performance-critical applications. Thread affinity helps with:

- **Cache locality**: Keeping a thread on the same core improves cache hit rates
- **Real-time performance**: Ensuring critical threads run on dedicated cores
- **NUMA optimization**: Placing threads near the memory they access

### Platform-Specific Thread Affinity Example (Linux)

```cpp
#include <iostream>
#include <thread>
#include <vector>

#ifdef __linux__
#include <pthread.h>
#include <sched.h>

void set_thread_affinity(std::thread& thread, int core_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(core_id, &cpuset);
    
    int result = pthread_setaffinity_np(thread.native_handle(), 
                                       sizeof(cpu_set_t), 
                                       &cpuset);
    if (result != 0) {
        std::cerr << "Error setting thread affinity\n";
    }
}

void worker_with_affinity(int task_id) {
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    pthread_getaffinity_np(pthread_self(), sizeof(cpu_set_t), &cpuset);
    
    std::cout << "Task " << task_id << " running on core(s): ";
    for (int i = 0; i < CPU_SETSIZE; ++i) {
        if (CPU_ISSET(i, &cpuset)) {
            std::cout << i << " ";
        }
    }
    std::cout << std::endl;
    
    // Simulate work
    volatile long long sum = 0;
    for (long long i = 0; i < 100000000; ++i) {
        sum += i;
    }
}

#endif

int main() {
#ifdef __linux__
    unsigned int num_cores = std::thread::hardware_concurrency();
    std::cout << "Available cores: " << num_cores << std::endl;
    
    std::vector<std::thread> threads;
    
    // Create threads and pin them to specific cores
    for (unsigned int i = 0; i < std::min(4u, num_cores); ++i) {
        threads.emplace_back(worker_with_affinity, i);
        set_thread_affinity(threads.back(), i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
#else
    std::cout << "Thread affinity example is Linux-specific\n";
#endif
    
    return 0;
}
```

### Cross-Platform Thread Priority (Conceptual)

```cpp
#include <iostream>
#include <thread>
#include <chrono>

enum class ThreadPriority {
    LOW,
    NORMAL,
    HIGH
};

class PriorityThread {
private:
    std::thread thread;
    
    void set_priority(ThreadPriority priority) {
#ifdef _WIN32
        // Windows implementation
        HANDLE handle = thread.native_handle();
        int win_priority;
        switch (priority) {
            case ThreadPriority::LOW:
                win_priority = THREAD_PRIORITY_BELOW_NORMAL;
                break;
            case ThreadPriority::HIGH:
                win_priority = THREAD_PRIORITY_ABOVE_NORMAL;
                break;
            default:
                win_priority = THREAD_PRIORITY_NORMAL;
        }
        SetThreadPriority(handle, win_priority);
#elif defined(__linux__)
        // Linux implementation
        pthread_t handle = thread.native_handle();
        sched_param param;
        int policy;
        pthread_getschedparam(handle, &policy, &param);
        
        switch (priority) {
            case ThreadPriority::LOW:
                param.sched_priority = sched_get_priority_min(SCHED_OTHER);
                break;
            case ThreadPriority::HIGH:
                param.sched_priority = sched_get_priority_max(SCHED_OTHER);
                break;
            default:
                param.sched_priority = (sched_get_priority_min(SCHED_OTHER) + 
                                       sched_get_priority_max(SCHED_OTHER)) / 2;
        }
        pthread_setschedparam(handle, SCHED_OTHER, &param);
#endif
    }

public:
    template<typename Func, typename... Args>
    PriorityThread(ThreadPriority priority, Func&& func, Args&&... args) 
        : thread(std::forward<Func>(func), std::forward<Args>(args)...) {
        set_priority(priority);
    }
    
    void join() { thread.join(); }
    std::thread::id get_id() const { return thread.get_id(); }
};

void critical_task() {
    std::cout << "Critical task (HIGH priority) on thread " 
              << std::this_thread::get_id() << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Critical: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void background_task() {
    std::cout << "Background task (LOW priority) on thread " 
              << std::this_thread::get_id() << std::endl;
    for (int i = 0; i < 5; ++i) {
        std::cout << "Background: " << i << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}
```

## Practical Example: Thread-Aware Logging System

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <sstream>
#include <iomanip>
#include <chrono>
#include <map>

class ThreadLogger {
private:
    std::mutex log_mutex;
    std::map<std::thread::id, std::string> thread_names;
    int unnamed_counter = 0;

    std::string get_timestamp() {
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
            now.time_since_epoch()) % 1000;
        
        std::stringstream ss;
        ss << std::put_time(std::localtime(&time), "%H:%M:%S");
        ss << '.' << std::setfill('0') << std::setw(3) << ms.count();
        return ss.str();
    }
    
    std::string get_thread_name(std::thread::id id) {
        auto it = thread_names.find(id);
        if (it != thread_names.end()) {
            return it->second;
        }
        return "Thread-" + std::to_string(unnamed_counter++);
    }

public:
    void register_thread(const std::string& name) {
        std::lock_guard<std::mutex> lock(log_mutex);
        thread_names[std::this_thread::get_id()] = name;
    }
    
    void log(const std::string& message) {
        std::lock_guard<std::mutex> lock(log_mutex);
        std::thread::id id = std::this_thread::get_id();
        
        std::cout << "[" << get_timestamp() << "] "
                  << "[" << get_thread_name(id) << "] "
                  << message << std::endl;
    }
};

ThreadLogger logger;

void database_worker() {
    logger.register_thread("DB-Worker");
    logger.log("Starting database operations");
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    logger.log("Database query completed");
}

void network_worker() {
    logger.register_thread("Network-Worker");
    logger.log("Establishing connection");
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    logger.log("Data received");
}

int main() {
    logger.register_thread("MainThread");
    
    unsigned int cores = std::thread::hardware_concurrency();
    logger.log("System has " + std::to_string(cores) + " hardware threads");
    
    std::thread t1(database_worker);
    std::thread t2(network_worker);
    
    logger.log("Worker threads launched");
    
    t1.join();
    t2.join();
    
    logger.log("All workers completed");
    
    return 0;
}
```

## Summary

**Thread Identification** provides essential capabilities for managing and debugging concurrent applications. The `std::thread::id` type serves as a unique identifier for threads and can be used as keys in containers for per-thread data management. You can obtain a thread's ID using `std::this_thread::get_id()` from within the thread or `thread.get_id()` from the thread object.

**Hardware Concurrency** through `std::thread::hardware_concurrency()` helps you write adaptive concurrent code that scales with the available hardware. This function returns the number of concurrent threads the hardware can support, which is invaluable for sizing thread pools and distributing workloads optimally. Always handle the case where this function returns 0, which indicates the value is not computable.

**Thread Affinity** concepts, while not standardized in C++, are crucial for performance optimization in production systems. By binding threads to specific CPU cores through platform-specific APIs, you can improve cache locality, reduce context switching overhead, and optimize NUMA (Non-Uniform Memory Access) performance. Thread affinity is particularly important for real-time systems, high-performance computing, and latency-sensitive applications.

Together, these features enable you to build robust, efficient, and hardware-aware concurrent applications that can adapt to different execution environments while maintaining observability through proper thread identification and logging.