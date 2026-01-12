# Exception Handling Across Threads

Exception handling in concurrent C++ programs presents unique challenges because exceptions cannot directly cross thread boundaries. When an exception is thrown in one thread, it remains confined to that thread's execution context. However, C++ provides mechanisms to capture, transport, and rethrow exceptions across threads, primarily through `std::promise`, `std::future`, and `std::exception_ptr`.

## The Problem with Traditional Exception Handling

In single-threaded programs, exceptions propagate up the call stack until caught by an appropriate handler. In multi-threaded programs, each thread has its own stack, making traditional exception propagation impossible across thread boundaries. If an exception escapes a thread's entry function without being caught, `std::terminate()` is called, abruptly ending the program.

```cpp
#include <iostream>
#include <thread>
#include <stdexcept>

void dangerous_function() {
    throw std::runtime_error("Error in worker thread");
}

void unsafe_thread_example() {
    std::thread t(dangerous_function);
    
    try {
        t.join();
    } catch (const std::exception& e) {
        // This will NEVER catch the exception from the thread
        std::cout << "Caught: " << e.what() << std::endl;
    }
    // Program terminates because exception wasn't caught in the thread
}
```

## Using std::promise and std::future for Exception Propagation

The `std::promise` and `std::future` pair provides the standard mechanism for transferring exceptions between threads. When a promise's associated task throws an exception, that exception is automatically stored in the shared state and rethrown when `future::get()` is called.

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <stdexcept>
#include <string>

int divide(int a, int b) {
    if (b == 0) {
        throw std::invalid_argument("Division by zero");
    }
    return a / b;
}

void worker_with_promise(std::promise<int> prom, int a, int b) {
    try {
        int result = divide(a, b);
        prom.set_value(result);
    } catch (...) {
        // Capture and store any exception in the promise
        prom.set_exception(std::current_exception());
    }
}

void promise_future_example() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    
    std::thread t(worker_with_promise, std::move(prom), 10, 0);
    
    try {
        // Exception will be rethrown here
        int result = fut.get();
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    t.join();
}
```

## Using std::async for Automatic Exception Handling

`std::async` simplifies exception handling by automatically capturing exceptions and storing them in the returned future. This eliminates the need for manual try-catch blocks in the worker function.

```cpp
#include <iostream>
#include <future>
#include <stdexcept>
#include <vector>

double compute_risky_operation(int value) {
    if (value < 0) {
        throw std::domain_error("Negative values not allowed");
    }
    if (value > 1000) {
        throw std::overflow_error("Value too large");
    }
    return 1.0 / value;
}

void async_exception_example() {
    std::vector<std::future<double>> futures;
    
    // Launch multiple async tasks
    for (int i : {10, -5, 2000, 50}) {
        futures.push_back(std::async(std::launch::async, 
                                    compute_risky_operation, i));
    }
    
    // Retrieve results and handle exceptions
    for (size_t i = 0; i < futures.size(); ++i) {
        try {
            double result = futures[i].get();
            std::cout << "Result " << i << ": " << result << std::endl;
        } catch (const std::domain_error& e) {
            std::cout << "Domain error in task " << i << ": " 
                     << e.what() << std::endl;
        } catch (const std::overflow_error& e) {
            std::cout << "Overflow error in task " << i << ": " 
                     << e.what() << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Error in task " << i << ": " 
                     << e.what() << std::endl;
        }
    }
}
```

## Using std::exception_ptr for Advanced Exception Management

`std::exception_ptr` provides a way to store and transport exceptions as objects. This is useful when you need to delay exception handling, store exceptions in containers, or implement custom error propagation mechanisms.

```cpp
#include <iostream>
#include <thread>
#include <exception>
#include <vector>
#include <stdexcept>

class ThreadPool {
private:
    std::vector<std::exception_ptr> exceptions;
    std::mutex exceptions_mutex;
    
public:
    template<typename Func>
    void execute(Func func) {
        std::thread([this, func]() {
            try {
                func();
            } catch (...) {
                std::lock_guard<std::mutex> lock(exceptions_mutex);
                exceptions.push_back(std::current_exception());
            }
        }).detach();
    }
    
    void check_exceptions() {
        std::lock_guard<std::mutex> lock(exceptions_mutex);
        for (auto& eptr : exceptions) {
            if (eptr) {
                std::rethrow_exception(eptr);
            }
        }
        exceptions.clear();
    }
};

void exception_ptr_example() {
    ThreadPool pool;
    
    pool.execute([]() {
        throw std::runtime_error("Error in task 1");
    });
    
    pool.execute([]() {
        std::cout << "Task 2 completed successfully" << std::endl;
    });
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    try {
        pool.check_exceptions();
    } catch (const std::exception& e) {
        std::cout << "Caught stored exception: " << e.what() << std::endl;
    }
}
```

## Handling Multiple Exceptions from Parallel Tasks

When running multiple parallel tasks, you may need to collect and handle exceptions from all tasks before proceeding. This pattern ensures comprehensive error handling across concurrent operations.

```cpp
#include <iostream>
#include <future>
#include <vector>
#include <string>

struct TaskResult {
    int task_id;
    bool success;
    std::string result_or_error;
};

TaskResult process_data(int id, const std::string& data) {
    if (data.empty()) {
        throw std::invalid_argument("Empty data");
    }
    if (data.length() > 100) {
        throw std::length_error("Data too long");
    }
    return {id, true, "Processed: " + data};
}

void multiple_exception_handling() {
    std::vector<std::string> data_items = {
        "valid data",
        "",  // Will cause exception
        "another valid item",
        std::string(150, 'x')  // Will cause exception
    };
    
    std::vector<std::future<TaskResult>> futures;
    
    // Launch all tasks
    for (size_t i = 0; i < data_items.size(); ++i) {
        futures.push_back(std::async(std::launch::async, 
                                    process_data, i, data_items[i]));
    }
    
    // Collect all results and exceptions
    std::vector<TaskResult> results;
    for (auto& fut : futures) {
        try {
            results.push_back(fut.get());
        } catch (const std::exception& e) {
            results.push_back({-1, false, 
                             std::string("Exception: ") + e.what()});
        }
    }
    
    // Process all results
    for (const auto& result : results) {
        if (result.success) {
            std::cout << "Task " << result.task_id << " succeeded: " 
                     << result.result_or_error << std::endl;
        } else {
            std::cout << "Task failed: " << result.result_or_error << std::endl;
        }
    }
}
```

## Custom Exception Transport Wrapper

For complex applications, you might want to create a wrapper that provides more context about where and when exceptions occurred in your concurrent system.

```cpp
#include <iostream>
#include <future>
#include <chrono>
#include <sstream>

struct ExceptionContext {
    std::exception_ptr exception;
    std::string thread_id;
    std::chrono::system_clock::time_point timestamp;
    std::string function_name;
    
    void rethrow_with_context() const {
        if (exception) {
            std::ostringstream oss;
            oss << "Exception in thread " << thread_id 
                << " from " << function_name;
            std::cout << oss.str() << std::endl;
            std::rethrow_exception(exception);
        }
    }
};

template<typename Func, typename... Args>
auto safe_async(const std::string& func_name, Func&& func, Args&&... args) {
    return std::async(std::launch::async, [func_name, func, args...]() {
        try {
            return func(args...);
        } catch (...) {
            ExceptionContext ctx{
                std::current_exception(),
                std::to_string(std::hash<std::thread::id>{}(
                    std::this_thread::get_id())),
                std::chrono::system_clock::now(),
                func_name
            };
            throw ctx;
        }
    });
}

void custom_wrapper_example() {
    auto fut = safe_async("compute_task", [](int x) -> int {
        if (x == 0) throw std::runtime_error("Zero not allowed");
        return 100 / x;
    }, 0);
    
    try {
        int result = fut.get();
        std::cout << "Result: " << result << std::endl;
    } catch (const ExceptionContext& ctx) {
        ctx.rethrow_with_context();
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << std::endl;
    }
}
```

## Summary

Exception handling across threads in C++ requires careful consideration because exceptions cannot naturally cross thread boundaries. The key mechanisms for handling async errors include:

**std::promise/std::future**: Provides explicit control over exception propagation. Exceptions are captured with `set_exception()` and automatically rethrown when `get()` is called on the future.

**std::async**: Automatically captures exceptions from the launched task and stores them in the returned future, simplifying error handling for asynchronous operations.

**std::exception_ptr**: Enables storing, copying, and rethrowing exceptions as objects, useful for implementing custom error propagation schemes or collecting exceptions from multiple threads.

**Best Practices**: Always protect thread entry functions with try-catch blocks when not using futures. Call `future::get()` within try-catch blocks to handle propagated exceptions. Consider collecting exceptions from multiple parallel tasks before handling them. Provide meaningful context when rethrowing exceptions from worker threads.

The combination of these mechanisms allows you to build robust concurrent applications that handle errors gracefully across thread boundaries, preventing unexpected program termination and enabling proper error recovery strategies.