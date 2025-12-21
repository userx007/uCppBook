# Cache-Aware Programming in C++

Cache-aware programming is the practice of writing code that leverages CPU cache hierarchies to maximize performance. Understanding cache behavior can lead to **10-100x performance improvements** in many applications.

## Understanding CPU Cache

Modern CPUs have a hierarchy of caches:
- **L1 Cache**: ~32-64KB, 1-4 cycles, per-core
- **L2 Cache**: ~256KB-1MB, 10-20 cycles, per-core
- **L3 Cache**: ~8-64MB, 40-75 cycles, shared across cores
- **RAM**: GBs, 200-300 cycles

**Cache lines** are the fundamental unit (typically 64 bytes). When you access one byte, the entire cache line is loaded.

## Key Principles

### 1. **Spatial Locality** (Data Structure Layout)

Accessing memory locations close together in address space.### 2. **Temporal Locality** (Access Patterns)

Reusing recently accessed data while it's still in cache.### 3. **Cache Line Alignment and Padding**

Avoiding false sharing in multithreaded code:### 4. **Prefetching and Sequential Access**### 5. **Cache-Oblivious Algorithms and Practical Tips**## Key Cache-Aware Programming Strategies

### **Loop Optimization Techniques**

1. **Loop Interchange**: Reorder nested loops to access memory sequentially
2. **Loop Blocking/Tiling**: Process data in cache-sized blocks
3. **Loop Fusion**: Combine loops to improve data reuse
4. **Loop Unrolling**: Reduce loop overhead and improve instruction-level parallelism

### **Data Structure Choices**

| Structure | Cache Behavior | Use Case |
|-----------|---------------|----------|
| `std::vector` | Excellent (contiguous) | Default choice |
| `std::deque` | Good (chunked) | When resizing matters |
| `std::list` | Poor (pointer chasing) | Avoid unless insertion/deletion is critical |
| `std::map`/`set` | Poor (tree traversal) | Consider flat alternatives |
| `std::unordered_map` | Moderate | Good for lookups, poor for iteration |

### **Practical Guidelines**

**✅ DO:**
- Use contiguous containers (`std::vector`, `std::array`)
- Access data sequentially whenever possible
- Keep related data together (SoA for partial access patterns)
- Use cache line alignment for frequently accessed shared data
- Profile before optimizing (use tools like `perf`, `valgrind --tool=cachegrind`)
- Consider data access patterns during design phase

**❌ DON'T:**
- Use linked lists for large datasets
- Access multi-dimensional arrays in column-major order (for C/C++)
- Ignore alignment for parallel/SIMD code
- Allocate many small objects individually
- Mix hot and cold data in the same structure

### **Measurement Tools**

```cpp
// Linux: Use perf for cache statistics
// perf stat -e cache-references,cache-misses ./your_program

// Or valgrind:
// valgrind --tool=cachegrind ./your_program
// cg_annotate cachegrind.out.<pid>
```

### **Compiler Hints**

```cpp
// Prefetching
__builtin_prefetch(addr, rw, locality);

// Alignment
alignas(64) int data[16];

// Restrict keyword for aliasing
void process(float* __restrict__ a, float* __restrict__ b);

// Likely/unlikely branches
if (__builtin_expect(condition, 1)) { ... }
```

### **Real-World Impact**

Cache optimization can provide dramatic improvements:
- **Image processing**: 5-10x speedup with proper loop ordering
- **Matrix operations**: 10-50x speedup with blocking
- **Game engines**: 2-5x speedup with SoA and hot/cold separation
- **Database queries**: 3-8x speedup with cache-conscious data structures
- **Scientific computing**: 5-20x speedup with proper memory access patterns

The key is understanding your data access patterns and designing algorithms and data structures that work **with** the cache hierarchy rather than against it.

```cpp
#include <iostream>
#include <chrono>
#include <vector>

// Array of Structures (AoS) - Cache unfriendly for partial access
struct Particle_AoS {
    float x, y, z;      // position
    float vx, vy, vz;   // velocity
    float mass;
    float charge;
};

// Structure of Arrays (SoA) - Cache friendly
struct ParticleSystem_SoA {
    std::vector<float> x, y, z;
    std::vector<float> vx, vy, vz;
    std::vector<float> mass;
    std::vector<float> charge;
};

void updatePositions_AoS(std::vector<Particle_AoS>& particles, float dt) {
    for (auto& p : particles) {
        // Even though we only need position and velocity,
        // we load entire 32-byte struct (inefficient!)
        p.x += p.vx * dt;
        p.y += p.vy * dt;
        p.z += p.vz * dt;
    }
}

void updatePositions_SoA(ParticleSystem_SoA& particles, float dt, size_t count) {
    // Cache friendly: only touch the arrays we need
    // Better vectorization potential
    for (size_t i = 0; i < count; ++i) {
        particles.x[i] += particles.vx[i] * dt;
        particles.y[i] += particles.vy[i] * dt;
        particles.z[i] += particles.vz[i] * dt;
    }
}

int main() {
    const size_t N = 1'000'000;
    const int ITERATIONS = 100;
    
    // Setup AoS
    std::vector<Particle_AoS> particles_aos(N);
    for (size_t i = 0; i < N; ++i) {
        particles_aos[i] = {1.0f, 2.0f, 3.0f, 0.1f, 0.2f, 0.3f, 1.0f, 1.0f};
    }
    
    // Setup SoA
    ParticleSystem_SoA particles_soa;
    particles_soa.x.resize(N, 1.0f);
    particles_soa.y.resize(N, 2.0f);
    particles_soa.z.resize(N, 3.0f);
    particles_soa.vx.resize(N, 0.1f);
    particles_soa.vy.resize(N, 0.2f);
    particles_soa.vz.resize(N, 0.3f);
    
    // Benchmark AoS
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        updatePositions_AoS(particles_aos, 0.016f);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto aos_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Benchmark SoA
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < ITERATIONS; ++i) {
        updatePositions_SoA(particles_soa, 0.016f, N);
    }
    end = std::chrono::high_resolution_clock::now();
    auto soa_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "AoS time: " << aos_time << "ms\n";
    std::cout << "SoA time: " << soa_time << "ms\n";
    std::cout << "Speedup: " << static_cast<double>(aos_time) / soa_time << "x\n";
    
    return 0;
}
```


---
```cpp
#include <iostream>
#include <chrono>
#include <vector>

const size_t N = 512;

// Cache-unfriendly: poor temporal locality
void matmul_ijk(const std::vector<std::vector<float>>& A,
                const std::vector<std::vector<float>>& B,
                std::vector<std::vector<float>>& C) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t j = 0; j < N; ++j) {
            for (size_t k = 0; k < N; ++k) {
                // B[k][j] access pattern jumps around in memory (column-wise)
                // Poor cache reuse on B
                C[i][j] += A[i][k] * B[k][j];
            }
        }
    }
}

// Cache-friendly: better temporal locality
void matmul_ikj(const std::vector<std::vector<float>>& A,
                const std::vector<std::vector<float>>& B,
                std::vector<std::vector<float>>& C) {
    for (size_t i = 0; i < N; ++i) {
        for (size_t k = 0; k < N; ++k) {
            // Keep A[i][k] in register
            float aik = A[i][k];
            for (size_t j = 0; j < N; ++j) {
                // B[k][j] access is now row-wise (cache friendly)
                // C[i][j] is reused in inner loop (temporal locality)
                C[i][j] += aik * B[k][j];
            }
        }
    }
}

// Even better: cache blocking (tiling)
void matmul_blocked(const std::vector<std::vector<float>>& A,
                    const std::vector<std::vector<float>>& B,
                    std::vector<std::vector<float>>& C) {
    const size_t BLOCK_SIZE = 64;
    
    for (size_t ii = 0; ii < N; ii += BLOCK_SIZE) {
        for (size_t jj = 0; jj < N; jj += BLOCK_SIZE) {
            for (size_t kk = 0; kk < N; kk += BLOCK_SIZE) {
                // Work on blocks that fit in cache
                size_t i_end = std::min(ii + BLOCK_SIZE, N);
                size_t j_end = std::min(jj + BLOCK_SIZE, N);
                size_t k_end = std::min(kk + BLOCK_SIZE, N);
                
                for (size_t i = ii; i < i_end; ++i) {
                    for (size_t k = kk; k < k_end; ++k) {
                        float aik = A[i][k];
                        for (size_t j = jj; j < j_end; ++j) {
                            C[i][j] += aik * B[k][j];
                        }
                    }
                }
            }
        }
    }
}

int main() {
    // Initialize matrices
    std::vector<std::vector<float>> A(N, std::vector<float>(N, 1.0f));
    std::vector<std::vector<float>> B(N, std::vector<float>(N, 2.0f));
    std::vector<std::vector<float>> C1(N, std::vector<float>(N, 0.0f));
    std::vector<std::vector<float>> C2(N, std::vector<float>(N, 0.0f));
    std::vector<std::vector<float>> C3(N, std::vector<float>(N, 0.0f));
    
    // Benchmark IJK
    auto start = std::chrono::high_resolution_clock::now();
    matmul_ijk(A, B, C1);
    auto end = std::chrono::high_resolution_clock::now();
    auto ijk_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Benchmark IKJ
    start = std::chrono::high_resolution_clock::now();
    matmul_ikj(A, B, C2);
    end = std::chrono::high_resolution_clock::now();
    auto ikj_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Benchmark Blocked
    start = std::chrono::high_resolution_clock::now();
    matmul_blocked(A, B, C3);
    end = std::chrono::high_resolution_clock::now();
    auto blocked_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "IJK time: " << ijk_time << "ms\n";
    std::cout << "IKJ time: " << ikj_time << "ms (speedup: " 
              << static_cast<double>(ijk_time) / ikj_time << "x)\n";
    std::cout << "Blocked time: " << blocked_time << "ms (speedup: " 
              << static_cast<double>(ijk_time) / blocked_time << "x)\n";
    
    return 0;
}
```

---

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <atomic>

// BAD: False sharing - counters share cache lines
struct CountersBad {
    std::atomic<long> counter1{0};
    std::atomic<long> counter2{0};
    std::atomic<long> counter3{0};
    std::atomic<long> counter4{0};
};

// GOOD: Cache line padding prevents false sharing
struct CountersGood {
    alignas(64) std::atomic<long> counter1{0};
    char pad1[64 - sizeof(std::atomic<long>)];
    
    alignas(64) std::atomic<long> counter2{0};
    char pad2[64 - sizeof(std::atomic<long>)];
    
    alignas(64) std::atomic<long> counter3{0};
    char pad3[64 - sizeof(std::atomic<long>)];
    
    alignas(64) std::atomic<long> counter4{0};
    char pad4[64 - sizeof(std::atomic<long>)];
};

// Modern C++17 way
struct CountersModern {
    alignas(std::hardware_destructive_interference_size) std::atomic<long> counter1{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<long> counter2{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<long> counter3{0};
    alignas(std::hardware_destructive_interference_size) std::atomic<long> counter4{0};
};

void incrementCounter(std::atomic<long>& counter, int iterations) {
    for (int i = 0; i < iterations; ++i) {
        counter.fetch_add(1, std::memory_order_relaxed);
    }
}

template<typename T>
void benchmarkCounters(T& counters, const std::string& name) {
    const int ITERATIONS = 10'000'000;
    
    auto start = std::chrono::high_resolution_clock::now();
    
    std::thread t1(incrementCounter, std::ref(counters.counter1), ITERATIONS);
    std::thread t2(incrementCounter, std::ref(counters.counter2), ITERATIONS);
    std::thread t3(incrementCounter, std::ref(counters.counter3), ITERATIONS);
    std::thread t4(incrementCounter, std::ref(counters.counter4), ITERATIONS);
    
    t1.join();
    t2.join();
    t3.join();
    t4.join();
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << name << " time: " << duration << "ms\n";
    std::cout << "  Counter values: " << counters.counter1 << ", " 
              << counters.counter2 << ", " << counters.counter3 << ", " 
              << counters.counter4 << "\n";
}

int main() {
    std::cout << "Cache line size: " << std::hardware_constructive_interference_size << " bytes\n\n";
    
    CountersBad bad;
    CountersGood good;
    
    std::cout << "Size of CountersBad: " << sizeof(CountersBad) << " bytes\n";
    std::cout << "Size of CountersGood: " << sizeof(CountersGood) << " bytes\n\n";
    
    benchmarkCounters(bad, "Without padding (false sharing)");
    std::cout << "\n";
    benchmarkCounters(good, "With padding (no false sharing)");
    
    return 0;
}
```

---

```cpp
#include <iostream>
#include <vector>
#include <chrono>
#include <random>
#include <algorithm>

// BAD: Random access pattern - poor cache utilization
long sumRandomAccess(const std::vector<int>& data, const std::vector<size_t>& indices) {
    long sum = 0;
    for (size_t idx : indices) {
        // Random jumps through memory - cache misses
        sum += data[idx];
    }
    return sum;
}

// GOOD: Sequential access - excellent cache utilization
long sumSequentialAccess(const std::vector<int>& data) {
    long sum = 0;
    // Hardware prefetcher can predict and load ahead
    for (int val : data) {
        sum += val;
    }
    return sum;
}

// BETTER: Sorted indices for better cache behavior
long sumSortedAccess(const std::vector<int>& data, std::vector<size_t> indices) {
    // Sort indices to access data more sequentially
    std::sort(indices.begin(), indices.end());
    
    long sum = 0;
    for (size_t idx : indices) {
        sum += data[idx];
    }
    return sum;
}

// Manual prefetching (compiler-specific)
long sumWithPrefetch(const std::vector<int>& data, const std::vector<size_t>& indices) {
    long sum = 0;
    const size_t prefetch_distance = 8;
    
    for (size_t i = 0; i < indices.size(); ++i) {
        // Prefetch ahead
        if (i + prefetch_distance < indices.size()) {
            __builtin_prefetch(&data[indices[i + prefetch_distance]], 0, 1);
        }
        sum += data[indices[i]];
    }
    return sum;
}

// Linked list example - inherently cache-unfriendly
struct Node {
    int value;
    Node* next;
};

long sumLinkedList(Node* head) {
    long sum = 0;
    // Terrible cache behavior - each access is a cache miss
    while (head) {
        sum += head->value;
        head = head->next;
    }
    return sum;
}

// Array-based alternative - cache-friendly
long sumArray(const std::vector<int>& data) {
    long sum = 0;
    for (int val : data) {
        sum += val;
    }
    return sum;
}

int main() {
    const size_t N = 10'000'000;
    
    // Setup data
    std::vector<int> data(N);
    for (size_t i = 0; i < N; ++i) {
        data[i] = i % 100;
    }
    
    // Create random indices
    std::vector<size_t> indices(N);
    for (size_t i = 0; i < N; ++i) {
        indices[i] = i;
    }
    std::random_device rd;
    std::mt19937 gen(rd());
    std::shuffle(indices.begin(), indices.end(), gen);
    
    // Benchmark sequential access
    auto start = std::chrono::high_resolution_clock::now();
    long sum1 = sumSequentialAccess(data);
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Benchmark random access
    start = std::chrono::high_resolution_clock::now();
    long sum2 = sumRandomAccess(data, indices);
    end = std::chrono::high_resolution_clock::now();
    auto rand_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Benchmark sorted access
    start = std::chrono::high_resolution_clock::now();
    long sum3 = sumSortedAccess(data, indices);
    end = std::chrono::high_resolution_clock::now();
    auto sorted_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    // Benchmark with prefetch
    start = std::chrono::high_resolution_clock::now();
    long sum4 = sumWithPrefetch(data, indices);
    end = std::chrono::high_resolution_clock::now();
    auto prefetch_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();
    
    std::cout << "Sequential access: " << seq_time << "ms (sum: " << sum1 << ")\n";
    std::cout << "Random access: " << rand_time << "ms (sum: " << sum2 << ")\n";
    std::cout << "Sorted indices: " << sorted_time << "ms (sum: " << sum3 << ")\n";
    std::cout << "With prefetch: " << prefetch_time << "ms (sum: " << sum4 << ")\n";
    std::cout << "\nRandom vs Sequential slowdown: " 
              << static_cast<double>(rand_time) / seq_time << "x\n";
    
    return 0;
}
```

---

```cpp
#include <iostream>
#include <vector>
#include <memory>
#include <cstring>

// ============================================================================
// 1. Data Alignment for SIMD and Cache Efficiency
// ============================================================================

struct alignas(64) CacheLineAlignedData {
    float data[16];  // Exactly one cache line
};

void* aligned_alloc_wrapper(size_t size, size_t alignment) {
    void* ptr = nullptr;
    #ifdef _WIN32
        ptr = _aligned_malloc(size, alignment);
    #else
        posix_memalign(&ptr, alignment, size);
    #endif
    return ptr;
}

// ============================================================================
// 2. Loop Transformations for Cache Efficiency
// ============================================================================

// Bad: Column-major access on row-major array
void processImageBad(std::vector<std::vector<int>>& image, int rows, int cols) {
    for (int x = 0; x < cols; ++x) {
        for (int y = 0; y < rows; ++y) {
            image[y][x] *= 2;  // Column-wise access - cache miss on each access
        }
    }
}

// Good: Row-major access
void processImageGood(std::vector<std::vector<int>>& image, int rows, int cols) {
    for (int y = 0; y < rows; ++y) {
        for (int x = 0; x < cols; ++x) {
            image[y][x] *= 2;  // Row-wise access - sequential, cache friendly
        }
    }
}

// ============================================================================
// 3. Data Structure Design for Cache Efficiency
// ============================================================================

// Bad: Pointer chasing, scattered allocations
struct TreeNodeBad {
    int value;
    TreeNodeBad* left;
    TreeNodeBad* right;
    // Each node is separate allocation - terrible cache behavior
};

// Good: Array-based tree (implicit heap)
class TreeGood {
    std::vector<int> data;  // All data contiguous
    
public:
    TreeGood(size_t size) : data(size) {}
    
    int& operator[](size_t i) { return data[i]; }
    
    size_t left_child(size_t i) { return 2 * i + 1; }
    size_t right_child(size_t i) { return 2 * i + 2; }
    size_t parent(size_t i) { return (i - 1) / 2; }
};

// ============================================================================
// 4. Memory Pool for Cache-Friendly Allocations
// ============================================================================

template<typename T, size_t ChunkSize = 1024>
class CacheFriendlyPool {
    struct Chunk {
        T data[ChunkSize];
        Chunk* next = nullptr;
    };
    
    Chunk* head = nullptr;
    size_t current_index = ChunkSize;
    
public:
    ~CacheFriendlyPool() {
        while (head) {
            Chunk* next = head->next;
            delete head;
            head = next;
        }
    }
    
    T* allocate() {
        if (current_index >= ChunkSize) {
            Chunk* new_chunk = new Chunk();
            new_chunk->next = head;
            head = new_chunk;
            current_index = 0;
        }
        return &head->data[current_index++];
    }
    
    // Objects are allocated contiguously within chunks
    // Much better cache behavior than individual allocations
};

// ============================================================================
// 5. Loop Unrolling for Better Cache Utilization
// ============================================================================

// Manual unrolling
void sumArrayUnrolled(const float* data, size_t n, float& result) {
    float sum1 = 0, sum2 = 0, sum3 = 0, sum4 = 0;
    size_t i = 0;
    
    // Process 4 elements at a time
    for (; i + 3 < n; i += 4) {
        sum1 += data[i];
        sum2 += data[i+1];
        sum3 += data[i+2];
        sum4 += data[i+3];
    }
    
    // Handle remainder
    for (; i < n; ++i) {
        sum1 += data[i];
    }
    
    result = sum1 + sum2 + sum3 + sum4;
}

// ============================================================================
// 6. Hot/Cold Data Separation
// ============================================================================

struct EntityBad {
    int id;                    // Hot - accessed frequently
    float x, y, z;            // Hot
    char name[64];            // Cold - rarely accessed
    char description[256];    // Cold
    float velocity_x, velocity_y, velocity_z;  // Hot
};

// Better: Separate hot and cold data
struct EntityHotData {
    int id;
    float x, y, z;
    float velocity_x, velocity_y, velocity_z;
    int cold_data_index;  // Index into cold data array
};

struct EntityColdData {
    char name[64];
    char description[256];
};

// ============================================================================
// 7. Demonstrating Cache Effects
// ============================================================================

void demonstrateCacheEffects() {
    const size_t SIZE = 64 * 1024 * 1024;  // 64MB array
    std::vector<int> data(SIZE);
    
    std::cout << "Cache-aware programming effects:\n\n";
    
    // Sequential access - hardware prefetcher helps
    std::cout << "1. Sequential access patterns are ~10-20x faster than random\n";
    std::cout << "2. Structure of Arrays (SoA) can be 2-4x faster than Array of Structures (AoS)\n";
    std::cout << "3. Cache line padding eliminates false sharing (2-10x speedup in multithreaded code)\n";
    std::cout << "4. Loop blocking/tiling can provide 2-5x speedup for large matrices\n";
    std::cout << "5. Proper alignment improves SIMD operations and reduces cache misses\n\n";
    
    std::cout << "Key principles:\n";
    std::cout << "- Access data sequentially when possible\n";
    std::cout << "- Keep frequently accessed data together\n";
    std::cout << "- Avoid pointer chasing (linked lists, trees with pointers)\n";
    std::cout << "- Use cache blocking for large data structures\n";
    std::cout << "- Pad data structures to avoid false sharing\n";
    std::cout << "- Align data to cache line boundaries when appropriate\n";
    std::cout << "- Prefetch data when access patterns are predictable\n";
}

int main() {
    demonstrateCacheEffects();
    
    // Example: Using memory pool
    CacheFriendlyPool<TreeNodeBad> pool;
    
    // Allocations are contiguous within chunks
    for (int i = 0; i < 100; ++i) {
        auto* node = pool.allocate();
        node->value = i;
    }
    
    std::cout << "\nMemory pool allocates objects contiguously,\n";
    std::cout << "improving cache locality compared to individual allocations.\n";
    
    return 0;
}
```

