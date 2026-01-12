# Thread Management and RAII in C++

## Introduction

RAII (Resource Acquisition Is Initialization) is a fundamental C++ idiom where resource lifetime is tied to object lifetime. When applied to thread management, RAII ensures threads are properly joined or detached before destruction, preventing resource leaks and undefined behavior. This article explores modern C++ thread management using RAII principles, focusing on `std::jthread` (C++20) and custom RAII wrappers for `std::thread`.

## The Problem with Raw std::thread

The `std::thread` class requires explicit handling before destruction. If a `std::thread` object is destroyed while still joinable (not yet joined or detached), the program terminates by calling `std::terminate()`.

```cpp
#include <thread>
#include <iostream>

void dangerous_function() {
    std::thread t([]() {
        std::cout << "Thread running\n";
    });
    
    // If an exception occurs here, t's destructor is called
    // while t is still joinable -> std::terminate() is called!
    
    // We might forget to join:
    // t.join();
} // Destructor called on joinable thread -> program terminates!

int main() {
    try {
        dangerous_function();
    } catch (...) {
        std::cout << "Caught exception\n";
    }
    return 0;
}
```

## RAII Thread Wrapper for std::thread

A proper RAII wrapper ensures the thread is always properly handled in the destructor:

```cpp
#include <thread>
#include <iostream>
#include <stdexcept>

class ThreadGuard {
private:
    std::thread& thread_;
    
public:
    explicit ThreadGuard(std::thread& t) : thread_(t) {}
    
    ~ThreadGuard() {
        if (thread_.joinable()) {
            thread_.join();  // or thread_.detach() depending on use case
        }
    }
    
    // Prevent copying
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void safe_function() {
    std::thread t([]() {
        std::cout << "Thread running safely\n";
    });
    
    ThreadGuard guard(t);
    
    // Even if an exception occurs, the guard's destructor
    // will join the thread before unwinding
    throw std::runtime_error("Something went wrong!");
    
} // guard's destructor joins the thread automatically

int main() {
    try {
        safe_function();
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    return 0;
}
```

## Scoped Thread: Owning RAII Wrapper

A more complete RAII solution involves owning the thread object directly:

```cpp
#include <thread>
#include <iostream>
#include <chrono>

class ScopedThread {
private:
    std::thread thread_;
    
public:
    // Constructor takes any callable and its arguments
    template<typename Callable, typename... Args>
    explicit ScopedThread(Callable&& func, Args&&... args)
        : thread_(std::forward<Callable>(func), std::forward<Args>(args)...) {}
    
    // Destructor ensures thread is joined
    ~ScopedThread() {
        if (thread_.joinable()) {
            thread_.join();
        }
    }
    
    // Delete copy operations
    ScopedThread(const ScopedThread&) = delete;
    ScopedThread& operator=(const ScopedThread&) = delete;
    
    // Allow move operations
    ScopedThread(ScopedThread&& other) noexcept 
        : thread_(std::move(other.thread_)) {}
    
    ScopedThread& operator=(ScopedThread&& other) noexcept {
        if (this != &other) {
            if (thread_.joinable()) {
                thread_.join();
            }
            thread_ = std::move(other.thread_);
        }
        return *this;
    }
    
    // Provide access to underlying thread if needed
    std::thread::id get_id() const { return thread_.get_id(); }
    bool joinable() const { return thread_.joinable(); }
};

void worker(int id, int duration) {
    std::cout << "Worker " << id << " starting\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    std::cout << "Worker " << id << " finished\n";
}

int main() {
    {
        ScopedThread t1(worker, 1, 100);
        ScopedThread t2(worker, 2, 150);
        
        std::cout << "Main thread continues...\n";
        
    } // Both threads automatically joined here
    
    std::cout << "All threads completed\n";
    return 0;
}
```

## std::jthread: C++20's RAII Thread (Recommended)

C++20 introduced `std::jthread` (joining thread), which automatically joins in its destructor and supports cooperative cancellation via `std::stop_token`:

```cpp
#include <thread>
#include <iostream>
#include <chrono>

void simple_task() {
    std::cout << "Simple task running\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Simple task completed\n";
}

void cancellable_task(std::stop_token stop_token) {
    int count = 0;
    while (!stop_token.stop_requested() && count < 10) {
        std::cout << "Count: " << count++ << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (stop_token.stop_requested()) {
        std::cout << "Task cancelled!\n";
    } else {
        std::cout << "Task completed normally\n";
    }
}

int main() {
    // Basic usage - automatically joins
    {
        std::jthread t(simple_task);
        std::cout << "Main continues while thread runs\n";
    } // Automatic join happens here
    
    std::cout << "\n--- Cooperative Cancellation ---\n";
    
    // Cooperative cancellation
    {
        std::jthread t(cancellable_task);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        
        std::cout << "Requesting stop...\n";
        t.request_stop();  // Signal the thread to stop
        
    } // Still joins automatically, but thread will exit early
    
    std::cout << "All operations completed\n";
    return 0;
}
```

## Thread Ownership Patterns

### Pattern 1: Unique Ownership with Move Semantics

```cpp
#include <thread>
#include <vector>
#include <iostream>

class ThreadPool {
private:
    std::vector<std::jthread> threads_;
    
public:
    void add_task(auto&& task) {
        threads_.emplace_back(std::forward<decltype(task)>(task));
    }
    
    size_t size() const { return threads_.size(); }
    
    // Move-only semantics
    ThreadPool() = default;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator=(ThreadPool&&) = default;
    
    ~ThreadPool() {
        std::cout << "ThreadPool destructor: all threads will auto-join\n";
    }
};

int main() {
    ThreadPool pool;
    
    for (int i = 0; i < 5; ++i) {
        pool.add_task([i]() {
            std::cout << "Task " << i << " executing\n";
        });
    }
    
    std::cout << "Pool has " << pool.size() << " threads\n";
    
    // All threads automatically joined when pool goes out of scope
    return 0;
}
```

### Pattern 2: Detachable Background Tasks

```cpp
#include <thread>
#include <iostream>
#include <chrono>

class DetachableThread {
private:
    std::thread thread_;
    
public:
    template<typename Callable, typename... Args>
    explicit DetachableThread(Callable&& func, Args&&... args)
        : thread_(std::forward<Callable>(func), std::forward<Args>(args)...) {
        thread_.detach();  // Immediately detach
    }
    
    // No destructor needed - thread is already detached
    ~DetachableThread() = default;
    
    DetachableThread(const DetachableThread&) = delete;
    DetachableThread& operator=(const DetachableThread&) = delete;
};

void background_logger(std::string message) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Background: " << message << "\n";
}

int main() {
    {
        DetachableThread t(background_logger, "Hello from detached thread");
        std::cout << "Main thread doesn't wait\n";
    } // Destructor doesn't block
    
    std::cout << "Continuing immediately...\n";
    
    // Give detached thread time to complete
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    return 0;
}
```

## Exception Safety with Thread Management

```cpp
#include <thread>
#include <iostream>
#include <stdexcept>
#include <vector>

class ExceptionSafeThreadManager {
private:
    std::vector<std::jthread> threads_;
    
public:
    void add_task(auto&& task) {
        try {
            threads_.emplace_back(std::forward<decltype(task)>(task));
        } catch (const std::system_error& e) {
            std::cerr << "Failed to create thread: " << e.what() << "\n";
            throw;
        }
    }
    
    ~ExceptionSafeThreadManager() {
        // jthread automatically joins even if exception was thrown
        std::cout << "Cleaning up " << threads_.size() << " threads\n";
    }
};

void risky_operation(int id) {
    std::cout << "Thread " << id << " starting\n";
    
    if (id == 2) {
        throw std::runtime_error("Thread " + std::to_string(id) + " failed!");
    }
    
    std::cout << "Thread " << id << " completed\n";
}

int main() {
    try {
        ExceptionSafeThreadManager manager;
        
        for (int i = 0; i < 4; ++i) {
            manager.add_task([i]() {
                try {
                    risky_operation(i);
                } catch (const std::exception& e) {
                    std::cerr << "Caught in thread: " << e.what() << "\n";
                }
            });
        }
        
        std::cout << "All threads created successfully\n";
        
    } catch (const std::exception& e) {
        std::cerr << "Fatal error: " << e.what() << "\n";
    }
    
    // All threads properly joined before program exits
    return 0;
}
```

## Advanced: Thread Pool with Work Queue

```cpp
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>

class ThreadPoolRAII {
private:
    std::vector<std::jthread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::condition_variable cv_;
    
    void worker_thread(std::stop_token stop_token) {
        while (!stop_token.stop_requested()) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this, &stop_token] {
                    return !tasks_.empty() || stop_token.stop_requested();
                });
                
                if (stop_token.stop_requested() && tasks_.empty()) {
                    return;
                }
                
                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }
            
            if (task) {
                task();
            }
        }
    }
    
public:
    explicit ThreadPoolRAII(size_t num_threads) {
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this](std::stop_token st) {
                worker_thread(st);
            });
        }
    }
    
    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }
    
    ~ThreadPoolRAII() {
        std::cout << "ThreadPool shutting down...\n";
        // jthreads automatically request stop and join
    }
};

int main() {
    {
        ThreadPoolRAII pool(3);
        
        for (int i = 0; i < 10; ++i) {
            pool.enqueue([i]() {
                std::cout << "Task " << i << " executing on thread " 
                          << std::this_thread::get_id() << "\n";
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
        }
        
        std::cout << "All tasks enqueued\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
    } // Pool destructor gracefully shuts down all threads
    
    std::cout << "All work completed\n";
    return 0;
}
```

## Summary

**Key Principles of RAII Thread Management:**

1. **Automatic Resource Management**: Thread lifetime is bound to object lifetime, ensuring proper cleanup through destructors.

2. **Exception Safety**: RAII wrappers guarantee threads are properly joined or detached even when exceptions occur, preventing `std::terminate()` calls.

3. **Prefer std::jthread (C++20)**: It provides automatic joining, cooperative cancellation via `std::stop_token`, and eliminates the need for custom RAII wrappers in most cases.

4. **Clear Ownership Semantics**: Use move-only types for thread ownership, preventing accidental copying and making ownership transfer explicit.

5. **Choose Join vs Detach Carefully**: Joining threads ensures they complete before destruction (safer, preferred). Detaching allows independent execution but requires careful lifetime management of captured data.

6. **Thread Safety**: Always protect shared state with mutexes and use condition variables for thread coordination within RAII-managed threads.

**Best Practices:**
- Use `std::jthread` for new C++20 code
- Create custom RAII wrappers when using `std::thread` in C++17 or earlier
- Implement move semantics but delete copy operations for thread-owning classes
- Handle thread creation exceptions appropriately
- Use thread pools for managing multiple worker threads efficiently
- Leverage `std::stop_token` for cooperative cancellation in `std::jthread`

RAII thread management transforms thread handling from error-prone manual management into safe, automatic resource handling that integrates naturally with C++'s scope-based resource management model.