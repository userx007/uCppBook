# Guide on C++ condition variables and synchronization primitives

**Key Topics:**
- **Core Primitives**: Mutexes, lock guards (`lock_guard`, `unique_lock`, `scoped_lock`)
- **Condition Variables**: Detailed explanation with wait/notify mechanisms
- **Practical Examples**:
  - Producer-consumer pattern with multiple consumers
  - Thread pool with work queue
  - Custom barrier implementation
- **Modern Features**: Atomics, semaphores (C++20), latches and barriers
- **Common Pitfalls**: Spurious wakeups, deadlocks, lost notifications, lock contention
- **Best Practices**: RAII patterns, predicate-based waiting, performance optimization

The examples are production-ready and demonstrate proper synchronization patterns including handling spurious wakeups, avoiding deadlocks, and efficient resource management. The guide also explains when to use each primitive and their performance characteristics.

# C++ Condition Variables & Synchronization Primitives

## Overview

Synchronization primitives are fundamental tools for coordinating access to shared resources in concurrent programs. They prevent race conditions and ensure thread safety when multiple threads access shared data.

## Core Synchronization Primitives

### 1. Mutex (Mutual Exclusion)

A mutex ensures only one thread can access a critical section at a time.

```cpp
#include <mutex>
#include <thread>
#include <iostream>

std::mutex mtx;
int shared_counter = 0;

void increment() {
    for (int i = 0; i < 1000; ++i) {
        mtx.lock();
        ++shared_counter;
        mtx.unlock();
    }
}

// Better approach using RAII
void increment_safe() {
    for (int i = 0; i < 1000; ++i) {
        std::lock_guard<std::mutex> lock(mtx);
        ++shared_counter;
        // Automatically unlocks when lock goes out of scope
    }
}
```

### 2. Lock Guards

RAII wrappers that automatically manage mutex locking/unlocking:

- **`std::lock_guard`**: Basic RAII lock (C++11)
- **`std::unique_lock`**: More flexible, can be manually unlocked (C++11)
- **`std::scoped_lock`**: Can lock multiple mutexes (C++17)

```cpp
#include <mutex>

std::mutex m1, m2;

void safe_function() {
    // Locks multiple mutexes without deadlock
    std::scoped_lock lock(m1, m2);
    // Critical section
}

void flexible_function() {
    std::unique_lock<std::mutex> lock(m1);
    // Can unlock manually if needed
    lock.unlock();
    // Do some work
    lock.lock();
    // Lock again
}
```

## Condition Variables

Condition variables allow threads to wait for specific conditions to become true. They work in conjunction with mutexes to enable efficient thread synchronization.

### Basic Concepts

- **`std::condition_variable`**: Works with `std::unique_lock<std::mutex>`
- **`std::condition_variable_any`**: Works with any lock type (more flexible, slight overhead)

### Key Methods

1. **`wait(lock)`**: Atomically unlocks the mutex and blocks until notified
2. **`wait(lock, predicate)`**: Waits until predicate returns true (handles spurious wakeups)
3. **`notify_one()`**: Wakes up one waiting thread
4. **`notify_all()`**: Wakes up all waiting threads

### Example 1: Producer-Consumer Pattern

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::queue<int> data_queue;
std::mutex mtx;
std::condition_variable cv;
bool finished = false;

void producer() {
    for (int i = 0; i < 10; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        
        cv.notify_one(); // Wake up one consumer
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_all(); // Wake all consumers to exit
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait until data is available or production is finished
        cv.wait(lock, []{ return !data_queue.empty() || finished; });
        
        if (data_queue.empty() && finished) {
            break; // Exit if no more data and production finished
        }
        
        if (!data_queue.empty()) {
            int value = data_queue.front();
            data_queue.pop();
            lock.unlock(); // Unlock early to allow other threads to work
            
            std::cout << "Consumer " << id << " consumed: " << value << std::endl;
        }
    }
}

int main() {
    std::thread prod(producer);
    std::thread cons1(consumer, 1);
    std::thread cons2(consumer, 2);
    
    prod.join();
    cons1.join();
    cons2.join();
    
    return 0;
}
```

### Example 2: Thread Pool with Work Queue

```cpp
#include <vector>
#include <queue>
#include <functional>
#include <thread>
#include <mutex>
#include <condition_variable>

class ThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop = false;
    
public:
    ThreadPool(size_t num_threads) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        
                        // Wait for task or stop signal
                        condition.wait(lock, [this] { 
                            return stop || !tasks.empty(); 
                        });
                        
                        if (stop && tasks.empty()) {
                            return; // Exit thread
                        }
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    
                    task(); // Execute task outside the lock
                }
            });
        }
    }
    
    template<class F>
    void enqueue(F&& f) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
    
    ~ThreadPool() {
        {
            std::lock_guard<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};
```

### Example 3: Barrier Synchronization

```cpp
#include <mutex>
#include <condition_variable>

class Barrier {
private:
    std::mutex mtx;
    std::condition_variable cv;
    size_t threshold;
    size_t count = 0;
    size_t generation = 0;
    
public:
    explicit Barrier(size_t num_threads) : threshold(num_threads) {}
    
    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        size_t current_gen = generation;
        
        if (++count == threshold) {
            // Last thread to arrive
            count = 0;
            ++generation;
            cv.notify_all();
        } else {
            // Wait until all threads arrive
            cv.wait(lock, [this, current_gen] { 
                return current_gen != generation; 
            });
        }
    }
};

// Usage example
void parallel_task(int id, Barrier& barrier) {
    // Phase 1
    std::cout << "Thread " << id << " - Phase 1\n";
    barrier.wait(); // Synchronize
    
    // Phase 2
    std::cout << "Thread " << id << " - Phase 2\n";
    barrier.wait(); // Synchronize again
}
```

## Other Synchronization Primitives

### Atomic Operations

For simple operations, atomics are more efficient than mutexes:

```cpp
#include <atomic>

std::atomic<int> counter(0);

void increment_atomic() {
    for (int i = 0; i < 1000; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}
```

### Semaphores (C++20)

```cpp
#include <semaphore>

std::counting_semaphore<5> sem(5); // Allow 5 concurrent accesses

void limited_resource() {
    sem.acquire();
    // Use resource
    sem.release();
}
```

### Latches and Barriers (C++20)

```cpp
#include <latch>
#include <barrier>

// Latch: Single-use countdown
std::latch work_done(3);

void worker() {
    // Do work
    work_done.count_down();
}

// Barrier: Reusable synchronization point
std::barrier sync_point(3);

void phase_worker() {
    // Phase 1
    sync_point.arrive_and_wait();
    // Phase 2
    sync_point.arrive_and_wait();
}
```

## Common Pitfalls

### 1. Spurious Wakeups

Always use predicate-based `wait()`:

```cpp
// BAD: Doesn't handle spurious wakeups
cv.wait(lock);

// GOOD: Handles spurious wakeups
cv.wait(lock, []{ return data_ready; });
```

### 2. Deadlocks

```cpp
// BAD: Can deadlock
thread1: lock(m1); lock(m2);
thread2: lock(m2); lock(m1);

// GOOD: Lock in consistent order or use scoped_lock
std::scoped_lock lock(m1, m2); // Deadlock-free
```

### 3. Lost Notifications

```cpp
// BAD: Notification sent before wait
notify(); 
wait(); // Might miss the notification

// GOOD: Check condition under lock
{
    std::unique_lock<std::mutex> lock(mtx);
    cv.wait(lock, []{ return condition_met; });
}
```

### 4. Holding Lock Too Long

```cpp
// BAD: Holding lock during expensive operation
{
    std::lock_guard<std::mutex> lock(mtx);
    expensive_operation(); // Blocks other threads unnecessarily
}

// GOOD: Release lock before expensive operation
{
    std::lock_guard<std::mutex> lock(mtx);
    auto data = get_data();
} // Lock released here
expensive_operation(data);
```

## Best Practices

1. **Prefer RAII locks** (`lock_guard`, `unique_lock`, `scoped_lock`) over manual locking
2. **Use predicate-based `wait()`** to handle spurious wakeups automatically
3. **Minimize critical sections** - hold locks for the shortest time necessary
4. **Use atomics for simple counters** instead of mutexes when possible
5. **Lock in consistent order** to prevent deadlocks
6. **Prefer `notify_one()` over `notify_all()`** unless all threads need to wake
7. **Consider lock-free data structures** for high-performance scenarios
8. **Use C++20 features** (latches, barriers, semaphores) when available

## Performance Considerations

- **Mutexes**: Low overhead for uncontended locks, but expensive under contention
- **Atomics**: Very fast for simple operations, no context switching
- **Condition Variables**: Efficient for waiting (threads sleep instead of spin)
- **Spin locks**: Fast for very short critical sections, waste CPU otherwise
- **Lock-free algorithms**: Maximum performance but complex to implement correctly