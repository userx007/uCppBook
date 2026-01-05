# Guide on C++ futures, promises, and async

**Core Concepts:**
- **`std::future<T>`** - A placeholder for a value that will be computed asynchronously
- **`std::promise<T>`** - Allows manual control over setting a future's value from another thread
- **`std::async`** - High-level function to launch asynchronous tasks easily

**Key Examples Include:**

1. **Basic async operations** - Simple asynchronous function calls
2. **Launch policies** - Difference between `async`, `deferred`, and default behavior
3. **Promise-future pairs** - Manual thread communication
4. **Exception handling** - How exceptions propagate through futures
5. **Status checking** - Non-blocking checks with `wait_for()`
6. **Parallel computation** - Real-world example of parallel summation
7. **Shared futures** - Multiple threads reading the same result
8. **Practical application** - Async file processing simulation

**Important Points:**
- `future.get()` blocks until the value is ready and can only be called once
- Exceptions thrown in async tasks are stored and re-thrown when `get()` is called
- `std::async` destructors block until completion (important for RAII)
- `std::shared_future` allows multiple threads to access the same result

The code compiles with C++11 and later. You can run it to see all examples in action!

```cpp

/*
===============================================================================
C++ FUTURES, PROMISES, AND ASYNC - COMPREHENSIVE GUIDE
===============================================================================

OVERVIEW:
---------
Futures, promises, and async are part of C++11's concurrency library (<future>)
that enable asynchronous programming and communication between threads.

KEY CONCEPTS:
1. std::future<T>  - Represents a value that will be available in the future
2. std::promise<T> - Provides a way to set a value that can be read by a future
3. std::async      - Launches asynchronous tasks and returns a future

===============================================================================
*/

#include <iostream>
#include <future>
#include <thread>
#include <chrono>
#include <vector>
#include <numeric>
#include <stdexcept>

// ============================================================================
// 1. BASIC std::async USAGE
// ============================================================================

// Simple function that takes time to execute
int expensiveComputation(int x) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return x * x;
}

void example_basic_async() {
    std::cout << "\n=== Example 1: Basic std::async ===\n";
    
    // Launch async task
    std::future<int> result = std::async(std::launch::async, 
                                         expensiveComputation, 10);
    
    std::cout << "Task launched, doing other work...\n";
    
    // Do other work while computation runs
    for(int i = 0; i < 5; i++) {
        std::cout << "Working... " << i << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(400));
    }
    
    // Get the result (blocks if not ready)
    int value = result.get();
    std::cout << "Result: " << value << "\n";
}

// ============================================================================
// 2. std::async LAUNCH POLICIES
// ============================================================================

void example_launch_policies() {
    std::cout << "\n=== Example 2: Launch Policies ===\n";
    
    // std::launch::async - guaranteed to run in a new thread
    auto future1 = std::async(std::launch::async, []() {
        std::cout << "Running in separate thread\n";
        return 42;
    });
    
    // std::launch::deferred - lazy evaluation, runs when get() is called
    auto future2 = std::async(std::launch::deferred, []() {
        std::cout << "Running in calling thread (deferred)\n";
        return 100;
    });
    
    std::cout << "Before get() calls\n";
    std::cout << "Result 1: " << future1.get() << "\n";
    std::cout << "Result 2: " << future2.get() << "\n";
    
    // std::launch::async | std::launch::deferred (default)
    // Implementation decides whether to create a new thread
    auto future3 = std::async([]() { return 200; });
}

// ============================================================================
// 3. PROMISE AND FUTURE COMMUNICATION
// ============================================================================

void example_promise_future() {
    std::cout << "\n=== Example 3: Promise-Future Communication ===\n";
    
    // Create a promise
    std::promise<int> prom;
    
    // Get the associated future
    std::future<int> fut = prom.get_future();
    
    // Launch a thread that will set the promise value
    std::thread t([](std::promise<int> p) {
        std::cout << "Thread working...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        // Set the value
        p.set_value(42);
        std::cout << "Promise value set\n";
    }, std::move(prom));
    
    std::cout << "Waiting for result...\n";
    
    // Wait for the result
    int result = fut.get();
    std::cout << "Received: " << result << "\n";
    
    t.join();
}

// ============================================================================
// 4. EXCEPTION HANDLING WITH FUTURES
// ============================================================================

int riskyOperation(int x) {
    if (x < 0) {
        throw std::invalid_argument("Negative input not allowed");
    }
    return x * 2;
}

void example_exception_handling() {
    std::cout << "\n=== Example 4: Exception Handling ===\n";
    
    // Example with async
    auto future1 = std::async(std::launch::async, riskyOperation, 10);
    auto future2 = std::async(std::launch::async, riskyOperation, -5);
    
    try {
        std::cout << "Result 1: " << future1.get() << "\n";
    } catch(const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    
    try {
        std::cout << "Result 2: " << future2.get() << "\n";
    } catch(const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
    
    // Example with promise
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread t([](std::promise<int> p) {
        try {
            throw std::runtime_error("Something went wrong!");
        } catch(...) {
            // Set exception instead of value
            p.set_exception(std::current_exception());
        }
    }, std::move(prom));
    
    try {
        fut.get();
    } catch(const std::exception& e) {
        std::cout << "Caught from promise: " << e.what() << "\n";
    }
    
    t.join();
}

// ============================================================================
// 5. CHECKING FUTURE STATUS
// ============================================================================

void example_future_status() {
    std::cout << "\n=== Example 5: Future Status ===\n";
    
    auto future = std::async(std::launch::async, []() {
        std::this_thread::sleep_for(std::chrono::seconds(2));
        return 42;
    });
    
    // Check if future is ready without blocking
    while (true) {
        std::future_status status = future.wait_for(std::chrono::milliseconds(500));
        
        if (status == std::future_status::ready) {
            std::cout << "Result is ready!\n";
            std::cout << "Value: " << future.get() << "\n";
            break;
        } else if (status == std::future_status::timeout) {
            std::cout << "Still waiting...\n";
        } else if (status == std::future_status::deferred) {
            std::cout << "Task is deferred\n";
        }
    }
}

// ============================================================================
// 6. PARALLEL COMPUTATION EXAMPLE
// ============================================================================

long long sumRange(long long start, long long end) {
    long long sum = 0;
    for (long long i = start; i < end; i++) {
        sum += i;
    }
    return sum;
}

void example_parallel_sum() {
    std::cout << "\n=== Example 6: Parallel Computation ===\n";
    
    const long long N = 1000000000;
    const int NUM_THREADS = 4;
    const long long chunk = N / NUM_THREADS;
    
    auto start_time = std::chrono::high_resolution_clock::now();
    
    // Launch multiple async tasks
    std::vector<std::future<long long>> futures;
    
    for (int i = 0; i < NUM_THREADS; i++) {
        long long start = i * chunk;
        long long end = (i == NUM_THREADS - 1) ? N : (i + 1) * chunk;
        
        futures.push_back(std::async(std::launch::async, sumRange, start, end));
    }
    
    // Collect results
    long long total = 0;
    for (auto& fut : futures) {
        total += fut.get();
    }
    
    auto end_time = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        end_time - start_time);
    
    std::cout << "Total sum: " << total << "\n";
    std::cout << "Time taken: " << duration.count() << " ms\n";
}

// ============================================================================
// 7. SHARED FUTURE
// ============================================================================

void example_shared_future() {
    std::cout << "\n=== Example 7: Shared Future ===\n";
    
    // Regular future can only be get() once
    std::promise<int> prom;
    std::shared_future<int> shared_fut = prom.get_future().share();
    
    // Multiple threads can access the same shared_future
    auto worker = [](std::shared_future<int> fut, int id) {
        int value = fut.get(); // Can be called multiple times
        std::cout << "Thread " << id << " received: " << value << "\n";
    };
    
    std::thread t1(worker, shared_fut, 1);
    std::thread t2(worker, shared_fut, 2);
    std::thread t3(worker, shared_fut, 3);
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    prom.set_value(100);
    
    t1.join();
    t2.join();
    t3.join();
}

// ============================================================================
// 8. PRACTICAL EXAMPLE: ASYNC FILE PROCESSING
// ============================================================================

struct ProcessResult {
    std::string filename;
    int lineCount;
    int wordCount;
};

ProcessResult processFile(const std::string& filename) {
    // Simulate file processing
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    ProcessResult result;
    result.filename = filename;
    result.lineCount = 100 + rand() % 900;
    result.wordCount = 500 + rand() % 4500;
    
    return result;
}

void example_async_file_processing() {
    std::cout << "\n=== Example 8: Async File Processing ===\n";
    
    std::vector<std::string> files = {
        "file1.txt", "file2.txt", "file3.txt", "file4.txt"
    };
    
    // Launch async tasks for each file
    std::vector<std::future<ProcessResult>> futures;
    
    for (const auto& file : files) {
        futures.push_back(std::async(std::launch::async, processFile, file));
    }
    
    // Process results as they become available
    for (auto& fut : futures) {
        ProcessResult result = fut.get();
        std::cout << "File: " << result.filename 
                  << " | Lines: " << result.lineCount 
                  << " | Words: " << result.wordCount << "\n";
    }
}

// ============================================================================
// MAIN - RUN ALL EXAMPLES
// ============================================================================

int main() {
    std::cout << "C++ FUTURES, PROMISES, AND ASYNC EXAMPLES\n";
    std::cout << "==========================================\n";
    
    example_basic_async();
    example_launch_policies();
    example_promise_future();
    example_exception_handling();
    example_future_status();
    example_parallel_sum();
    example_shared_future();
    example_async_file_processing();
    
    std::cout << "\n=== All examples completed ===\n";
    
    return 0;
}

/*
===============================================================================
KEY TAKEAWAYS:
===============================================================================

1. std::async:
   - Easiest way to run tasks asynchronously
   - Automatically manages thread creation
   - Returns std::future<T> for result retrieval

2. std::promise:
   - Manual control over setting future values
   - Useful for custom threading scenarios
   - Can set either value or exception

3. std::future:
   - Represents a value that will be available later
   - get() retrieves the value (blocking if not ready)
   - Can only be get() once (use shared_future for multiple access)
   - wait() and wait_for() for checking status

4. Best Practices:
   - Use std::async for simple async operations
   - Use promise/future for complex inter-thread communication
   - Always handle exceptions from future.get()
   - Check future status before blocking with get()
   - Use shared_future when multiple threads need the same result

5. Common Pitfalls:
   - Calling get() twice on same future (undefined behavior)
   - Not calling get() on async (destructor blocks)
   - Forgetting to check for exceptions
   - Creating too many threads (consider thread pools)

===============================================================================
*/

```
