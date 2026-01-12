# Shared Futures in C++

## Overview

`std::shared_future` is a C++ concurrency primitive that allows multiple threads to wait for and access the same asynchronous result. Unlike `std::future`, which can only be accessed by a single thread (it's move-only), `std::shared_future` can be copied and shared among multiple threads, enabling a broadcast pattern where one operation's result is consumed by many waiters.

## Key Concepts

### std::future vs std::shared_future

- **std::future**: Move-only, single consumer. Once you call `get()`, the future is consumed and cannot be reused.
- **std::shared_future**: Copyable, multiple consumers. All copies share the same shared state, and `get()` can be called multiple times, returning a const reference to the stored value.

### Creating a Shared Future

You can create a `std::shared_future` in two main ways:

1. Calling `share()` on a `std::future`
2. Using `std::async` or `std::packaged_task` and explicitly converting to `std::shared_future`

## Code Examples

### Example 1: Basic Shared Future Usage

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <chrono>

// Function that performs a computation
int expensive_computation() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Computation complete!\n";
    return 42;
}

void worker(int id, std::shared_future<int> sf) {
    std::cout << "Thread " << id << " waiting for result...\n";
    
    // Multiple threads can call get() on the same shared_future
    int result = sf.get();
    
    std::cout << "Thread " << id << " received: " << result << "\n";
}

int main() {
    // Create a future from async
    std::future<int> fut = std::async(std::launch::async, expensive_computation);
    
    // Convert to shared_future
    std::shared_future<int> shared_fut = fut.share();
    
    // Launch multiple threads that all wait for the same result
    std::vector<std::thread> threads;
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i, shared_fut);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 2: Broadcasting Configuration or Initialization Data

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <string>
#include <chrono>

struct Configuration {
    std::string database_url;
    int max_connections;
    bool enable_logging;
};

Configuration load_configuration() {
    std::cout << "Loading configuration...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    return {"postgresql://localhost:5432", 100, true};
}

class Worker {
public:
    Worker(int id, std::shared_future<Configuration> config_future)
        : id_(id), config_future_(config_future) {}
    
    void run() {
        std::cout << "Worker " << id_ << " starting...\n";
        
        // Wait for configuration to be ready
        const Configuration& config = config_future_.get();
        
        std::cout << "Worker " << id_ << " configured with:\n"
                  << "  DB: " << config.database_url << "\n"
                  << "  Max connections: " << config.max_connections << "\n"
                  << "  Logging: " << (config.enable_logging ? "enabled" : "disabled") << "\n";
        
        // Do actual work...
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Worker " << id_ << " finished work\n";
    }

private:
    int id_;
    std::shared_future<Configuration> config_future_;
};

int main() {
    // Start configuration loading asynchronously
    std::shared_future<Configuration> config_future = 
        std::async(std::launch::async, load_configuration).share();
    
    // Create workers that will all wait for the same configuration
    std::vector<std::thread> workers;
    for (int i = 0; i < 3; ++i) {
        workers.emplace_back([i, config_future]() {
            Worker w(i, config_future);
            w.run();
        });
    }
    
    for (auto& w : workers) {
        w.join();
    }
    
    return 0;
}
```

### Example 3: Synchronization Barrier Pattern

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <chrono>
#include <random>

// Shared future used as a starting signal for multiple threads
void racer(int id, std::shared_future<void> start_signal) {
    // Prepare phase
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> prep_time(100, 500);
    
    std::cout << "Racer " << id << " preparing...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(prep_time(gen)));
    std::cout << "Racer " << id << " ready!\n";
    
    // Wait for the starting signal (all racers wait here)
    start_signal.wait();
    
    // Race phase
    std::cout << "Racer " << id << " GO!\n";
    std::uniform_int_distribution<> race_time(1000, 3000);
    std::this_thread::sleep_for(std::chrono::milliseconds(race_time(gen)));
    std::cout << "Racer " << id << " finished!\n";
}

int main() {
    std::promise<void> start_promise;
    std::shared_future<void> start_signal = start_promise.get_future().share();
    
    std::vector<std::thread> racers;
    for (int i = 0; i < 5; ++i) {
        racers.emplace_back(racer, i, start_signal);
    }
    
    std::cout << "Waiting for all racers to be ready...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::cout << "\n=== STARTING THE RACE ===\n\n";
    start_promise.set_value(); // Signal all threads to start
    
    for (auto& r : racers) {
        r.join();
    }
    
    return 0;
}
```

### Example 4: Error Handling with Shared Futures

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <stdexcept>

int risky_operation() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    throw std::runtime_error("Operation failed!");
    return 42;
}

void error_handling_worker(int id, std::shared_future<int> sf) {
    try {
        std::cout << "Worker " << id << " waiting...\n";
        int result = sf.get();
        std::cout << "Worker " << id << " got result: " << result << "\n";
    } catch (const std::exception& e) {
        std::cout << "Worker " << id << " caught exception: " << e.what() << "\n";
    }
}

int main() {
    std::shared_future<int> shared_fut = 
        std::async(std::launch::async, risky_operation).share();
    
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(error_handling_worker, i, shared_fut);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "All workers have handled the exception\n";
    
    return 0;
}
```

### Example 5: Using shared_future with Packaged Tasks

```cpp
#include <iostream>
#include <future>
#include <thread>
#include <vector>
#include <numeric>

std::vector<int> compute_data() {
    std::cout << "Computing large dataset...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    std::vector<int> data(1000);
    std::iota(data.begin(), data.end(), 1);
    return data;
}

void process_data(int id, std::shared_future<std::vector<int>> data_future) {
    std::cout << "Processor " << id << " waiting for data...\n";
    
    const std::vector<int>& data = data_future.get();
    
    // Each thread processes a different part of the shared data
    size_t chunk_size = data.size() / 4;
    size_t start = id * chunk_size;
    size_t end = (id == 3) ? data.size() : start + chunk_size;
    
    long long sum = 0;
    for (size_t i = start; i < end; ++i) {
        sum += data[i];
    }
    
    std::cout << "Processor " << id << " computed sum: " << sum 
              << " for range [" << start << ", " << end << ")\n";
}

int main() {
    // Create a packaged_task
    std::packaged_task<std::vector<int>()> task(compute_data);
    
    // Get the shared_future before moving the task
    std::shared_future<std::vector<int>> data_future = task.get_future().share();
    
    // Run the task in a separate thread
    std::thread task_thread(std::move(task));
    
    // Launch multiple processors that will share the result
    std::vector<std::thread> processors;
    for (int i = 0; i < 4; ++i) {
        processors.emplace_back(process_data, i, data_future);
    }
    
    task_thread.join();
    for (auto& p : processors) {
        p.join();
    }
    
    return 0;
}
```

## Important Considerations

### Thread Safety

- `std::shared_future` itself is thread-safe for copying and for calling member functions from multiple threads.
- The returned value from `get()` is const-qualified to prevent modification races.
- If you need to modify the result, each thread should make its own copy.

### Performance

- Creating multiple copies of `std::shared_future` has some overhead due to reference counting of the shared state.
- All waiting threads will wake up when the value becomes available, which could cause a thundering herd effect.

### Validity

- Always check `valid()` before calling `get()` or `wait()`.
- Once a `std::future` is converted to `std::shared_future` using `share()`, the original future becomes invalid.

### Best Practices

1. Use `std::shared_future` when multiple threads need the same result
2. Pass by value when distributing to threads (copying is cheap due to shared state)
3. Remember that `get()` returns a const reference for shared futures
4. Consider using `std::shared_future<void>` for pure synchronization scenarios

## Summary

`std::shared_future` provides an elegant solution for broadcasting asynchronous results to multiple threads. It extends the promise-future pattern to support multiple consumers, making it ideal for scenarios like distributing configuration data, implementing synchronization barriers, or sharing computed results among worker threads. The key advantage is that a single expensive operation can be performed once, with its result efficiently shared among many waiting threads. This pattern is particularly useful in producer-consumer scenarios where one producer needs to notify multiple consumers, or when initializing multiple components that depend on a shared resource that takes time to prepare.