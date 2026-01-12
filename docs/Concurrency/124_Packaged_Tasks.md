# Packaged Tasks in C++ Concurrency

## Overview

`std::packaged_task` is a powerful C++ concurrency utility that wraps any callable object (function, lambda, functor) and allows its asynchronous execution while providing a mechanism to retrieve the result through a `std::future`. It acts as a bridge between callable objects and the future/promise mechanism, offering more flexibility than `std::async` for managing asynchronous operations.

## Core Concept

A `std::packaged_task` bundles together:
- A callable object (the task to execute)
- A shared state that stores the result or exception
- A `std::future` to retrieve that result

Unlike `std::async`, which immediately launches a task, `std::packaged_task` gives you control over when and how the task executes. You can move it between threads, store it in containers, or execute it at a later time.

## Basic Syntax

```cpp
#include <future>
#include <thread>
#include <iostream>

// Create a packaged_task with a callable
std::packaged_task<return_type(arg_types)> task(callable);

// Get the associated future
std::future<return_type> result = task.get_future();

// Execute the task (can be in any thread)
task(arguments);

// Retrieve the result
auto value = result.get();
```

## Detailed Code Examples

### Example 1: Basic Packaged Task Usage

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <chrono>

// A simple function to be packaged
int calculate_sum(int a, int b) {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Calculating sum in thread: " 
              << std::this_thread::get_id() << std::endl;
    return a + b;
}

int main() {
    // Create a packaged task wrapping the function
    std::packaged_task<int(int, int)> task(calculate_sum);
    
    // Get the future before moving the task
    std::future<int> result = task.get_future();
    
    // Execute the task in a separate thread
    std::thread worker(std::move(task), 10, 20);
    
    std::cout << "Main thread continues..." << std::endl;
    
    // Wait for result
    int sum = result.get();
    std::cout << "Result: " << sum << std::endl;
    
    worker.join();
    
    return 0;
}
```

### Example 2: Using Lambda Expressions

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>

int main() {
    // Packaged task with a lambda
    std::packaged_task<double(const std::vector<int>&)> task(
        [](const std::vector<int>& numbers) {
            double sum = 0;
            for (int num : numbers) {
                sum += num;
            }
            return sum / numbers.size();
        }
    );
    
    std::future<double> avg_future = task.get_future();
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Execute in separate thread
    std::thread t(std::move(task), std::cref(data));
    
    double average = avg_future.get();
    std::cout << "Average: " << average << std::endl;
    
    t.join();
    
    return 0;
}
```

### Example 3: Task Queue with Packaged Tasks

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <functional>

class TaskQueue {
private:
    std::queue<std::packaged_task<void()>> tasks;
    std::mutex mtx;
    std::condition_variable cv;
    bool stop = false;
    
public:
    // Add a task to the queue
    template<typename Func, typename... Args>
    auto enqueue(Func&& f, Args&&... args) 
        -> std::future<typename std::result_of<Func(Args...)>::type> {
        
        using return_type = typename std::result_of<Func(Args...)>::type;
        
        // Create packaged task with bound arguments
        auto task = std::packaged_task<return_type()>(
            std::bind(std::forward<Func>(f), std::forward<Args>(args)...)
        );
        
        std::future<return_type> result = task.get_future();
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            
            // Wrap in void() packaged_task for queue storage
            tasks.emplace([task = std::move(task)]() mutable { task(); });
        }
        
        cv.notify_one();
        return result;
    }
    
    // Worker thread function
    void worker() {
        while (true) {
            std::packaged_task<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(mtx);
                cv.wait(lock, [this] { return stop || !tasks.empty(); });
                
                if (stop && tasks.empty()) return;
                
                task = std::move(tasks.front());
                tasks.pop();
            }
            
            task();
        }
    }
    
    void shutdown() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stop = true;
        }
        cv.notify_all();
    }
};

// Example usage
int compute_square(int x) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    return x * x;
}

int main() {
    TaskQueue queue;
    
    // Start worker thread
    std::thread worker([&queue]() { queue.worker(); });
    
    // Enqueue tasks
    std::vector<std::future<int>> results;
    for (int i = 1; i <= 5; ++i) {
        results.push_back(queue.enqueue(compute_square, i));
    }
    
    // Collect results
    for (size_t i = 0; i < results.size(); ++i) {
        std::cout << "Square of " << (i + 1) << " = " 
                  << results[i].get() << std::endl;
    }
    
    queue.shutdown();
    worker.join();
    
    return 0;
}
```

### Example 4: Exception Handling

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <stdexcept>

int risky_operation(int value) {
    if (value < 0) {
        throw std::invalid_argument("Negative values not allowed");
    }
    return value * 2;
}

int main() {
    // Task that may throw an exception
    std::packaged_task<int(int)> task(risky_operation);
    std::future<int> result = task.get_future();
    
    std::thread t(std::move(task), -5);
    
    try {
        // Exception is propagated through the future
        int value = result.get();
        std::cout << "Result: " << value << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    t.join();
    
    return 0;
}
```

### Example 5: Resetting and Reusing Packaged Tasks

```cpp
#include <iostream>
#include <future>
#include <thread>

int multiply(int a, int b) {
    return a * b;
}

int main() {
    std::packaged_task<int(int, int)> task(multiply);
    
    // First execution
    std::future<int> result1 = task.get_future();
    std::thread t1(std::move(task), 3, 4);
    std::cout << "First result: " << result1.get() << std::endl;
    t1.join();
    
    // Reset the task to reuse it
    task.reset();
    
    // Second execution
    std::future<int> result2 = task.get_future();
    std::thread t2(std::move(task), 5, 6);
    std::cout << "Second result: " << result2.get() << std::endl;
    t2.join();
    
    return 0;
}
```

### Example 6: Comparison with std::async

```cpp
#include <iostream>
#include <future>
#include <thread>

int work(int n) {
    return n * n;
}

int main() {
    std::cout << "=== Using std::async ===" << std::endl;
    // Immediate launch (usually), less control
    auto future1 = std::async(std::launch::async, work, 5);
    std::cout << "Result: " << future1.get() << std::endl;
    
    std::cout << "\n=== Using std::packaged_task ===" << std::endl;
    // More control over execution
    std::packaged_task<int(int)> task(work);
    auto future2 = task.get_future();
    
    std::cout << "Task created but not yet executed..." << std::endl;
    
    // Execute when ready
    std::thread t(std::move(task), 5);
    std::cout << "Task now executing in thread" << std::endl;
    
    std::cout << "Result: " << future2.get() << std::endl;
    t.join();
    
    return 0;
}
```

## Key Features and Characteristics

**Type Safety**: The template parameters specify the exact function signature, providing compile-time type checking.

**Move-Only Semantics**: `std::packaged_task` cannot be copied, only moved, ensuring exclusive ownership of the shared state.

**Exception Propagation**: Any exception thrown during task execution is captured and propagated through the future when `get()` is called.

**Delayed Execution**: Unlike `std::async`, you control exactly when and where the task executes.

**Single Execution**: By default, a packaged task can only be executed once. Use `reset()` to prepare it for another execution.

## Common Use Cases

1. **Thread Pools**: Store packaged tasks in a queue for worker threads to execute
2. **Custom Scheduling**: Execute tasks based on priority, timing, or other criteria
3. **Task Pipelines**: Chain multiple asynchronous operations with explicit control
4. **Testing**: Mock asynchronous behavior in unit tests
5. **Resource Management**: Control when expensive operations run based on resource availability

## Advantages Over std::async

- Greater control over thread creation and management
- Ability to store tasks before execution
- Better integration with custom thread pools
- More explicit about execution timing
- Can be used with existing thread management infrastructure

## Important Considerations

- Always call `get_future()` before moving or executing the task
- A future can only be retrieved once per task (unless reset)
- The task must be executed exactly once before the future is destroyed (unless reset or moved)
- Tasks are move-only; plan your ownership transfer carefully
- Remember to join or detach threads you create

## Summary

`std::packaged_task` provides a flexible mechanism for wrapping callable objects and executing them asynchronously while maintaining clean separation between task creation, execution, and result retrieval. It offers more control than `std::async` for scenarios requiring custom scheduling, thread pool integration, or delayed execution. The packaged task pattern is particularly valuable in building sophisticated concurrent systems where you need fine-grained control over when and how asynchronous work is performed. By combining packaged tasks with futures, you can build robust, type-safe asynchronous workflows that handle both results and exceptions cleanly across thread boundaries.