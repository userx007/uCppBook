# Async Task Launching in C++

## Introduction

`std::async` is a high-level concurrency facility introduced in C++11 that provides a convenient way to launch asynchronous tasks and retrieve their results through `std::future` objects. It abstracts away much of the complexity of thread management while offering flexibility through launch policies that control how and when tasks execute.

Unlike manually creating threads with `std::thread`, `std::async` automatically manages thread lifecycle, handles exceptions across thread boundaries, and provides a clean mechanism for returning values from asynchronous operations.

## Launch Policies

`std::async` accepts launch policies that determine the execution strategy for the submitted task:

**`std::launch::async`**
- Forces the task to run asynchronously on a separate thread
- Execution begins immediately (or as soon as resources are available)
- Guarantees parallel execution

**`std::launch::deferred`**
- Delays task execution until the result is explicitly requested
- The task runs synchronously on the thread that calls `get()` or `wait()` on the future
- No new thread is created
- Useful for lazy evaluation

**`std::launch::async | std::launch::deferred` (default)**
- Allows the implementation to choose between async and deferred execution
- Provides maximum flexibility but less predictability
- The standard library can optimize based on system resources

## Basic Usage

Here's a comprehensive example demonstrating different launch policies:

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <string>

// Simple computation function
int calculate_sum(int a, int b) {
    std::cout << "Calculating on thread: " 
              << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return a + b;
}

// More complex task returning a string
std::string fetch_data(const std::string& source) {
    std::cout << "Fetching from " << source 
              << " on thread: " << std::this_thread::get_id() << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    return "Data from " + source;
}

int main() {
    std::cout << "Main thread: " << std::this_thread::get_id() << std::endl;
    
    // 1. Async launch policy - guaranteed to run on separate thread
    std::cout << "\n--- Async Launch ---" << std::endl;
    auto future_async = std::async(std::launch::async, calculate_sum, 10, 20);
    std::cout << "Task launched asynchronously" << std::endl;
    std::cout << "Result: " << future_async.get() << std::endl;
    
    // 2. Deferred launch policy - runs when result is requested
    std::cout << "\n--- Deferred Launch ---" << std::endl;
    auto future_deferred = std::async(std::launch::deferred, calculate_sum, 5, 15);
    std::cout << "Task created but not yet executed" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Now requesting result..." << std::endl;
    std::cout << "Result: " << future_deferred.get() << std::endl;
    
    // 3. Default policy (implementation-defined)
    std::cout << "\n--- Default Launch Policy ---" << std::endl;
    auto future_default = std::async(fetch_data, "server");
    std::cout << "Task launched with default policy" << std::endl;
    std::cout << "Result: " << future_default.get() << std::endl;
    
    return 0;
}
```

## Practical Example: Parallel Data Processing

Here's a realistic example that demonstrates using `std::async` for parallel data processing:

```cpp
#include <iostream>
#include <future>
#include <vector>
#include <numeric>
#include <algorithm>
#include <chrono>

// Simulate expensive computation on a data chunk
double process_chunk(const std::vector<int>& data, size_t start, size_t end) {
    double sum = 0.0;
    for (size_t i = start; i < end; ++i) {
        // Simulate complex calculation
        sum += data[i] * data[i] * 0.5;
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
    return sum;
}

// Process data in parallel using multiple async tasks
double parallel_process(const std::vector<int>& data, size_t num_threads) {
    size_t chunk_size = data.size() / num_threads;
    std::vector<std::future<double>> futures;
    
    // Launch async tasks for each chunk
    for (size_t i = 0; i < num_threads; ++i) {
        size_t start = i * chunk_size;
        size_t end = (i == num_threads - 1) ? data.size() : (i + 1) * chunk_size;
        
        futures.push_back(
            std::async(std::launch::async, process_chunk, 
                      std::cref(data), start, end)
        );
    }
    
    // Collect results from all tasks
    double total = 0.0;
    for (auto& future : futures) {
        total += future.get();
    }
    
    return total;
}

int main() {
    // Create test data
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 1); // Fill with 1, 2, 3, ...
    
    // Sequential processing
    auto start = std::chrono::high_resolution_clock::now();
    double sequential_result = process_chunk(data, 0, data.size());
    auto end = std::chrono::high_resolution_clock::now();
    auto sequential_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential result: " << sequential_result 
              << " (Time: " << sequential_time.count() << "ms)" << std::endl;
    
    // Parallel processing
    start = std::chrono::high_resolution_clock::now();
    double parallel_result = parallel_process(data, 4);
    end = std::chrono::high_resolution_clock::now();
    auto parallel_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Parallel result: " << parallel_result 
              << " (Time: " << parallel_time.count() << "ms)" << std::endl;
    std::cout << "Speedup: " << static_cast<double>(sequential_time.count()) / parallel_time.count() 
              << "x" << std::endl;
    
    return 0;
}
```

## Advanced Example: Task Pipeline with Error Handling

This example shows exception handling and chaining async operations:

```cpp
#include <iostream>
#include <future>
#include <vector>
#include <string>
#include <stdexcept>
#include <chrono>

// Simulated stages of a data pipeline
std::vector<int> load_data(const std::string& filename) {
    std::cout << "Loading data from " << filename << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(300));
    
    if (filename.empty()) {
        throw std::runtime_error("Invalid filename");
    }
    
    return {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
}

std::vector<int> transform_data(const std::vector<int>& input) {
    std::cout << "Transforming data" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(400));
    
    std::vector<int> result;
    for (int val : input) {
        result.push_back(val * 2);
    }
    return result;
}

int aggregate_data(const std::vector<int>& input) {
    std::cout << "Aggregating data" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    
    int sum = 0;
    for (int val : input) {
        sum += val;
    }
    return sum;
}

int main() {
    try {
        // Launch pipeline stages asynchronously
        auto load_future = std::async(std::launch::async, load_data, "data.txt");
        
        // Wait for loading to complete, then transform
        auto loaded = load_future.get();
        auto transform_future = std::async(std::launch::async, transform_data, loaded);
        
        // Wait for transformation, then aggregate
        auto transformed = transform_future.get();
        auto aggregate_future = std::async(std::launch::async, aggregate_data, transformed);
        
        // Get final result
        int result = aggregate_future.get();
        std::cout << "Final result: " << result << std::endl;
        
    } catch (const std::exception& e) {
        std::cerr << "Pipeline error: " << e.what() << std::endl;
        return 1;
    }
    
    // Example with parallel independent tasks
    std::cout << "\n--- Parallel Independent Tasks ---" << std::endl;
    
    auto task1 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 100;
    });
    
    auto task2 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 200;
    });
    
    auto task3 = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        return 300;
    });
    
    // All tasks run in parallel; collect results
    int total = task1.get() + task2.get() + task3.get();
    std::cout << "Combined result: " << total << std::endl;
    
    return 0;
}
```

## Deferred Execution Example

Demonstrating the power of lazy evaluation with deferred execution:

```cpp
#include <iostream>
#include <future>
#include <chrono>

int expensive_calculation(int x) {
    std::cout << "Performing expensive calculation for " << x << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    return x * x;
}

int main() {
    std::cout << "Creating deferred tasks..." << std::endl;
    
    // Create multiple deferred tasks - none execute yet
    auto deferred1 = std::async(std::launch::deferred, expensive_calculation, 5);
    auto deferred2 = std::async(std::launch::deferred, expensive_calculation, 10);
    auto deferred3 = std::async(std::launch::deferred, expensive_calculation, 15);
    
    std::cout << "All tasks created (but not executed)" << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    // Conditional execution - only execute what we need
    bool need_first = true;
    bool need_second = false;
    
    if (need_first) {
        std::cout << "\nRequesting first result..." << std::endl;
        std::cout << "Result 1: " << deferred1.get() << std::endl;
    }
    
    if (need_second) {
        std::cout << "\nRequesting second result..." << std::endl;
        std::cout << "Result 2: " << deferred2.get() << std::endl;
    }
    
    std::cout << "\nTask 3 was never executed (never needed)" << std::endl;
    // deferred3 is destroyed without ever executing
    
    return 0;
}
```

## Important Considerations

**Return Value and Exceptions**: `std::async` returns a `std::future` object that holds the eventual result. If the task throws an exception, it's stored in the future and rethrown when `get()` is called.

**Blocking Behavior**: Calling `get()` on a future blocks until the result is available. You can call `wait_for()` or `wait_until()` for timed waiting, or use `valid()` to check if the future has a shared state.

**Destructor Blocking**: The destructor of a future returned by `std::async` blocks until the task completes. This ensures tasks aren't abandoned but can cause unexpected blocking if futures aren't properly managed.

**Thread Pool Absence**: The C++ standard doesn't mandate a thread pool implementation, so `std::async` might create a new thread for each task, which can be expensive for many small tasks.

**Launch Policy Selection**: When using the default policy, be aware that the implementation may choose deferred execution, which means no parallelism occurs. For guaranteed parallelism, explicitly specify `std::launch::async`.

## Summary

`std::async` provides a powerful, high-level abstraction for asynchronous task execution in C++. The launch policies offer flexibility, with `std::launch::async` guaranteeing parallel execution on a separate thread and `std::launch::deferred` enabling lazy evaluation where tasks execute synchronously only when results are requested. The default policy gives the implementation freedom to optimize based on system resources.

The key advantages include automatic thread management, clean exception propagation across thread boundaries, and a simple interface for returning values through `std::future` objects. This makes `std::async` ideal for scenarios like parallel data processing, I/O-bound operations, and computational tasks that can benefit from concurrent execution. However, developers should be mindful of destructor blocking behavior, the absence of a standardized thread pool, and the importance of explicitly specifying launch policies when predictable behavior is required. When used appropriately, `std::async` significantly simplifies concurrent programming while maintaining type safety and exception safety.