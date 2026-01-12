# Thread-Safe Queue Implementation

## Overview

A thread-safe queue is a fundamental building block in concurrent programming that allows multiple threads to safely enqueue and dequeue elements without data races or corruption. Unlike standard queues, thread-safe queues use synchronization primitives to ensure that concurrent operations maintain consistency and correctness.

This implementation is critical for producer-consumer patterns, task scheduling systems, message passing architectures, and any scenario where multiple threads need to communicate through a shared data structure.

## Core Concepts

### Why Thread Safety Matters

Standard queue implementations from the STL are not thread-safe. Concurrent access without proper synchronization leads to:

- **Data races**: Multiple threads modifying the queue simultaneously
- **Memory corruption**: Internal pointers becoming invalid
- **Lost updates**: Operations being overwritten by concurrent modifications
- **Undefined behavior**: Queue state becoming inconsistent

### Synchronization Requirements

A production-ready thread-safe queue needs:

1. **Mutual exclusion**: Only one thread modifies the queue at a time
2. **Condition variables**: Efficient waiting when the queue is empty
3. **Exception safety**: Proper cleanup even when operations fail
4. **Move semantics**: Efficient transfer of elements
5. **Timeouts**: Preventing indefinite blocking

## Implementation

Here's a comprehensive thread-safe queue implementation with multiple dequeue variants:

```cpp
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
#include <chrono>
#include <memory>
#include <stdexcept>

template<typename T>
class ThreadSafeQueue {
private:
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
    std::queue<T> queue_;
    bool closed_ = false;
    size_t max_size_ = 0; // 0 means unbounded

public:
    // Constructor with optional capacity limit
    explicit ThreadSafeQueue(size_t max_size = 0) 
        : max_size_(max_size) {}

    // Disable copy operations (queue owns its data)
    ThreadSafeQueue(const ThreadSafeQueue&) = delete;
    ThreadSafeQueue& operator=(const ThreadSafeQueue&) = delete;

    // Enable move operations
    ThreadSafeQueue(ThreadSafeQueue&& other) noexcept {
        std::lock_guard<std::mutex> lock(other.mutex_);
        queue_ = std::move(other.queue_);
        closed_ = other.closed_;
        max_size_ = other.max_size_;
    }

    // Push an element (blocks if queue is full)
    void push(T value) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait if queue is at capacity
        if (max_size_ > 0) {
            cond_var_.wait(lock, [this] { 
                return queue_.size() < max_size_ || closed_; 
            });
        }
        
        if (closed_) {
            throw std::runtime_error("Queue is closed");
        }
        
        queue_.push(std::move(value));
        lock.unlock();
        cond_var_.notify_one(); // Wake up a waiting consumer
    }

    // Try to push with timeout
    template<typename Rep, typename Period>
    bool try_push_for(T value, 
                      const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (max_size_ > 0) {
            if (!cond_var_.wait_for(lock, timeout, [this] { 
                return queue_.size() < max_size_ || closed_; 
            })) {
                return false; // Timeout occurred
            }
        }
        
        if (closed_) {
            return false;
        }
        
        queue_.push(std::move(value));
        lock.unlock();
        cond_var_.notify_one();
        return true;
    }

    // Pop an element (blocks if queue is empty)
    T pop() {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until queue has data or is closed
        cond_var_.wait(lock, [this] { 
            return !queue_.empty() || closed_; 
        });
        
        if (queue_.empty()) {
            throw std::runtime_error("Queue is closed and empty");
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        lock.unlock();
        cond_var_.notify_one(); // Wake up a waiting producer
        return value;
    }

    // Try to pop without blocking
    std::optional<T> try_pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (queue_.empty()) {
            return std::nullopt;
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        cond_var_.notify_one();
        return value;
    }

    // Try to pop with timeout
    template<typename Rep, typename Period>
    std::optional<T> try_pop_for(
        const std::chrono::duration<Rep, Period>& timeout) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        if (!cond_var_.wait_for(lock, timeout, [this] { 
            return !queue_.empty() || closed_; 
        })) {
            return std::nullopt; // Timeout occurred
        }
        
        if (queue_.empty()) {
            return std::nullopt; // Queue was closed
        }
        
        T value = std::move(queue_.front());
        queue_.pop();
        lock.unlock();
        cond_var_.notify_one();
        return value;
    }

    // Check if queue is empty
    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.empty();
    }

    // Get current size
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }

    // Close the queue (prevents new pushes, allows remaining pops)
    void close() {
        std::lock_guard<std::mutex> lock(mutex_);
        closed_ = true;
        cond_var_.notify_all(); // Wake all waiting threads
    }

    // Check if queue is closed
    bool is_closed() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return closed_;
    }

    // Clear all elements
    void clear() {
        std::lock_guard<std::mutex> lock(mutex_);
        std::queue<T> empty;
        std::swap(queue_, empty);
        cond_var_.notify_all();
    }
};
```

## Practical Examples

### Example 1: Producer-Consumer Pattern

```cpp
#include <iostream>
#include <thread>
#include <vector>

void producer(ThreadSafeQueue<int>& queue, int id, int count) {
    for (int i = 0; i < count; ++i) {
        int value = id * 1000 + i;
        queue.push(value);
        std::cout << "Producer " << id << " pushed: " << value << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void consumer(ThreadSafeQueue<int>& queue, int id) {
    try {
        while (true) {
            int value = queue.pop();
            std::cout << "Consumer " << id << " popped: " << value << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(150));
        }
    } catch (const std::runtime_error& e) {
        std::cout << "Consumer " << id << " exiting: " << e.what() << std::endl;
    }
}

int main() {
    ThreadSafeQueue<int> queue;
    
    // Start multiple producers and consumers
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(producer, std::ref(queue), i, 5);
    }
    
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(consumer, std::ref(queue), i);
    }
    
    // Wait for producers to finish
    for (int i = 0; i < 3; ++i) {
        threads[i].join();
    }
    
    // Allow consumers to drain the queue
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    // Close queue and wait for consumers
    queue.close();
    for (size_t i = 3; i < threads.size(); ++i) {
        threads[i].join();
    }
    
    return 0;
}
```

### Example 2: Task Queue with Timeouts

```cpp
#include <functional>
#include <iostream>

using Task = std::function<void()>;

void worker(ThreadSafeQueue<Task>& task_queue, int id) {
    while (true) {
        // Try to get a task with 1 second timeout
        auto task_opt = task_queue.try_pop_for(std::chrono::seconds(1));
        
        if (task_opt) {
            std::cout << "Worker " << id << " executing task" << std::endl;
            (*task_opt)(); // Execute the task
        } else if (task_queue.is_closed()) {
            std::cout << "Worker " << id << " shutting down" << std::endl;
            break;
        } else {
            std::cout << "Worker " << id << " timeout, checking again..." 
                      << std::endl;
        }
    }
}

int main() {
    ThreadSafeQueue<Task> task_queue;
    
    // Start worker threads
    std::vector<std::thread> workers;
    for (int i = 0; i < 4; ++i) {
        workers.emplace_back(worker, std::ref(task_queue), i);
    }
    
    // Submit tasks
    for (int i = 0; i < 10; ++i) {
        task_queue.push([i]() {
            std::cout << "Executing task " << i << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(500));
        });
    }
    
    // Signal shutdown
    std::this_thread::sleep_for(std::chrono::seconds(3));
    task_queue.close();
    
    // Wait for workers to finish
    for (auto& worker : workers) {
        worker.join();
    }
    
    return 0;
}
```

### Example 3: Bounded Queue with Backpressure

```cpp
#include <iostream>

void bounded_producer(ThreadSafeQueue<int>& queue, int id) {
    for (int i = 0; i < 20; ++i) {
        int value = id * 100 + i;
        
        // Try to push with timeout (handles backpressure)
        if (queue.try_push_for(value, std::chrono::milliseconds(500))) {
            std::cout << "Producer " << id << " pushed: " << value << std::endl;
        } else {
            std::cout << "Producer " << id << " timeout on value: " << value 
                      << std::endl;
        }
        
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
    }
}

void slow_consumer(ThreadSafeQueue<int>& queue) {
    while (true) {
        auto value_opt = queue.try_pop_for(std::chrono::seconds(2));
        
        if (value_opt) {
            std::cout << "Consumer processing: " << *value_opt << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        } else {
            break;
        }
    }
}

int main() {
    // Create bounded queue with capacity of 5
    ThreadSafeQueue<int> queue(5);
    
    std::thread prod1(bounded_producer, std::ref(queue), 1);
    std::thread prod2(bounded_producer, std::ref(queue), 2);
    std::thread cons(slow_consumer, std::ref(queue));
    
    prod1.join();
    prod2.join();
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    queue.close();
    
    cons.join();
    
    return 0;
}
```

## Key Design Decisions

### Using std::optional for Non-Blocking Operations

The `try_pop()` and timeout variants return `std::optional<T>` rather than using output parameters. This provides:
- Clear indication of success/failure
- Exception safety
- Modern C++ idioms
- No need for default-constructible types

### Condition Variable Predicates

All `wait()` calls include predicates to handle spurious wakeups correctly. The predicate is checked atomically with the mutex held, preventing race conditions.

### Move Semantics

The queue uses `std::move()` extensively to avoid unnecessary copies, making it efficient for both small types and large objects.

### Close Mechanism

The `close()` method provides graceful shutdown, allowing consumers to drain remaining items while preventing new insertions.

## Performance Considerations

### Lock Contention

Every operation acquires the mutex, creating a potential bottleneck under high contention. For extremely high-throughput scenarios, consider:
- Lock-free queues using atomics
- Multiple queues with sharding
- Batch operations to amortize lock overhead

### Notification Strategy

The implementation uses `notify_one()` instead of `notify_all()` when appropriate, reducing unnecessary context switches.

### Memory Allocation

The underlying `std::queue` may allocate memory during push operations. For real-time systems, consider using a fixed-size ring buffer.

## Summary

A thread-safe queue implementation requires careful coordination of mutexes and condition variables to ensure correctness while maintaining reasonable performance. The key components include mutual exclusion for all operations, condition variables for efficient waiting, proper handling of edge cases like queue closure, support for both blocking and non-blocking operations, and optional capacity limits for backpressure control.

This production-ready implementation provides a robust foundation for concurrent programming patterns, particularly producer-consumer architectures. The use of modern C++ features like `std::optional`, move semantics, and template parameters makes it both efficient and type-safe. While suitable for many applications, extremely high-performance scenarios may require lock-free alternatives or more sophisticated designs like Michael-Scott queues or segmented queues with fine-grained locking.