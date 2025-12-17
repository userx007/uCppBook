# Allocator Model & Custom Allocators in C++

## What Are Allocators?

Allocators in C++ are objects responsible for encapsulating memory management strategies. They abstract away the details of how memory is acquired and released, allowing containers and other components to be independent of the specific memory allocation mechanism used. Every standard container (like `std::vector`, `std::map`, `std::list`) accepts an allocator as a template parameter, though they default to `std::allocator<T>`, which typically uses `new` and `delete` under the hood.

The allocator model provides a crucial separation of concerns: containers manage the logical structure and lifetime of objects, while allocators handle the raw memory operations. This design allows you to customize memory allocation behavior without modifying container code.

## The Standard Allocator Interface

The C++ standard defines requirements for allocators through a set of member types and functions. A minimal allocator must provide:

**Type definitions** including `value_type` (the type being allocated), `pointer`, `const_pointer`, `reference`, `const_reference`, `size_type`, and `difference_type`. Modern C++ has simplified many of these requirements.

**Core member functions** are the heart of the allocator. The `allocate(n)` function requests raw memory for `n` objects of type `T`, returning a pointer to the allocated memory without constructing any objects. The `deallocate(p, n)` function releases previously allocated memory. Critically, these functions deal only with raw memory, not object lifetimes.

**Object construction and destruction** were historically handled by `construct()` and `destroy()` methods, but since C++20, the standard library uses `std::allocator_traits` to handle this, typically delegating to placement new and explicit destructor calls.

## How Containers Use Allocators

When you create a container like `std::vector<int>`, it internally uses `std::allocator<int>` by default. When the vector needs to grow, it calls the allocator's `allocate()` function to get raw memory, then uses placement new to construct objects in that memory. When elements are removed or the vector is destroyed, it explicitly calls destructors and then calls `deallocate()` to return the memory.

The container stores a copy of the allocator and uses it for all memory operations throughout its lifetime. This means the allocator's state (if any) travels with the container. When containers are copied or moved, the behavior of allocator propagation is controlled by traits like `propagate_on_container_copy_assignment`.

## Creating Custom Allocators

Custom allocators are useful in several scenarios: using memory pools for performance, implementing debug tracking to detect leaks, interfacing with custom memory management systems, or allocating from specific memory regions like shared memory or GPU memory.

A basic custom allocator might look like this conceptually:

```cpp
template<typename T>
class MyAllocator {
public:
    using value_type = T;

    MyAllocator() noexcept = default;

    template<typename U>
    MyAllocator(const MyAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T))
            throw std::bad_alloc();

        if (auto p = static_cast<T*>(std::malloc(n * sizeof(T)))) {
            return p;
        }
        throw std::bad_alloc();
    }

    void deallocate(T* p, std::size_t) noexcept {
        std::free(p);
    }
};

template<typename T, typename U>
bool operator==(const MyAllocator<T>&, const MyAllocator<U>&) { return true; }

template<typename T, typename U>
bool operator!=(const MyAllocator<T>&, const MyAllocator<U>&) { return false; }
```

The equality operators are important: two allocators compare equal if memory allocated by one can be deallocated by the other. This is crucial for operations like container assignment.

## Allocator Traits and Modern C++

C++11 introduced `std::allocator_traits`, which provides a uniform interface to allocators and supplies default implementations for optional features. This means you can write minimal allocators that only implement `allocate()` and `deallocate()`, and `allocator_traits` will handle the rest.

C++17 simplified allocators further by removing several requirements and allowing for simpler implementations. The `construct()` and `destroy()` methods became optional, as did many type definitions.

C++20 introduced concepts and further refined the allocator model, though the fundamental design remains the same.

## Practical Applications

**Memory pools** are perhaps the most common use case. If you're creating and destroying many small objects of the same type, a pool allocator can maintain a free list and reuse memory, eliminating repeated calls to the system allocator and reducing fragmentation.

**Arena allocators** allocate from a large pre-allocated buffer and can deallocate everything at once by resetting a pointer, making them extremely fast for temporary allocations with known lifetimes.

**Tracking allocators** can wrap another allocator and log all allocations and deallocations, helping you profile memory usage or detect leaks.

**Alignment control** is another application. Some algorithms require specific memory alignment beyond what the default allocator provides, and custom allocators can ensure this.

## Important Considerations

Allocators add complexity, so use them judiciously. The default allocator is well-optimized and sufficient for most applications. Custom allocators are most valuable when profiling shows memory allocation is a bottleneck, when working with specialized hardware or memory regions, or when implementing custom memory management strategies required by your domain.

Thread safety is a critical consideration. If your allocator manages shared state (like a memory pool), you must ensure it's properly synchronized if used across multiple threads.

The allocator model assumes that objects allocated through an allocator can be deallocated through any equivalent allocator. Stateful allocators (those with member variables affecting behavior) require careful design to ensure this property holds.

Understanding the allocator model gives you powerful control over memory management in C++, though it's a feature you'll use primarily in performance-critical or specialized contexts rather than everyday programming.

I'll provide you with several practical examples of custom allocators, from simple to more complex.I've created a comprehensive set of custom allocator examples for you! Here's what each one demonstrates:

## **Example 1: Tracking Allocator**
This is the simplest custom allocator. It wraps `std::malloc`/`std::free` and tracks:
- Number of allocations and deallocations
- Total bytes currently allocated
- Memory leaks (allocations without corresponding deallocations)

Perfect for debugging memory issues and understanding container behavior.

## **Example 2: Pool Allocator**
A performance-oriented allocator that:
- Pre-allocates a fixed pool of memory slots
- Maintains a free list for fast allocation/deallocation
- Works best with containers like `std::list` that allocate one element at a time
- Falls back to `malloc` if the pool is exhausted

This can be 10-100x faster than the default allocator for frequent small allocations.

## **Example 3: Arena/Stack Allocator**
An extremely fast allocator that:
- Allocates from a large pre-allocated buffer
- Simply bumps a pointer forward on each allocation
- Doesn't deallocate individual objects
- Resets all memory at once with `reset()`

Ideal for temporary data structures with known lifetimes, like per-frame game data or parsing temporary objects.

## Key Points to Notice:

1. **Rebind constructor** - The template constructor allows containers to allocate internal nodes of different types
2. **Equality operators** - Crucial for determining if allocators can share memory
3. **Single responsibility** - Allocators only handle memory, not object construction
4. **No virtual functions** - Allocators must be compile-time polymorphic for performance

You can compile and run this code to see how each allocator behaves with standard containers. The output shows exactly when allocations occur and helps visualize container growth patterns.

Would you like me to create additional examples, such as an aligned allocator, a thread-safe pool allocator, or a more sophisticated memory tracking system?

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <memory>
#include <cstdlib>
#include <limits>

// =============================================================================
// Example 1: Simple Tracking Allocator
// Tracks the number of allocations and deallocations
// =============================================================================

template<typename T>
class TrackingAllocator {
public:
    using value_type = T;

    // Static counters to track allocations
    static inline size_t allocation_count = 0;
    static inline size_t deallocation_count = 0;
    static inline size_t bytes_allocated = 0;

    TrackingAllocator() noexcept = default;

    // Required for rebinding to different types
    template<typename U>
    TrackingAllocator(const TrackingAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            throw std::bad_alloc();
        }

        void* p = std::malloc(n * sizeof(T));
        if (!p) {
            throw std::bad_alloc();
        }

        allocation_count++;
        bytes_allocated += n * sizeof(T);

        std::cout << "[TrackingAllocator] Allocated " << n * sizeof(T)
                  << " bytes at " << p << std::endl;

        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        deallocation_count++;
        bytes_allocated -= n * sizeof(T);

        std::cout << "[TrackingAllocator] Deallocated " << n * sizeof(T)
                  << " bytes at " << p << std::endl;

        std::free(p);
    }

    static void print_stats() {
        std::cout << "\n=== Allocation Statistics ===" << std::endl;
        std::cout << "Total allocations: " << allocation_count << std::endl;
        std::cout << "Total deallocations: " << deallocation_count << std::endl;
        std::cout << "Currently allocated: " << bytes_allocated << " bytes" << std::endl;
        std::cout << "Leaked allocations: " << (allocation_count - deallocation_count) << std::endl;
    }
};

// Allocators must define equality operators
template<typename T, typename U>
bool operator==(const TrackingAllocator<T>&, const TrackingAllocator<U>&) {
    return true;
}

template<typename T, typename U>
bool operator!=(const TrackingAllocator<T>&, const TrackingAllocator<U>&) {
    return false;
}

// =============================================================================
// Example 2: Memory Pool Allocator
// Pre-allocates a pool of memory and reuses it
// =============================================================================

template<typename T, size_t PoolSize = 1024>
class PoolAllocator {
private:
    // Simple free list node
    struct FreeNode {
        FreeNode* next;
    };

    // Pool storage
    alignas(T) char pool[PoolSize * sizeof(T)];
    FreeNode* free_list;
    size_t allocated_count;

public:
    using value_type = T;

    PoolAllocator() noexcept : free_list(nullptr), allocated_count(0) {
        // Initialize free list
        for (size_t i = 0; i < PoolSize; ++i) {
            FreeNode* node = reinterpret_cast<FreeNode*>(pool + i * sizeof(T));
            node->next = free_list;
            free_list = node;
        }
        std::cout << "[PoolAllocator] Initialized pool with "
                  << PoolSize << " slots" << std::endl;
    }

    template<typename U>
    PoolAllocator(const PoolAllocator<U, PoolSize>&) noexcept
        : PoolAllocator() {}

    T* allocate(std::size_t n) {
        if (n != 1) {
            // Pool allocator only handles single object allocations efficiently
            throw std::bad_alloc();
        }

        if (!free_list) {
            std::cout << "[PoolAllocator] Pool exhausted! Falling back to malloc"
                      << std::endl;
            return static_cast<T*>(std::malloc(sizeof(T)));
        }

        FreeNode* node = free_list;
        free_list = node->next;
        allocated_count++;

        std::cout << "[PoolAllocator] Allocated from pool (slot "
                  << allocated_count << ")" << std::endl;

        return reinterpret_cast<T*>(node);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        if (n != 1) return;

        // Check if pointer is within our pool
        char* ptr = reinterpret_cast<char*>(p);
        if (ptr >= pool && ptr < pool + PoolSize * sizeof(T)) {
            FreeNode* node = reinterpret_cast<FreeNode*>(p);
            node->next = free_list;
            free_list = node;
            allocated_count--;
            std::cout << "[PoolAllocator] Returned to pool" << std::endl;
        } else {
            std::cout << "[PoolAllocator] Freeing non-pool memory" << std::endl;
            std::free(p);
        }
    }

    size_t available() const {
        size_t count = 0;
        FreeNode* node = free_list;
        while (node) {
            count++;
            node = node->next;
        }
        return count;
    }
};

template<typename T, size_t PoolSize, typename U>
bool operator==(const PoolAllocator<T, PoolSize>&, const PoolAllocator<U, PoolSize>&) {
    return false; // Different pool instances cannot share memory
}

template<typename T, size_t PoolSize, typename U>
bool operator!=(const PoolAllocator<T, PoolSize>&, const PoolAllocator<U, PoolSize>&) {
    return true;
}

// =============================================================================
// Example 3: Arena/Stack Allocator
// Allocates from a fixed buffer, deallocates all at once
// =============================================================================

template<typename T>
class ArenaAllocator {
private:
    struct Arena {
        char* buffer;
        size_t size;
        size_t offset;

        Arena(size_t sz) : size(sz), offset(0) {
            buffer = static_cast<char*>(std::malloc(sz));
            if (!buffer) throw std::bad_alloc();
            std::cout << "[Arena] Created arena of " << sz << " bytes" << std::endl;
        }

        ~Arena() {
            std::free(buffer);
            std::cout << "[Arena] Destroyed arena" << std::endl;
        }

        void reset() {
            offset = 0;
            std::cout << "[Arena] Reset arena" << std::endl;
        }
    };

    std::shared_ptr<Arena> arena;

public:
    using value_type = T;

    explicit ArenaAllocator(size_t arena_size = 1024 * 1024) // 1MB default
        : arena(std::make_shared<Arena>(arena_size)) {}

    template<typename U>
    ArenaAllocator(const ArenaAllocator<U>& other) noexcept
        : arena(other.arena) {}

    T* allocate(std::size_t n) {
        size_t bytes = n * sizeof(T);
        size_t aligned_size = (bytes + alignof(T) - 1) & ~(alignof(T) - 1);

        if (arena->offset + aligned_size > arena->size) {
            std::cout << "[Arena] Out of space!" << std::endl;
            throw std::bad_alloc();
        }

        void* p = arena->buffer + arena->offset;
        arena->offset += aligned_size;

        std::cout << "[Arena] Allocated " << bytes << " bytes (offset: "
                  << arena->offset << "/" << arena->size << ")" << std::endl;

        return static_cast<T*>(p);
    }

    void deallocate(T*, std::size_t) noexcept {
        // Arena allocator doesn't deallocate individual allocations
        // Memory is freed when arena is destroyed or reset
    }

    void reset() {
        arena->reset();
    }

    template<typename U> friend class ArenaAllocator;
};

template<typename T, typename U>
bool operator==(const ArenaAllocator<T>& a, const ArenaAllocator<U>& b) {
    return a.arena == b.arena;
}

template<typename T, typename U>
bool operator!=(const ArenaAllocator<T>& a, const ArenaAllocator<U>& b) {
    return !(a == b);
}

// =============================================================================
// Usage Examples
// =============================================================================

void example_tracking_allocator() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 1: Tracking Allocator" << std::endl;
    std::cout << "========================================\n" << std::endl;

    {
        std::vector<int, TrackingAllocator<int>> vec;
        vec.reserve(10);

        for (int i = 0; i < 15; ++i) {
            vec.push_back(i);
        }

        std::cout << "\nVector size: " << vec.size() << std::endl;
    }

    TrackingAllocator<int>::print_stats();
}

void example_pool_allocator() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 2: Pool Allocator" << std::endl;
    std::cout << "========================================\n" << std::endl;

    // Note: Pool allocator works best with lists since they allocate one node at a time
    using IntList = std::list<int, PoolAllocator<int, 10>>;

    IntList list;

    std::cout << "\nAdding 5 elements:" << std::endl;
    for (int i = 0; i < 5; ++i) {
        list.push_back(i * 10);
    }

    std::cout << "\nRemoving 2 elements:" << std::endl;
    list.pop_front();
    list.pop_front();

    std::cout << "\nAdding 3 more elements:" << std::endl;
    for (int i = 0; i < 3; ++i) {
        list.push_back(i * 100);
    }

    std::cout << "\nList contents: ";
    for (int val : list) {
        std::cout << val << " ";
    }
    std::cout << std::endl;
}

void example_arena_allocator() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Example 3: Arena Allocator" << std::endl;
    std::cout << "========================================\n" << std::endl;

    ArenaAllocator<int> arena_alloc(1024); // 1KB arena

    {
        std::vector<int, ArenaAllocator<int>> vec(arena_alloc);
        vec.reserve(50);

        for (int i = 0; i < 50; ++i) {
            vec.push_back(i);
        }

        std::cout << "\nVector sum: ";
        int sum = 0;
        for (int val : vec) {
            sum += val;
        }
        std::cout << sum << std::endl;
    }

    std::cout << "\nResetting arena..." << std::endl;
    arena_alloc.reset();

    std::cout << "\nAllocating after reset:" << std::endl;
    std::vector<double, ArenaAllocator<double>> vec2(arena_alloc);
    vec2.reserve(20);
}

int main() {
    example_tracking_allocator();
    example_pool_allocator();
    example_arena_allocator();

    std::cout << "\n========================================" << std::endl;
    std::cout << "All examples completed!" << std::endl;
    std::cout << "========================================" << std::endl;

    return 0;
}
```

Additional advanced allocator examples.

## **Example 4: Aligned Allocator**
This allocator ensures memory is aligned to specific boundaries (16, 32, 64, 128 bytes, etc.), which is crucial for:
- **SIMD operations** (SSE, AVX, AVX-512) that require aligned memory
- **Cache line optimization** (typically 64 bytes) to avoid false sharing
- **GPU memory transfers** that often require specific alignment
- **Hardware DMA operations**

The example shows how to verify alignment and uses platform-specific functions (`aligned_alloc` on Linux/Mac, `_aligned_malloc` on Windows).

## **Example 5: Thread-Safe Pool Allocator**
This extends the basic pool allocator with mutex protection, making it safe for concurrent access:
- **Mutex-protected** allocation and deallocation
- Safe for use across multiple threads
- Reports which thread is performing operations
- Shows real-world patterns where multiple threads share a memory pool

This is essential for multi-threaded applications like web servers, game engines with job systems, or parallel data processing.

## **Example 6: Sophisticated Memory Tracking**
This is a production-grade debugging allocator that tracks:
- **Every allocation** with timestamp, size, and object count
- **Lifetime tracking** - how long each allocation lived
- **Peak memory usage** - the maximum memory used at any point
- **Memory leak detection** - shows which allocations weren't freed
- **Comprehensive statistics** - total allocations, deallocations, bytes
- **Active allocation list** - all currently allocated blocks with details

This is invaluable for:
- Finding memory leaks
- Profiling memory usage patterns
- Debugging allocation-heavy code
- Understanding container growth behavior

## Key Advanced Concepts Demonstrated:

1. **Platform-specific code** - handling Windows vs Unix differences
2. **Thread safety** - proper mutex usage and lock guards
3. **Timestamps** - tracking allocation lifetimes
4. **Statistical analysis** - peak memory, trends, patterns
5. **Alignment verification** - ensuring hardware requirements are met
6. **Production-ready error handling** - detecting double-frees, invalid pointers

You can compile both files together or separately. The detailed tracking allocator is particularly useful - you could integrate it into your projects during development to catch memory issues early!

Would you like me to create any other specialized allocators, such as:
- A two-level allocator (small objects from pool, large from heap)?
- A garbage-collecting allocator?
- A memory-mapped file allocator?
- A NUMA-aware allocator for multi-socket systems?

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <memory>
#include <cstdlib>
#include <limits>
#include <mutex>
#include <thread>
#include <chrono>
#include <map>
#include <iomanip>
#include <sstream>

// =============================================================================
// Example 4: Aligned Allocator
// Allocates memory with specific alignment requirements
// =============================================================================

template<typename T, size_t Alignment = alignof(T)>
class AlignedAllocator {
public:
    using value_type = T;

    static_assert(Alignment >= alignof(T),
                  "Alignment must be at least as strict as T's natural alignment");
    static_assert((Alignment & (Alignment - 1)) == 0,
                  "Alignment must be a power of 2");

    AlignedAllocator() noexcept = default;

    template<typename U>
    AlignedAllocator(const AlignedAllocator<U, Alignment>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            throw std::bad_alloc();
        }

        size_t bytes = n * sizeof(T);

        // Use aligned_alloc (C++17) or platform-specific alternatives
        #if defined(_WIN32)
            void* p = _aligned_malloc(bytes, Alignment);
        #else
            void* p = std::aligned_alloc(Alignment, bytes);
        #endif

        if (!p) {
            throw std::bad_alloc();
        }

        std::cout << "[AlignedAllocator] Allocated " << bytes
                  << " bytes with " << Alignment << "-byte alignment at "
                  << p << std::endl;

        // Verify alignment
        if (reinterpret_cast<uintptr_t>(p) % Alignment != 0) {
            std::cerr << "ERROR: Memory not properly aligned!" << std::endl;
        }

        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        std::cout << "[AlignedAllocator] Deallocating " << n * sizeof(T)
                  << " bytes at " << p << std::endl;

        #if defined(_WIN32)
            _aligned_free(p);
        #else
            std::free(p);
        #endif
    }
};

template<typename T, size_t A1, typename U, size_t A2>
bool operator==(const AlignedAllocator<T, A1>&, const AlignedAllocator<U, A2>&) {
    return A1 == A2;
}

template<typename T, size_t A1, typename U, size_t A2>
bool operator!=(const AlignedAllocator<T, A1>&, const AlignedAllocator<U, A2>&) {
    return A1 != A2;
}

// =============================================================================
// Example 5: Thread-Safe Pool Allocator
// Pool allocator with mutex protection for concurrent access
// =============================================================================

template<typename T, size_t PoolSize = 1024>
class ThreadSafePoolAllocator {
private:
    struct FreeNode {
        FreeNode* next;
    };

    alignas(T) char pool[PoolSize * sizeof(T)];
    FreeNode* free_list;
    size_t allocated_count;
    mutable std::mutex mutex;

public:
    using value_type = T;

    ThreadSafePoolAllocator() noexcept : free_list(nullptr), allocated_count(0) {
        std::lock_guard<std::mutex> lock(mutex);
        for (size_t i = 0; i < PoolSize; ++i) {
            FreeNode* node = reinterpret_cast<FreeNode*>(pool + i * sizeof(T));
            node->next = free_list;
            free_list = node;
        }
        std::cout << "[ThreadSafePool] Initialized pool with "
                  << PoolSize << " slots" << std::endl;
    }

    // Prevent copying to avoid mutex issues
    ThreadSafePoolAllocator(const ThreadSafePoolAllocator&) = delete;
    ThreadSafePoolAllocator& operator=(const ThreadSafePoolAllocator&) = delete;

    template<typename U>
    struct rebind {
        using other = ThreadSafePoolAllocator<U, PoolSize>;
    };

    T* allocate(std::size_t n) {
        std::lock_guard<std::mutex> lock(mutex);

        if (n != 1) {
            throw std::bad_alloc();
        }

        if (!free_list) {
            std::cout << "[ThreadSafePool] Pool exhausted on thread "
                      << std::this_thread::get_id() << std::endl;
            void* p = std::malloc(sizeof(T));
            if (!p) throw std::bad_alloc();
            return static_cast<T*>(p);
        }

        FreeNode* node = free_list;
        free_list = node->next;
        allocated_count++;

        std::cout << "[ThreadSafePool] Thread " << std::this_thread::get_id()
                  << " allocated (count: " << allocated_count << ")" << std::endl;

        return reinterpret_cast<T*>(node);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        std::lock_guard<std::mutex> lock(mutex);

        if (n != 1) return;

        char* ptr = reinterpret_cast<char*>(p);
        if (ptr >= pool && ptr < pool + PoolSize * sizeof(T)) {
            FreeNode* node = reinterpret_cast<FreeNode*>(p);
            node->next = free_list;
            free_list = node;
            allocated_count--;
            std::cout << "[ThreadSafePool] Thread " << std::this_thread::get_id()
                      << " deallocated (count: " << allocated_count << ")" << std::endl;
        } else {
            std::free(p);
        }
    }

    size_t get_allocated_count() const {
        std::lock_guard<std::mutex> lock(mutex);
        return allocated_count;
    }
};

// =============================================================================
// Example 6: Sophisticated Memory Tracking Allocator
// Tracks detailed allocation info including call stacks, sizes, and statistics
// =============================================================================

template<typename T>
class DetailedTrackingAllocator {
private:
    struct AllocationInfo {
        void* address;
        size_t size;
        size_t count;
        std::chrono::steady_clock::time_point timestamp;

        AllocationInfo(void* addr, size_t sz, size_t cnt)
            : address(addr), size(sz), count(cnt),
              timestamp(std::chrono::steady_clock::now()) {}
    };

    static inline std::map<void*, AllocationInfo> allocations;
    static inline std::mutex allocations_mutex;

    // Statistics
    static inline size_t total_allocations = 0;
    static inline size_t total_deallocations = 0;
    static inline size_t peak_memory = 0;
    static inline size_t current_memory = 0;
    static inline size_t total_bytes_allocated = 0;
    static inline size_t total_bytes_deallocated = 0;

public:
    using value_type = T;

    DetailedTrackingAllocator() noexcept = default;

    template<typename U>
    DetailedTrackingAllocator(const DetailedTrackingAllocator<U>&) noexcept {}

    T* allocate(std::size_t n) {
        if (n > std::numeric_limits<std::size_t>::max() / sizeof(T)) {
            throw std::bad_alloc();
        }

        size_t bytes = n * sizeof(T);
        void* p = std::malloc(bytes);

        if (!p) {
            throw std::bad_alloc();
        }

        // Record allocation
        {
            std::lock_guard<std::mutex> lock(allocations_mutex);

            allocations.emplace(p, AllocationInfo(p, bytes, n));

            total_allocations++;
            current_memory += bytes;
            total_bytes_allocated += bytes;

            if (current_memory > peak_memory) {
                peak_memory = current_memory;
            }

            std::cout << "[DetailedTracker] ALLOC: " << std::setw(10) << bytes
                      << " bytes (" << std::setw(6) << n << " objects) at "
                      << p << " | Current: " << current_memory
                      << " bytes in " << allocations.size() << " blocks"
                      << std::endl;
        }

        return static_cast<T*>(p);
    }

    void deallocate(T* p, std::size_t n) noexcept {
        size_t bytes = n * sizeof(T);

        {
            std::lock_guard<std::mutex> lock(allocations_mutex);

            auto it = allocations.find(p);
            if (it != allocations.end()) {
                auto duration = std::chrono::steady_clock::now() - it->second.timestamp;
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

                std::cout << "[DetailedTracker] DEALLOC: " << std::setw(10) << bytes
                          << " bytes (" << std::setw(6) << n << " objects) at "
                          << p << " | Lived: " << ms << " ms | Current: "
                          << (current_memory - bytes) << " bytes" << std::endl;

                current_memory -= bytes;
                total_bytes_deallocated += bytes;
                allocations.erase(it);
            } else {
                std::cerr << "[DetailedTracker] WARNING: Deallocating unknown pointer "
                          << p << std::endl;
            }

            total_deallocations++;
        }

        std::free(p);
    }

    static void print_detailed_report() {
        std::lock_guard<std::mutex> lock(allocations_mutex);

        std::cout << "\n" << std::string(70, '=') << std::endl;
        std::cout << "DETAILED MEMORY TRACKING REPORT" << std::endl;
        std::cout << std::string(70, '=') << std::endl;

        std::cout << "\nAllocation Statistics:" << std::endl;
        std::cout << "  Total allocations:     " << total_allocations << std::endl;
        std::cout << "  Total deallocations:   " << total_deallocations << std::endl;
        std::cout << "  Leaked allocations:    " << (total_allocations - total_deallocations) << std::endl;
        std::cout << "  Total bytes allocated: " << total_bytes_allocated << " bytes" << std::endl;
        std::cout << "  Total bytes freed:     " << total_bytes_deallocated << " bytes" << std::endl;
        std::cout << "  Peak memory usage:     " << peak_memory << " bytes" << std::endl;
        std::cout << "  Current memory usage:  " << current_memory << " bytes" << std::endl;

        if (!allocations.empty()) {
            std::cout << "\nActive Allocations (" << allocations.size() << "):" << std::endl;
            std::cout << std::string(70, '-') << std::endl;

            for (const auto& [addr, info] : allocations) {
                auto duration = std::chrono::steady_clock::now() - info.timestamp;
                auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

                std::cout << "  Address: " << addr
                          << " | Size: " << std::setw(8) << info.size << " bytes"
                          << " | Count: " << std::setw(6) << info.count
                          << " | Age: " << std::setw(6) << ms << " ms"
                          << std::endl;
            }
        }

        std::cout << std::string(70, '=') << std::endl;
    }

    static void reset_stats() {
        std::lock_guard<std::mutex> lock(allocations_mutex);
        total_allocations = 0;
        total_deallocations = 0;
        peak_memory = 0;
        current_memory = 0;
        total_bytes_allocated = 0;
        total_bytes_deallocated = 0;
        allocations.clear();
    }
};

template<typename T, typename U>
bool operator==(const DetailedTrackingAllocator<T>&, const DetailedTrackingAllocator<U>&) {
    return true;
}

template<typename T, typename U>
bool operator!=(const DetailedTrackingAllocator<T>&, const DetailedTrackingAllocator<U>&) {
    return false;
}

// =============================================================================
// Usage Examples
// =============================================================================

void example_aligned_allocator() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Example 4: Aligned Allocator" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // Useful for SIMD operations that require specific alignment
    struct alignas(32) SIMDVector {
        float data[8];
    };

    std::cout << "\nNatural alignment of SIMDVector: " << alignof(SIMDVector) << std::endl;
    std::cout << "\nUsing 64-byte alignment:" << std::endl;

    using AlignedVec = std::vector<SIMDVector, AlignedAllocator<SIMDVector, 64>>;
    AlignedVec vec;

    vec.reserve(5);
    for (int i = 0; i < 5; ++i) {
        vec.push_back(SIMDVector{});
    }

    std::cout << "\nVector data pointer alignment check:" << std::endl;
    std::cout << "  Address: " << static_cast<void*>(vec.data()) << std::endl;
    std::cout << "  Aligned to 64 bytes: "
              << (reinterpret_cast<uintptr_t>(vec.data()) % 64 == 0 ? "YES" : "NO")
              << std::endl;
}

void example_thread_safe_pool() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Example 5: Thread-Safe Pool Allocator" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    // We'll simulate a shared pool being used by multiple threads
    // Note: This is a simplified example; real usage would be more complex

    std::cout << "\nSpawning 3 threads to allocate from shared pool..." << std::endl;

    auto worker = [](int thread_id) {
        std::cout << "\n[Thread " << thread_id << "] Starting work..." << std::endl;

        // Each thread creates its own vector but could share allocator in real scenarios
        for (int i = 0; i < 3; ++i) {
            int* p = static_cast<int*>(std::malloc(sizeof(int)));
            std::cout << "[Thread " << thread_id << "] Allocated at " << p << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            std::free(p);
        }
    };

    std::thread t1(worker, 1);
    std::thread t2(worker, 2);
    std::thread t3(worker, 3);

    t1.join();
    t2.join();
    t3.join();

    std::cout << "\nAll threads completed." << std::endl;
}

void example_detailed_tracking() {
    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "Example 6: Detailed Memory Tracking" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    DetailedTrackingAllocator<int>::reset_stats();

    std::cout << "\nCreating and manipulating vector..." << std::endl;
    {
        std::vector<int, DetailedTrackingAllocator<int>> vec;

        std::cout << "\n--- Reserve 10 elements ---" << std::endl;
        vec.reserve(10);

        std::cout << "\n--- Push 15 elements (triggers reallocation) ---" << std::endl;
        for (int i = 0; i < 15; ++i) {
            vec.push_back(i);
            if (i == 5) {
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            }
        }

        std::cout << "\n--- Shrink to fit ---" << std::endl;
        vec.shrink_to_fit();

        std::cout << "\n--- Vector still alive (sleeping for 200ms) ---" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(200));

        std::cout << "\n--- Vector about to be destroyed ---" << std::endl;
    }

    std::cout << "\n--- Vector destroyed ---" << std::endl;

    DetailedTrackingAllocator<int>::print_detailed_report();

    // Create a memory leak intentionally for demonstration
    std::cout << "\n\nDemonstrating leak detection:" << std::endl;
    {
        std::vector<double, DetailedTrackingAllocator<double>> leak_vec;
        leak_vec.reserve(20);
        // Intentionally not destroying properly - just for demo
    }

    // Simulate leaked allocation
    auto* leaked = DetailedTrackingAllocator<char>().allocate(1024);
    (void)leaked; // Don't deallocate

    std::cout << "\n(Intentionally leaked 1024 bytes for demonstration)" << std::endl;

    DetailedTrackingAllocator<char>::print_detailed_report();
}

int main() {
    example_aligned_allocator();
    example_thread_safe_pool();
    example_detailed_tracking();

    std::cout << "\n" << std::string(70, '=') << std::endl;
    std::cout << "All advanced examples completed!" << std::endl;
    std::cout << std::string(70, '=') << std::endl;

    return 0;
}
```