# Future and Promise Basics

## Introduction

`std::future` and `std::promise` are C++ concurrency primitives introduced in C++11 that enable asynchronous communication between threads. They provide a mechanism for one thread to pass a value or exception to another thread at some point in the future, establishing a channel for one-time data transfer between producer and consumer threads.

## Core Concepts

### std::promise

A `std::promise` is a write-end channel that allows a thread to store a value (or exception) that will be retrieved asynchronously by another thread. Think of it as a container that promises to hold a value eventually.

Key characteristics:
- Can be set exactly once with either a value or an exception
- Thread-safe for setting the value
- Associated with a `std::future` for retrieving the result
- Destroying a promise without setting a value breaks the promise (causes `std::future_error`)

### std::future

A `std::future` is the read-end channel that provides access to the value stored by a `std::promise`. It represents a value that will become available in the future.

Key characteristics:
- Can be retrieved from exactly once (moved on retrieval)
- Blocks when calling `get()` until the value is available
- Can check readiness with `wait_for()` or `wait_until()`
- Automatically receives exceptions thrown in the promise

## Basic Usage Pattern

The typical workflow involves:
1. Create a `std::promise<T>` object
2. Obtain a `std::future<T>` from the promise using `get_future()`
3. Pass the promise to a worker thread
4. The worker thread sets the value using `set_value()` or `set_exception()`
5. The main thread retrieves the value using `future.get()`

## Code Examples

### Example 1: Basic Future and Promise

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

void calculateSquare(std::promise<int> resultPromise, int number) {
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Calculate and set the result
    int result = number * number;
    resultPromise.set_value(result);
}

int main() {
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    
    // Launch thread with the promise
    std::thread worker(calculateSquare, std::move(promise), 5);
    
    std::cout << "Calculating square...\n";
    
    // Wait for and retrieve the result
    int result = future.get();  // Blocks until value is available
    
    std::cout << "Result: " << result << std::endl;
    
    worker.join();
    
    return 0;
}
```

### Example 2: Checking Future Status

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <chrono>

void slowComputation(std::promise<double> p) {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    p.set_value(3.14159);
}

int main() {
    std::promise<double> promise;
    std::future<double> future = promise.get_future();
    
    std::thread t(slowComputation, std::move(promise));
    
    // Check if result is ready without blocking
    while (future.wait_for(std::chrono::milliseconds(500)) 
           != std::future_status::ready) {
        std::cout << "Still waiting...\n";
    }
    
    std::cout << "Result is ready: " << future.get() << std::endl;
    
    t.join();
    return 0;
}
```

### Example 3: Exception Handling

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <stdexcept>

void riskyOperation(std::promise<int> p, int value) {
    try {
        if (value < 0) {
            throw std::runtime_error("Negative values not allowed");
        }
        p.set_value(value * 2);
    } catch (...) {
        // Propagate exception to the future
        p.set_exception(std::current_exception());
    }
}

int main() {
    std::promise<int> promise;
    std::future<int> future = promise.get_future();
    
    std::thread t(riskyOperation, std::move(promise), -5);
    
    try {
        int result = future.get();  // Will rethrow the exception
        std::cout << "Result: " << result << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    t.join();
    return 0;
}
```

### Example 4: Multiple Futures with std::async

```cpp
#include <iostream>
#include <future>
#include <vector>
#include <numeric>

int computeSum(const std::vector<int>& data, size_t start, size_t end) {
    return std::accumulate(data.begin() + start, data.begin() + end, 0);
}

int main() {
    std::vector<int> numbers(1000);
    std::iota(numbers.begin(), numbers.end(), 1);  // Fill with 1, 2, 3, ...
    
    // Launch multiple async tasks
    std::future<int> fut1 = std::async(std::launch::async, 
                                       computeSum, std::ref(numbers), 0, 250);
    std::future<int> fut2 = std::async(std::launch::async, 
                                       computeSum, std::ref(numbers), 250, 500);
    std::future<int> fut3 = std::async(std::launch::async, 
                                       computeSum, std::ref(numbers), 500, 750);
    std::future<int> fut4 = std::async(std::launch::async, 
                                       computeSum, std::ref(numbers), 750, 1000);
    
    // Collect results
    int total = fut1.get() + fut2.get() + fut3.get() + fut4.get();
    
    std::cout << "Total sum: " << total << std::endl;
    
    return 0;
}
```

### Example 5: Shared Future

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <vector>

void waitForSignal(std::shared_future<void> signal, int id) {
    signal.get();  // Wait for the signal
    std::cout << "Thread " << id << " proceeding after signal\n";
}

int main() {
    std::promise<void> signalPromise;
    std::shared_future<void> signal = signalPromise.get_future().share();
    
    // Launch multiple threads waiting on the same signal
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(waitForSignal, signal, i);
    }
    
    std::cout << "Sleeping before sending signal...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "Sending signal to all threads\n";
    signalPromise.set_value();  // Unblock all waiting threads
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 6: Returning Complex Objects

```cpp
#include <iostream>
#include <thread>
#include <future>
#include <string>
#include <vector>

struct DataPacket {
    std::string name;
    std::vector<int> values;
    double average;
};

void processData(std::promise<DataPacket> p, std::string name, 
                 std::vector<int> data) {
    DataPacket packet;
    packet.name = name;
    packet.values = data;
    
    // Calculate average
    double sum = 0;
    for (int val : data) {
        sum += val;
    }
    packet.average = sum / data.size();
    
    p.set_value(std::move(packet));
}

int main() {
    std::promise<DataPacket> promise;
    std::future<DataPacket> future = promise.get_future();
    
    std::vector<int> data = {10, 20, 30, 40, 50};
    std::thread t(processData, std::move(promise), "Sample Data", data);
    
    DataPacket result = future.get();
    
    std::cout << "Data name: " << result.name << std::endl;
    std::cout << "Average: " << result.average << std::endl;
    std::cout << "Values: ";
    for (int val : result.values) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
    
    t.join();
    return 0;
}
```

## Important Considerations

**Single-Use Nature**: Both `std::future` and `std::promise` are designed for one-time use. Once `get()` is called on a future, it cannot be called again. For multiple retrievals, use `std::shared_future`.

**Broken Promise**: If a promise is destroyed without having its value set, any future waiting on it will throw a `std::future_error` with the error code `std::future_errc::broken_promise`.

**Thread Safety**: While the promise-future mechanism itself is thread-safe, the value being passed must be safely transferable between threads.

**Move Semantics**: Promises and futures are move-only types and cannot be copied, ensuring single ownership of the communication channel.

**Blocking Behavior**: Calling `get()` on a future blocks the calling thread until the value is available, which can lead to deadlocks if not carefully managed.

## Summary

`std::future` and `std::promise` provide a clean, type-safe mechanism for asynchronous value passing between threads in C++. The promise acts as a write-once channel that a producer thread uses to deliver a value, while the future serves as the corresponding read-end that a consumer thread uses to retrieve that value. This pattern is particularly useful for returning results from worker threads, implementing task-based parallelism, and coordinating complex multi-threaded workflows. While they offer simplicity and exception propagation, developers must be mindful of their single-use nature and blocking behavior to avoid common pitfalls like broken promises and deadlocks.