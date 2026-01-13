# Counting Semaphores in C++

## Overview

A counting semaphore is a synchronization primitive that maintains an internal counter representing the number of available resources. Unlike binary semaphores (which act like mutexes with a count of 0 or 1), counting semaphores can have any non-negative integer value, making them ideal for managing pools of resources or limiting concurrent access to a finite set of resources.

C++20 introduced `std::counting_semaphore<LeastMaxValue>` in the `<semaphore>` header, providing a standardized way to implement resource counting and throttling patterns.

## Core Concepts

**Resource Counting**: The semaphore maintains a count of available resources. Threads can acquire resources (decrementing the count) and release them (incrementing the count).

**Blocking Behavior**: When a thread tries to acquire a resource but the count is zero, it blocks until another thread releases a resource.

**Use Cases**:
- Limiting concurrent access to a resource pool (database connections, thread pools)
- Implementing producer-consumer patterns with bounded buffers
- Throttling parallel operations
- Managing limited hardware resources

## Key Operations

The primary operations on a counting semaphore are:

- `acquire()`: Decrements the counter. Blocks if the counter is zero.
- `release(update = 1)`: Increments the counter by the specified amount.
- `try_acquire()`: Attempts to decrement without blocking. Returns true on success.
- `try_acquire_for(duration)`: Attempts to acquire with a timeout.
- `try_acquire_until(time_point)`: Attempts to acquire until a specific time.

## Code Examples

### Example 1: Basic Resource Pool Management

```cpp
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>
#include <chrono>

// Simulate a database connection pool with limited connections
class DatabaseConnectionPool {
private:
    std::counting_semaphore<10> available_connections;
    int connection_id = 0;
    
public:
    DatabaseConnectionPool(int max_connections) 
        : available_connections(max_connections) {}
    
    void execute_query(int thread_id) {
        // Try to acquire a connection
        available_connections.acquire();
        
        int conn_id = ++connection_id;
        std::cout << "Thread " << thread_id 
                  << " acquired connection " << conn_id << "\n";
        
        // Simulate database work
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
        
        std::cout << "Thread " << thread_id 
                  << " releasing connection " << conn_id << "\n";
        
        // Release the connection back to the pool
        available_connections.release();
    }
};

int main() {
    DatabaseConnectionPool pool(3); // Only 3 connections available
    std::vector<std::thread> threads;
    
    // Launch 10 threads, but only 3 can work simultaneously
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back(&DatabaseConnectionPool::execute_query, 
                           &pool, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 2: Producer-Consumer with Bounded Buffer

```cpp
#include <iostream>
#include <semaphore>
#include <thread>
#include <queue>
#include <mutex>

template<typename T, size_t Capacity>
class BoundedQueue {
private:
    std::queue<T> queue;
    std::mutex mutex;
    std::counting_semaphore<Capacity> empty_slots;
    std::counting_semaphore<Capacity> filled_slots;
    
public:
    BoundedQueue() 
        : empty_slots(Capacity), filled_slots(0) {}
    
    void push(T item) {
        // Wait for an empty slot
        empty_slots.acquire();
        
        {
            std::lock_guard<std::mutex> lock(mutex);
            queue.push(std::move(item));
        }
        
        // Signal that a slot is now filled
        filled_slots.release();
    }
    
    T pop() {
        // Wait for a filled slot
        filled_slots.acquire();
        
        T item;
        {
            std::lock_guard<std::mutex> lock(mutex);
            item = std::move(queue.front());
            queue.pop();
        }
        
        // Signal that a slot is now empty
        empty_slots.release();
        return item;
    }
};

void producer(BoundedQueue<int, 5>& queue, int id) {
    for (int i = 0; i < 10; ++i) {
        int value = id * 100 + i;
        std::cout << "Producer " << id << " producing: " << value << "\n";
        queue.push(value);
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(BoundedQueue<int, 5>& queue, int id) {
    for (int i = 0; i < 10; ++i) {
        int value = queue.pop();
        std::cout << "Consumer " << id << " consumed: " << value << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
}

int main() {
    BoundedQueue<int, 5> queue;
    
    std::thread prod1(producer, std::ref(queue), 1);
    std::thread prod2(producer, std::ref(queue), 2);
    std::thread cons1(consumer, std::ref(queue), 1);
    std::thread cons2(consumer, std::ref(queue), 2);
    
    prod1.join();
    prod2.join();
    cons1.join();
    cons2.join();
    
    return 0;
}
```

### Example 3: Parallel Task Throttling

```cpp
#include <iostream>
#include <semaphore>
#include <thread>
#include <vector>
#include <chrono>
#include <string>

class TaskThrottler {
private:
    std::counting_semaphore<100> semaphore;
    
public:
    explicit TaskThrottler(size_t max_concurrent)
        : semaphore(max_concurrent) {}
    
    template<typename Func>
    void execute(Func&& task) {
        semaphore.acquire();
        
        try {
            task();
        } catch (...) {
            semaphore.release();
            throw;
        }
        
        semaphore.release();
    }
    
    bool try_execute(auto&& task, std::chrono::milliseconds timeout) {
        if (semaphore.try_acquire_for(timeout)) {
            try {
                task();
            } catch (...) {
                semaphore.release();
                throw;
            }
            semaphore.release();
            return true;
        }
        return false;
    }
};

void process_task(int task_id) {
    std::cout << "Processing task " << task_id << "\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(200));
    std::cout << "Completed task " << task_id << "\n";
}

int main() {
    TaskThrottler throttler(3); // Max 3 concurrent tasks
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&throttler, i]() {
            throttler.execute([i]() { process_task(i); });
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 4: Non-blocking Acquisition with try_acquire

```cpp
#include <iostream>
#include <semaphore>
#include <thread>
#include <atomic>

class RateLimiter {
private:
    std::counting_semaphore<1000> tokens;
    std::atomic<bool> running{true};
    std::thread refill_thread;
    
public:
    RateLimiter(size_t initial_tokens, size_t refill_rate_per_sec)
        : tokens(initial_tokens) {
        refill_thread = std::thread([this, refill_rate_per_sec]() {
            while (running) {
                std::this_thread::sleep_for(std::chrono::seconds(1));
                tokens.release(refill_rate_per_sec);
            }
        });
    }
    
    ~RateLimiter() {
        running = false;
        if (refill_thread.joinable()) {
            refill_thread.join();
        }
    }
    
    bool try_consume() {
        return tokens.try_acquire();
    }
    
    void consume_or_wait() {
        tokens.acquire();
    }
};

int main() {
    RateLimiter limiter(5, 2); // 5 initial tokens, refill 2 per second
    
    for (int i = 0; i < 15; ++i) {
        if (limiter.try_consume()) {
            std::cout << "Request " << i << " processed immediately\n";
        } else {
            std::cout << "Request " << i << " waiting for token...\n";
            limiter.consume_or_wait();
            std::cout << "Request " << i << " processed after waiting\n";
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
    }
    
    return 0;
}
```

## Summary

Counting semaphores are powerful synchronization primitives for managing finite resources in concurrent applications. The `std::counting_semaphore` template provides a type-safe, standardized interface with both blocking and non-blocking acquisition methods. Key advantages include natural resource pool management, built-in blocking behavior that eliminates busy-waiting, and flexibility through timeout-based operations. They're particularly effective for throttling concurrent operations, implementing producer-consumer patterns with bounded buffers, and managing shared resource pools like database connections or thread workers. When combined with proper exception safety (RAII patterns or try-catch blocks), counting semaphores provide robust solutions for complex concurrency scenarios while maintaining clear and maintainable code.