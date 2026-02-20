# Condition Variable Basics in C++

## **Table of content**

[**Basic Operations**](#basic-operations)<br>
[**Basic Producer-Consumer Pattern**](#basic-producer-consumer-pattern)<br>
[**Thread Synchronization for Task Completion**](#thread-synchronization-for-task-completion)<br>
[**Using wait_for with Timeout**](#using-wait_for-with-timeout)<br>
[**Multiple Threads Waiting - Barrier Pattern**](#multiple-threads-waiting---barrier-pattern)<br>
[**Spurious Wakeups and Predicates**](#spurious-wakeups-and-predicates)<br>
[**Solution - Predicate-Based Waiting**](#solution-predicate-based-waiting)<br>

[****](#)<br>
[****](#)<br>


A **condition variable** is a synchronization primitive that enables threads to wait until a particular condition is met. It provides an efficient mechanism for thread communication, allowing threads to block until they are notified by another thread that something of interest has occurred.

```cpp
// Bad approach - wastes CPU cycles
while (!dataReady) {
    // Spin and check repeatedly
}
```

```cpp
// Good approach - thread sleeps until notified
std::unique_lock<std::mutex> lock(mtx);
cv.wait(lock, []{ return dataReady; });
```

***Key Components***

1. **std::condition_variable**: The synchronization object itself
2. **std::mutex**: Protects the shared state
3. **std::unique_lock**: A lockable wrapper that can be unlocked/relocked automatically
4. **Predicate**: A condition that must be true for the thread to proceed


## ***Basic Operations***

***wait()***
Blocks the current thread until notified and the predicate is satisfied:
1. Atomically unlocks the mutex and blocks the thread
2. When notified, reacquires the lock and wakes up
3. If a predicate is provided, checks it and continues waiting if false

```cpp
// wait: blocks indefinitely until notified
void wait(std::unique_lock<std::mutex>& lock);
void wait(std::unique_lock<std::mutex>& lock, Predicate pred);

// wait_for: blocks until notified or duration expires
std::cv_status wait_for(std::unique_lock<std::mutex>& lock, const std::chrono::duration<Rep, Period>& rel_time);
bool wait_for(std::unique_lock<std::mutex>& lock, const std::chrono::duration<Rep, Period>& rel_time, Predicate pred);

// wait_until: blocks until notified or time_point deadline reached
std::cv_status wait_until(std::unique_lock<std::mutex>& lock, const std::chrono::time_point<Clock, Duration>& abs_time);
bool wait_until(std::unique_lock<std::mutex>& lock, const std::chrono::time_point<Clock, Duration>& abs_time, Predicate pred);
```

---

***notify_one()***
Wakes up one waiting thread:

```cpp
void notify_one() noexcept;
```

---

***notify_all()***
Wakes up all waiting threads:

```cpp
void notify_all() noexcept;
```
--- 

[Back to top ...](#table-of-content)<br>

## ***Basic Producer-Consumer Pattern***

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>

std::mutex mtx;
std::condition_variable cv;
std::queue<int> dataQueue;
bool finished = false;

void producer() {
    for (int i = 1; i <= 5; ++i) {
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        {
            std::lock_guard<std::mutex> lock(mtx);
            dataQueue.push(i);
            std::cout << "Produced: " << i << std::endl;
        }
        
        cv.notify_one(); // Notify one waiting consumer
    }
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        finished = true;
    }
    cv.notify_all(); // Notify all consumers to check finished flag
}

void consumer(int id) {
    while (true) {
        std::unique_lock<std::mutex> lock(mtx);
        
        // Wait until data is available or production is finished
        cv.wait(lock, []{ return !dataQueue.empty() || finished; });
        
        if (!dataQueue.empty()) {
            int value = dataQueue.front();
            dataQueue.pop();
            lock.unlock(); // Unlock before processing
            
            std::cout << "Consumer " << id << " consumed: " << value << std::endl;
        } else if (finished) {
            break; // Exit if finished and no data
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
[Back to top ...](#table-of-content)<br>

---

## ***Thread Synchronization for Task Completion***

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool taskCompleted = false;

void performTask() {
    std::cout << "Task started..." << std::endl;
    std::this_thread::sleep_for(std::chrono::seconds(2));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        taskCompleted = true;
        std::cout << "Task completed!" << std::endl;
    }
    
    cv.notify_one(); // Notify the waiting thread
}

void waitForTask() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Waiting for task to complete..." << std::endl;
    
    // Wait until taskCompleted is true
    cv.wait(lock, []{ return taskCompleted; });
    
    std::cout << "Task completion detected, proceeding..." << std::endl;
}

int main() {
    std::thread worker(performTask);
    std::thread waiter(waitForTask);
    
    worker.join();
    waiter.join();
    
    return 0;
}
```

[Back to top ...](#table-of-content)<br>

---

## ***Using wait_for with Timeout***

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>

std::mutex mtx;
std::condition_variable cv;
bool dataReady = false;

void prepareData() {
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    {
        std::lock_guard<std::mutex> lock(mtx);
        dataReady = true;
    }
    
    cv.notify_one();
}

void waitWithTimeout() {
    std::unique_lock<std::mutex> lock(mtx);
    
    // Wait for up to 2 seconds
    if (cv.wait_for(lock, std::chrono::seconds(2), []{ return dataReady; })) {
        std::cout << "Data is ready!" << std::endl;
    } else {
        std::cout << "Timeout: Data not ready within 2 seconds" << std::endl;
    }
}

int main() {
    std::thread preparer(prepareData);
    std::thread waiter(waitWithTimeout);
    
    preparer.join();
    waiter.join();
    
    return 0;
}
```

[Back to top ...](#table-of-content)<br>

---

## ***Multiple Threads Waiting - Barrier Pattern***

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>

std::mutex mtx;
std::condition_variable cv;
int threadsReady = 0;
const int TOTAL_THREADS = 3;

void workerThread(int id) {
    // Phase 1: Do some work
    std::cout << "Thread " << id << " doing initial work..." << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    
    // Signal ready and wait for all threads
    {
        std::unique_lock<std::mutex> lock(mtx);
        threadsReady++;
        std::cout << "Thread " << id << " ready, waiting for others..." << std::endl;
        
        // Wait until all threads are ready
        cv.wait(lock, []{ return threadsReady == TOTAL_THREADS; });
    }
    
    // Notify others that we've checked the condition
    cv.notify_all();
    
    // Phase 2: Continue after synchronization
    std::cout << "Thread " << id << " proceeding to phase 2" << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 1; i <= TOTAL_THREADS; ++i) {
        threads.emplace_back(workerThread, i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

[Back to top ...](#table-of-content)<br>

---

## ***Spurious Wakeups and Predicates***

A spurious wakeup occurs when a thread waiting on a condition variable wakes up without any explicit notification from another thread. They can occur due to:

- **Operating system behavior**: Some OS implementations may wake waiting threads as an optimization or due to internal scheduling decisions
- **Signal handling**: POSIX signals can interrupt wait operations
- **Implementation details**: The underlying threading library might wake threads for various internal reasons
- **Hardware considerations**: On multiprocessor systems, certain optimizations can trigger spurious wakeups


***Solution Predicate-Based Waiting***

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
/*
    if (!data_ready) {
        cv.wait(lock);
    }
*/
    // CORRECT: Loop protects against spurious wakeups
    while (!data_ready) {
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

[Back to top ...](#table-of-content)<br>

---

***Solution 2 Predicate Overload***

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

[Back to top ...](#table-of-content)<br>

---


***Timed Waits with Predicates***

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

[Back to top ...](#table-of-content)<br>

## Best Practices

***Always use predicates or loops***

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

***Use appropriate notification***

- `notify_one()`: When only one thread needs to wake up
- `notify_all()`: When multiple threads might need to check the condition

***Protect shared state***
Always modify shared state while holding the mutex, even when using condition variables.

**Consider using `wait_for` or `wait_until`**: For scenarios where indefinite waiting is unacceptable.

[Back to top ...](#table-of-content)<br>

---


***`std::condition_variable_any`***

- a more flexible variant of `std::condition_variable`

- can work with any type of lockable object not just `std::unique_lock<std::mutex>`
- - `std::shared_lock` 
- - `std::lock_guard` 
- - custom mutex wrappers
- - multiple mutexes locked together

- `std::condition_variable` is optimized for use with `std::unique_lock<std::mutex>`
- `std::condition_variable_any` greater flexibility at the cost of:
- - slightly reduced performance 
- - increased memory overhead

- useful when working with: 
- - shared mutexes for reader-writer scenarios
- - custom synchronization primitives
- - when need to wait on conditions while holding locks on multiple mutexes.


***Using with std::shared_lock***

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

[Back to top ...](#table-of-content)<br>

---

***Custom Lockable Type***

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

[Back to top ...](#table-of-content)<br>

---

***Timed Waiting with Different Lock Types***

```cpp

// 'std::condition_variable_any' supports timed waits with any lockable type.

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

    // read 'event_occurred', use 'std::shared_lock'
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

    // read 'event_occurred', use 'std::shared_lock'
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

    // write 'event_occurred', use 'std::unique_lock'
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

[Back to top ...](#table-of-content)<br>

## ***Producer-Consumer with Shared Reader Access***

```cpp

// multiple consumers can read data simultaneously using shared locks, 
// producers use exclusive locks.

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

[Back to top ...](#table-of-content)<br>

---

## Summary

**Condition variables** are essential synchronization primitives in C++ for efficient thread communication. The `std::condition_variable` class enables threads to wait for specific conditions without wasting CPU cycles in busy-wait loops.

Key takeaways:
- Condition variables work with `std::mutex` and `std::unique_lock` to provide wait/notify patterns
- `std::condition_variable::wait()` needs to unlock and relock the mutex with `unlock()` and `lock()` methods. 
- `std::unique_lock` provides this, while `std::lock_guard` does not.
- The shared state accessed in the predicate must always be protected by the same mutex used with the condition variable.
- The predicate is evaluated while the lock is held.
- The `wait()` function atomically releases the lock and blocks until notified
- Always use predicates to guard against spurious wakeups and lost notifications
- `notify_one()` wakes one thread, while `notify_all()` wakes all waiting threads
- Common use cases include producer-consumer patterns, task synchronization, and barrier implementations
- The shared condition must always be protected by the associated mutex

Condition variables are particularly useful when implementing thread pools, event systems, and any scenario where threads need to coordinate based on shared state changes.

**Spurious wakeups** are an inherent aspect of condition variable implementations that developers must handle correctly. The key takeaways are:

- **Spurious wakeups** can occur at any time, causing threads to wake without explicit notification
- **Always use a loop or predicate** when waiting on condition variables to recheck the actual condition after waking
- **Predicate-based `wait()`** provides cleaner, more maintainable code that automatically handles spurious wakeups
- The pattern `cv.wait(lock, predicate)` is equivalent to `while (!predicate()) cv.wait(lock)` but more concise
- **Timed waits** (`wait_for`, `wait_until`) also support predicates and handle spurious wakeups correctly
- Proper handling of spurious wakeups is essential for **correctness**, not just optimization
