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