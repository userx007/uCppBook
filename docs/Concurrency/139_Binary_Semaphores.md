# Binary Semaphores in C++

## Overview

Binary semaphores are a synchronization primitive introduced in C++20 that can have only two states: available (count = 1) or unavailable (count = 0). They're primarily used for signaling between threads rather than mutual exclusion, making them ideal for producer-consumer scenarios, event notifications, and thread coordination.

Unlike mutexes which enforce ownership semantics (the thread that locks must unlock), binary semaphores have no ownership concept—any thread can signal (release) the semaphore, making them more flexible for certain coordination patterns.

## Key Concepts

**Binary Semaphore Properties:**
- Can hold a maximum count of 1
- Useful for signaling and notifications
- No thread ownership requirements
- Lightweight compared to condition variables
- Part of `<semaphore>` header

**Core Operations:**
- `acquire()` - Decrements count, blocks if already 0
- `try_acquire()` - Non-blocking acquire attempt
- `try_acquire_for()` - Acquire with timeout
- `try_acquire_until()` - Acquire until specific time
- `release()` - Increments count (signals waiting threads)

## Code Examples

### Example 1: Basic Thread Signaling

```cpp
#include <iostream>
#include <thread>
#include <semaphore>
#include <chrono>

// Binary semaphore initialized to 0 (unavailable)
std::binary_semaphore signal_ready(0);

void worker_thread() {
    std::cout << "Worker: Waiting for signal...\n";
    
    // Block until signal is received
    signal_ready.acquire();
    
    std::cout << "Worker: Signal received! Performing work...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Worker: Work completed.\n";
}

void main_thread() {
    std::cout << "Main: Preparing data...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::cout << "Main: Data ready, signaling worker...\n";
    signal_ready.release();  // Signal the worker thread
}

int main() {
    std::thread worker(worker_thread);
    std::thread main(main_thread);
    
    worker.join();
    main.join();
    
    return 0;
}
```

### Example 2: Ping-Pong Communication

```cpp
#include <iostream>
#include <thread>
#include <semaphore>

std::binary_semaphore ping_sem(1);  // Start with ping
std::binary_semaphore pong_sem(0);  // Pong waits

void ping_thread(int count) {
    for (int i = 0; i < count; ++i) {
        ping_sem.acquire();  // Wait for turn
        std::cout << "Ping " << i + 1 << "\n";
        pong_sem.release();  // Signal pong
    }
}

void pong_thread(int count) {
    for (int i = 0; i < count; ++i) {
        pong_sem.acquire();  // Wait for ping
        std::cout << "Pong " << i + 1 << "\n";
        ping_sem.release();  // Signal ping
    }
}

int main() {
    const int exchanges = 5;
    
    std::thread t1(ping_thread, exchanges);
    std::thread t2(pong_thread, exchanges);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Example 3: Non-Blocking Operations

```cpp
#include <iostream>
#include <thread>
#include <semaphore>
#include <chrono>
#include <vector>

std::binary_semaphore resource_sem(1);

void try_acquire_resource(int thread_id) {
    std::cout << "Thread " << thread_id << ": Attempting to acquire...\n";
    
    if (resource_sem.try_acquire()) {
        std::cout << "Thread " << thread_id << ": Acquired! Working...\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        std::cout << "Thread " << thread_id << ": Releasing.\n";
        resource_sem.release();
    } else {
        std::cout << "Thread " << thread_id << ": Could not acquire, doing other work.\n";
    }
}

void timed_acquire_resource(int thread_id) {
    using namespace std::chrono_literals;
    
    std::cout << "Thread " << thread_id << ": Waiting up to 200ms...\n";
    
    if (resource_sem.try_acquire_for(200ms)) {
        std::cout << "Thread " << thread_id << ": Acquired within timeout!\n";
        std::this_thread::sleep_for(100ms);
        resource_sem.release();
    } else {
        std::cout << "Thread " << thread_id << ": Timeout expired.\n";
    }
}

int main() {
    std::vector<std::thread> threads;
    
    // Launch threads with try_acquire
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(try_acquire_resource, i);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    threads.clear();
    
    // Launch threads with timed acquire
    for (int i = 3; i < 6; ++i) {
        threads.emplace_back(timed_acquire_resource, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 4: Task Completion Notification

```cpp
#include <iostream>
#include <thread>
#include <semaphore>
#include <vector>
#include <chrono>

class TaskManager {
private:
    std::binary_semaphore completion_signal{0};
    bool task_completed = false;
    
public:
    void execute_task() {
        std::cout << "Task: Starting long-running operation...\n";
        std::this_thread::sleep_for(std::chrono::seconds(2));
        
        task_completed = true;
        std::cout << "Task: Completed! Notifying waiters...\n";
        completion_signal.release();
    }
    
    void wait_for_completion() {
        std::cout << "Waiter: Blocking until task completes...\n";
        completion_signal.acquire();
        std::cout << "Waiter: Task is done, proceeding...\n";
    }
    
    bool is_completed() const {
        return task_completed;
    }
};

int main() {
    TaskManager manager;
    
    // Multiple threads can wait, but only first gets signaled
    // (binary semaphore characteristic)
    std::thread task_thread([&manager]() {
        manager.execute_task();
    });
    
    std::thread waiter([&manager]() {
        manager.wait_for_completion();
        std::cout << "Processing task results...\n";
    });
    
    task_thread.join();
    waiter.join();
    
    return 0;
}
```

## Summary

Binary semaphores provide a lightweight, signal-based synchronization mechanism perfect for thread coordination and event notification. Unlike mutexes, they don't enforce ownership, allowing any thread to release the semaphore, which makes them ideal for producer-consumer patterns and cross-thread signaling.

**Key advantages include:**
- Simple signal/wait semantics without ownership constraints
- Better suited for notifications than condition variables in simple cases
- Support for non-blocking and timed operations
- Zero-overhead abstraction in modern C++

**Best used for:**
- Signaling between threads when one completes work
- Coordinating execution order (like ping-pong patterns)
- Event notifications without complex conditions
- Simple producer-consumer synchronization

**When to choose alternatives:**
- Use `std::mutex` when you need mutual exclusion with ownership
- Use `std::counting_semaphore` when you need counts > 1
- Use `std::condition_variable` for complex waiting conditions
- Use `std::atomic` for simple flag-based synchronization

Binary semaphores strike an excellent balance between simplicity and functionality for basic thread coordination tasks, making them a valuable addition to the C++ concurrency toolkit.