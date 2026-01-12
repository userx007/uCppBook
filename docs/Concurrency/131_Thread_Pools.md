# Thread Pools in C++

## Introduction

A **thread pool** is a software design pattern for managing a collection of worker threads that process tasks from a shared queue. Instead of creating and destroying threads for each task (which is expensive), a thread pool maintains a fixed or dynamic number of threads that remain alive and ready to execute work. This approach significantly improves performance and resource utilization in concurrent applications.

Thread pools are essential for building scalable applications because thread creation has overhead: allocating stack space, initializing thread-local storage, and OS-level context switching. By reusing threads, we amortize these costs across many tasks.

## Core Concepts

### Key Components

1. **Worker Threads**: A collection of threads that continuously wait for and execute tasks
2. **Task Queue**: A thread-safe queue holding pending tasks
3. **Synchronization Primitives**: Mutexes and condition variables for thread coordination
4. **Task Representation**: Function objects (callables) that encapsulate work to be done

### Design Considerations

- **Pool Size**: Fixed vs dynamic sizing based on workload
- **Queue Management**: Bounded vs unbounded queues, overflow policies
- **Task Scheduling**: FIFO, priority-based, or work-stealing approaches
- **Graceful Shutdown**: Completing in-flight tasks vs immediate termination
- **Exception Handling**: Propagating exceptions from worker threads

## Basic Thread Pool Implementation

Let's build a simple but functional thread pool from scratch:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>

class SimpleThreadPool {
public:
    explicit SimpleThreadPool(size_t num_threads) : stop(false) {
        // Create worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        
                        // Wait until there's a task or pool is stopping
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                        
                        // Exit if stopping and no tasks remain
                        if (stop && tasks.empty()) {
                            return;
                        }
                        
                        // Get next task
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    // Execute task outside the lock
                    task();
                }
            });
        }
    }
    
    // Submit a task to the pool
    template<typename F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }
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
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// Usage example
int main() {
    SimpleThreadPool pool(4);
    
    for (int i = 0; i < 8; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " executed by thread " 
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
```

## Advanced Thread Pool with Future Support

A more sophisticated thread pool that returns `std::future` objects for retrieving task results:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <memory>
#include <type_traits>

class ThreadPool {
public:
    explicit ThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                        
                        if (stop && tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    // Enqueue task and return future
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }
            
            tasks.emplace([task]() { (*task)(); });
        }
        
        condition.notify_one();
        return result;
    }
    
    ~ThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

// Usage example with futures
int main() {
    ThreadPool pool(4);
    std::vector<std::future<int>> results;
    
    // Submit tasks that return values
    for (int i = 0; i < 8; ++i) {
        results.emplace_back(
            pool.enqueue([i] {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                return i * i;
            })
        );
    }
    
    // Retrieve results
    for (auto& result : results) {
        std::cout << "Result: " << result.get() << std::endl;
    }
    
    return 0;
}
```

## Dynamic Thread Pool

A thread pool that adjusts its size based on workload:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <chrono>

class DynamicThreadPool {
public:
    DynamicThreadPool(size_t min_threads, size_t max_threads)
        : min_threads(min_threads), max_threads(max_threads),
          active_threads(0), stop(false) {
        
        for (size_t i = 0; i < min_threads; ++i) {
            add_thread();
        }
    }
    
    void enqueue(std::function<void()> task) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push(std::move(task));
            
            // Scale up if all threads are busy and below max
            if (active_threads >= workers.size() && 
                workers.size() < max_threads) {
                add_thread();
            }
        }
        condition.notify_one();
    }
    
    ~DynamicThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        
        for (auto& worker : workers) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }
    
private:
    void add_thread() {
        workers.emplace_back([this] {
            while (true) {
                std::function<void()> task;
                
                {
                    std::unique_lock<std::mutex> lock(queue_mutex);
                    
                    // Wait with timeout for scaling down
                    if (!condition.wait_for(lock, std::chrono::seconds(30),
                        [this] { return stop || !tasks.empty(); })) {
                        
                        // Timeout: scale down if above minimum
                        if (workers.size() > min_threads) {
                            return;
                        }
                        continue;
                    }
                    
                    if (stop && tasks.empty()) {
                        return;
                    }
                    
                    task = std::move(tasks.front());
                    tasks.pop();
                    ++active_threads;
                }
                
                task();
                --active_threads;
            }
        });
    }
    
    size_t min_threads;
    size_t max_threads;
    std::atomic<size_t> active_threads;
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

int main() {
    DynamicThreadPool pool(2, 8);
    
    // Submit burst of tasks
    for (int i = 0; i < 20; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " on thread " 
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        });
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(5));
    return 0;
}
```

## Priority-Based Thread Pool

A thread pool that executes higher-priority tasks first:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

struct PriorityTask {
    int priority;
    std::function<void()> task;
    
    bool operator<(const PriorityTask& other) const {
        return priority < other.priority; // Higher priority = larger number
    }
};

class PriorityThreadPool {
public:
    explicit PriorityThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                        
                        if (stop && tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks.top().task);
                        tasks.pop();
                    }
                    
                    task();
                }
            });
        }
    }
    
    void enqueue(std::function<void()> task, int priority = 0) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push({priority, std::move(task)});
        }
        condition.notify_one();
    }
    
    ~PriorityThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
    
private:
    std::vector<std::thread> workers;
    std::priority_queue<PriorityTask> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

int main() {
    PriorityThreadPool pool(4);
    
    // Submit tasks with different priorities
    pool.enqueue([] { std::cout << "Low priority task\n"; }, 1);
    pool.enqueue([] { std::cout << "High priority task\n"; }, 10);
    pool.enqueue([] { std::cout << "Medium priority task\n"; }, 5);
    pool.enqueue([] { std::cout << "Critical priority task\n"; }, 100);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return 0;
}
```

## Exception Handling in Thread Pools

Properly handling exceptions from worker threads:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>
#include <exception>

class SafeThreadPool {
public:
    explicit SafeThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });
                        
                        if (stop && tasks.empty()) {
                            return;
                        }
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    try {
                        task();
                    } catch (const std::exception& e) {
                        std::cerr << "Exception in worker thread: " 
                                  << e.what() << std::endl;
                    } catch (...) {
                        std::cerr << "Unknown exception in worker thread\n";
                    }
                }
            });
        }
    }
    
    template<typename F, typename... Args>
    auto enqueue(F&& f, Args&&... args) 
        -> std::future<typename std::invoke_result<F, Args...>::type> {
        
        using return_type = typename std::invoke_result<F, Args...>::type;
        
        auto task = std::make_shared<std::packaged_task<return_type()>>(
            std::bind(std::forward<F>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task->get_future();
        
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            if (stop) {
                throw std::runtime_error("Cannot enqueue on stopped ThreadPool");
            }
            
            tasks.emplace([task]() { (*task)(); });
        }
        
        condition.notify_one();
        return result;
    }
    
    ~SafeThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
    
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

int main() {
    SafeThreadPool pool(4);
    
    // Task that throws exception
    auto future1 = pool.enqueue([] {
        throw std::runtime_error("Task failed!");
        return 42;
    });
    
    // Normal task
    auto future2 = pool.enqueue([] {
        return 100;
    });
    
    try {
        std::cout << "Result 1: " << future1.get() << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    std::cout << "Result 2: " << future2.get() << std::endl;
    
    return 0;
}
```

## Performance Considerations

### Optimal Pool Size

The ideal number of threads depends on workload characteristics:

```cpp
#include <thread>
#include <iostream>

size_t optimal_thread_count(bool cpu_bound) {
    unsigned int hw_threads = std::thread::hardware_concurrency();
    
    if (cpu_bound) {
        // CPU-bound: typically hardware_concurrency or slightly less
        return hw_threads > 0 ? hw_threads : 4;
    } else {
        // I/O-bound: can benefit from more threads
        return hw_threads > 0 ? hw_threads * 2 : 8;
    }
}

int main() {
    std::cout << "Hardware concurrency: " 
              << std::thread::hardware_concurrency() << std::endl;
    std::cout << "CPU-bound optimal: " 
              << optimal_thread_count(true) << std::endl;
    std::cout << "I/O-bound optimal: " 
              << optimal_thread_count(false) << std::endl;
    return 0;
}
```

### Task Granularity

Tasks should be neither too fine-grained (overhead dominates) nor too coarse-grained (poor load balancing). A good rule of thumb is that each task should take at least a few milliseconds to execute.

## Summary

Thread pools are a fundamental concurrency pattern that provides efficient task execution through thread reuse. Key takeaways include:

**Benefits**: Thread pools eliminate the overhead of repeated thread creation/destruction, limit resource consumption through bounded parallelism, improve application responsiveness, and simplify concurrent programming by abstracting thread management.

**Core Implementation**: A basic thread pool requires worker threads, a synchronized task queue, condition variables for thread coordination, and proper shutdown mechanisms. The worker threads continuously fetch and execute tasks from the queue.

**Advanced Features**: Production-grade thread pools often include future-based result retrieval using `std::packaged_task` and `std::future`, priority scheduling for urgent tasks, dynamic sizing to adapt to workload, and robust exception handling with proper propagation.

**Design Decisions**: Critical choices include pool size (consider CPU-bound vs I/O-bound workloads and hardware concurrency), queue bounds (unbounded queues can cause memory issues, bounded queues need overflow policies), and shutdown strategy (graceful completion vs immediate termination).

**Performance**: The optimal thread count typically equals hardware concurrency for CPU-bound work and 2x hardware concurrency for I/O-bound work. Task granularity matters—tasks should be substantial enough to amortize queuing overhead but small enough for good load balancing.

Thread pools are widely used in server applications, parallel algorithms, asynchronous I/O systems, and any scenario requiring controlled concurrent execution. Modern C++ provides `std::async` and libraries like Boost.Asio that incorporate thread pool concepts, but understanding the underlying implementation remains valuable for building customized solutions.