# Mutexes (Mutual Exclusion Locks)

## C++ Implementation

**Basic Usage**

** **

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;
int shared_counter = 0;

void increment() {
    for (int i = 0; i < 100000; ++i) {
        mtx.lock();           // Acquire the lock
        ++shared_counter;     // Critical section
        mtx.unlock();         // Release the lock
    }
}

int main() {
    std::thread t1(increment);
    std::thread t2(increment);
    
    t1.join();
    t2.join();
    
    std::cout << "Final counter: " << shared_counter << std::endl;
    // Output: Final counter: 200000 (correct result)
    return 0;
}
```

---

**RAII : std::lock_guard<std::mutex> lock(mtx)**

- Exception Safety with lock_guard
- - Exception thrown, but mutex still unlocks automatically!

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

std::mutex mtx;
int shared_counter = 0;  // WARNING: Not encapsulated - can be accessed without locking

void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // RAII-style locking - auto-unlocks when scope ends
        std::lock_guard<std::mutex> lock(mtx);  

        ++shared_counter;
    }  // lock is released here automatically
}

int main() {
    std::vector<std::thread> threads;
    
    // Spawn 5 threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(increment_counter, 1000);
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter: " << shared_counter << std::endl;  // Expected: 5000
    return 0;
}
```
- The mutex and data are separate entities. 
- Nothing prevents accessing `shared_counter` without locking `mtx`.

---

***std::unique_lock<std::mutex> lock(mtx, std::defer_lock)***

- provides more flexibility than std::lock_guard
- deferred locking (create locks without locking immediately)
- timed locking, 
- manual unlocking/relocking, 
- can be moved (but not copied).

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void flexible_locking() {
    // Deferred locking - doesn't lock immediately
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    // Do some work that doesn't need the lock
    std::cout << "Doing work without lock..." << std::endl;
    
    // Now lock when needed
    lock.lock();
    std::cout << "Critical section" << std::endl;
    lock.unlock();
    
    // More work without lock
    std::cout << "More work..." << std::endl;
    
    // Lock again
    lock.lock();
    std::cout << "Another critical section" << std::endl;
    // Automatically unlocked when lock is destroyed
}

void try_lock_example() {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);
    
    if (lock.try_lock()) {
        std::cout << "Lock acquired successfully" << std::endl;
        // Do work
    } else {
        std::cout << "Could not acquire lock" << std::endl;
    }
}

int main() {
    std::thread t1(flexible_locking);
    t1.join();
    
    std::thread t2(try_lock_example);
    t2.join();
    
    return 0;
}
```

***Deferred Locking***

```cpp
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx1, mtx2;

void deferredLocking() {

    // Create locks without locking immediately
    std::unique_lock<std::mutex> lock1(mtx1, std::defer_lock);
    std::unique_lock<std::mutex> lock2(mtx2, std::defer_lock);
    
    // Lock both mutexes simultaneously (deadlock-free)
    std::lock(lock1, lock2);
    
    std::cout << "Both locks acquired safely" << std::endl;
    
    // Both unlock automatically at scope end
}

int main() {
    std::thread t1(deferredLocking);
    std::thread t2(deferredLocking);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

***Try Lock***

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::mutex mtx;

void tryLockExample(int id) {
    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
    
    if (lock.owns_lock()) {
        std::cout << "Thread " << id << " acquired lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else {
        std::cout << "Thread " << id << " failed to acquire lock" << std::endl;
    }
}

int main() {
    std::thread t1(tryLockExample, 1);
    std::thread t2(tryLockExample, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

***Timed Locking***

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::timed_mutex tmtx;

void timedLockExample(int id) {
    std::unique_lock<std::timed_mutex> lock(tmtx, std::defer_lock);
    
    // Try to lock for 50 milliseconds
    // - Waits up to 50 milliseconds for the mutex to become available
    // - Returns true if lock acquired within 50ms
    // - Returns false if timeout expires without getting the lock    
    if (lock.try_lock_for(std::chrono::milliseconds(50))) {
        std::cout << "Thread " << id << " acquired lock" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    } else { // couldn't acquire lock within 50ms
        std::cout << "Thread " << id << " try_lock_for() timeout" << std::endl;
    }
}

int main() {
    std::thread t1(timedLockExample, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::thread t2(timedLockExample, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```


***Transferring Lock Ownership***

```cpp
#include <iostream>
#include <mutex>
#include <thread>

std::mutex mtx;

std::unique_lock<std::mutex> getLock() {
    std::unique_lock<std::mutex> lock(mtx);
    std::cout << "Lock acquired in function" << std::endl;
    return lock; // Ownership transferred
}

void transferOwnership() {
    std::unique_lock<std::mutex> lock = getLock();
    std::cout << "Lock ownership transferred" << std::endl;
    
    // Lock is still held and will be released at scope end
}

int main() {
    std::thread t(transferOwnership);
    t.join();
    
    return 0;
}
```


***Comparison: lock_guard vs unique_lock***

```cpp
#include <iostream>
#include <mutex>
#include <chrono>

std::mutex mtx;

void demonstrateComparison() {
    // lock_guard: Simple, efficient, non-movable
    {
        std::lock_guard<std::mutex> lg(mtx);
        // Cannot unlock manually
        // Cannot be moved
        // Smallest overhead
    }
    
    // unique_lock: Flexible, movable, more features
    {
        std::unique_lock<std::mutex> ul(mtx);
        ul.unlock(); // Can unlock manually
        // Can try_lock, try_lock_for, try_lock_until
        // Can be moved
        ul.lock(); // Can relock
        // Slightly more overhead
    }
}

int main() {
    demonstrateComparison();
    return 0;
}
```


***Advanced: Adopting Pre-Locked Mutexes***

```cpp
#include <iostream>
#include <mutex>

std::mutex mtx;

void adoptLock() {
    mtx.lock(); // Manual lock
    
    // Transfer ownership to lock_guard
    std::lock_guard<std::mutex> lock(mtx, std::adopt_lock);
    
    std::cout << "Lock adopted by lock_guard" << std::endl;
    
    // lock_guard will unlock automatically
}

int main() {
    adoptLock();
    return 0;
}
```

---

***Deadlock Example***

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx1, mtx2;

void thread1_func() {
    std::lock_guard<std::mutex> lock1(mtx1);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock2(mtx2);  // Waits for mtx2
    std::cout << "Thread 1" << std::endl;
}

void thread2_func() {
    std::lock_guard<std::mutex> lock2(mtx2);
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock1(mtx1);  // Waits for mtx1
    std::cout << "Thread 2" << std::endl;
}

// This will deadlock! Don't run this code.
```

---

***Avoiding Deadlock with std::scoped_lock (C++17)***

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx1, mtx2;

void safe_thread1() {
    std::scoped_lock lock(mtx1, mtx2);  // Locks both atomically
    std::cout << "Thread 1 acquired both locks" << std::endl;
}

void safe_thread2() {
    std::scoped_lock lock(mtx2, mtx1);  // Order doesn't matter
    std::cout << "Thread 2 acquired both locks" << std::endl;
}

int main() {
    std::thread t1(safe_thread1);
    std::thread t2(safe_thread2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

---

***std::try_lock - Non-Blocking Attempt***

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

void try_lock_example() {
    if (mtx.try_lock()) {
        std::cout << "Lock acquired by " 
                  << std::this_thread::get_id() << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        mtx.unlock();
    } else {
        std::cout << "Lock not available for " 
                  << std::this_thread::get_id() << std::endl;
    }
}

int main() {
    std::thread t1(try_lock_example);
    std::thread t2(try_lock_example);
    std::thread t3(try_lock_example);
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

---

## Shared Mutex and Reader-Writer Locks in C++

### std::shared_mutex

***Exclusive ownership (for writers)***
Only one thread can hold exclusive ownership at a time

***Shared ownership (for readers)*** 
Multiple threads can hold shared ownership simultaneously, but only when no thread holds exclusive ownership

### Lock Types

**std::unique_lock**
**std::lock_guard** 
Used with `lock()` or `try_lock()` for exclusive (write) access

**std::shared_lock** 
Used with `lock_shared()` or `try_lock_shared()` for shared (read) access


***Basic Usage***

```cpp
/*
 * Reader-Writer Pattern Demo using std::shared_mutex
 * Compile: g++ -pthread --std=c++20 shmtx.cpp -o app
 * 
 * Demonstrates:
 * - Multiple concurrent readers with shared_lock
 * - Exclusive writer access with unique_lock
 * - Thread-safe cache implementation
 */

#include <iostream>
#include <syncstream>
#include <shared_mutex>
#include <thread>
#include <vector>
#include <string>
#include <mutex>
#include <unordered_map>


// sync_print("Producer ", id, " pushed:  ", value);

template<typename... Args>
void sync_print(Args&&... args) {
    std::osyncstream out(std::cout);
    (out << ... << args) << '\n';   // C++17 fold expression
}


class ThreadSafeCache {
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, std::string> cache_;

public:
    std::string read(const std::string& key) const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        auto it = cache_.find(key);
        return (it != cache_.end()) ? it->second : "Not found";
    }

    void write(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(mutex_);
        cache_[key] = value;
    }

    size_t size() const {
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return cache_.size();
    }
};


int main() {
    ThreadSafeCache cache;
    
    std::thread writer([&cache]() {
        for (int i = 0; i < 5; ++i) {
            cache.write("key" + std::to_string(i), "value" + std::to_string(i));
            sync_print("Writer ", i, " : ",  i);
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        }
    });
    
    std::vector<std::thread> readers;
    for (int i = 0; i < 3; ++i) {
        readers.emplace_back([&cache, i]() {
            for (int j = 0; j < 10; ++j) {
                std::string value = cache.read("key" + std::to_string(j % 5));
                sync_print("Reader ", i, " : ",  value);
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
        });
    }
    
    writer.join();
    for (auto& reader : readers) {
        reader.join();
    }
    
    return 0;
}
```

***Upgrade/Downgrade Patterns***

```cpp
#include <shared_mutex>
#include <unordered_map>
#include <string>

class SmartCache {
private:
    mutable std::shared_mutex mutex_;
    std::unordered_map<std::string, int> data_;

public:
    // Check if exists (shared), then insert if missing (exclusive)
    int get_or_create(const std::string& key, int default_value) {
        // First, try with shared lock
        {
            std::shared_lock<std::shared_mutex> read_lock(mutex_);
            auto it = data_.find(key);
            if (it != data_.end()) {
                return it->second;  // Found it, return value
            }
        }  // Release shared lock

        // Not found, acquire exclusive lock to insert
        std::unique_lock<std::shared_mutex> write_lock(mutex_);
        
        // Double-check (another thread might have inserted while we waited)
        auto it = data_.find(key);
        if (it != data_.end()) {
            return it->second;
        }

        // Still not there, insert it
        data_[key] = default_value;
        return default_value;
    }
};
```

***Try-Lock Variants***

```cpp
#include <shared_mutex>
#include <iostream>

class Resource {
    mutable std::shared_mutex mutex_;   // mutable: allows locking in const methods without
                                        // breaking const correctness (locking is an
                                        // implementation detail, not a logical state change)
    std::string data_;

public:
    bool try_read(std::string& out) const {
        if (mutex_.try_lock_shared()) {     // non-blocking attempt to acquire shared lock
                                            // returns false immediately if a writer holds the lock
                                            // multiple readers can hold shared lock simultaneously
            std::shared_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
                                            // adopt_lock: mutex already locked above, don't lock again
                                            // RAII: lock destructor will release the shared lock
                                            //       automatically when scope exits
            out = data_;                    // safe to read: shared lock allows concurrent readers
            return true;                    // caller knows read succeeded
        }
        return false;                       // lock was busy (writer active), caller decides what to do
    }

    bool try_write(const std::string& value) {
        if (mutex_.try_lock()) {            // non-blocking attempt to acquire exclusive lock
                                            // returns false immediately if any reader or writer holds the lock
                                            // only one writer can hold exclusive lock at a time
            std::unique_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
                                            // adopt_lock: mutex already locked above, don't lock again
                                            // RAII: lock destructor will release the exclusive lock
                                            //       automatically when scope exits
            data_ = value;                  // safe to write: exclusive lock, no other readers or writers
            return true;                    // caller knows write succeeded
        }
        return false;                       // lock was busy (readers or writer active), caller decides what to do
    }
};

int main() {
    Resource res;
    std::string out;

    if (!res.try_write("hello")) {          // attempt write, don't block if busy
        std::cout << "Write failed, resource busy\n";
    }

    if (!res.try_read(out)) {              // attempt read, don't block if busy
        std::cout << "Read failed, resource busy\n";
    } else {
        std::cout << "Read: " << out << '\n';
    }
}
```


***Timed Locks***

```cpp
#include <shared_mutex>
#include <chrono>
#include <string>

class TimedResource {
    mutable std::shared_mutex mutex_;
    std::string data_;

public:
    bool timed_read(std::string& out, std::chrono::milliseconds timeout) const {
        if (mutex_.try_lock_shared_for(timeout)) {
            std::shared_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
            out = data_;
            return true;
        }
        return false;
    }

    bool timed_write(const std::string& value, std::chrono::milliseconds timeout) {
        if (mutex_.try_lock_for(timeout)) {
            std::unique_lock<std::shared_mutex> lock(mutex_, std::adopt_lock);
            data_ = value;
            return true;
        }
        return false;
    }
};
```

---

### Common Pitfalls

***Writer Starvation***

```cpp
void potential_starvation() {
    std::shared_mutex mutex;
    
    // Many readers continuously acquiring shared locks
    // might prevent a writer from ever acquiring exclusive lock
}
```

***Deadlock with Multiple Shared Mutexes***


```cpp
class Account {
    mutable std::shared_mutex mutex_;
    double balance_;
public:
    // Always lock in order of memory address to prevent deadlock
    static void transfer(Account& from, Account& to, double amount) {
        std::shared_mutex& first = (&from < &to) ? from.mutex_ : to.mutex_;
        std::shared_mutex& second = (&from < &to) ? to.mutex_ : from.mutex_;
        
        std::unique_lock<std::shared_mutex> lock1(first);
        std::unique_lock<std::shared_mutex> lock2(second);
        
        from.balance_ -= amount;
        to.balance_ += amount;
    }
};
```

***Forgetting const for Read Operations***

```cpp
class Data {
    mutable std::shared_mutex mutex_;  // Note: mutable
    int value_;
public:
    int read() const {  // const allows this to work on const Data&
        std::shared_lock<std::shared_mutex> lock(mutex_);
        return value_;
    }
};
```

---

## Recursive Mutex

1. **Recursive Functions**: When a function calls itself and both the outer and inner calls need to lock the same resource
2. **Method Chaining**: When synchronized methods call other synchronized methods within the same class
3. **Complex Class Hierarchies**: When derived class methods call base class methods that both require locking
4. **Legacy Code Refactoring**: When converting non-thread-safe code to thread-safe without major restructuring

**Recursive mutexes** have ***slightly more overhead*** than regular mutexes because they must track ownership and lock count:


***Basic Usage***

```cpp
#include <iostream>
#include <mutex>
#include <thread>

class Counter {
private:
    std::recursive_mutex mtx;
    int value = 0;

public:
    // Public method that locks the mutex
    void increment() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        value++;
        std::cout << "Value incremented to: " << value << std::endl;
    }
    
    // Another public method that locks the mutex AND calls increment
    void incrementBy(int n) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        std::cout << "Incrementing by " << n << std::endl;
        for (int i = 0; i < n; i++) {
            increment(); // This would deadlock with std::mutex!
        }
    }
    
    int getValue() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return value;
    }
};

int main() {
    Counter counter;
    
    // This works fine because recursive_mutex allows re-locking
    counter.incrementBy(3);
    
    std::cout << "Final value: " << counter.getValue() << std::endl;
    
    return 0;
}
```

***Recursive Function Example***

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class TreeNode {
public:
    int data;
    std::vector<TreeNode*> children;
    mutable std::recursive_mutex mtx;
    
    TreeNode(int val) : data(val) {}
    
    void addChild(TreeNode* child) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        children.push_back(child);
    }
    
    // Recursive function that needs to lock at each level
    int sumTree() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        int sum = data;
        
        for (const auto* child : children) {
            sum += child->sumTree(); // Recursive call, locks again
        }
        
        return sum;
    }
    
    // Count total nodes in tree
    int countNodes() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        int count = 1; // Count this node
        
        for (const auto* child : children) {
            count += child->countNodes(); // Recursive locking
        }
        
        return count;
    }
};

int main() {
    TreeNode root(10);
    TreeNode* child1 = new TreeNode(5);
    TreeNode* child2 = new TreeNode(15);
    TreeNode* grandchild = new TreeNode(3);
    
    child1->addChild(grandchild);
    root.addChild(child1);
    root.addChild(child2);
    
    std::cout << "Sum of tree: " << root.sumTree() << std::endl;
    std::cout << "Node count: " << root.countNodes() << std::endl;
    
    // Cleanup
    delete grandchild;
    delete child1;
    delete child2;
    
    return 0;
}
```

***Lock Counting Mechanism***

```cpp
#include <iostream>
#include <mutex>
#include <thread>

class LockCounter {
private:
    std::recursive_mutex mtx;
    int lockDepth = 0;

public:
    void nestedLocking(int depth) {
        if (depth <= 0) return;
        
        mtx.lock();
        lockDepth++;
        std::cout << "Thread " << std::this_thread::get_id() 
                  << " locked at depth: " << lockDepth << std::endl;
        
        // Recursive call with the mutex already locked
        nestedLocking(depth - 1);
        
        lockDepth--;
        std::cout << "Thread " << std::this_thread::get_id() 
                  << " unlocking at depth: " << lockDepth + 1 << std::endl;
        mtx.unlock();
    }
    
    void demonstrateNesting() {
        nestedLocking(3);
    }
};

int main() {
    LockCounter counter;
    counter.demonstrateNesting();
    
    return 0;
}
```

---

## Timed Mutex Operations in C++

***`std::timed_mutex`***
***`std::recursive_timed_mutex`***


- **`try_lock_for(duration)`**: Attempts to lock the mutex for a specified duration
- **`try_lock_until(time_point)`**: Attempts to lock the mutex until a specific point in time

Both methods return `true` if the lock was acquired successfully, and `false` if the timeout expired before acquisition.



***Usage `try_lock_for`***

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::timed_mutex tmutex;
int shared_resource = 0;

void attempt_access(int thread_id, std::chrono::milliseconds timeout) {
    std::cout << "Thread " << thread_id << " attempting to acquire lock...\n";
    
    // Attempts to lock the mutex for a specified duration
    if (tmutex.try_lock_for(timeout)) {
        std::cout << "Thread " << thread_id << " acquired lock\n";
        
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        shared_resource++;
        
        std::cout << "Thread " << thread_id << " releasing lock\n";
        tmutex.unlock();
    } else {
        std::cout << "Thread " << thread_id << " timeout - couldn't acquire lock\n";
    }
}

int main() {
    std::thread t1(attempt_access, 1, std::chrono::milliseconds(50));
    std::thread t2(attempt_access, 2, std::chrono::milliseconds(200));
    
    t1.join();
    t2.join();
    
    std::cout << "Final resource value: " << shared_resource << "\n";
    return 0;
}
```

***Usage `try_lock_until`***

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::timed_mutex tmutex;

void scheduled_task(int task_id) {
    using namespace std::chrono;
    
    // Set a deadline 2 seconds from now
    auto deadline = system_clock::now() + seconds(2);
    
    std::cout << "Task " << task_id << " waiting until deadline...\n";
    
    // Attempts to lock the mutex until a specific point in time
    if (tmutex.try_lock_until(deadline)) {
        std::cout << "Task " << task_id << " acquired lock before deadline\n";
        
        // Do work
        std::this_thread::sleep_for(milliseconds(500));
        
        tmutex.unlock();
    } else {
        std::cout << "Task " << task_id << " failed - deadline expired\n";
    }
}

int main() {
    std::thread t1(scheduled_task, 1);
    std::thread t2(scheduled_task, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

***Usage `std::recursive_timed_mutex`***

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

std::recursive_timed_mutex rtmutex;

void recursive_function(int depth) {
    using namespace std::chrono_literals;
    
    if (rtmutex.try_lock_for(100ms)) {
        std::cout << "Locked at depth " << depth << "\n";
        
        if (depth > 0) {
            recursive_function(depth - 1);
        }
        
        rtmutex.unlock();
    } else {
        std::cout << "Timeout at depth " << depth << "\n";
    }
}

int main() {
    recursive_function(3);
    return 0;
}
```

| Lock Type          | `std::timed_mutex` | `std::recursive_timed_mutex` | Supports Timed Locking |
| ------------------ | ------------------ | ---------------------------- | ---------------------- |
| `std::lock_guard`  | Yes                | Yes                          | No                     |
| `std::unique_lock` | Yes                | Yes                          | Yes                    |
| `std::scoped_lock` | Yes                | Yes                          | No                     |
| `std::shared_lock` | No                 | No                           | No                     |

---

## Scoped Lock for Multiple Mutexes

- When multiple mutexes must be locked together, there's a significant risk of deadlock if threads acquire locks in different orders. 
- `std::scoped_lock` (introduced in C++17) provides a deadlock-free solution for locking multiple mutexes simultaneously.


***std::scoped_lock***

```cpp
#include <mutex>

std::mutex m1, m2, m3;

void safe_function() {
    std::scoped_lock lock(m1, m2, m3);
    // All mutexes are now locked
    // Protected code here
    
    // All mutexes automatically unlock when lock goes out of scope
}
```

***Comparison with `std::lock` before C++17***

```cpp
#include <mutex>

std::mutex m1, m2;

// Pre-C++17 approach
void old_way() {
    std::lock(m1, m2);  // Lock both atomically
    std::lock_guard<std::mutex> lock1(m1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(m2, std::adopt_lock);
    
    // Protected code
}

// C++17 approach - simpler and cleaner
void new_way() {
    std::scoped_lock lock(m1, m2);
    
    // Protected code
}
```

---

## Summary

***Key Concepts***

- **std::mutex** is the fundamental mutual exclusion primitive for protecting shared data
- **Race conditions** occur when multiple threads access shared data without synchronization
- **Critical sections** are code regions that access shared resources and must be protected

***Locking Mechanisms***

- **Manual locking**: `lock()` and `unlock()` (error-prone, avoid when possible)
- **std::lock_guard**: RAII-based automatic locking/unlocking (recommended for simple cases)
- **std::unique_lock**: Flexible locking with deferred and manual control
- **std::scoped_lock** (C++17): Deadlock-free locking of multiple mutexes

***Best Practices***

1. Always use RAII wrappers (`lock_guard`, `unique_lock`, `scoped_lock`) instead of manual locking
2. Keep critical sections as small as possible to minimize contention
3. Never lock the same mutex twice in the same thread (unless using `std::recursive_mutex`)
4. Use `std::scoped_lock` when locking multiple mutexes to avoid deadlock
5. Ensure all access to shared data is protected by the same mutex
6. Consider the granularity of locking—too coarse reduces parallelism, too fine increases overhead

***Common Pitfalls***

- Forgetting to unlock (solved by RAII)
- Deadlocks from circular lock dependencies
- Holding locks during expensive operations
- Unprotected access to shared data
- Accessing destroyed mutexes

---

***`std::shared_mutex` and `std::shared_lock`***

Provide an elegant solution for the reader-writer problem in C++. By allowing multiple concurrent readers while ensuring exclusive access for writers, they significantly improve performance in read-heavy scenarios.

**Key points:**
- Use `std::shared_lock` for read operations (allows multiple simultaneous readers)
- Use `std::unique_lock` or `std::lock_guard` for write operations (exclusive access)
- Mark read methods as `const` and the mutex as `mutable`
- Be aware of potential writer starvation in read-heavy workloads
- Consider the overhead: shared mutexes have slightly more overhead than regular mutexes, so they're only beneficial when reads significantly outnumber writes
- `std::shared_mutex` was introduced in C++17; use `std::shared_timed_mutex` for C++14

The reader-writer pattern with shared mutexes is a powerful tool for optimizing multithreaded applications where data is frequently read but infrequently modified, such as caches, configuration managers, and lookup tables.

***When to Use Shared Mutexes***

**Use shared_mutex when:**
- Reads greatly outnumber writes (typically 10:1 ratio or higher)
- Read operations are relatively quick
- You need maximum concurrency for read-heavy workloads

**Use regular mutex when:**
- Writes are frequent
- The protected section is very short
- Simplicity is more important than maximum performance
- You're working with C++14 or earlier (use `std::shared_timed_mutex` in C++14)

---

***Recursive Mutex***

**Recursive Mutex** (`std::recursive_mutex`) is a synchronization primitive that allows the same thread to lock a mutex multiple times without deadlocking. Key points include:

- **Use Cases**: Recursive functions, method chaining in classes, complex call hierarchies where methods call other synchronized methods
- **Mechanism**: Maintains a lock count and thread ownership, requiring an equal number of unlocks to fully release the mutex
- **Advantages**: Simplifies code in scenarios with reentrant locking needs, prevents deadlocks when the same thread needs multiple locks
- **Disadvantages**: Slightly higher overhead than regular mutexes, can mask poor design decisions, may indicate code that needs refactoring
- **Best Practice**: Use sparingly and consider whether code restructuring with internal unsafe methods would be cleaner
- **Thread Safety**: Only the owning thread can lock and unlock the recursive mutex; it provides no cross-thread reentrancy

While recursive mutexes solve specific problems elegantly, they should be a deliberate choice rather than a default. Well-structured code with clear separation between locked public interfaces and unlocked internal helpers often eliminates the need for recursive locking altogether.

---

***`std::timed_mutex`***

Timed mutex operations in C++ provide a flexible approach to thread synchronization with timeout capabilities. The key points to remember are:

**Core Features**: `std::timed_mutex` extends `std::mutex` with `try_lock_for()` (duration-based) and `try_lock_until()` (time-point-based) methods that attempt lock acquisition with timeouts.

**Use Cases**: Timed mutexes are ideal for scenarios requiring deadlock prevention, implementing fallback behavior when locks can't be acquired, maintaining application responsiveness, and handling resource pools with timeout policies.

**Variants**: The standard library provides both `std::timed_mutex` and `std::recursive_timed_mutex` for different locking patterns.

**Best Practices**: Always check the return value of timed lock attempts, ensure proper unlock in all code paths (especially exception scenarios), use RAII wrappers like `std::unique_lock` when possible, and be mindful of the performance overhead compared to regular mutexes.

**Common Pitfalls**: Avoid forgetting to unlock after successful acquisition, setting unrealistic timeout values, and using timed mutexes when regular mutexes suffice, as they have slightly higher overhead.

Timed mutex operations strike a balance between the blocking behavior of regular mutexes and the non-blocking `try_lock()`, making them an essential tool for building robust, responsive concurrent applications.

---

***`std::scoped_lock`***

`std::scoped_lock` is the modern C++17 solution for safely locking multiple mutexes simultaneously without deadlock risk. It combines the RAII benefits of `std::lock_guard` with the deadlock avoidance of `std::lock`, providing a clean, safe, and efficient interface. When you need to protect multiple resources accessed by concurrent threads, `std::scoped_lock` should be your default choice. It eliminates entire classes of threading bugs while keeping code readable and maintainable. For single mutex scenarios, `std::lock_guard` or `std::unique_lock` remain appropriate, but when two or more mutexes are involved, `std::scoped_lock` is the superior option.








---

## Rust Implementation

```rust
use std::sync::{Arc, Mutex};  // Arc = Atomic Reference Counting for shared ownership
use std::thread;

fn main() {
    // Mutex<T> wraps the data - compile-time guarantee of safe access
    let counter = Arc::new(Mutex::new(0));
    let mut handles = vec![];
    
    // Spawn 5 threads
    for _ in 0..5 {
        
        // Clone the Arc (reference count++), not the data
        let counter_clone = Arc::clone(&counter);  // / new Arc clone each iteration
        //  ^^^^^^^^^^^^^ Lives in this scope

        // counter_clone is MOVED here, now owned by this thread
        // Safe because Arc allows shared ownership across threads            
        let handle = thread::spawn(move || {
        //                         ^^^^ Transfers ownership into this closure
            for _ in 0..1000 {
                //         Lock acquired here ↓ uses the moved value
                let mut num = counter_clone.lock().unwrap();
                //    ^^^                          ^^^^^^^^
                //    |                            Extract MutexGuard from Result
                //    MutexGuard<i32> smart pointer
                
                *num += 1;  // Dereference to access the i32 inside
                //  ^
                //  Access the actual data through the guard

            }  // IMPORTANT: MutexGuard automatically unlocks when dropped

        }); // Thread owns counter_clone until it finishes

        // IMPORTANT: counter_clone is no longer accessible here - it's been moved

        // state handles for being joined later on
        handles.push(handle);
    }
    
    // Wait for all threads to complete
    for handle in handles {
        handle.join().unwrap();
    }
    
    println!("Final counter: {}", *counter.lock().unwrap());  // Expected: 5000
}
```
#### Why Arc Makes This Work:

- `Arc::clone(&counter)` creates a new reference-counted pointer
- Each thread gets its **own Arc** (not the data itself)
- All Arcs point to the **same underlying data**
- When moved into the thread, the Arc's reference count keeps the data alive
- When the thread finishes and drops its Arc, the reference count decreases

#### Summary:

- `move` transfers ownership of `counter_clone` into the thread
- This is safe because `Arc` allows **shared ownership across threads**
- Each thread owns its own `Arc` instance
- All `Arc` instances share access to the same `Mutex<i32>`


`Mutex<T>` wraps the data itself. 
The only way to access the data is through the lock, enforced at compile time.


