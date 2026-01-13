# Jthread and Stop Tokens: Cooperative Cancellation in Modern C++

## Introduction

C++20 introduced `std::jthread` (joining thread) and `std::stop_token` as improved alternatives to `std::thread`, addressing two major pain points in C++ concurrency: automatic thread cleanup and cooperative cancellation. While `std::thread` requires explicit calls to `join()` or `detach()` (otherwise terminating the program), `std::jthread` automatically joins in its destructor. More importantly, the stop token mechanism provides a standardized way for threads to cooperatively cancel their work.

## The Problem with std::thread

Traditional `std::thread` has several limitations:

```cpp
#include <thread>
#include <iostream>
#include <chrono>

void worker() {
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Working... " << i << '\n';
    }
}

int main() {
    std::thread t(worker);
    // If we forget to join or detach, program terminates!
    // t.join();  // Must remember to call this
    
    // Also, no standard way to request cancellation
    return 0;
} // Termination if thread not joined!
```

## std::jthread: Automatic Resource Management

`std::jthread` automatically joins on destruction, following RAII principles:

```cpp
#include <thread>
#include <iostream>
#include <chrono>

void simple_worker() {
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        std::cout << "Working... " << i << '\n';
    }
}

int main() {
    {
        std::jthread t(simple_worker);
        // No need to call join() - happens automatically
        std::cout << "Thread created\n";
    } // Automatic join happens here
    
    std::cout << "Thread finished\n";
    return 0;
}
```

## Stop Tokens: Cooperative Cancellation

The real power of `std::jthread` comes from its integration with `std::stop_token`. A stop token allows the parent thread to request cancellation, and the worker thread to check and respond to that request cooperatively.

### Basic Stop Token Usage

```cpp
#include <thread>
#include <iostream>
#include <chrono>

void cancellable_worker(std::stop_token stoken) {
    int count = 0;
    while (!stoken.stop_requested()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        std::cout << "Working... " << count++ << '\n';
        
        if (count >= 20) break; // Safeguard
    }
    
    if (stoken.stop_requested()) {
        std::cout << "Cancellation requested, cleaning up...\n";
    }
}

int main() {
    std::jthread t(cancellable_worker);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    std::cout << "Requesting stop...\n";
    t.request_stop(); // Signal cancellation
    
    // Thread will stop cooperatively and join automatically
    return 0;
}
```

### Stop Source and Multiple Tokens

`std::stop_source` can create multiple `std::stop_token` instances that share the same cancellation state:

```cpp
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>

void worker_with_id(int id, std::stop_token stoken) {
    while (!stoken.stop_requested()) {
        std::cout << "Worker " << id << " running\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
    }
    std::cout << "Worker " << id << " stopped\n";
}

int main() {
    std::stop_source ssource;
    std::vector<std::jthread> threads;
    
    // Create multiple threads sharing the same stop source
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker_with_id, i, ssource.get_token());
    }
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Stopping all workers...\n";
    ssource.request_stop(); // Stops all threads at once
    
    // All threads join automatically
    return 0;
}
```

### Stop Callbacks

You can register callbacks that execute when a stop is requested using `std::stop_callback`:

```cpp
#include <thread>
#include <iostream>
#include <chrono>
#include <atomic>

void worker_with_callback(std::stop_token stoken) {
    std::atomic<bool> cleanup_done{false};
    
    // Register a callback for when stop is requested
    std::stop_callback callback(stoken, [&cleanup_done]() {
        std::cout << "Stop callback invoked! Initiating cleanup...\n";
        cleanup_done = true;
    });
    
    int iteration = 0;
    while (!stoken.stop_requested() && iteration < 10) {
        std::cout << "Processing iteration " << iteration++ << '\n';
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    if (cleanup_done) {
        std::cout << "Cleanup completed\n";
    }
}

int main() {
    std::jthread t(worker_with_callback);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(800));
    t.request_stop();
    
    return 0;
}
```

### Condition Variable Integration

`std::condition_variable_any` can work with stop tokens for interruptible waits:

```cpp
#include <thread>
#include <iostream>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

std::mutex mtx;
std::condition_variable_any cv;
std::queue<int> data_queue;

void consumer(std::stop_token stoken) {
    while (true) {
        std::unique_lock lock(mtx);
        
        // Wait with stop token - returns false if stop requested
        bool stopped = !cv.wait(lock, stoken, [] {
            return !data_queue.empty();
        });
        
        if (stopped) {
            std::cout << "Consumer stopped\n";
            break;
        }
        
        int data = data_queue.front();
        data_queue.pop();
        lock.unlock();
        
        std::cout << "Consumed: " << data << '\n';
    }
}

int main() {
    std::jthread consumer_thread(consumer);
    
    // Produce some data
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(200));
        {
            std::lock_guard lock(mtx);
            data_queue.push(i);
        }
        cv.notify_one();
    }
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    consumer_thread.request_stop(); // Stop the consumer
    cv.notify_all(); // Wake it up so it can see the stop request
    
    return 0;
}
```

### Practical Example: Interruptible File Processor

```cpp
#include <thread>
#include <iostream>
#include <fstream>
#include <chrono>
#include <vector>
#include <string>

class FileProcessor {
    std::jthread worker_thread;
    
    void process_files(std::stop_token stoken, std::vector<std::string> files) {
        for (const auto& filename : files) {
            if (stoken.stop_requested()) {
                std::cout << "Processing cancelled at file: " << filename << '\n';
                return;
            }
            
            std::cout << "Processing: " << filename << '\n';
            // Simulate file processing
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
            
            // Check periodically during long operations
            if (stoken.stop_requested()) {
                std::cout << "Cancellation detected mid-process\n";
                return;
            }
            
            std::cout << "Completed: " << filename << '\n';
        }
        std::cout << "All files processed\n";
    }
    
public:
    void start(std::vector<std::string> files) {
        worker_thread = std::jthread(&FileProcessor::process_files, this, 
                                     std::placeholders::_1, std::move(files));
    }
    
    void cancel() {
        worker_thread.request_stop();
    }
    
    ~FileProcessor() {
        // Automatic cleanup - jthread joins automatically
    }
};

int main() {
    FileProcessor processor;
    
    std::vector<std::string> files = {
        "file1.txt", "file2.txt", "file3.txt", 
        "file4.txt", "file5.txt"
    };
    
    processor.start(files);
    
    // Cancel after some time
    std::this_thread::sleep_for(std::chrono::milliseconds(1200));
    std::cout << "\n--- Cancelling processing ---\n";
    processor.cancel();
    
    // Processor destructor handles cleanup automatically
    return 0;
}
```

## Key Concepts

**std::jthread**: A wrapper around `std::thread` that automatically joins in its destructor and supports cooperative cancellation through stop tokens.

**std::stop_token**: A lightweight, copyable token that allows checking if a stop has been requested. Passed to the thread function to enable cancellation checks.

**std::stop_source**: Creates and manages stop tokens. Calling `request_stop()` signals all associated tokens.

**std::stop_callback**: Registers a function to be called when stop is requested, useful for resource cleanup or notification.

**Cooperative Cancellation**: The thread voluntarily checks for stop requests and terminates gracefully, unlike forced termination which can leave resources in inconsistent states.

## Best Practices

1. **Always check stop_requested() in loops**: Place checks at appropriate intervals, especially before expensive operations.

2. **Use stop tokens with blocking operations**: Many standard library facilities (like `condition_variable_any`) support stop tokens for interruptible waits.

3. **Clean up resources on cancellation**: Use stop callbacks or manual cleanup code when cancellation is detected.

4. **Prefer jthread over thread**: Unless you have specific requirements for manual thread management, `std::jthread` is safer and more convenient.

5. **Share stop sources carefully**: Multiple threads can share a stop source when you want coordinated cancellation.

## Summary

`std::jthread` and `std::stop_token` represent a significant improvement in C++ concurrency by combining automatic resource management with standardized cooperative cancellation. The RAII-based automatic joining eliminates a common source of bugs, while stop tokens provide a clean, race-free mechanism for threads to communicate cancellation requests. This cooperative approach ensures threads can shut down gracefully, cleaning up resources and maintaining invariants, making concurrent programs more robust and easier to reason about. For new code targeting C++20 and later, `std::jthread` should be the default choice over `std::thread`.