# Condition Variable Any in C++ Concurrency

## Overview

`std::condition_variable_any` is a more flexible variant of `std::condition_variable` that can work with any type of lockable object, not just `std::unique_lock<std::mutex>`. While `std::condition_variable` is optimized for use with `std::unique_lock<std::mutex>`, `std::condition_variable_any` provides greater flexibility at the cost of slightly reduced performance and increased memory overhead.

## Key Characteristics

**Flexibility**: Works with any object that meets the BasicLockable requirements (has `lock()` and `unlock()` methods), including `std::shared_lock`, `std::lock_guard`, custom mutex wrappers, and even multiple mutexes locked together.

**Performance Trade-off**: The additional flexibility comes with a small performance penalty compared to `std::condition_variable`. For most applications using standard mutexes, `std::condition_variable` is preferred unless the extra flexibility is needed.

**Use Cases**: Particularly useful when working with shared mutexes for reader-writer scenarios, custom synchronization primitives, or when you need to wait on conditions while holding locks on multiple mutexes.

## Basic Usage with Different Lock Types

### Example 1: Using with std::shared_lock

One of the most practical uses of `std::condition_variable_any` is with `std::shared_lock` for reader-writer synchronization patterns.

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <vector>
#include <chrono>

class SharedDataStore {
private:
    std::shared_mutex mtx;
    std::condition_variable_any cv;
    std::vector<int> data;
    bool data_ready = false;

public:
    // Writer thread - uses exclusive lock
    void write_data(int value) {
        std::unique_lock lock(mtx);
        data.push_back(value);
        data_ready = true;
        std::cout << "Writer: Added " << value << " to data\n";
        cv.notify_all();
    }

    // Reader thread - uses shared lock for reading
    void read_data(int reader_id) {
        std::shared_lock lock(mtx);
        
        // Wait until data is ready (multiple readers can wait simultaneously)
        cv.wait(lock, [this] { return data_ready; });
        
        std::cout << "Reader " << reader_id << ": Data contains " 
                  << data.size() << " elements\n";
        
        // Simulate reading time
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }

    // Reader that promotes to writer
    void read_and_modify(int reader_id, int new_value) {
        // Start with shared lock for reading
        std::shared_lock shared_lock(mtx);
        cv.wait(shared_lock, [this] { return data_ready; });
        
        std::cout << "Reader-Writer " << reader_id 
                  << ": Read current size: " << data.size() << "\n";
        
        // Unlock shared lock before acquiring exclusive lock
        shared_lock.unlock();
        
        // Acquire exclusive lock for writing
        std::unique_lock unique_lock(mtx);
        data.push_back(new_value);
        std::cout << "Reader-Writer " << reader_id 
                  << ": Added " << new_value << "\n";
        cv.notify_all();
    }
};

int main() {
    SharedDataStore store;
    
    // Start multiple readers
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(&SharedDataStore::read_data, &store, i);
    }
    
    // Writer thread
    threads.emplace_back(&SharedDataStore::write_data, &store, 42);
    
    // Reader-writer thread
    threads.emplace_back(&SharedDataStore::read_and_modify, &store, 10, 99);
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 2: Custom Lockable Type

`std::condition_variable_any` can work with any custom type that implements the BasicLockable concept.

```cpp
#include <iostream>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <chrono>

// Custom lockable wrapper that adds logging
class LoggingMutex {
private:
    std::mutex mtx;
    std::string name;

public:
    explicit LoggingMutex(const std::string& n) : name(n) {}

    void lock() {
        std::cout << "[" << name << "] Attempting to lock...\n";
        mtx.lock();
        std::cout << "[" << name << "] Locked\n";
    }

    void unlock() {
        std::cout << "[" << name << "] Unlocking\n";
        mtx.unlock();
    }

    bool try_lock() {
        bool result = mtx.try_lock();
        std::cout << "[" << name << "] Try lock: " 
                  << (result ? "success" : "failed") << "\n";
        return result;
    }
};

class TaskQueue {
private:
    LoggingMutex mtx;
    std::condition_variable_any cv;
    int task_count = 0;
    bool finished = false;

public:
    TaskQueue() : mtx("TaskQueueMutex") {}

    void add_task() {
        std::unique_lock<LoggingMutex> lock(mtx);
        ++task_count;
        std::cout << "Added task. Total: " << task_count << "\n";
        cv.notify_one();
    }

    void process_tasks() {
        std::unique_lock<LoggingMutex> lock(mtx);
        
        while (!finished) {
            cv.wait(lock, [this] { return task_count > 0 || finished; });
            
            if (task_count > 0) {
                --task_count;
                std::cout << "Processing task. Remaining: " << task_count << "\n";
                
                // Unlock while processing to allow other operations
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                lock.lock();
            }
        }
    }

    void finish() {
        std::unique_lock<LoggingMutex> lock(mtx);
        finished = true;
        cv.notify_all();
    }
};

int main() {
    TaskQueue queue;
    
    std::thread processor(&TaskQueue::process_tasks, &queue);
    
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    queue.add_task();
    queue.add_task();
    queue.add_task();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    queue.finish();
    
    processor.join();
    
    return 0;
}
```

### Example 3: Timed Waiting with Different Lock Types

`std::condition_variable_any` supports timed waits with any lockable type.

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <chrono>

class TimeoutExample {
private:
    std::shared_mutex mtx;
    std::condition_variable_any cv;
    bool event_occurred = false;

public:
    void wait_for_event_with_timeout(int thread_id) {
        std::shared_lock lock(mtx);
        
        auto timeout = std::chrono::milliseconds(500);
        auto result = cv.wait_for(lock, timeout, 
                                   [this] { return event_occurred; });
        
        if (result) {
            std::cout << "Thread " << thread_id 
                      << ": Event occurred!\n";
        } else {
            std::cout << "Thread " << thread_id 
                      << ": Timeout - event did not occur\n";
        }
    }

    void wait_until_event(int thread_id) {
        std::shared_lock lock(mtx);
        
        auto deadline = std::chrono::steady_clock::now() + 
                        std::chrono::seconds(1);
        auto result = cv.wait_until(lock, deadline,
                                     [this] { return event_occurred; });
        
        if (result) {
            std::cout << "Thread " << thread_id 
                      << ": Event occurred before deadline!\n";
        } else {
            std::cout << "Thread " << thread_id 
                      << ": Deadline reached without event\n";
        }
    }

    void trigger_event() {
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        std::unique_lock lock(mtx);
        event_occurred = true;
        std::cout << "Event triggered!\n";
        cv.notify_all();
    }
};

int main() {
    TimeoutExample example;
    
    std::thread t1(&TimeoutExample::wait_for_event_with_timeout, &example, 1);
    std::thread t2(&TimeoutExample::wait_until_event, &example, 2);
    std::thread t3(&TimeoutExample::trigger_event, &example);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

### Example 4: Producer-Consumer with Shared Reader Access

This example demonstrates a scenario where multiple consumers can read data simultaneously using shared locks, while producers use exclusive locks.

```cpp
#include <iostream>
#include <thread>
#include <shared_mutex>
#include <condition_variable>
#include <queue>
#include <chrono>
#include <vector>

class DataBuffer {
private:
    std::shared_mutex mtx;
    std::condition_variable_any cv_data;
    std::condition_variable_any cv_space;
    std::queue<int> buffer;
    const size_t capacity = 5;
    bool done = false;

public:
    void produce(int producer_id, int count) {
        for (int i = 0; i < count; ++i) {
            std::unique_lock lock(mtx);
            
            // Wait for space in buffer
            cv_space.wait(lock, [this] { 
                return buffer.size() < capacity || done; 
            });
            
            if (done) break;
            
            int value = producer_id * 100 + i;
            buffer.push(value);
            std::cout << "Producer " << producer_id 
                      << " produced: " << value 
                      << " (buffer size: " << buffer.size() << ")\n";
            
            cv_data.notify_all();
            
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    }

    void consume(int consumer_id) {
        while (true) {
            std::unique_lock lock(mtx);
            
            cv_data.wait(lock, [this] { 
                return !buffer.empty() || done; 
            });
            
            if (buffer.empty() && done) break;
            
            if (!buffer.empty()) {
                int value = buffer.front();
                buffer.pop();
                std::cout << "Consumer " << consumer_id 
                          << " consumed: " << value 
                          << " (buffer size: " << buffer.size() << ")\n";
                
                cv_space.notify_all();
                
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(150));
            }
        }
    }

    void inspect(int inspector_id) {
        for (int i = 0; i < 3; ++i) {
            std::shared_lock lock(mtx);
            
            cv_data.wait(lock, [this] { 
                return !buffer.empty() || done; 
            });
            
            if (!buffer.empty()) {
                std::cout << "Inspector " << inspector_id 
                          << " sees buffer size: " << buffer.size() 
                          << " (non-intrusive read)\n";
            }
            
            lock.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
    }

    void shutdown() {
        std::unique_lock lock(mtx);
        done = true;
        cv_data.notify_all();
        cv_space.notify_all();
    }
};

int main() {
    DataBuffer buffer;
    
    std::vector<std::thread> threads;
    
    // Start producers
    threads.emplace_back(&DataBuffer::produce, &buffer, 1, 5);
    threads.emplace_back(&DataBuffer::produce, &buffer, 2, 5);
    
    // Start consumers
    threads.emplace_back(&DataBuffer::consume, &buffer, 1);
    threads.emplace_back(&DataBuffer::consume, &buffer, 2);
    
    // Start inspectors (using shared locks)
    threads.emplace_back(&DataBuffer::inspect, &buffer, 1);
    threads.emplace_back(&DataBuffer::inspect, &buffer, 2);
    
    // Let them run for a while
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    buffer.shutdown();
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## Performance Considerations

While `std::condition_variable_any` provides flexibility, there are performance implications:

**Memory Overhead**: `std::condition_variable_any` typically has a larger memory footprint than `std::condition_variable` because it needs to accommodate any lockable type.

**Runtime Overhead**: The type-erased nature of working with any lockable type introduces small runtime costs. For performance-critical code using standard mutexes, prefer `std::condition_variable`.

**When to Use Each**:
- Use `std::condition_variable` when working exclusively with `std::unique_lock<std::mutex>`
- Use `std::condition_variable_any` when you need flexibility with different lock types, especially `std::shared_lock` or custom lockable types

## Common Pitfalls

**Spurious Wakeups**: Like `std::condition_variable`, `std::condition_variable_any` can experience spurious wakeups. Always use a predicate with `wait()` operations.

**Deadlock with Shared Locks**: Be careful when upgrading from shared to exclusive locks. Always release the shared lock before acquiring an exclusive lock to avoid deadlocks.

**Lock Type Consistency**: Ensure that the lock type used with `wait()` is appropriate for the operation being performed. Use shared locks for read-only waits and exclusive locks when the predicate or subsequent operations modify shared state.

## Summary

`std::condition_variable_any` extends the capabilities of condition variables in C++ by supporting any type of lockable object, making it invaluable for advanced synchronization patterns. Its primary advantage is flexibility with `std::shared_lock` for reader-writer scenarios and custom mutex types, though this comes at a small performance cost. Use it when you need to wait on conditions while holding shared locks or working with non-standard mutex types. For standard mutex-based synchronization, `std::condition_variable` remains the preferred choice due to its optimized performance. The key to effective use is understanding when the additional flexibility justifies the overhead, particularly in scenarios involving reader-writer patterns where multiple threads need to wait while holding shared access to resources.