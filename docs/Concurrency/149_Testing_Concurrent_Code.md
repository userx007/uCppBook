# Testing Concurrent Code

Testing concurrent code presents unique challenges compared to testing sequential programs. Race conditions, deadlocks, and timing-dependent bugs may only manifest under specific thread interleavings that are difficult to reproduce. This guide covers strategies for both deterministic and stress testing of multithreaded applications.

## Challenges in Testing Concurrent Code

Concurrent code testing faces several fundamental difficulties:

- **Non-determinism**: Thread scheduling is unpredictable, making bugs intermittent
- **Heisenbugs**: Bugs that disappear when you try to observe them (e.g., adding logging changes timing)
- **State space explosion**: The number of possible thread interleavings grows exponentially
- **Timing dependencies**: Bugs may only appear under specific timing conditions
- **Limited reproducibility**: Failures may be impossible to reproduce consistently

## Deterministic Testing Strategies

### 1. Synchronization Point Testing

Insert explicit synchronization points to control thread interleaving:

```cpp
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <iostream>

class SyncPoint {
    std::mutex mtx;
    std::condition_variable cv;
    std::atomic<int> counter{0};
    int target;

public:
    explicit SyncPoint(int num_threads) : target(num_threads) {}
    
    void wait() {
        std::unique_lock<std::mutex> lock(mtx);
        counter++;
        if (counter == target) {
            cv.notify_all();
        } else {
            cv.wait(lock, [this] { return counter == target; });
        }
    }
};

// Example: Testing a shared counter
class Counter {
    int value = 0;
    std::mutex mtx;
public:
    void increment() {
        std::lock_guard<std::mutex> lock(mtx);
        value++;
    }
    
    int get() const {
        return value;
    }
};

void test_concurrent_increment() {
    Counter counter;
    SyncPoint sync(3);
    
    auto worker = [&]() {
        sync.wait(); // All threads start simultaneously
        for (int i = 0; i < 1000; ++i) {
            counter.increment();
        }
    };
    
    std::thread t1(worker);
    std::thread t2(worker);
    std::thread t3(worker);
    
    t1.join();
    t2.join();
    t3.join();
    
    assert(counter.get() == 3000);
    std::cout << "Test passed: counter = " << counter.get() << std::endl;
}
```

### 2. Invariant Checking

Continuously verify invariants during execution:

```cpp
#include <vector>
#include <algorithm>

class BankAccount {
    mutable std::mutex mtx;
    double balance;
    std::vector<double> transaction_log;
    
    void check_invariant() const {
        // Invariant: balance equals sum of all transactions
        double sum = 0;
        for (double t : transaction_log) {
            sum += t;
        }
        assert(std::abs(balance - sum) < 0.01);
    }
    
public:
    explicit BankAccount(double initial) : balance(initial) {
        transaction_log.push_back(initial);
    }
    
    void deposit(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
        transaction_log.push_back(amount);
        check_invariant();
    }
    
    void withdraw(double amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance -= amount;
        transaction_log.push_back(-amount);
        check_invariant();
    }
    
    double get_balance() const {
        std::lock_guard<std::mutex> lock(mtx);
        check_invariant();
        return balance;
    }
};
```

### 3. Mock Schedulers

Create a deterministic scheduler for unit tests:

```cpp
#include <queue>
#include <functional>
#include <map>

class MockScheduler {
    std::map<std::thread::id, std::queue<std::function<void()>>> task_queues;
    std::vector<std::thread::id> execution_order;
    size_t current_thread_index = 0;
    
public:
    void add_task(std::thread::id tid, std::function<void()> task) {
        task_queues[tid].push(task);
    }
    
    void set_execution_order(std::vector<std::thread::id> order) {
        execution_order = order;
    }
    
    void run_next_task() {
        if (current_thread_index >= execution_order.size()) return;
        
        auto tid = execution_order[current_thread_index];
        if (!task_queues[tid].empty()) {
            auto task = task_queues[tid].front();
            task_queues[tid].pop();
            task();
        }
        current_thread_index++;
    }
    
    void run_all() {
        while (current_thread_index < execution_order.size()) {
            run_next_task();
        }
    }
};
```

## Stress Testing Strategies

### 1. Load Testing with Multiple Threads

```cpp
#include <chrono>
#include <random>

class ThreadSafeQueue {
    std::queue<int> queue;
    mutable std::mutex mtx;
    std::condition_variable cv;
    bool done = false;
    
public:
    void push(int value) {
        std::lock_guard<std::mutex> lock(mtx);
        queue.push(value);
        cv.notify_one();
    }
    
    bool pop(int& value) {
        std::unique_lock<std::mutex> lock(mtx);
        cv.wait(lock, [this] { return !queue.empty() || done; });
        
        if (queue.empty()) return false;
        
        value = queue.front();
        queue.pop();
        return true;
    }
    
    void finish() {
        std::lock_guard<std::mutex> lock(mtx);
        done = true;
        cv.notify_all();
    }
};

void stress_test_queue() {
    ThreadSafeQueue queue;
    std::atomic<int> items_produced{0};
    std::atomic<int> items_consumed{0};
    
    const int num_producers = 4;
    const int num_consumers = 4;
    const int items_per_producer = 10000;
    
    std::vector<std::thread> producers;
    std::vector<std::thread> consumers;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    // Launch producers
    for (int i = 0; i < num_producers; ++i) {
        producers.emplace_back([&, i]() {
            std::mt19937 rng(i);
            for (int j = 0; j < items_per_producer; ++j) {
                queue.push(rng());
                items_produced++;
                
                // Random small delays to vary timing
                if (j % 100 == 0) {
                    std::this_thread::sleep_for(std::chrono::microseconds(1));
                }
            }
        });
    }
    
    // Launch consumers
    for (int i = 0; i < num_consumers; ++i) {
        consumers.emplace_back([&]() {
            int value;
            while (queue.pop(value)) {
                items_consumed++;
            }
        });
    }
    
    // Wait for producers
    for (auto& t : producers) {
        t.join();
    }
    
    queue.finish();
    
    // Wait for consumers
    for (auto& t : consumers) {
        t.join();
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Stress test completed in " << duration.count() << "ms\n";
    std::cout << "Items produced: " << items_produced << std::endl;
    std::cout << "Items consumed: " << items_consumed << std::endl;
    
    assert(items_produced == items_consumed);
    assert(items_produced == num_producers * items_per_producer);
}
```

### 2. Race Condition Detection with Thread Sanitizer

```cpp
// Compile with: g++ -std=c++17 -fsanitize=thread -g test.cpp
// ThreadSanitizer will detect data races

class UnsafeCounter {
    int value = 0; // Unprotected shared state
    
public:
    void increment() {
        value++; // Data race!
    }
    
    int get() const {
        return value; // Data race!
    }
};

void test_with_tsan() {
    UnsafeCounter counter;
    
    auto worker = [&]() {
        for (int i = 0; i < 10000; ++i) {
            counter.increment();
        }
    };
    
    std::thread t1(worker);
    std::thread t2(worker);
    
    t1.join();
    t2.join();
    
    // ThreadSanitizer will report the race condition
    std::cout << "Counter: " << counter.get() << std::endl;
}
```

### 3. Chaos Testing

Introduce random delays and failures:

```cpp
#include <optional>

class ChaosMonkey {
    std::mt19937 rng;
    double failure_rate;
    
public:
    explicit ChaosMonkey(double rate = 0.01) 
        : rng(std::random_device{}()), failure_rate(rate) {}
    
    void maybe_delay() {
        std::uniform_real_distribution<> dist(0.0, 1.0);
        if (dist(rng) < 0.1) { // 10% chance of delay
            std::uniform_int_distribution<> delay_dist(1, 100);
            std::this_thread::sleep_for(
                std::chrono::microseconds(delay_dist(rng))
            );
        }
    }
    
    bool should_fail() {
        std::uniform_real_distribution<> dist(0.0, 1.0);
        return dist(rng) < failure_rate;
    }
};

template<typename T>
class FlakyService {
    ChaosMonkey chaos;
    std::function<T()> operation;
    
public:
    FlakyService(std::function<T()> op, double failure_rate = 0.01)
        : chaos(failure_rate), operation(op) {}
    
    std::optional<T> execute() {
        chaos.maybe_delay();
        
        if (chaos.should_fail()) {
            return std::nullopt; // Simulated failure
        }
        
        return operation();
    }
};

void chaos_test() {
    FlakyService<int> service([]() { return 42; }, 0.2);
    
    int successes = 0;
    int failures = 0;
    
    for (int i = 0; i < 1000; ++i) {
        auto result = service.execute();
        if (result) {
            successes++;
            assert(*result == 42);
        } else {
            failures++;
        }
    }
    
    std::cout << "Successes: " << successes << ", Failures: " << failures << std::endl;
    // Should see approximately 80% success rate with 0.2 failure rate
}
```

### 4. Deadlock Detection

```cpp
#include <chrono>

class DeadlockDetector {
    std::chrono::seconds timeout;
    
public:
    explicit DeadlockDetector(std::chrono::seconds t = std::chrono::seconds(5))
        : timeout(t) {}
    
    template<typename Func>
    bool run_with_timeout(Func func) {
        std::atomic<bool> completed{false};
        
        std::thread worker([&]() {
            func();
            completed = true;
        });
        
        auto start = std::chrono::steady_clock::now();
        
        while (!completed) {
            auto elapsed = std::chrono::steady_clock::now() - start;
            if (elapsed > timeout) {
                // Potential deadlock detected
                std::cerr << "Timeout detected - possible deadlock!" << std::endl;
                worker.detach(); // Can't safely join
                return false;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
        
        worker.join();
        return true;
    }
};

void test_potential_deadlock() {
    std::mutex m1, m2;
    DeadlockDetector detector(std::chrono::seconds(2));
    
    auto deadlock_prone = [&]() {
        std::thread t1([&]() {
            std::lock_guard<std::mutex> lock1(m1);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock2(m2);
        });
        
        std::thread t2([&]() {
            std::lock_guard<std::mutex> lock2(m2);
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::lock_guard<std::mutex> lock1(m1);
        });
        
        t1.join();
        t2.join();
    };
    
    bool success = detector.run_with_timeout(deadlock_prone);
    if (!success) {
        std::cout << "Deadlock detected in test!" << std::endl;
    }
}
```

## Best Practices

1. **Use sanitizers**: Enable ThreadSanitizer (`-fsanitize=thread`) and AddressSanitizer during testing
2. **Test at scale**: Run tests with many more threads than production will use
3. **Vary timing**: Introduce random delays to expose timing-dependent bugs
4. **Repeat tests**: Run concurrent tests thousands of times to catch rare race conditions
5. **Test failure scenarios**: Simulate network delays, resource exhaustion, and failures
6. **Use assertions liberally**: Check invariants frequently, especially in debug builds
7. **Isolate tests**: Ensure tests don't share state that could cause interference
8. **Monitor resources**: Watch for memory leaks, file descriptor leaks, and deadlocks

## Summary

Testing concurrent code requires a multi-faceted approach combining deterministic and stress testing strategies. Deterministic testing uses synchronization points, invariant checking, and mock schedulers to create reproducible test scenarios. Stress testing employs high loads, chaos engineering, and race condition detectors to uncover bugs that only manifest under specific conditions.

Key takeaways:
- Non-determinism makes concurrent bugs difficult to reproduce and diagnose
- Deterministic testing provides control over thread interleaving for specific scenarios
- Stress testing exposes rare race conditions through volume and chaos
- Tools like ThreadSanitizer are essential for detecting data races
- Combining multiple testing strategies provides the best coverage
- Testing should verify both correctness (invariants) and liveness (no deadlocks)
- Always run concurrent tests repeatedly and at high load to increase confidence

Effective concurrent testing builds confidence that multithreaded code will behave correctly under the unpredictable conditions of real-world deployment.