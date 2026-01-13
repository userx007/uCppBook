# Read-Copy-Update (RCU) Pattern

## Overview

Read-Copy-Update (RCU) is a synchronization mechanism optimized for read-heavy workloads where reads vastly outnumber writes. Originally developed for the Linux kernel, RCU allows multiple readers to access data structures concurrently without locks, while writers create modified copies of data rather than updating in place.

The core principle: readers access data without synchronization overhead, while writers update data by creating new versions and deferring the reclamation of old versions until all readers have finished with them.

## Key Concepts

**Read-Side Critical Sections**: Readers access shared data without acquiring locks. They simply mark the beginning and end of their access period.

**Copy-on-Write Updates**: Writers never modify data in place. Instead, they create a modified copy and atomically switch pointers to the new version.

**Grace Periods**: The time interval during which all pre-existing readers complete their read-side critical sections. Old data can only be reclaimed after a grace period.

**Quiescent States**: Points where threads are guaranteed not to be holding references to RCU-protected data.

## Implementation Example

Here's a practical RCU-like implementation for a concurrent data structure:

```cpp
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <iostream>
#include <chrono>
#include <functional>

// Simple RCU implementation for managing versioned data
template<typename T>
class RCUProtected {
private:
    std::atomic<T*> data_ptr;
    
    // Track active readers per thread
    struct ReaderEpoch {
        std::atomic<uint64_t> epoch{0};
        char padding[64 - sizeof(std::atomic<uint64_t>)]; // Avoid false sharing
    };
    
    static constexpr size_t MAX_THREADS = 128;
    ReaderEpoch reader_epochs[MAX_THREADS];
    std::atomic<uint64_t> global_epoch{1};
    
    // Retired data waiting for grace period
    struct RetiredNode {
        T* ptr;
        uint64_t retire_epoch;
    };
    std::vector<RetiredNode> retired_list;
    std::mutex retired_mutex;
    
    thread_local static size_t thread_id;
    static std::atomic<size_t> next_thread_id;

public:
    RCUProtected(const T& initial_value) 
        : data_ptr(new T(initial_value)) {}
    
    ~RCUProtected() {
        delete data_ptr.load();
        std::lock_guard<std::mutex> lock(retired_mutex);
        for (auto& node : retired_list) {
            delete node.ptr;
        }
    }
    
    // RAII helper for read-side critical sections
    class ReadGuard {
        RCUProtected* rcu;
        size_t tid;
        
    public:
        ReadGuard(RCUProtected* r) : rcu(r) {
            tid = get_thread_id();
            // Mark this thread as active in current epoch
            rcu->reader_epochs[tid].epoch.store(
                rcu->global_epoch.load(std::memory_order_acquire),
                std::memory_order_release
            );
        }
        
        ~ReadGuard() {
            // Mark this thread as inactive
            rcu->reader_epochs[tid].epoch.store(0, std::memory_order_release);
        }
        
        const T* operator->() const {
            return rcu->data_ptr.load(std::memory_order_consume);
        }
        
        const T& operator*() const {
            return *rcu->data_ptr.load(std::memory_order_consume);
        }
    };
    
    // Read access - lock-free
    ReadGuard read() {
        return ReadGuard(this);
    }
    
    // Update operation - copy-modify-swap pattern
    template<typename UpdateFunc>
    void update(UpdateFunc func) {
        // Create a copy of current data
        T* old_ptr = data_ptr.load(std::memory_order_acquire);
        T* new_ptr = new T(*old_ptr);
        
        // Modify the copy
        func(*new_ptr);
        
        // Atomically swap to new version
        data_ptr.store(new_ptr, std::memory_order_release);
        
        // Increment global epoch to mark this update
        uint64_t retire_epoch = global_epoch.fetch_add(1, std::memory_order_acq_rel);
        
        // Retire old version
        {
            std::lock_guard<std::mutex> lock(retired_mutex);
            retired_list.push_back({old_ptr, retire_epoch});
        }
        
        // Try to reclaim retired objects
        reclaim_retired();
    }
    
private:
    static size_t get_thread_id() {
        if (thread_id == 0) {
            thread_id = next_thread_id.fetch_add(1) + 1;
        }
        return thread_id - 1;
    }
    
    // Reclaim objects that are no longer accessible by any reader
    void reclaim_retired() {
        uint64_t min_epoch = global_epoch.load(std::memory_order_acquire);
        
        // Find minimum active reader epoch
        for (size_t i = 0; i < MAX_THREADS; ++i) {
            uint64_t epoch = reader_epochs[i].epoch.load(std::memory_order_acquire);
            if (epoch != 0 && epoch < min_epoch) {
                min_epoch = epoch;
            }
        }
        
        // Reclaim objects retired before min_epoch
        std::lock_guard<std::mutex> lock(retired_mutex);
        auto it = retired_list.begin();
        while (it != retired_list.end()) {
            if (it->retire_epoch < min_epoch) {
                delete it->ptr;
                it = retired_list.erase(it);
            } else {
                ++it;
            }
        }
    }
};

template<typename T>
thread_local size_t RCUProtected<T>::thread_id = 0;

template<typename T>
std::atomic<size_t> RCUProtected<T>::next_thread_id{0};

// Example: RCU-protected configuration object
struct Config {
    int max_connections;
    std::string server_name;
    double timeout_seconds;
    
    void print() const {
        std::cout << "Config: " << server_name 
                  << ", max_conn=" << max_connections
                  << ", timeout=" << timeout_seconds << "s\n";
    }
};

// Example usage with read-heavy workload
void demonstrate_rcu() {
    Config initial_config{100, "Server-v1", 30.0};
    RCUProtected<Config> config(initial_config);
    
    std::atomic<bool> stop{false};
    std::atomic<uint64_t> read_count{0};
    
    // Multiple reader threads (read-heavy workload)
    std::vector<std::thread> readers;
    for (int i = 0; i < 8; ++i) {
        readers.emplace_back([&config, &stop, &read_count, i]() {
            while (!stop.load()) {
                // Lock-free read access
                auto guard = config.read();
                
                // Access data without contention
                int connections = guard->max_connections;
                std::string name = guard->server_name;
                
                // Simulate some work
                volatile int dummy = connections * 2;
                (void)dummy;
                
                read_count.fetch_add(1, std::memory_order_relaxed);
            }
        });
    }
    
    // Single writer thread (infrequent updates)
    std::thread writer([&config, &stop]() {
        int version = 2;
        while (!stop.load()) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            
            // Update by modifying a copy
            config.update([&version](Config& cfg) {
                cfg.server_name = "Server-v" + std::to_string(version++);
                cfg.max_connections += 10;
                cfg.timeout_seconds += 0.5;
            });
            
            std::cout << "Updated configuration to version " << (version - 1) << "\n";
        }
    });
    
    // Run for a short time
    std::this_thread::sleep_for(std::chrono::seconds(2));
    stop.store(true);
    
    // Join all threads
    for (auto& t : readers) {
        t.join();
    }
    writer.join();
    
    std::cout << "Total reads completed: " << read_count.load() << "\n";
    
    // Final state
    auto final = config.read();
    std::cout << "Final configuration: ";
    final->print();
}

int main() {
    std::cout << "=== RCU Pattern Demonstration ===\n\n";
    demonstrate_rcu();
    return 0;
}
```

## Advanced RCU Pattern: Concurrent Linked List

Here's a more complex example showing RCU applied to a linked list:

```cpp
#include <atomic>
#include <memory>
#include <thread>
#include <vector>
#include <iostream>

template<typename T>
class RCUList {
private:
    struct Node {
        T data;
        std::atomic<Node*> next;
        
        Node(const T& val) : data(val), next(nullptr) {}
    };
    
    std::atomic<Node*> head{nullptr};
    
    // Thread-local reader tracking
    struct ThreadInfo {
        std::atomic<bool> in_read_section{false};
        char padding[63]; // Cache line padding
    };
    
    static constexpr size_t MAX_THREADS = 64;
    ThreadInfo thread_info[MAX_THREADS];
    
    thread_local static size_t thread_id;
    static std::atomic<size_t> next_id;
    
    // Garbage collection
    std::atomic<Node*> retired_list{nullptr};
    
public:
    ~RCUList() {
        // Clean up all nodes
        Node* current = head.load();
        while (current) {
            Node* next = current->next.load();
            delete current;
            current = next;
        }
        
        // Clean up retired nodes
        current = retired_list.load();
        while (current) {
            Node* next = current->next.load();
            delete current;
            current = next;
        }
    }
    
    // RAII guard for read operations
    class ReadLock {
        RCUList* list;
        size_t tid;
        
    public:
        ReadLock(RCUList* l) : list(l) {
            tid = get_thread_id();
            list->thread_info[tid].in_read_section.store(true, std::memory_order_release);
            std::atomic_thread_fence(std::memory_order_seq_cst);
        }
        
        ~ReadLock() {
            list->thread_info[tid].in_read_section.store(false, std::memory_order_release);
        }
    };
    
    // Insert at head - writer operation
    void push_front(const T& value) {
        Node* new_node = new Node(value);
        Node* old_head = head.load(std::memory_order_acquire);
        
        do {
            new_node->next.store(old_head, std::memory_order_relaxed);
        } while (!head.compare_exchange_weak(old_head, new_node,
                                              std::memory_order_release,
                                              std::memory_order_acquire));
    }
    
    // Remove first occurrence - writer operation
    bool remove(const T& value) {
        Node* prev = nullptr;
        Node* current = head.load(std::memory_order_acquire);
        
        // Find the node to remove
        while (current) {
            if (current->data == value) {
                Node* next = current->next.load(std::memory_order_acquire);
                
                // Update pointer
                if (prev) {
                    prev->next.store(next, std::memory_order_release);
                } else {
                    head.store(next, std::memory_order_release);
                }
                
                // Retire the node instead of immediate deletion
                retire_node(current);
                return true;
            }
            prev = current;
            current = current->next.load(std::memory_order_acquire);
        }
        
        return false;
    }
    
    // Read operation - lock-free traversal
    template<typename Func>
    void for_each(Func func) {
        ReadLock lock(this);
        
        Node* current = head.load(std::memory_order_consume);
        while (current) {
            func(current->data);
            current = current->next.load(std::memory_order_consume);
        }
    }
    
    // Check if value exists
    bool contains(const T& value) {
        ReadLock lock(this);
        
        Node* current = head.load(std::memory_order_consume);
        while (current) {
            if (current->data == value) {
                return true;
            }
            current = current->next.load(std::memory_order_consume);
        }
        return false;
    }
    
private:
    static size_t get_thread_id() {
        if (thread_id == 0) {
            thread_id = next_id.fetch_add(1, std::memory_order_relaxed) + 1;
        }
        return thread_id - 1;
    }
    
    void retire_node(Node* node) {
        // Add to retired list
        Node* old_retired = retired_list.load(std::memory_order_acquire);
        do {
            node->next.store(old_retired, std::memory_order_relaxed);
        } while (!retired_list.compare_exchange_weak(old_retired, node,
                                                      std::memory_order_release,
                                                      std::memory_order_acquire));
        
        // Try to reclaim memory
        try_reclaim();
    }
    
    void try_reclaim() {
        // Check if any thread is in a read section
        for (size_t i = 0; i < MAX_THREADS; ++i) {
            if (thread_info[i].in_read_section.load(std::memory_order_acquire)) {
                return; // Can't reclaim yet
            }
        }
        
        // Safe to reclaim all retired nodes
        Node* retired = retired_list.exchange(nullptr, std::memory_order_acquire);
        while (retired) {
            Node* next = retired->next.load(std::memory_order_relaxed);
            delete retired;
            retired = next;
        }
    }
};

template<typename T>
thread_local size_t RCUList<T>::thread_id = 0;

template<typename T>
std::atomic<size_t> RCUList<T>::next_id{0};

// Demonstration
void demonstrate_rcu_list() {
    RCUList<int> list;
    
    // Initialize list
    for (int i = 0; i < 10; ++i) {
        list.push_front(i);
    }
    
    std::atomic<bool> stop{false};
    
    // Reader threads
    std::vector<std::thread> readers;
    for (int i = 0; i < 4; ++i) {
        readers.emplace_back([&list, &stop, i]() {
            int read_count = 0;
            while (!stop.load()) {
                list.for_each([](int val) {
                    volatile int x = val * 2; // Simulate work
                    (void)x;
                });
                ++read_count;
            }
            std::cout << "Reader " << i << " completed " << read_count << " iterations\n";
        });
    }
    
    // Writer thread
    std::thread writer([&list, &stop]() {
        for (int i = 10; i < 20 && !stop.load(); ++i) {
            list.push_front(i);
            std::this_thread::sleep_for(std::chrono::milliseconds(50));
        }
    });
    
    std::this_thread::sleep_for(std::chrono::seconds(1));
    stop.store(true);
    
    for (auto& t : readers) {
        t.join();
    }
    writer.join();
    
    std::cout << "\nFinal list contents: ";
    list.for_each([](int val) {
        std::cout << val << " ";
    });
    std::cout << "\n";
}

int main() {
    std::cout << "=== RCU List Demonstration ===\n\n";
    demonstrate_rcu_list();
    return 0;
}
```

## Summary

**Key Benefits of RCU:**

- **Scalable reads**: Multiple readers operate without synchronization overhead, locks, or cache line bouncing
- **Wait-free reads**: Read operations never block or spin, providing bounded execution time
- **Writer flexibility**: Writers can perform complex updates without holding locks during the entire operation
- **Memory efficiency**: Old versions are reclaimed automatically after grace periods

**When to Use RCU:**

- Read-to-write ratio is very high (10:1 or higher)
- Read operations are performance-critical and occur frequently
- Data structures are relatively small and copying is feasible
- Consistency requirements allow readers to see slightly stale data briefly

**Trade-offs:**

- Writers are more expensive due to copy-on-write overhead
- Memory usage increases temporarily during updates
- Implementation complexity is higher than simple locking
- Grace period detection adds bookkeeping overhead

**Common Applications:**

- Configuration management systems
- Route tables and network packet processing
- File system path lookups
- Registry/cache implementations in operating systems
- Shared lookup tables in high-performance servers

The RCU pattern excels in scenarios where data is read frequently but modified rarely, offering near-optimal read performance by eliminating synchronization overhead entirely from the read path.