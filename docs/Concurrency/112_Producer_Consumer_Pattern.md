# Producer-Consumer Pattern in C++

**Key Topics:**
- Core components (shared buffer, mutex, condition variables)
- Complete implementation of a thread-safe queue template class
- Practical usage example with multiple producers and consumers
- Advanced implementation using separate condition variables
- Detailed explanations of condition variables and synchronization primitives

**Code Examples Include:**
- Full `ThreadSafeQueue` template class with bounded buffer
- Producer and consumer functions demonstrating the pattern
- Complete main() showing multi-threaded coordination
- Advanced queue implementation with separate condition variables for producers/consumers

**Additional Sections:**
- Best practices for thread-safe queue implementation
- Real-world applications across different domains
- Performance considerations and optimization strategies
- Explanation of spurious wakeups and why predicates are essential

---

# Producer-Consumer Pattern in C++

## Overview

The Producer-Consumer pattern is a classic concurrency design pattern where one or more producer threads generate data and place it into a shared buffer, while one or more consumer threads retrieve and process that data. This pattern decouples data production from consumption, enabling efficient parallel processing.

The key challenge is coordinating access to the shared buffer in a thread-safe manner while avoiding busy-waiting and ensuring efficient synchronization.

## Core Components

### 1. Shared Buffer (Queue)
A data structure that holds items between producers and consumers.

### 2. Mutex
Protects the shared buffer from concurrent access race conditions.

### 3. Condition Variables
Enable threads to wait efficiently for specific conditions:
- **Producers** wait when the buffer is full
- **Consumers** wait when the buffer is empty

## Basic Implementation

```cpp
#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

template<typename T>
class ThreadSafeQueue {
private:
    std::queue<T> queue_;
    mutable std::mutex mutex_;
    std::condition_variable cond_var_;
    size_t max_size_;
    bool done_ = false;

public:
    explicit ThreadSafeQueue(size_t max_size = 10) 
        : max_size_(max_size) {}

    // Producer: Add item to queue
    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until queue has space
        cond_var_.wait(lock, [this]() { 
            return queue_.size() < max_size_ || done_; 
        });
        
        if (done_) return;
        
        queue_.push(std::move(item));
        cond_var_.notify_one(); // Wake up a consumer
    }

    // Consumer: Remove item from queue
    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        
        // Wait until queue has items or is done
        cond_var_.wait(lock, [this]() { 
            return !queue_.empty() || done_; 
        });
        
        if (queue_.empty()) {
            return false; // Queue is done and empty
        }
        
        item = std::move(queue_.front());
        queue_.pop();
        cond_var_.notify_one(); // Wake up a producer
        return true;
    }

    // Signal that no more items will be produced
    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        done_ = true;
        cond_var_.notify_all(); // Wake all waiting threads
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return queue_.size();
    }
};
```

## Usage Example

```cpp
#include <vector>
#include <random>

void producer(ThreadSafeQueue<int>& queue, int id, int count) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 100);
    
    for (int i = 0; i < count; ++i) {
        int value = dis(gen);
        queue.push(value);
        std::cout << "Producer " << id << " produced: " << value << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::cout << "Producer " << id << " finished\n";
}

void consumer(ThreadSafeQueue<int>& queue, int id) {
    int item;
    while (queue.pop(item)) {
        std::cout << "Consumer " << id << " consumed: " << item << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(150));
    }
    
    std::cout << "Consumer " << id << " finished\n";
}

int main() {
    ThreadSafeQueue<int> queue(5); // Buffer size of 5
    
    // Start multiple producers and consumers
    std::vector<std::thread> threads;
    
    // 2 producers, each producing 10 items
    for (int i = 0; i < 2; ++i) {
        threads.emplace_back(producer, std::ref(queue), i, 10);
    }
    
    // 3 consumers
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(consumer, std::ref(queue), i);
    }
    
    // Wait for producers to finish
    for (int i = 0; i < 2; ++i) {
        threads[i].join();
    }
    
    // Signal consumers that production is done
    queue.finish();
    
    // Wait for consumers to finish
    for (int i = 2; i < 5; ++i) {
        threads[i].join();
    }
    
    return 0;
}
```

## Advanced: Multiple Condition Variables

For more control, you can use separate condition variables for producers and consumers:

```cpp
template<typename T>
class AdvancedQueue {
private:
    std::queue<T> queue_;
    std::mutex mutex_;
    std::condition_variable not_full_;  // For producers
    std::condition_variable not_empty_; // For consumers
    size_t max_size_;
    bool done_ = false;

public:
    explicit AdvancedQueue(size_t max_size) : max_size_(max_size) {}

    void push(T item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_full_.wait(lock, [this]() { 
            return queue_.size() < max_size_ || done_; 
        });
        
        if (done_) return;
        
        queue_.push(std::move(item));
        not_empty_.notify_one(); // Only wake consumers
    }

    bool pop(T& item) {
        std::unique_lock<std::mutex> lock(mutex_);
        not_empty_.wait(lock, [this]() { 
            return !queue_.empty() || done_; 
        });
        
        if (queue_.empty()) return false;
        
        item = std::move(queue_.front());
        queue_.pop();
        not_full_.notify_one(); // Only wake producers
        return true;
    }

    void finish() {
        std::lock_guard<std::mutex> lock(mutex_);
        done_ = true;
        not_full_.notify_all();
        not_empty_.notify_all();
    }
};
```

## Key Concepts Explained

### Condition Variables

Condition variables allow threads to sleep until a specific condition is met, avoiding wasteful busy-waiting (spinning in a loop checking a condition).

**Key operations:**
- `wait(lock, predicate)`: Atomically releases the lock and waits until notified AND the predicate is true
- `notify_one()`: Wakes up one waiting thread
- `notify_all()`: Wakes up all waiting threads

### Why Use `unique_lock`?

`std::unique_lock` is required with condition variables because `wait()` needs to unlock and relock the mutex. `std::lock_guard` cannot be unlocked manually.

### Spurious Wakeups

Condition variables can wake up even when not notified (spurious wakeups). Always use a predicate (lambda) with `wait()` to verify the actual condition.

## Best Practices

1. **Always use predicates with wait()**: Protects against spurious wakeups and missed notifications
2. **Move semantics**: Use `std::move()` when pushing/popping to avoid unnecessary copies
3. **Bounded buffers**: Set a maximum queue size to prevent unbounded memory growth
4. **Graceful shutdown**: Implement a `finish()` or `done` mechanism to cleanly stop consumers
5. **Exception safety**: Consider what happens if queue operations throw exceptions

## Real-World Applications

- **Task scheduling**: Thread pools distributing work among worker threads
- **Data processing pipelines**: ETL (Extract, Transform, Load) operations
- **Event-driven systems**: GUI event loops, network packet processing
- **Logging systems**: Multiple threads writing to a centralized log processor
- **Media streaming**: Decoupling data acquisition from playback

## Performance Considerations

**Advantages:**
- Decouples production and consumption rates
- Enables parallel processing
- Reduces contention through buffering

**Potential issues:**
- Lock contention with many threads
- Context switching overhead
- Memory overhead from buffering

**Optimizations:**
- Use lock-free queues for high-performance scenarios
- Batch operations to reduce lock acquisitions
- Consider multiple queues to reduce contention

## Summary

The Producer-Consumer pattern with thread-safe queues and condition variables is fundamental to concurrent programming. It enables efficient coordination between threads that produce and consume data at different rates. The pattern relies on mutexes for thread safety and condition variables for efficient waiting, avoiding busy-waiting while ensuring threads wake up promptly when conditions change. Proper implementation requires careful attention to synchronization, graceful shutdown, and predicate-based waiting to handle spurious wakeups correctly.