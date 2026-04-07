# std::mutex: lock() vs try_lock()

## Key Differences

**`lock()`**
- **Blocks** until the mutex is acquired
- Guarantees eventual lock acquisition
- Thread waits indefinitely if mutex is held by another thread
- No return value (void)

**`try_lock()`**
- **Non-blocking** - returns immediately
- Returns `bool`: `true` if lock acquired, `false` if mutex already locked
- Allows thread to do other work if lock unavailable
- Useful for avoiding deadlocks

## When to Use What

### Use `lock()` when:
- You **must** acquire the lock to proceed safely
- The critical section is short and contention is low
- Waiting is acceptable for your use case
- You want simpler, cleaner code

### Use `try_lock()` when:
- You have **alternative work** to do if lock unavailable
- You want to **avoid blocking** indefinitely
- Implementing **deadlock avoidance** strategies
- You need **timeout behavior** (combine with loops/delays)
- Building **responsive systems** that can't afford to wait

## Practical Examples

### Example 1: Basic lock() usage

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

std::mutex mtx;
int shared_counter = 0;

void increment_with_lock(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        mtx.lock();
        ++shared_counter;  // Critical section
        mtx.unlock();
    }
}

int main() {
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(increment_with_lock, 1000);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter: " << shared_counter << std::endl;
    // Output: Final counter: 5000 (guaranteed)
    return 0;
}
```

### Example 2: try_lock() with alternative work

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex data_mtx;
int shared_data = 0;
int skipped_operations = 0;

void process_with_trylock(int id) {
    for (int i = 0; i < 10; ++i) {
        if (data_mtx.try_lock()) {
            // Got the lock - do critical work
            shared_data += id;
            std::cout << "Thread " << id << " acquired lock, data = " 
                      << shared_data << std::endl;
            
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            data_mtx.unlock();
        } else {
            // Couldn't get lock - do alternative work
            std::cout << "Thread " << id << " couldn't acquire lock, "
                      << "doing other work..." << std::endl;
            ++skipped_operations;
            
            // Do non-critical work here
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    }
}

int main() {
    std::thread t1(process_with_trylock, 1);
    std::thread t2(process_with_trylock, 2);
    
    t1.join();
    t2.join();
    
    std::cout << "Final data: " << shared_data << std::endl;
    std::cout << "Skipped operations: " << skipped_operations << std::endl;
    return 0;
}
```

### Example 3: Deadlock avoidance with try_lock()

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex mtx1, mtx2;

// BAD: Potential deadlock with lock()
void deadlock_prone(int id) {
    if (id == 1) {
        mtx1.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        mtx2.lock();  // May deadlock here
        mtx2.unlock();
        mtx1.unlock();
    } else {
        mtx2.lock();
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
        mtx1.lock();  // May deadlock here
        mtx1.unlock();
        mtx2.unlock();
    }
}

// GOOD: Deadlock avoidance with try_lock()
void deadlock_free(int id) {
    std::mutex& first = (id == 1) ? mtx1 : mtx2;
    std::mutex& second = (id == 1) ? mtx2 : mtx1;
    
    while (true) {
        first.lock();
        
        if (second.try_lock()) {
            // Got both locks!
            std::cout << "Thread " << id << " acquired both locks" << std::endl;
            
            // Do work with both resources
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            second.unlock();
            first.unlock();
            break;
        } else {
            // Couldn't get second lock - release first and retry
            first.unlock();
            std::this_thread::sleep_for(std::chrono::milliseconds(1));
        }
    }
}

int main() {
    std::thread t1(deadlock_free, 1);
    std::thread t2(deadlock_free, 2);
    
    t1.join();
    t2.join();
    
    std::cout << "Completed without deadlock!" << std::endl;
    return 0;
}
```

### Example 4: try_lock() with timeout pattern

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <chrono>

std::mutex resource_mtx;

bool try_lock_with_timeout(std::mutex& mtx, std::chrono::milliseconds timeout) {
    auto start = std::chrono::steady_clock::now();
    
    while (std::chrono::steady_clock::now() - start < timeout) {
        if (mtx.try_lock()) {
            return true;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
    }
    
    return false;  // Timeout
}

void process_with_timeout(int id) {
    std::cout << "Thread " << id << " attempting to acquire lock..." << std::endl;
    
    if (try_lock_with_timeout(resource_mtx, std::chrono::milliseconds(500))) {
        std::cout << "Thread " << id << " got the lock!" << std::endl;
        
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(300));
        
        resource_mtx.unlock();
    } else {
        std::cout << "Thread " << id << " timed out!" << std::endl;
    }
}

int main() {
    std::thread t1(process_with_timeout, 1);
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    std::thread t2(process_with_timeout, 2);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

### Example 5: Best Practice with lock_guard and unique_lock

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mtx;

// BETTER: Using lock_guard (RAII - automatic unlock)
void safe_increment_with_guard(int& counter) {
    std::lock_guard<std::mutex> lock(mtx);  // Automatic lock/unlock
    ++counter;
    // Automatically unlocks when lock goes out of scope
}

// BEST for try_lock: Using unique_lock
void safe_trylock_with_unique(int& counter) {
    std::unique_lock<std::mutex> lock(mtx, std::try_to_lock);
    
    if (lock.owns_lock()) {
        ++counter;
        std::cout << "Successfully incremented" << std::endl;
    } else {
        std::cout << "Couldn't acquire lock" << std::endl;
    }
    // Automatically unlocks if locked
}

int main() {
    int counter = 0;
    
    std::thread t1(safe_increment_with_guard, std::ref(counter));
    std::thread t2(safe_trylock_with_unique, std::ref(counter));
    
    t1.join();
    t2.join();
    
    std::cout << "Counter: " << counter << std::endl;
    return 0;
}
```

## Summary Table

| Feature | lock() | try_lock() |
|---------|--------|------------|
| Blocking | Yes | No |
| Return type | void | bool |
| Guaranteed acquisition | Yes (eventually) | No |
| Use case | Must-have critical sections | Optional/alternative paths |
| Deadlock risk | Higher | Lower (with proper patterns) |
| Performance | Simpler | More flexible |

## Pro Tips

1. **Prefer RAII wrappers** (`std::lock_guard`, `std::unique_lock`) over manual lock/unlock
2. **Use `std::unique_lock` with `std::try_to_lock`** for exception-safe try_lock operations
3. **For multiple mutexes**, use `std::lock()` or `std::scoped_lock` (C++17) to avoid deadlocks
4. **Keep critical sections short** regardless of which method you use
5. **Consider `std::timed_mutex`** if you need built-in timeout functionality