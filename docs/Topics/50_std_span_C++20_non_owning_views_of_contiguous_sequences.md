# std::span - A Comprehensive Guide

`std::span` is a C++20 feature that provides a lightweight, non-owning view over a contiguous sequence of objects. It's essentially a safer and more expressive alternative to passing raw pointers with size information.

## Key Characteristics

- **Non-owning**: Doesn't manage memory, just refers to existing data
- **Contiguous memory**: Works with arrays, vectors, and other contiguous containers
- **Bounds-aware**: Knows its size, preventing many common errors
- **Zero-overhead**: No runtime cost compared to pointer arithmetic
- **Can have static or dynamic extent**: Size can be known at compile-time or runtime

Let me provide comprehensive examples covering all major use cases:## Detailed Explanation

### What is std::span?

`std::span` is a template class that represents a view over a contiguous sequence of objects. Think of it as a "reference" to an array that knows its own size. It consists of just two pieces of information: a pointer to the first element and the number of elements.

### Why Use std::span?

**Problems it solves:**
1. **Pointer decay**: When arrays decay to pointers, size information is lost
2. **Interface bloat**: Need separate overloads for different container types
3. **Unsafe buffer access**: Easy to read/write beyond array bounds
4. **C-style API limitations**: `(pointer, size)` pairs are error-prone

### Key Use Cases in Detail

**1. Function Parameters**
Instead of multiple overloads for `vector<T>`, `array<T,N>`, `T*`, you write one function accepting `span<T>`. This provides a uniform interface without templates.

**2. Zero-copy Views**
Creating subviews (first N elements, last N elements, middle section) without copying data. Perfect for processing chunks of large datasets.

**3. API Boundaries**
When interfacing with C libraries or when you need to pass contiguous data without exposing implementation details of your container.

**4. Const Correctness**
`span<const T>` ensures read-only access while `span<T>` allows modification. The type system enforces this at compile time.

**5. Algorithm Compatibility**
Works seamlessly with STL algorithms via `begin()` and `end()` iterators.

### Static vs Dynamic Extent

```cpp
std::span<int, 5>    // Static: size known at compile time
std::span<int>       // Dynamic: size known at runtime (default)
```

Static extent enables compiler optimizations and compile-time size checks but is less flexible.

### Important Safety Notes

- **Lifetime management**: `std::span` doesn't own data. Ensure the underlying data outlives the span
- **No bounds checking**: `operator[]` doesn't check bounds (use manual checks)
- **Contiguous memory only**: Won't work with `std::list`, `std::deque`, etc.

### Common Pitfalls

1. **Dangling spans**: Returning span to local variable
2. **Temporary lifetime**: Creating span from temporary object
3. **Implicit conversions**: Be careful with narrowing conversions

The code artifact I've created shows 13 different use cases with complete, runnable examples. Each demonstrates best practices for real-world scenarios!

```cpp
#include <span>
#include <vector>
#include <array>
#include <iostream>
#include <algorithm>
#include <numeric>

// ============================================================================
// USE CASE 1: Replacing pointer + size pairs
// ============================================================================

// Old C-style approach (error-prone)
void processArrayOldWay(const int* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        std::cout << data[i] << " ";
    }
}

// Modern C++ with std::span (safer)
void processArrayWithSpan(std::span<const int> data) {
    for (int value : data) {  // Can use range-based for!
        std::cout << value << " ";
    }
}

// ============================================================================
// USE CASE 2: Accepting different container types uniformly
// ============================================================================

// Single function works with vector, array, C-array, etc.
double calculateAverage(std::span<const double> values) {
    if (values.empty()) return 0.0;
    return std::accumulate(values.begin(), values.end(), 0.0) / values.size();
}

// ============================================================================
// USE CASE 3: Mutable vs Read-only spans
// ============================================================================

// Mutable span - can modify elements
void fillWithValue(std::span<int> data, int value) {
    std::fill(data.begin(), data.end(), value);
}

// Read-only span - cannot modify elements
int sumValues(std::span<const int> data) {
    return std::accumulate(data.begin(), data.end(), 0);
}

// ============================================================================
// USE CASE 4: Static extent (compile-time size)
// ============================================================================

// Size known at compile time - enables optimizations
template<size_t N>
void processFixedArray(std::span<int, N> data) {
    static_assert(N > 0, "Array must not be empty");
    std::cout << "Processing array of exactly " << N << " elements\n";
    // Compiler knows size, can optimize better
}

// ============================================================================
// USE CASE 5: Subspans and slicing
// ============================================================================

void demonstrateSubspans(std::span<int> data) {
    // First 3 elements
    auto first3 = data.first(3);
    
    // Last 3 elements
    auto last3 = data.last(3);
    
    // Middle elements (from index 2, count 4)
    auto middle = data.subspan(2, 4);
    
    // Everything from index 2 onwards
    auto fromIndex2 = data.subspan(2);
    
    std::cout << "First 3: ";
    for (int v : first3) std::cout << v << " ";
    std::cout << "\n";
}

// ============================================================================
// USE CASE 6: Working with multi-dimensional data
// ============================================================================

// Treating 1D array as 2D matrix rows
void processMatrixRow(std::span<const double> row, size_t cols) {
    std::cout << "Row: ";
    for (size_t i = 0; i < cols; ++i) {
        std::cout << row[i] << " ";
    }
    std::cout << "\n";
}

void processMatrix(std::span<const double> matrix, size_t rows, size_t cols) {
    for (size_t r = 0; r < rows; ++r) {
        auto row = matrix.subspan(r * cols, cols);
        processMatrixRow(row, cols);
    }
}

// ============================================================================
// USE CASE 7: Interfacing with C APIs
// ============================================================================

// C API expects pointer and size
extern "C" void c_api_function(const char* data, size_t size);

void wrapperForCAPI(std::span<const char> data) {
    // Safely extract pointer and size for C API
    c_api_function(data.data(), data.size());
}

// ============================================================================
// USE CASE 8: Byte-level access with std::byte
// ============================================================================

void inspectBytes(std::span<const std::byte> bytes) {
    std::cout << "Byte values: ";
    for (auto byte : bytes) {
        std::cout << std::to_integer<int>(byte) << " ";
    }
    std::cout << "\n";
}

// ============================================================================
// USE CASE 9: Converting between spans (const correctness)
// ============================================================================

void demonstrateConversions() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // Mutable span
    std::span<int> mutableSpan(vec);
    
    // Can implicitly convert to const span
    std::span<const int> constSpan = mutableSpan;
    
    // Cannot convert back (compilation error if uncommented)
    // std::span<int> backToMutable = constSpan; // ERROR!
}

// ============================================================================
// USE CASE 10: Working with algorithms
// ============================================================================

void sortInPlace(std::span<int> data) {
    std::sort(data.begin(), data.end());
}

bool isSorted(std::span<const int> data) {
    return std::is_sorted(data.begin(), data.end());
}

auto findValue(std::span<const int> data, int value) {
    return std::find(data.begin(), data.end(), value);
}

// ============================================================================
// USE CASE 11: Range checking with bounds safety
// ============================================================================

void safeBoundsAccess(std::span<int> data) {
    // Using operator[] - no bounds checking (like raw pointer)
    // int value = data[100]; // Undefined behavior if out of bounds!
    
    // Using at() - throws exception if out of bounds (C++26 expected)
    // In current C++20, use manual checking:
    size_t index = 100;
    if (index < data.size()) {
        int value = data[index];
    }
}

// ============================================================================
// USE CASE 12: Dynamic extent (default)
// ============================================================================

// std::span<int> is same as std::span<int, std::dynamic_extent>
void processDynamicSpan(std::span<int> data) {
    std::cout << "Processing " << data.size() << " elements\n";
    // Size known at runtime only
}

// ============================================================================
// USE CASE 13: Efficient data partitioning
// ============================================================================

struct DataPartition {
    std::span<const int> training;
    std::span<const int> validation;
    std::span<const int> test;
};

DataPartition splitData(std::span<const int> data) {
    size_t trainSize = data.size() * 70 / 100;
    size_t valSize = data.size() * 15 / 100;
    
    return DataPartition{
        data.subspan(0, trainSize),
        data.subspan(trainSize, valSize),
        data.subspan(trainSize + valSize)
    };
}

// ============================================================================
// MAIN: Demonstrating all use cases
// ============================================================================

int main() {
    std::cout << "=== USE CASE 1: Replacing pointer + size ===\n";
    int arr[] = {1, 2, 3, 4, 5};
    processArrayWithSpan(arr);
    std::cout << "\n\n";
    
    std::cout << "=== USE CASE 2: Different container types ===\n";
    std::vector<double> vec = {1.5, 2.5, 3.5, 4.5};
    std::array<double, 4> arr2 = {5.5, 6.5, 7.5, 8.5};
    double cArray[] = {9.5, 10.5, 11.5};
    
    std::cout << "Vector average: " << calculateAverage(vec) << "\n";
    std::cout << "Array average: " << calculateAverage(arr2) << "\n";
    std::cout << "C-array average: " << calculateAverage(cArray) << "\n\n";
    
    std::cout << "=== USE CASE 3: Mutable vs Read-only ===\n";
    std::vector<int> nums = {1, 2, 3, 4, 5};
    std::cout << "Sum before: " << sumValues(nums) << "\n";
    fillWithValue(nums, 10);
    std::cout << "Sum after fill: " << sumValues(nums) << "\n\n";
    
    std::cout << "=== USE CASE 4: Static extent ===\n";
    std::array<int, 5> fixedArray = {1, 2, 3, 4, 5};
    processFixedArray(std::span<int, 5>(fixedArray));
    std::cout << "\n";
    
    std::cout << "=== USE CASE 5: Subspans ===\n";
    std::vector<int> data = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    demonstrateSubspans(data);
    std::cout << "\n";
    
    std::cout << "=== USE CASE 6: Multi-dimensional data ===\n";
    std::vector<double> matrix = {
        1.0, 2.0, 3.0,
        4.0, 5.0, 6.0,
        7.0, 8.0, 9.0
    };
    processMatrix(matrix, 3, 3);
    std::cout << "\n";
    
    std::cout << "=== USE CASE 8: Byte-level access ===\n";
    int value = 0x12345678;
    auto bytes = std::as_bytes(std::span(&value, 1));
    inspectBytes(bytes);
    std::cout << "\n";
    
    std::cout << "=== USE CASE 10: Algorithms ===\n";
    std::vector<int> unsorted = {5, 2, 8, 1, 9, 3};
    std::cout << "Is sorted before: " << isSorted(unsorted) << "\n";
    sortInPlace(unsorted);
    std::cout << "Is sorted after: " << isSorted(unsorted) << "\n";
    std::cout << "Sorted data: ";
    for (int v : unsorted) std::cout << v << " ";
    std::cout << "\n\n";
    
    std::cout << "=== USE CASE 13: Data partitioning ===\n";
    std::vector<int> dataset(100);
    std::iota(dataset.begin(), dataset.end(), 0);
    auto partitions = splitData(dataset);
    std::cout << "Training size: " << partitions.training.size() << "\n";
    std::cout << "Validation size: " << partitions.validation.size() << "\n";
    std::cout << "Test size: " << partitions.test.size() << "\n";
    
    return 0;
}

// ============================================================================
// ADDITIONAL ADVANCED PATTERNS
// ============================================================================

// Pattern 1: Template function accepting any span extent
template<typename T, size_t Extent>
void genericSpanFunction(std::span<T, Extent> data) {
    // Works with both dynamic and static extent
    std::cout << "Processing " << data.size() << " elements\n";
}

// Pattern 2: Returning spans (be careful with lifetime!)
std::span<int> getSpanFromVector(std::vector<int>& vec) {
    return vec;  // OK - vec outlives the span
    // DANGER: Don't return span to local variable!
}

// Pattern 3: Span of spans (jagged array)
void processJaggedArray(std::span<std::span<int>> rows) {
    for (auto row : rows) {
        for (int val : row) {
            std::cout << val << " ";
        }
        std::cout << "\n";
    }
}

// Pattern 4: Creating read-only view of mutable data
void createReadOnlyView() {
    std::vector<int> data = {1, 2, 3};
    std::span<const int> readOnly = data;
    // readOnly[0] = 5; // ERROR: cannot modify through const span
}
```