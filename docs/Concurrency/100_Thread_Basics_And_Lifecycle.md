# Thread Basics and Lifecycle in C++

## Introduction

Modern C++ provides robust multithreading support through the `<thread>` library, introduced in C++11. The `std::thread` class allows you to create and manage threads of execution, enabling concurrent programming and parallel task execution. Understanding thread basics and lifecycle management is fundamental to writing correct, efficient multithreaded applications.

## Creating Threads

A thread represents an independent path of execution within a program. You can create a thread by constructing a `std::thread` object and passing it a callable entity (function, lambda, functor, or member function).

### Basic Thread Creation

```cpp
#include <iostream>
#include <thread>
#include <chrono>

// Simple function to run in a thread
void printMessage(const std::string& message, int count) {
    for (int i = 0; i < count; ++i) {
        std::cout << message << " (" << i + 1 << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    // Create a thread that executes printMessage
    std::thread t1(printMessage, "Hello from thread", 3);
    
    // Main thread continues execution
    std::cout << "Main thread running" << std::endl;
    
    // Wait for the thread to complete
    t1.join();
    
    std::cout << "Thread completed" << std::endl;
    return 0;
}
```

### Creating Threads with Different Callable Types

```cpp
#include <iostream>
#include <thread>

// 1. Function pointer
void functionThread() {
    std::cout << "Function thread" << std::endl;
}

// 2. Functor (function object)
class FunctorThread {
public:
    void operator()() const {
        std::cout << "Functor thread" << std::endl;
    }
};

// 3. Member function
class MyClass {
public:
    void memberFunction(int value) {
        std::cout << "Member function thread: " << value << std::endl;
    }
};

int main() {
    // Using a function
    std::thread t1(functionThread);
    
    // Using a lambda
    std::thread t2([]() {
        std::cout << "Lambda thread" << std::endl;
    });
    
    // Using a functor
    FunctorThread functor;
    std::thread t3(functor);
    
    // Using a member function
    MyClass obj;
    std::thread t4(&MyClass::memberFunction, &obj, 42);
    
    // Join all threads
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    return 0;
}
```

## Thread Joinability

A thread is considered **joinable** if it represents an active thread of execution. Once a thread is created, it must be either joined or detached before the thread object is destroyed. Failing to do so will cause the program to terminate.

### Checking Joinability

```cpp
#include <iostream>
#include <thread>

void simpleTask() {
    std::cout << "Task executing" << std::endl;
}

int main() {
    std::thread t1(simpleTask);
    
    // Check if thread is joinable
    if (t1.joinable()) {
        std::cout << "Thread is joinable" << std::endl;
        t1.join();
    }
    
    // After joining, thread is no longer joinable
    if (!t1.joinable()) {
        std::cout << "Thread is no longer joinable" << std::endl;
    }
    
    // Default-constructed threads are not joinable
    std::thread t2;
    std::cout << "Default thread joinable: " << std::boolalpha 
              << t2.joinable() << std::endl;
    
    return 0;
}
```

## Joining Threads

When you call `join()` on a thread, the calling thread blocks and waits until the target thread completes its execution. This ensures synchronization and orderly cleanup of thread resources.

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

void worker(int id, int duration) {
    std::cout << "Worker " << id << " started" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    std::cout << "Worker " << id << " finished" << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    
    // Create multiple threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i, (i + 1) * 100);
    }
    
    std::cout << "All threads created, waiting for completion..." << std::endl;
    
    // Join all threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cout << "All workers completed" << std::endl;
    return 0;
}
```

## Detaching Threads

When you call `detach()` on a thread, it separates from the thread object and continues execution independently. The detached thread runs in the background, and its resources are automatically cleaned up upon completion. You cannot join a detached thread.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void backgroundTask(int id) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Background task " << id << " completed" << std::endl;
}

int main() {
    std::thread t1(backgroundTask, 1);
    
    // Detach the thread - it will run independently
    t1.detach();
    
    std::cout << "Thread detached, is joinable: " << std::boolalpha 
              << t1.joinable() << std::endl;
    
    // Main thread continues without waiting
    std::cout << "Main thread continuing..." << std::endl;
    
    // Give detached thread time to complete before program exits
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    return 0;
}
```

**Warning:** Be cautious with detached threads. If the main thread exits before a detached thread completes, the detached thread may access invalid resources, leading to undefined behavior.

## Thread Lifecycle Management

Proper thread lifecycle management is crucial to prevent resource leaks and program crashes. Here's a comprehensive example demonstrating best practices:

```cpp
#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

class ThreadGuard {
    std::thread& t;
public:
    explicit ThreadGuard(std::thread& thread) : t(thread) {}
    
    ~ThreadGuard() {
        if (t.joinable()) {
            t.join();
        }
    }
    
    // Prevent copying
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void riskyOperation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Risky operation completed" << std::endl;
}

void demonstrateThreadGuard() {
    std::thread t(riskyOperation);
    ThreadGuard guard(t);
    
    // Even if an exception is thrown, ThreadGuard destructor
    // will ensure the thread is joined
    std::cout << "Doing other work..." << std::endl;
    
    // Simulate potential exception
    // throw std::runtime_error("Something went wrong!");
}

int main() {
    try {
        demonstrateThreadGuard();
        std::cout << "Thread guard demonstration completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    return 0;
}
```

## Thread Identification and Hardware Concurrency

```cpp
#include <iostream>
#include <thread>

void printThreadInfo() {
    std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
}

int main() {
    // Get number of hardware threads available
    unsigned int numThreads = std::thread::hardware_concurrency();
    std::cout << "Hardware supports " << numThreads << " concurrent threads" << std::endl;
    
    // Main thread ID
    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
    
    // Create and identify another thread
    std::thread t(printThreadInfo);
    std::cout << "Created thread ID (from main): " << t.get_id() << std::endl;
    
    t.join();
    
    return 0;
}
```

## Common Pitfalls and Best Practices

### Pitfall 1: Forgetting to Join or Detach

```cpp
#include <thread>

void task() {}

int main() {
    std::thread t(task);
    // ERROR: Thread object destroyed without join() or detach()
    // This will call std::terminate()
    return 0;
}
```

### Best Practice: Always Join or Detach

```cpp
#include <thread>

void task() {}

int main() {
    std::thread t(task);
    t.join();  // or t.detach();
    return 0;
}
```

### Pitfall 2: Accessing Local Variables After Detach

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void danglingReference() {
    int localValue = 42;
    
    std::thread t([&localValue]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << localValue << std::endl;  // DANGER: localValue may be invalid
    });
    
    t.detach();
    // Function returns, localValue is destroyed, but thread still references it
}

int main() {
    danglingReference();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
```

### Best Practice: Pass by Value or Use Proper Synchronization

```cpp
#include <iostream>
#include <thread>
#include <chrono>

void safeDetach() {
    int localValue = 42;
    
    // Pass by value to avoid dangling reference
    std::thread t([localValue]() {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << localValue << std::endl;  // Safe: copy of localValue
    });
    
    t.detach();
}

int main() {
    safeDetach();
    std::this_thread::sleep_for(std::chrono::seconds(2));
    return 0;
}
```

## Move Semantics with Threads

Threads cannot be copied but can be moved, which is useful for transferring ownership of threads between objects or containers.

```cpp
#include <iostream>
#include <thread>
#include <vector>

void task(int id) {
    std::cout << "Task " << id << " executing" << std::endl;
}

std::thread createThread(int id) {
    return std::thread(task, id);
}

int main() {
    // Move construction
    std::thread t1(task, 1);
    std::thread t2 = std::move(t1);  // t1 is now empty, t2 owns the thread
    
    // Move from function return
    std::thread t3 = createThread(3);
    
    // Store threads in a vector
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.push_back(std::thread(task, i + 10));
    }
    
    // Join all threads
    t2.join();
    t3.join();
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## Summary

Understanding thread basics and lifecycle management in C++ is essential for concurrent programming. The key concepts include: `std::thread` objects represent independent execution paths that can be created with various callable types including functions, lambdas, functors, and member functions. Every thread must be either joined (blocking until completion) or detached (running independently) before destruction, otherwise the program terminates. Joinability can be checked with the `joinable()` method, and threads become non-joinable after being joined, detached, or moved from. Proper lifecycle management requires careful attention to resource ownership, especially with detached threads that can create dangling references if not handled correctly. Threads support move semantics but not copy semantics, enabling ownership transfer. RAII wrappers like ThreadGuard ensure threads are properly joined even in the presence of exceptions, and `std::thread::hardware_concurrency()` provides insight into available parallelism. Following best practices such as always joining or detaching threads, avoiding dangling references with detached threads, using RAII for automatic cleanup, passing data by value to detached threads, and checking joinability before operations will help you write robust, thread-safe applications.