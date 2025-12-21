/*
=============================================================================
                    PARALLEL STL IN C++ - COMPREHENSIVE GUIDE
=============================================================================

Introduction:
The Parallel STL (Standard Template Library) was introduced in C++17 as part 
of the parallelism technical specification. It extends many STL algorithms 
with execution policies that allow developers to request parallel and 
vectorized execution without writing explicit threading code.

Key Benefits:
- Easy parallelization of existing code
- Hardware-agnostic parallel execution
- Automatic load balancing
- Compatible with existing STL algorithms

Requirements:
- C++17 or later
- Compiler support (GCC 9+, Clang 9+, MSVC 2019+)
- Intel TBB library (for some implementations)
- Compile with: g++ -std=c++17 -ltbb program.cpp
=============================================================================
*/

#include <algorithm>
#include <execution>
#include <vector>
#include <numeric>
#include <iostream>
#include <chrono>
#include <random>

// ============================================================================
// 1. EXECUTION POLICIES
// ============================================================================

/*
C++17 defines four standard execution policies in <execution>:

1. std::execution::seq
   - Sequential execution (no parallelism)
   - Same as traditional STL algorithms

2. std::execution::par
   - Parallel execution
   - Operations may be distributed across multiple threads
   - No guaranteed vectorization

3. std::execution::par_unseq
   - Parallel and vectorized execution
   - Most aggressive optimization
   - Operations can be interleaved within threads

4. std::execution::unseq (C++20)
   - Vectorized execution only
   - Single-threaded but SIMD-optimized
*/

void demonstrateExecutionPolicies() {
    std::vector<int> data(1'000'000);
    std::iota(data.begin(), data.end(), 0);
    
    // Sequential (traditional)
    std::sort(data.begin(), data.end());
    
    // Parallel
    std::sort(std::execution::par, data.begin(), data.end());
    
    // Parallel + Vectorized
    std::sort(std::execution::par_unseq, data.begin(), data.end());
}

// ============================================================================
// 2. PARALLEL SORTING
// ============================================================================

void parallelSortingExample() {
    const size_t N = 10'000'000;
    std::vector<int> data(N);
    
    // Generate random data
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> dis(1, 1000);
    
    for (auto& val : data) {
        val = dis(gen);
    }
    
    // Sequential sort
    auto seq_data = data;
    auto start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::seq, seq_data.begin(), seq_data.end());
    auto end = std::chrono::high_resolution_clock::now();
    auto seq_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Parallel sort
    auto par_data = data;
    start = std::chrono::high_resolution_clock::now();
    std::sort(std::execution::par, par_data.begin(), par_data.end());
    end = std::chrono::high_resolution_clock::now();
    auto par_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sequential sort: " << seq_time.count() << "ms\n";
    std::cout << "Parallel sort: " << par_time.count() << "ms\n";
    std::cout << "Speedup: " << (double)seq_time.count() / par_time.count() << "x\n";
}

// ============================================================================
// 3. PARALLEL TRANSFORMATIONS
// ============================================================================

void parallelTransformExample() {
    std::vector<double> input(10'000'000);
    std::vector<double> output(input.size());
    
    // Fill with data
    std::iota(input.begin(), input.end(), 1.0);
    
    // Parallel transform: compute square roots
    std::transform(std::execution::par,
                   input.begin(), input.end(),
                   output.begin(),
                   [](double x) { return std::sqrt(x); });
    
    std::cout << "First 5 square roots:\n";
    for (size_t i = 0; i < 5; ++i) {
        std::cout << output[i] << " ";
    }
    std::cout << "\n";
}

// ============================================================================
// 4. PARALLEL SEARCH AND FIND
// ============================================================================

void parallelSearchExample() {
    std::vector<int> data(100'000'000);
    std::iota(data.begin(), data.end(), 0);
    
    int target = 75'000'000;
    
    // Parallel find
    auto start = std::chrono::high_resolution_clock::now();
    auto it = std::find(std::execution::par, data.begin(), data.end(), target);
    auto end = std::chrono::high_resolution_clock::now();
    
    if (it != data.end()) {
        std::cout << "Found " << *it << " at position " 
                  << std::distance(data.begin(), it) << "\n";
    }
    
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    std::cout << "Parallel search took: " << duration.count() << "μs\n";
    
    // Parallel find_if with predicate
    auto pred_it = std::find_if(std::execution::par,
                                 data.begin(), data.end(),
                                 [](int x) { return x > 90'000'000; });
    
    if (pred_it != data.end()) {
        std::cout << "First element > 90M: " << *pred_it << "\n";
    }
}

// ============================================================================
// 5. PARALLEL REDUCTIONS (ACCUMULATE, REDUCE)
// ============================================================================

void parallelReductionExample() {
    std::vector<long long> data(10'000'000);
    std::iota(data.begin(), data.end(), 1LL);
    
    // std::accumulate is NOT parallelizable (requires sequential order)
    auto seq_sum = std::accumulate(data.begin(), data.end(), 0LL);
    
    // std::reduce allows parallel execution (order-independent)
    auto par_sum = std::reduce(std::execution::par, 
                               data.begin(), data.end(), 
                               0LL);
    
    std::cout << "Sequential sum: " << seq_sum << "\n";
    std::cout << "Parallel sum: " << par_sum << "\n";
    
    // Parallel reduction with custom operation
    auto product = std::reduce(std::execution::par,
                               data.begin(), data.begin() + 20,
                               1LL,
                               std::multiplies<long long>());
    
    std::cout << "Product of first 20: " << product << "\n";
}

// ============================================================================
// 6. PARALLEL TRANSFORM_REDUCE
// ============================================================================

void parallelTransformReduceExample() {
    std::vector<double> vec1(1'000'000, 2.0);
    std::vector<double> vec2(1'000'000, 3.0);
    
    // Compute dot product in parallel
    double dot_product = std::transform_reduce(
        std::execution::par,
        vec1.begin(), vec1.end(),
        vec2.begin(),
        0.0,  // initial value
        std::plus<>(),      // reduction operation
        std::multiplies<>() // transformation operation
    );
    
    std::cout << "Dot product: " << dot_product << "\n";
    
    // Sum of squares
    double sum_squares = std::transform_reduce(
        std::execution::par,
        vec1.begin(), vec1.end(),
        0.0,
        std::plus<>(),
        [](double x) { return x * x; }
    );
    
    std::cout << "Sum of squares: " << sum_squares << "\n";
}

// ============================================================================
// 7. PARALLEL FOR_EACH
// ============================================================================

void parallelForEachExample() {
    std::vector<int> data(1'000'000);
    std::iota(data.begin(), data.end(), 0);
    
    // Parallel for_each: square each element
    std::for_each(std::execution::par,
                  data.begin(), data.end(),
                  [](int& x) { x = x * x; });
    
    std::cout << "First 5 squared values: ";
    for (size_t i = 0; i < 5; ++i) {
        std::cout << data[i] << " ";
    }
    std::cout << "\n";
    
    // IMPORTANT: for_each with side effects requires synchronization
    // This is UNSAFE without proper synchronization:
    // int counter = 0;
    // std::for_each(std::execution::par, data.begin(), data.end(),
    //               [&counter](int x) { ++counter; }); // DATA RACE!
}

// ============================================================================
// 8. PARALLEL COPY AND FILL
// ============================================================================

void parallelCopyFillExample() {
    std::vector<int> source(10'000'000);
    std::iota(source.begin(), source.end(), 0);
    
    std::vector<int> dest(source.size());
    
    // Parallel copy
    std::copy(std::execution::par, source.begin(), source.end(), dest.begin());
    
    // Parallel copy_if (conditional copy)
    std::vector<int> evens;
    evens.reserve(source.size() / 2);
    std::copy_if(std::execution::par,
                 source.begin(), source.end(),
                 std::back_inserter(evens),
                 [](int x) { return x % 2 == 0; });
    
    // Parallel fill
    std::fill(std::execution::par, dest.begin(), dest.end(), 42);
    
    std::cout << "Copied " << evens.size() << " even numbers\n";
}

// ============================================================================
// 9. IMPORTANT CONSIDERATIONS AND BEST PRACTICES
// ============================================================================

/*
THREAD SAFETY:
- Lambda functions must be thread-safe
- Avoid shared state without synchronization
- Use atomics or thread-local storage for counters

PERFORMANCE:
- Parallel algorithms have overhead
- Only beneficial for large datasets (typically > 10,000 elements)
- Algorithm complexity matters: O(n log n) benefits more than O(n)

ITERATORS:
- Random access iterators work best
- Forward iterators may limit parallelization

EXCEPTIONS:
- Exceptions in parallel regions are collected and rethrown
- Use std::execution::exception_list for multiple exceptions

MEMORY:
- Parallel algorithms may use more memory
- Consider cache locality and false sharing

COMPILER SUPPORT:
- Check compiler documentation
- May require linking against TBB: -ltbb
- Some implementations require explicit flags
*/

// Example of thread-safe parallel operation
void threadSafeParallelExample() {
    std::vector<int> data(1'000'000);
    std::iota(data.begin(), data.end(), 1);
    
    // Using atomic for thread-safe counting
    std::atomic<int> count{0};
    
    std::for_each(std::execution::par,
                  data.begin(), data.end(),
                  [&count](int x) {
                      if (x % 2 == 0) {
                          count.fetch_add(1, std::memory_order_relaxed);
                      }
                  });
    
    std::cout << "Count of even numbers: " << count << "\n";
}

// ============================================================================
// 10. COMPLETE EXAMPLE: IMAGE PROCESSING
// ============================================================================

struct Pixel {
    unsigned char r, g, b;
};

void parallelImageProcessing() {
    const size_t width = 1920;
    const size_t height = 1080;
    std::vector<Pixel> image(width * height);
    
    // Initialize with some pattern
    std::for_each(std::execution::par,
                  image.begin(), image.end(),
                  [](Pixel& p) {
                      p.r = 100;
                      p.g = 150;
                      p.b = 200;
                  });
    
    // Apply grayscale filter in parallel
    std::transform(std::execution::par,
                   image.begin(), image.end(),
                   image.begin(),
                   [](const Pixel& p) {
                       unsigned char gray = 
                           static_cast<unsigned char>(
                               0.299 * p.r + 0.587 * p.g + 0.114 * p.b
                           );
                       return Pixel{gray, gray, gray};
                   });
    
    std::cout << "Processed " << image.size() << " pixels in parallel\n";
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=== Parallel STL Examples ===\n\n";
    
    try {
        std::cout << "1. Parallel Sorting:\n";
        parallelSortingExample();
        
        std::cout << "\n2. Parallel Transform:\n";
        parallelTransformExample();
        
        std::cout << "\n3. Parallel Search:\n";
        parallelSearchExample();
        
        std::cout << "\n4. Parallel Reduction:\n";
        parallelReductionExample();
        
        std::cout << "\n5. Parallel Transform-Reduce:\n";
        parallelTransformReduceExample();
        
        std::cout << "\n6. Parallel For-Each:\n";
        parallelForEachExample();
        
        std::cout << "\n7. Parallel Copy/Fill:\n";
        parallelCopyFillExample();
        
        std::cout << "\n8. Thread-Safe Example:\n";
        threadSafeParallelExample();
        
        std::cout << "\n9. Image Processing:\n";
        parallelImageProcessing();
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << "\n";
        return 1;
    }
    
    return 0;
}

/*
COMPILATION INSTRUCTIONS:
--------------------------
GCC/Clang:
  g++ -std=c++17 -O3 -ltbb parallel_stl.cpp -o parallel_stl
  clang++ -std=c++17 -O3 -ltbb parallel_stl.cpp -o parallel_stl

MSVC:
  cl /std:c++17 /O2 /EHsc parallel_stl.cpp

Notes:
- Intel TBB may need to be installed separately
- On Linux: sudo apt-get install libtbb-dev
- On macOS: brew install tbb
- Performance varies by CPU core count and algorithm
*/