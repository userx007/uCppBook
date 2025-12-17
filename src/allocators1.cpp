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