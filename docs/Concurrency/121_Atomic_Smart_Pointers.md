# Atomic Smart Pointers

## Overview

Atomic smart pointers provide thread-safe operations on `std::shared_ptr` and `std::weak_ptr` without requiring external synchronization mechanisms like mutexes. Prior to C++20, working with shared pointers in concurrent environments required careful manual synchronization. C++20 introduced `std::atomic<std::shared_ptr<T>>` and `std::atomic<std::weak_ptr<T>>`, making it possible to perform atomic operations on smart pointers directly.

## The Problem with Non-Atomic Shared Pointers

When multiple threads access a `std::shared_ptr` concurrently, race conditions can occur. The shared pointer itself contains two pointers: one to the managed object and one to the control block (which tracks reference counts). Operations like assignment, copy construction, or reset involve multiple steps that aren't inherently atomic:

```cpp
// Non-thread-safe example - DO NOT USE in concurrent code
std::shared_ptr<int> global_ptr = std::make_shared<int>(42);

void thread_function() {
    // Race condition: reading and copying involves multiple operations
    auto local_copy = global_ptr;
    
    // Race condition: assignment involves updating two pointers
    global_ptr = std::make_shared<int>(100);
}
```

## Pre-C++20 Solutions

Before C++20, the standard library provided free functions for atomic operations on shared pointers:

```cpp
#include <memory>
#include <atomic>

std::shared_ptr<int> global_ptr = std::make_shared<int>(42);

void legacy_thread_safe_access() {
    // Load atomically
    auto local_copy = std::atomic_load(&global_ptr);
    
    // Store atomically
    auto new_ptr = std::make_shared<int>(100);
    std::atomic_store(&global_ptr, new_ptr);
    
    // Compare and exchange
    auto expected = local_copy;
    auto desired = std::make_shared<int>(200);
    std::atomic_compare_exchange_strong(&global_ptr, &expected, desired);
}
```

While functional, this approach has drawbacks: the syntax is verbose, it's easy to forget to use the atomic functions, and the implementation may use locks internally.

## C++20 Atomic Smart Pointers

C++20 introduces explicit specializations of `std::atomic` for smart pointers:

```cpp
#include <atomic>
#include <memory>
#include <iostream>
#include <thread>
#include <vector>

// Thread-safe shared pointer
std::atomic<std::shared_ptr<int>> atomic_ptr;

void writer_thread(int value) {
    auto new_ptr = std::make_shared<int>(value);
    atomic_ptr.store(new_ptr, std::memory_order_release);
    std::cout << "Stored value: " << value << std::endl;
}

void reader_thread() {
    auto local_ptr = atomic_ptr.load(std::memory_order_acquire);
    if (local_ptr) {
        std::cout << "Read value: " << *local_ptr << std::endl;
    }
}

int main() {
    atomic_ptr.store(std::make_shared<int>(0));
    
    std::vector<std::thread> threads;
    
    // Launch writer threads
    for (int i = 1; i <= 5; ++i) {
        threads.emplace_back(writer_thread, i * 10);
    }
    
    // Launch reader threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(reader_thread);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## Key Operations

### Load and Store

```cpp
#include <atomic>
#include <memory>

std::atomic<std::shared_ptr<std::string>> atomic_str;

void demonstrate_load_store() {
    // Store a new shared pointer
    auto new_value = std::make_shared<std::string>("Hello");
    atomic_str.store(new_value, std::memory_order_release);
    
    // Load the current value
    auto current = atomic_str.load(std::memory_order_acquire);
    if (current) {
        std::cout << *current << std::endl;
    }
    
    // Exchange (store new value and return old)
    auto old_value = atomic_str.exchange(
        std::make_shared<std::string>("World"),
        std::memory_order_acq_rel
    );
}
```

### Compare and Exchange

Compare-and-exchange (CAS) is crucial for implementing lock-free algorithms:

```cpp
#include <atomic>
#include <memory>
#include <thread>
#include <vector>

struct Node {
    int value;
    std::shared_ptr<Node> next;
    
    Node(int v) : value(v), next(nullptr) {}
};

class LockFreeStack {
private:
    std::atomic<std::shared_ptr<Node>> head;
    
public:
    LockFreeStack() : head(nullptr) {}
    
    void push(int value) {
        auto new_node = std::make_shared<Node>(value);
        auto current_head = head.load(std::memory_order_relaxed);
        
        do {
            new_node->next = current_head;
        } while (!head.compare_exchange_weak(
            current_head,
            new_node,
            std::memory_order_release,
            std::memory_order_relaxed
        ));
    }
    
    bool pop(int& result) {
        auto current_head = head.load(std::memory_order_acquire);
        
        while (current_head) {
            auto next = current_head->next;
            
            if (head.compare_exchange_weak(
                current_head,
                next,
                std::memory_order_release,
                std::memory_order_acquire
            )) {
                result = current_head->value;
                return true;
            }
        }
        
        return false;
    }
};

void test_lock_free_stack() {
    LockFreeStack stack;
    std::vector<std::thread> threads;
    
    // Push values from multiple threads
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&stack, i]() {
            stack.push(i);
        });
    }
    
    // Pop values from multiple threads
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&stack]() {
            int value;
            if (stack.pop(value)) {
                std::cout << "Popped: " << value << std::endl;
            }
        });
    }
    
    for (auto& t : threads) {
        t.join();
    }
}
```

## Atomic Weak Pointers

`std::atomic<std::weak_ptr<T>>` is also supported:

```cpp
#include <atomic>
#include <memory>
#include <thread>

std::atomic<std::weak_ptr<int>> atomic_weak;

void demonstrate_weak_ptr() {
    auto shared = std::make_shared<int>(42);
    
    // Store weak pointer atomically
    atomic_weak.store(shared, std::memory_order_release);
    
    // Load and try to lock
    auto weak = atomic_weak.load(std::memory_order_acquire);
    if (auto locked = weak.lock()) {
        std::cout << "Value still alive: " << *locked << std::endl;
    } else {
        std::cout << "Object has been destroyed" << std::endl;
    }
}
```

## Real-World Example: Shared Configuration

```cpp
#include <atomic>
#include <memory>
#include <string>
#include <thread>
#include <chrono>
#include <iostream>

struct Configuration {
    std::string database_url;
    int max_connections;
    bool debug_mode;
    
    Configuration(const std::string& url, int max_conn, bool debug)
        : database_url(url), max_connections(max_conn), debug_mode(debug) {}
};

class ConfigManager {
private:
    std::atomic<std::shared_ptr<Configuration>> current_config;
    
public:
    ConfigManager() {
        current_config.store(
            std::make_shared<Configuration>("localhost:5432", 10, false)
        );
    }
    
    void update_config(std::shared_ptr<Configuration> new_config) {
        current_config.store(new_config, std::memory_order_release);
        std::cout << "Configuration updated" << std::endl;
    }
    
    std::shared_ptr<Configuration> get_config() const {
        return current_config.load(std::memory_order_acquire);
    }
    
    bool try_update_if_matches(
        std::shared_ptr<Configuration> expected,
        std::shared_ptr<Configuration> desired
    ) {
        return current_config.compare_exchange_strong(
            expected,
            desired,
            std::memory_order_acq_rel
        );
    }
};

void worker_thread(ConfigManager& manager, int id) {
    for (int i = 0; i < 3; ++i) {
        auto config = manager.get_config();
        std::cout << "Worker " << id << " using config: "
                  << config->database_url << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

void config_updater(ConfigManager& manager) {
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    auto new_config = std::make_shared<Configuration>(
        "production-db:5432", 50, true
    );
    manager.update_config(new_config);
}

int main() {
    ConfigManager manager;
    
    std::vector<std::thread> threads;
    threads.emplace_back(config_updater, std::ref(manager));
    
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back(worker_thread, std::ref(manager), i);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

## Memory Ordering Considerations

Atomic smart pointer operations support all standard memory orderings:

```cpp
// Relaxed ordering - no synchronization guarantees
auto ptr1 = atomic_ptr.load(std::memory_order_relaxed);

// Acquire-Release ordering - synchronizes with stores
auto ptr2 = atomic_ptr.load(std::memory_order_acquire);
atomic_ptr.store(new_ptr, std::memory_order_release);

// Sequential consistency - strongest guarantee (default)
auto ptr3 = atomic_ptr.load(std::memory_order_seq_cst);
atomic_ptr.store(new_ptr, std::memory_order_seq_cst);
```

## Performance Characteristics

Atomic smart pointers provide thread safety but come with performance costs. Depending on the platform and implementation, they may use lock-free atomic operations on the control block pointer, or they may use internal locking mechanisms. The performance is generally better than using a mutex to protect a regular shared pointer, but not as fast as using atomic operations on primitive types.

## Common Pitfalls

### Dereferencing Without Local Copy

```cpp
// WRONG - not thread-safe
if (atomic_ptr.load()) {
    // Another thread could reset atomic_ptr here
    int value = *atomic_ptr.load(); // Potential null dereference
}

// CORRECT - keep a local copy
auto local_ptr = atomic_ptr.load();
if (local_ptr) {
    int value = *local_ptr; // Safe
}
```

### Assuming Reference Count Atomicity

```cpp
// The individual loads are atomic, but the comparison is not
// Another thread could modify atomic_ptr between the two loads
auto ptr1 = atomic_ptr.load();
auto ptr2 = atomic_ptr.load();
// ptr1 and ptr2 might be different even if executed "immediately" after each other
```

## Summary

Atomic smart pointers introduced in C++20 provide a robust, standardized way to share ownership of objects across threads safely. Key points include:

- **Thread Safety**: `std::atomic<std::shared_ptr<T>>` and `std::atomic<std::weak_ptr<T>>` provide lock-free or internally synchronized operations
- **Standard Interface**: Consistent with other atomic types, supporting load, store, exchange, and compare-exchange operations
- **Memory Ordering**: Full support for memory ordering semantics enables fine-grained control over synchronization
- **Practical Applications**: Ideal for shared configuration objects, lock-free data structures, and concurrent resource management
- **Performance Trade-offs**: More efficient than mutex-protected shared pointers but slower than atomic operations on primitive types

Atomic smart pointers eliminate entire categories of concurrency bugs related to shared pointer manipulation, making concurrent code safer and more maintainable while preserving the benefits of automatic memory management.