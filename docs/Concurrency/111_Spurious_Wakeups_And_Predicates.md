# Spurious Wakeups and Predicates in C++

## Introduction

In multithreaded C++ programming, condition variables are essential synchronization primitives that allow threads to wait for specific conditions to become true. However, they come with a subtle but critical challenge: **spurious wakeups**. A spurious wakeup occurs when a thread waiting on a condition variable wakes up without any explicit notification from another thread. Understanding and properly handling spurious wakeups is crucial for writing correct concurrent programs.

## What Are Spurious Wakeups?

Spurious wakeups are unexpected wake-ups of threads waiting on condition variables. They can occur due to:

- **Operating system behavior**: Some OS implementations may wake waiting threads as an optimization or due to internal scheduling decisions
- **Signal handling**: POSIX signals can interrupt wait operations
- **Implementation details**: The underlying threading library might wake threads for various internal reasons
- **Hardware considerations**: On multiprocessor systems, certain optimizations can trigger spurious wakeups

The C++ standard explicitly allows spurious wakeups, meaning your code must be prepared to handle them regardless of the platform.

## The Problem: Naive Waiting

Here's an example that demonstrates the problem with naive condition variable usage:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>

std::mutex mtx;
std::condition_variable cv;
bool data_ready = false;

void producer() {
    std::this_thread::sleep_for(std::chrono::seconds(1));
    
    std::lock_guard<std::mutex> lock(mtx);
    data_ready = true;
    std::cout << "Producer: Data is ready\n";
    cv.notify_one();
}

void consumer_incorrect() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // INCORRECT: No loop, vulnerable to spurious wakeups
    if (!data_ready) {
        cv.wait(lock);
    }
    
    // If spurious wakeup occurred, data_ready might still be false!
    std::cout << "Consumer: Processing data (might be wrong!)\n";
}

int main() {
    std::thread t1(producer);
    std::thread t2(consumer_incorrect);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

**Problem**: If a spurious wakeup occurs, the consumer thread will wake up even though `data_ready` is still `false`, leading to incorrect behavior.

## The Solution: Predicate-Based Waiting

The correct approach is to always check the condition in a loop:

```cpp
void consumer_correct_manual() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // CORRECT: Loop protects against spurious wakeups
    while (!data_ready) {
        cv.wait(lock);
    }
    
    std::cout << "Consumer: Processing data (guaranteed correct)\n";
}
```

This pattern works because:
1. When `wait()` is called, it releases the mutex and blocks
2. When the thread wakes (spuriously or legitimately), it reacquires the mutex
3. The loop checks if the condition is actually true
4. If the wakeup was spurious, the thread goes back to waiting
5. If the condition is true, the thread proceeds

## Better Solution: Predicate Overload

C++ provides a more elegant solution with the predicate-based overload of `wait()`:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> data_queue;

void producer_with_predicate(int id) {
    for (int i = 0; i < 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            data_queue.push(id * 10 + i);
            std::cout << "Producer " << id << " produced: " << (id * 10 + i) << "\n";
        }
        cv.notify_one();
    }
}

void consumer_with_predicate(int id) {
    for (int i = 0; i < 5; ++i) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait with predicate - automatically handles spurious wakeups
        cv.wait(lock, []{ return !data_queue.empty(); });
        
        int value = data_queue.front();
        data_queue.pop();
        
        std::cout << "Consumer " << id << " consumed: " << value << "\n";
    }
}

int main() {
    std::thread p1(producer_with_predicate, 1);
    std::thread c1(consumer_with_predicate, 1);
    
    p1.join();
    c1.join();
    
    return 0;
}
```

The predicate version `cv.wait(lock, predicate)` is equivalent to:
```cpp
while (!predicate()) {
    cv.wait(lock);
}
```

## Complete Producer-Consumer Example

Here's a comprehensive example demonstrating proper handling of spurious wakeups:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <chrono>

class ThreadSafeQueue {
private:
    std::queue<int> queue_;
    mutable std::mutex mtx_;
    std::condition_variable cv_;
    bool finished_ = false;
    
public:
    void push(int value) {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            queue_.push(value);
        }
        cv_.notify_one();
    }
    
    bool pop(int& value) {
        std::unique_lock<std::mutex> lock(mtx_);
        
        // Wait until queue has data OR production is finished
        cv_.wait(lock, [this]{ return !queue_.empty() || finished_; });
        
        if (queue_.empty()) {
            return false; // No more data and production finished
        }
        
        value = queue_.front();
        queue_.pop();
        return true;
    }
    
    void finish() {
        {
            std::lock_guard<std::mutex> lock(mtx_);
            finished_ = true;
        }
        cv_.notify_all(); // Wake all waiting consumers
    }
};

void producer(ThreadSafeQueue& queue, int id, int count) {
    for (int i = 0; i < count; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(50));
        int value = id * 100 + i;
        queue.push(value);
        std::cout << "Producer " << id << " pushed: " << value << "\n";
    }
}

void consumer(ThreadSafeQueue& queue, int id) {
    int value;
    while (queue.pop(value)) {
        std::cout << "Consumer " << id << " popped: " << value << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(75));
    }
    std::cout << "Consumer " << id << " finished\n";
}

int main() {
    ThreadSafeQueue queue;
    
    std::thread p1(producer, std::ref(queue), 1, 5);
    std::thread p2(producer, std::ref(queue), 2, 5);
    std::thread c1(consumer, std::ref(queue), 1);
    std::thread c2(consumer, std::ref(queue), 2);
    
    p1.join();
    p2.join();
    
    queue.finish(); // Signal that production is complete
    
    c1.join();
    c2.join();
    
    return 0;
}
```

## Timed Waits with Predicates

Condition variables also support timed waits with predicates:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool ready = false;

void waiter_with_timeout() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Wait for up to 2 seconds for the condition
    if (cv.wait_for(lock, std::chrono::seconds(2), []{ return ready; })) {
        std::cout << "Condition met!\n";
    } else {
        std::cout << "Timeout - condition not met\n";
    }
}

void waiter_until() {
    std::unique_lock<std::mutex> lock(mtx);
    
    auto deadline = std::chrono::system_clock::now() + std::chrono::seconds(2);
    
    // Wait until a specific time point
    if (cv.wait_until(lock, deadline, []{ return ready; })) {
        std::cout << "Condition met before deadline!\n";
    } else {
        std::cout << "Deadline reached - condition not met\n";
    }
}

int main() {
    std::thread t1(waiter_with_timeout);
    std::thread t2(waiter_until);
    
    // Uncomment to make threads succeed:
    // std::this_thread::sleep_for(std::chrono::seconds(1));
    // {
    //     std::lock_guard<std::mutex> lock(mtx);
    //     ready = true;
    // }
    // cv.notify_all();
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Best Practices

**Always use predicates or loops**: Never rely on a single `wait()` call without checking the condition.

```cpp
// GOOD
cv.wait(lock, [&]{ return condition_is_true; });

// ALSO GOOD
while (!condition_is_true) {
    cv.wait(lock);
}

// BAD - vulnerable to spurious wakeups
if (!condition_is_true) {
    cv.wait(lock);
}
```

**Use appropriate notification**: Choose between `notify_one()` and `notify_all()` based on your needs:
- `notify_one()`: When only one thread needs to wake up
- `notify_all()`: When multiple threads might need to check the condition

**Protect shared state**: Always modify shared state while holding the mutex, even when using condition variables.

**Consider using `wait_for` or `wait_until`**: For scenarios where indefinite waiting is unacceptable.

## Summary

Spurious wakeups are an inherent aspect of condition variable implementations that developers must handle correctly. The key takeaways are:

- **Spurious wakeups** can occur at any time, causing threads to wake without explicit notification
- **Always use a loop or predicate** when waiting on condition variables to recheck the actual condition after waking
- **Predicate-based `wait()`** provides cleaner, more maintainable code that automatically handles spurious wakeups
- The pattern `cv.wait(lock, predicate)` is equivalent to `while (!predicate()) cv.wait(lock)` but more concise
- **Timed waits** (`wait_for`, `wait_until`) also support predicates and handle spurious wakeups correctly
- Proper handling of spurious wakeups is essential for **correctness**, not just optimization

By consistently using predicate-based waiting, you ensure your multithreaded code behaves correctly regardless of spurious wakeups, making your programs more robust and portable across different platforms and threading implementations.