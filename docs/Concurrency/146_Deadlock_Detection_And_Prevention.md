# Deadlock Detection and Prevention in C++

## Overview

Deadlock occurs when two or more threads are blocked indefinitely, each waiting for resources held by the others. This creates a circular dependency where no thread can proceed. Deadlock is one of the most challenging issues in concurrent programming, as it can cause applications to hang completely.

## The Four Conditions for Deadlock (Coffman Conditions)

Deadlock can only occur when all four of these conditions are present simultaneously:

1. **Mutual Exclusion**: Resources cannot be shared; only one thread can hold a resource at a time
2. **Hold and Wait**: Threads holding resources can request additional resources
3. **No Preemption**: Resources cannot be forcibly taken from threads
4. **Circular Wait**: A circular chain of threads exists, where each thread waits for a resource held by the next thread in the chain

## Classic Deadlock Example

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1;
std::mutex mutex2;

void thread1_function() {
    std::lock_guard<std::mutex> lock1(mutex1);
    std::cout << "Thread 1: Locked mutex1\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Trying to lock mutex2 while holding mutex1
    std::lock_guard<std::mutex> lock2(mutex2);
    std::cout << "Thread 1: Locked mutex2\n";
}

void thread2_function() {
    std::lock_guard<std::mutex> lock2(mutex2);
    std::cout << "Thread 2: Locked mutex2\n";
    
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    
    // Trying to lock mutex1 while holding mutex2
    std::lock_guard<std::mutex> lock1(mutex1);
    std::cout << "Thread 2: Locked mutex1\n";
}

int main() {
    std::thread t1(thread1_function);
    std::thread t2(thread2_function);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

This code creates a classic deadlock: Thread 1 locks mutex1 and waits for mutex2, while Thread 2 locks mutex2 and waits for mutex1.

## Prevention Strategy 1: Lock Ordering

The most common prevention technique is to establish a global ordering of locks and always acquire them in the same order.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

class BankAccount {
private:
    int balance;
    mutable std::mutex mtx;
    int id;
    
public:
    BankAccount(int initial_balance, int account_id) 
        : balance(initial_balance), id(account_id) {}
    
    int get_id() const { return id; }
    
    void deposit(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        balance += amount;
    }
    
    bool withdraw(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        if (balance >= amount) {
            balance -= amount;
            return true;
        }
        return false;
    }
    
    int get_balance() const {
        std::lock_guard<std::mutex> lock(mtx);
        return balance;
    }
    
    std::mutex& get_mutex() { return mtx; }
};

// Deadlock-free transfer using lock ordering
void transfer(BankAccount& from, BankAccount& to, int amount) {
    // Always lock in order of account ID to prevent deadlock
    BankAccount* first = &from;
    BankAccount* second = &to;
    
    if (from.get_id() > to.get_id()) {
        std::swap(first, second);
    }
    
    std::lock_guard<std::mutex> lock1(first->get_mutex());
    std::lock_guard<std::mutex> lock2(second->get_mutex());
    
    if (from.withdraw(amount)) {
        to.deposit(amount);
        std::cout << "Transferred " << amount << " from account " 
                  << from.get_id() << " to " << to.get_id() << "\n";
    }
}

int main() {
    BankAccount acc1(1000, 1);
    BankAccount acc2(2000, 2);
    
    std::thread t1([&]() { transfer(acc1, acc2, 100); });
    std::thread t2([&]() { transfer(acc2, acc1, 200); });
    
    t1.join();
    t2.join();
    
    std::cout << "Account 1: " << acc1.get_balance() << "\n";
    std::cout << "Account 2: " << acc2.get_balance() << "\n";
    
    return 0;
}
```

## Prevention Strategy 2: std::lock for Simultaneous Locking

C++ provides `std::lock` which can lock multiple mutexes simultaneously without risk of deadlock.

```cpp
#include <iostream>
#include <thread>
#include <mutex>

class Resource {
private:
    std::string name;
    mutable std::mutex mtx;
    
public:
    Resource(const std::string& n) : name(n) {}
    
    void use_with(Resource& other) {
        // Lock both mutexes simultaneously without deadlock
        std::lock(mtx, other.mtx);
        
        // Adopt the already-locked mutexes
        std::lock_guard<std::mutex> lock1(mtx, std::adopt_lock);
        std::lock_guard<std::mutex> lock2(other.mtx, std::adopt_lock);
        
        std::cout << "Using " << name << " with " << other.name << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    std::string get_name() const { return name; }
};

int main() {
    Resource r1("Resource1");
    Resource r2("Resource2");
    
    std::thread t1([&]() { r1.use_with(r2); });
    std::thread t2([&]() { r2.use_with(r1); });
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Prevention Strategy 3: std::scoped_lock (C++17)

C++17 introduced `std::scoped_lock`, which simplifies simultaneous locking even further.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <string>

class SharedResource {
private:
    std::string data;
    mutable std::mutex mtx;
    
public:
    SharedResource(const std::string& initial) : data(initial) {}
    
    void swap_data(SharedResource& other) {
        // Automatically locks both mutexes without deadlock
        std::scoped_lock lock(mtx, other.mtx);
        
        std::swap(data, other.data);
        std::cout << "Swapped data between resources\n";
    }
    
    std::string get_data() const {
        std::lock_guard<std::mutex> lock(mtx);
        return data;
    }
    
    std::mutex& get_mutex() { return mtx; }
};

int main() {
    SharedResource res1("Data1");
    SharedResource res2("Data2");
    
    std::thread t1([&]() { res1.swap_data(res2); });
    std::thread t2([&]() { res2.swap_data(res1); });
    
    t1.join();
    t2.join();
    
    std::cout << "Resource 1: " << res1.get_data() << "\n";
    std::cout << "Resource 2: " << res2.get_data() << "\n";
    
    return 0;
}
```

## Prevention Strategy 4: Try-Lock with Timeout

Using `try_lock` or timed locking can prevent indefinite blocking.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mutex1;
std::mutex mutex2;

void attempt_transfer() {
    while (true) {
        // Try to lock both mutexes
        std::unique_lock<std::mutex> lock1(mutex1, std::defer_lock);
        std::unique_lock<std::mutex> lock2(mutex2, std::defer_lock);
        
        // Try to lock both, returns -1 if successful, else index of failed lock
        int result = std::try_lock(lock1, lock2);
        
        if (result == -1) {
            // Successfully acquired both locks
            std::cout << "Thread " << std::this_thread::get_id() 
                      << ": Acquired both locks\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            return;
        } else {
            // Failed to acquire locks, retry after a small delay
            std::cout << "Thread " << std::this_thread::get_id() 
                      << ": Failed to acquire locks, retrying...\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
        }
    }
}

int main() {
    std::thread t1(attempt_transfer);
    std::thread t2(attempt_transfer);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Detection Strategy: Timeout-Based Detection

While C++ doesn't have built-in deadlock detection, you can implement timeout-based detection.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

class DeadlockDetector {
private:
    std::timed_mutex mutex1;
    std::timed_mutex mutex2;
    
public:
    bool safe_operation(int thread_id) {
        using namespace std::chrono_literals;
        
        std::unique_lock<std::timed_mutex> lock1(mutex1, std::defer_lock);
        std::unique_lock<std::timed_mutex> lock2(mutex2, std::defer_lock);
        
        // Try to acquire locks with timeout
        if (!lock1.try_lock_for(500ms)) {
            std::cout << "Thread " << thread_id 
                      << ": Potential deadlock detected on mutex1\n";
            return false;
        }
        
        std::this_thread::sleep_for(100ms); // Simulate work
        
        if (!lock2.try_lock_for(500ms)) {
            std::cout << "Thread " << thread_id 
                      << ": Potential deadlock detected on mutex2\n";
            return false;
        }
        
        std::cout << "Thread " << thread_id << ": Operation completed\n";
        return true;
    }
};

int main() {
    DeadlockDetector detector;
    
    std::thread t1([&]() {
        if (!detector.safe_operation(1)) {
            std::cout << "Thread 1: Aborting due to timeout\n";
        }
    });
    
    std::thread t2([&]() {
        if (!detector.safe_operation(2)) {
            std::cout << "Thread 2: Aborting due to timeout\n";
        }
    });
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Advanced: Hierarchical Locking

Implementing a lock hierarchy can systematically prevent deadlocks.

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <stdexcept>

class HierarchicalMutex {
private:
    std::mutex internal_mutex;
    unsigned long const hierarchy_value;
    unsigned long previous_hierarchy_value;
    
    static thread_local unsigned long this_thread_hierarchy_value;
    
    void check_for_hierarchy_violation() {
        if (this_thread_hierarchy_value <= hierarchy_value) {
            throw std::logic_error("Mutex hierarchy violated");
        }
    }
    
    void update_hierarchy_value() {
        previous_hierarchy_value = this_thread_hierarchy_value;
        this_thread_hierarchy_value = hierarchy_value;
    }
    
public:
    explicit HierarchicalMutex(unsigned long value) 
        : hierarchy_value(value), previous_hierarchy_value(0) {}
    
    void lock() {
        check_for_hierarchy_violation();
        internal_mutex.lock();
        update_hierarchy_value();
    }
    
    void unlock() {
        if (this_thread_hierarchy_value != hierarchy_value) {
            throw std::logic_error("Mutex hierarchy violated on unlock");
        }
        this_thread_hierarchy_value = previous_hierarchy_value;
        internal_mutex.unlock();
    }
    
    bool try_lock() {
        check_for_hierarchy_violation();
        if (!internal_mutex.try_lock()) {
            return false;
        }
        update_hierarchy_value();
        return true;
    }
};

thread_local unsigned long HierarchicalMutex::this_thread_hierarchy_value(ULONG_MAX);

// Usage example
HierarchicalMutex high_level_mutex(10000);
HierarchicalMutex low_level_mutex(5000);

void correct_usage() {
    std::lock_guard<HierarchicalMutex> lk1(high_level_mutex);
    std::lock_guard<HierarchicalMutex> lk2(low_level_mutex);
    // This works: locking in descending hierarchy order
}

void incorrect_usage() {
    std::lock_guard<HierarchicalMutex> lk1(low_level_mutex);
    // This would throw: trying to lock higher hierarchy mutex
    // std::lock_guard<HierarchicalMutex> lk2(high_level_mutex);
}

int main() {
    std::thread t1(correct_usage);
    t1.join();
    
    std::cout << "Hierarchical locking enforced successfully\n";
    
    return 0;
}
```

## Summary

**Key Takeaways:**

1. **Deadlock occurs when four conditions are met**: mutual exclusion, hold and wait, no preemption, and circular wait. Breaking any one condition prevents deadlock.

2. **Prevention is better than detection**: Use lock ordering, `std::lock`, or `std::scoped_lock` to prevent deadlocks proactively.

3. **Lock ordering** is the most fundamental technique: always acquire locks in a consistent global order across all threads.

4. **Use C++ standard tools**: `std::lock` and `std::scoped_lock` (C++17) provide safe ways to acquire multiple locks simultaneously.

5. **Try-lock mechanisms** offer a way to avoid indefinite blocking by attempting to acquire locks with timeouts and retrying on failure.

6. **Hierarchical locking** enforces a compile-time or runtime structure that makes certain deadlock patterns impossible.

7. **Timeout-based detection** can identify potential deadlocks by measuring how long threads wait for locks, though this doesn't prevent all cases.

8. **Design principle**: Minimize the scope and duration of locks, avoid nested locking when possible, and never call user-provided code while holding a lock.

Deadlock prevention requires careful design and disciplined coding practices, but with proper techniques, you can build robust concurrent systems that avoid this critical failure mode.