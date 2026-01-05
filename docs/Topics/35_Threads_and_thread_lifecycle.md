# Guide on C++ threads and their lifecycle

**Thread Lifecycle Stages:**
- Created → Runnable → Running → Blocked/Waiting → Terminated

**Main Concepts Covered:**
- **Creating threads** - with functions, parameters, and lambdas
- **join()** - blocks until thread completes (use when you need results)
- **detach()** - thread runs independently (use sparingly)
- **Reference parameters** - must use `std::ref()` to pass by reference

**Important Rules:**
1. Every thread must be either joined or detached before destruction
2. Always check `joinable()` before calling `join()` or `detach()`
3. Use RAII patterns for automatic thread management

The examples progress from simple thread creation to practical applications like parallel computation and thread pools. Each example is complete and can be compiled with C++11 or later using:

```bash
g++ -std=c++11 -pthread filename.cpp -o output
```

# C++ Threads & Thread Lifecycle

## Introduction to Threads

A thread is the smallest unit of execution within a process. Modern C++ (C++11 and later) provides built-in support for multithreading through the `<thread>` header, allowing programs to execute multiple tasks concurrently.

## Thread Lifecycle

A thread goes through several states during its lifetime:

1. **Created/New**: Thread object is created but not yet started
2. **Runnable**: Thread is ready to run and waiting for CPU time
3. **Running**: Thread is actively executing
4. **Blocked/Waiting**: Thread is waiting for a resource or condition
5. **Terminated**: Thread has completed execution

## Creating Threads

### Basic Thread Creation

```cpp
#include <iostream>
#include <thread>

void simpleFunction() {
    std::cout << "Hello from thread!\n";
}

int main() {
    std::thread t1(simpleFunction);  // Create and start thread
    t1.join();  // Wait for thread to finish
    return 0;
}
```

### Thread with Function Parameters

```cpp
#include <iostream>
#include <thread>

void printNumbers(int start, int end) {
    for (int i = start; i <= end; ++i) {
        std::cout << "Thread ID: " << std::this_thread::get_id() 
                  << " - Number: " << i << "\n";
    }
}

int main() {
    std::thread t1(printNumbers, 1, 5);
    std::thread t2(printNumbers, 6, 10);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Thread with Lambda Function

```cpp
#include <iostream>
#include <thread>

int main() {
    int value = 42;
    
    std::thread t([value]() {
        std::cout << "Captured value: " << value << "\n";
    });
    
    t.join();
    return 0;
}
```

## Thread Management

### Join vs Detach

**join()**: Blocks the calling thread until the thread finishes execution.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void work(int id) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Thread " << id << " finished\n";
}

int main() {
    std::thread t1(work, 1);
    std::thread t2(work, 2);
    
    std::cout << "Waiting for threads...\n";
    t1.join();  // Main thread waits here
    t2.join();  // Then waits here
    std::cout << "All threads completed\n";
    
    return 0;
}
```

**detach()**: Separates the thread from the thread object, allowing it to execute independently.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void backgroundWork() {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Background work completed\n";
}

int main() {
    std::thread t(backgroundWork);
    t.detach();  // Thread runs independently
    
    std::cout << "Main continues immediately\n";
    
    // Need to wait here or background thread might not complete
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    return 0;
}
```

### Checking Thread State

```cpp
#include <iostream>
#include <thread>

void task() {
    std::cout << "Task running\n";
}

int main() {
    std::thread t(task);
    
    if (t.joinable()) {
        std::cout << "Thread is joinable\n";
        t.join();
    }
    
    // After join, thread is no longer joinable
    if (!t.joinable()) {
        std::cout << "Thread is no longer joinable\n";
    }
    
    return 0;
}
```

## Practical Examples

### Example 1: Concurrent Computation

```cpp
#include <iostream>
#include <thread>
#include <vector>

void calculateSum(const std::vector<int>& data, int start, int end, long long& result) {
    result = 0;
    for (int i = start; i < end; ++i) {
        result += data[i];
    }
}

int main() {
    std::vector<int> numbers(1000);
    for (int i = 0; i < 1000; ++i) {
        numbers[i] = i + 1;
    }
    
    long long sum1 = 0, sum2 = 0;
    
    std::thread t1(calculateSum, std::ref(numbers), 0, 500, std::ref(sum1));
    std::thread t2(calculateSum, std::ref(numbers), 500, 1000, std::ref(sum2));
    
    t1.join();
    t2.join();
    
    std::cout << "Total sum: " << (sum1 + sum2) << "\n";
    return 0;
}
```

### Example 2: Thread Pool Pattern

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <functional>

void worker(int id, std::function<void()> task) {
    std::cout << "Worker " << id << " starting\n";
    task();
    std::cout << "Worker " << id << " finished\n";
}

int main() {
    std::vector<std::thread> threadPool;
    
    // Create 4 worker threads
    for (int i = 0; i < 4; ++i) {
        threadPool.emplace_back(worker, i, []() {
            // Simulate some work
            std::this_thread::sleep_for(std::chrono::seconds(1));
        });
    }
    
    // Wait for all threads to complete
    for (auto& t : threadPool) {
        t.join();
    }
    
    std::cout << "All workers completed\n";
    return 0;
}
```

### Example 3: Thread with Class Member Function

```cpp
#include <iostream>
#include <thread>

class Counter {
private:
    int count;
public:
    Counter() : count(0) {}
    
    void increment(int times) {
        for (int i = 0; i < times; ++i) {
            ++count;
        }
        std::cout << "Final count: " << count << "\n";
    }
};

int main() {
    Counter counter;
    
    // Create thread with member function
    std::thread t(&Counter::increment, &counter, 1000);
    
    t.join();
    return 0;
}
```

## Thread Lifecycle Management Best Practices

1. **Always join or detach**: Every thread must be either joined or detached before its destructor runs, otherwise `std::terminate()` is called.

2. **RAII Pattern**: Use RAII wrappers to ensure proper cleanup:

```cpp
class ThreadGuard {
    std::thread& t;
public:
    explicit ThreadGuard(std::thread& t_) : t(t_) {}
    ~ThreadGuard() {
        if (t.joinable()) {
            t.join();
        }
    }
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};
```

3. **Avoid detached threads when possible**: They're harder to manage and can cause issues during program shutdown.

4. **Use `std::ref()` for reference parameters**: By default, arguments are copied. Use `std::ref()` to pass by reference.

## Hardware Concurrency

```cpp
#include <iostream>
#include <thread>

int main() {
    unsigned int cores = std::thread::hardware_concurrency();
    std::cout << "System supports " << cores << " concurrent threads\n";
    return 0;
}
```

## Common Pitfalls

1. **Forgetting to join/detach**: Results in program termination
2. **Data races**: Multiple threads accessing shared data without synchronization
3. **Deadlocks**: Threads waiting for each other indefinitely
4. **Exception safety**: Exceptions can prevent join() from being called

## Summary

The C++ thread lifecycle involves creating a thread object, starting execution, managing its lifetime with join() or detach(), and ensuring proper cleanup. Understanding these concepts is essential for writing correct and efficient multithreaded programs.