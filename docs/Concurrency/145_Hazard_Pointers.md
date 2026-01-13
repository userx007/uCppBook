# Hazard Pointers: Safe Memory Reclamation in Lock-Free Data Structures

## Overview

Hazard pointers are a memory reclamation technique for lock-free data structures that solves the ABA problem and prevents use-after-free bugs. In lock-free programming, we can't simply delete objects when we're done with them because other threads might still be accessing them. Hazard pointers provide a way for threads to announce which pointers they're currently using, allowing safe deferred reclamation.

## The Problem

In lock-free data structures, a thread might read a pointer, get preempted, and by the time it resumes, another thread could have removed and deleted that object. Consider this scenario:

```cpp
// Thread 1 reads head
Node* old_head = head.load();
// Thread 1 gets preempted here

// Thread 2 pops the node and deletes it
Node* node = head.load();
head.store(node->next);
delete node;  // Frees memory thread 1 is about to use!

// Thread 1 resumes and accesses freed memory
return old_head->data;  // Use-after-free!
```

## How Hazard Pointers Work

1. **Announcement**: Before accessing a pointer, a thread "announces" it by storing it in a hazard pointer
2. **Validation**: The thread verifies the pointer is still valid after announcement
3. **Protection**: Other threads check hazard pointers before reclaiming memory
4. **Deferred Reclamation**: Objects are added to a retire list and only freed when no hazard pointers reference them

## Implementation

### Basic Hazard Pointer Manager

```cpp
#include <atomic>
#include <vector>
#include <thread>
#include <algorithm>
#include <memory>

// Represents a single hazard pointer slot
struct HazardPointer {
    std::atomic<std::thread::id> owner{std::thread::id()};
    std::atomic<void*> pointer{nullptr};
};

class HazardPointerManager {
private:
    static constexpr size_t MAX_HAZARD_POINTERS = 100;
    std::vector<HazardPointer> hazard_pointers_;
    
public:
    HazardPointerManager() : hazard_pointers_(MAX_HAZARD_POINTERS) {}
    
    // Acquire a hazard pointer for the current thread
    HazardPointer* acquire() {
        auto this_id = std::this_thread::get_id();
        
        // Try to reuse an existing slot
        for (auto& hp : hazard_pointers_) {
            std::thread::id expected{};
            if (hp.owner.compare_exchange_strong(expected, this_id)) {
                return &hp;
            }
        }
        
        // Should handle running out of slots in production
        throw std::runtime_error("No hazard pointers available");
    }
    
    // Release a hazard pointer
    void release(HazardPointer* hp) {
        hp->pointer.store(nullptr);
        hp->owner.store(std::thread::id());
    }
    
    // Check if a pointer is protected by any hazard pointer
    bool is_protected(void* ptr) {
        for (const auto& hp : hazard_pointers_) {
            if (hp.pointer.load() == ptr) {
                return true;
            }
        }
        return false;
    }
};

// Global hazard pointer manager
static HazardPointerManager hazard_manager;
```

### RAII Hazard Pointer Guard

```cpp
template<typename T>
class HazardPointerGuard {
private:
    HazardPointer* hp_;
    
public:
    HazardPointerGuard() : hp_(hazard_manager.acquire()) {}
    
    ~HazardPointerGuard() {
        if (hp_) {
            hazard_manager.release(hp_);
        }
    }
    
    // Non-copyable but movable
    HazardPointerGuard(const HazardPointerGuard&) = delete;
    HazardPointerGuard& operator=(const HazardPointerGuard&) = delete;
    
    HazardPointerGuard(HazardPointerGuard&& other) noexcept 
        : hp_(other.hp_) {
        other.hp_ = nullptr;
    }
    
    // Protect a pointer and verify it's still valid
    T* protect(const std::atomic<T*>& atomic_ptr) {
        T* ptr = atomic_ptr.load();
        
        while (ptr) {
            // Announce the pointer
            hp_->pointer.store(ptr);
            
            // Memory barrier to ensure ordering
            std::atomic_thread_fence(std::memory_order_seq_cst);
            
            // Verify it's still valid
            T* current = atomic_ptr.load();
            if (current == ptr) {
                return ptr;  // Successfully protected
            }
            
            // Pointer changed, try again
            ptr = current;
        }
        
        hp_->pointer.store(nullptr);
        return nullptr;
    }
    
    void reset() {
        if (hp_) {
            hp_->pointer.store(nullptr);
        }
    }
};
```

### Lock-Free Stack with Hazard Pointers

```cpp
template<typename T>
class LockFreeStack {
private:
    struct Node {
        T data;
        Node* next;
        
        Node(const T& value) : data(value), next(nullptr) {}
    };
    
    std::atomic<Node*> head_{nullptr};
    std::atomic<size_t> retire_count_{0};
    
    // Thread-local retire list
    thread_local static std::vector<Node*> retire_list_;
    static constexpr size_t RETIRE_THRESHOLD = 10;
    
    void retire_node(Node* node) {
        retire_list_.push_back(node);
        
        if (retire_list_.size() >= RETIRE_THRESHOLD) {
            reclaim_memory();
        }
    }
    
    void reclaim_memory() {
        std::vector<Node*> still_in_use;
        
        for (Node* node : retire_list_) {
            if (hazard_manager.is_protected(node)) {
                // Still protected by some thread
                still_in_use.push_back(node);
            } else {
                // Safe to delete
                delete node;
            }
        }
        
        retire_list_ = std::move(still_in_use);
    }
    
public:
    void push(const T& value) {
        Node* new_node = new Node(value);
        new_node->next = head_.load();
        
        while (!head_.compare_exchange_weak(new_node->next, new_node)) {
            // CAS failed, retry with updated next pointer
        }
    }
    
    bool pop(T& result) {
        HazardPointerGuard<Node> guard;
        
        while (true) {
            // Protect the head pointer
            Node* old_head = guard.protect(head_);
            
            if (!old_head) {
                return false;  // Stack is empty
            }
            
            Node* next = old_head->next;
            
            // Try to remove the head
            if (head_.compare_exchange_strong(old_head, next)) {
                // Successfully removed
                result = old_head->data;
                
                // Can't delete immediately - retire instead
                guard.reset();
                retire_node(old_head);
                
                return true;
            }
            
            // CAS failed, another thread modified head
            // Loop will retry with new protection
        }
    }
    
    ~LockFreeStack() {
        T dummy;
        while (pop(dummy)) {}
        
        // Force final reclamation
        reclaim_memory();
    }
};

// Initialize thread-local storage
template<typename T>
thread_local std::vector<typename LockFreeStack<T>::Node*> 
    LockFreeStack<T>::retire_list_;
```

### Complete Usage Example

```cpp
#include <iostream>
#include <thread>
#include <vector>

void producer(LockFreeStack<int>& stack, int start, int count) {
    for (int i = start; i < start + count; ++i) {
        stack.push(i);
        std::this_thread::sleep_for(std::chrono::microseconds(10));
    }
}

void consumer(LockFreeStack<int>& stack, int id, std::atomic<int>& sum) {
    int value;
    int local_sum = 0;
    int items_processed = 0;
    
    while (items_processed < 50) {  // Process 50 items
        if (stack.pop(value)) {
            local_sum += value;
            items_processed++;
        } else {
            std::this_thread::yield();
        }
    }
    
    sum.fetch_add(local_sum);
    std::cout << "Consumer " << id << " processed " 
              << items_processed << " items, sum: " << local_sum << "\n";
}

int main() {
    LockFreeStack<int> stack;
    std::atomic<int> total_sum{0};
    
    std::vector<std::thread> threads;
    
    // Start producers
    threads.emplace_back(producer, std::ref(stack), 0, 100);
    threads.emplace_back(producer, std::ref(stack), 100, 100);
    
    // Start consumers
    for (int i = 0; i < 4; ++i) {
        threads.emplace_back(consumer, std::ref(stack), i, std::ref(total_sum));
    }
    
    // Wait for all threads
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Total sum from consumers: " << total_sum.load() << "\n";
    std::cout << "Expected sum (0-199): " << (199 * 200 / 2) << "\n";
    
    return 0;
}
```

## Advanced Considerations

### Performance Optimization

```cpp
// Use thread-local caching to reduce contention
class OptimizedHazardPointerManager {
private:
    struct alignas(64) CacheLine {  // Prevent false sharing
        std::atomic<void*> pointer{nullptr};
        std::atomic<bool> in_use{false};
    };
    
    std::vector<CacheLine> hp_array_;
    
    // Thread-local index cache
    thread_local static int cached_index_;
    
public:
    CacheLine* acquire_fast() {
        // Try cached index first
        if (cached_index_ >= 0 && 
            !hp_array_[cached_index_].in_use.exchange(true)) {
            return &hp_array_[cached_index_];
        }
        
        // Fall back to linear search
        for (size_t i = 0; i < hp_array_.size(); ++i) {
            if (!hp_array_[i].in_use.exchange(true)) {
                cached_index_ = i;
                return &hp_array_[i];
            }
        }
        
        return nullptr;
    }
};
```

## Summary

**Hazard Pointers** provide safe memory reclamation for lock-free data structures through a protect-validate-reclaim pattern:

- **Protection**: Threads announce which pointers they're using by storing them in hazard pointer slots
- **Validation**: After announcing, threads verify the pointer hasn't changed to prevent ABA issues
- **Deferred Reclamation**: Objects are retired to a list and only freed when no hazard pointers reference them

**Key Benefits**:
- **Safety**: Prevents use-after-free bugs in lock-free code
- **Lock-Free**: No blocking operations, maintaining progress guarantees
- **Predictable**: Bounded memory overhead (unlike epoch-based schemes)

**Trade-offs**:
- **Memory Overhead**: Requires hazard pointer slots for each thread
- **Scanning Cost**: Must scan all hazard pointers before reclaiming
- **Complexity**: More intricate than garbage-collected approaches

Hazard pointers are ideal for lock-free data structures where you need deterministic memory reclamation without garbage collection, making them popular in systems programming and high-performance concurrent applications.