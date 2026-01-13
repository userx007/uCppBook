# Double-Checked Locking in C++

## Overview

Double-Checked Locking (DCL) is a concurrency design pattern used to reduce the overhead of acquiring a lock by first testing the locking criterion without actually acquiring the lock. Only if the check indicates that locking is required does the actual locking logic proceed. This pattern is commonly used for lazy initialization in multithreaded environments, particularly for singleton implementations.

The pattern gets its name because it checks the condition twice: once without locking (for performance) and once with locking (for safety).

## The Problem

In multithreaded programs, lazy initialization of shared resources poses a challenge. A naive implementation might look like this:

```cpp
class Singleton {
private:
    static Singleton* instance;
    Singleton() {}
    
public:
    static Singleton* getInstance() {
        if (instance == nullptr) {  // First check - RACE CONDITION!
            instance = new Singleton();
        }
        return instance;
    }
};
```

This code has a critical race condition: multiple threads could simultaneously see `instance == nullptr` and each create a new instance, violating the singleton pattern and causing memory leaks.

## Naive Lock-Based Solution

A simple fix is to always acquire a lock:

```cpp
#include <mutex>

class Singleton {
private:
    static Singleton* instance;
    static std::mutex mtx;
    Singleton() {}
    
public:
    static Singleton* getInstance() {
        std::lock_guard<std::mutex> lock(mtx);
        if (instance == nullptr) {
            instance = new Singleton();
        }
        return instance;
    }
};
```

While thread-safe, this approach is inefficient because every call to `getInstance()` acquires the lock, even after initialization is complete. This creates unnecessary contention and performance degradation.

## Classic Double-Checked Locking (Broken)

The original DCL pattern attempted to optimize by checking before locking:

```cpp
// WARNING: This code is BROKEN and should NOT be used!
class Singleton {
private:
    static Singleton* instance;
    static std::mutex mtx;
    Singleton() {}
    
public:
    static Singleton* getInstance() {
        if (instance == nullptr) {  // First check (unlocked)
            std::lock_guard<std::mutex> lock(mtx);
            if (instance == nullptr) {  // Second check (locked)
                instance = new Singleton();
            }
        }
        return instance;
    }
};
```

**This code is fundamentally broken** due to memory reordering issues. The compiler or CPU can reorder operations such that `instance` becomes non-null before the `Singleton` object is fully constructed, allowing other threads to access a partially-constructed object.

## Correct Implementation with Atomics

The correct modern implementation uses `std::atomic` with appropriate memory ordering:

```cpp
#include <atomic>
#include <mutex>
#include <memory>

class Singleton {
private:
    static std::atomic<Singleton*> instance;
    static std::mutex mtx;
    Singleton() {}
    
public:
    static Singleton* getInstance() {
        Singleton* tmp = instance.load(std::memory_order_acquire);
        
        if (tmp == nullptr) {  // First check (unlocked)
            std::lock_guard<std::mutex> lock(mtx);
            tmp = instance.load(std::memory_order_relaxed);
            
            if (tmp == nullptr) {  // Second check (locked)
                tmp = new Singleton();
                instance.store(tmp, std::memory_order_release);
            }
        }
        
        return tmp;
    }
};

// Static member initialization
std::atomic<Singleton*> Singleton::instance{nullptr};
std::mutex Singleton::mtx;
```

### Memory Ordering Explanation

- **`memory_order_acquire`** on the first load ensures that if we see a non-null pointer, all writes that happened before the corresponding release are visible
- **`memory_order_relaxed`** inside the lock is safe because the mutex provides the necessary synchronization
- **`memory_order_release`** on the store ensures that the fully constructed object is visible to other threads before they can see the non-null pointer

## Modern C++11+ Solution: `std::call_once`

C++11 introduced `std::call_once`, which provides a cleaner and guaranteed thread-safe way to implement lazy initialization:

```cpp
#include <mutex>

class Singleton {
private:
    static Singleton* instance;
    static std::once_flag initFlag;
    Singleton() {}
    
public:
    static Singleton* getInstance() {
        std::call_once(initFlag, []() {
            instance = new Singleton();
        });
        return instance;
    }
};

Singleton* Singleton::instance = nullptr;
std::once_flag Singleton::initFlag;
```

## Best Practice: Function-Local Static (C++11+)

The simplest and recommended approach in modern C++ is to use a function-local static variable, which is guaranteed to be thread-safe since C++11:

```cpp
class Singleton {
private:
    Singleton() {}
    
public:
    static Singleton& getInstance() {
        static Singleton instance;  // Thread-safe in C++11+
        return instance;
    }
    
    // Delete copy and move operations
    Singleton(const Singleton&) = delete;
    Singleton& operator=(const Singleton&) = delete;
};
```

This is known as the "Meyers Singleton" and is the preferred modern approach because:
- The compiler and runtime handle all thread-safety concerns
- No manual synchronization primitives needed
- Lazy initialization is guaranteed
- Exception-safe
- No memory leaks

## Practical Example: Lazy Resource Initialization

Here's a practical example showing double-checked locking for expensive resource initialization:

```cpp
#include <atomic>
#include <mutex>
#include <memory>
#include <iostream>

class DatabaseConnection {
private:
    static std::atomic<DatabaseConnection*> instance;
    static std::mutex mtx;
    
    std::string connectionString;
    
    DatabaseConnection(const std::string& connStr) 
        : connectionString(connStr) {
        // Simulate expensive initialization
        std::cout << "Establishing database connection...\n";
    }
    
public:
    static DatabaseConnection* getInstance(const std::string& connStr) {
        DatabaseConnection* tmp = instance.load(std::memory_order_acquire);
        
        if (tmp == nullptr) {
            std::lock_guard<std::mutex> lock(mtx);
            tmp = instance.load(std::memory_order_relaxed);
            
            if (tmp == nullptr) {
                tmp = new DatabaseConnection(connStr);
                instance.store(tmp, std::memory_order_release);
            }
        }
        
        return tmp;
    }
    
    void query(const std::string& sql) {
        std::cout << "Executing: " << sql << "\n";
    }
};

std::atomic<DatabaseConnection*> DatabaseConnection::instance{nullptr};
std::mutex DatabaseConnection::mtx;
```

## Summary

**Double-Checked Locking** is a concurrency pattern that optimizes lazy initialization by checking a condition twice: once without locking for performance, and once with locking for safety. The key points are:

1. **The Problem**: Classic DCL without atomics is broken due to memory reordering issues
2. **The Solution**: Use `std::atomic` with acquire-release memory ordering to ensure proper synchronization
3. **Memory Ordering**: Acquire semantics on load and release semantics on store prevent observing partially-constructed objects
4. **Modern Alternatives**: `std::call_once` or function-local static variables (C++11+) are simpler and safer
5. **Best Practice**: Prefer function-local static variables for singleton patterns—they're thread-safe by language guarantee and require no manual synchronization

While understanding double-checked locking is valuable for working with legacy code and understanding low-level concurrency, modern C++ provides better alternatives that should be preferred in new code. The pattern remains relevant when you need explicit control over initialization timing and memory ordering, but for most use cases, let the language handle the complexity.