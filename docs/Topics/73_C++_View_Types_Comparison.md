# C++ View Types Comparison

C++ offers several "view" types that provide non-owning references to data. Here's a comprehensive comparison:

## `std::string_view` (C++17)

`std::string_view` is a lightweight, non-owning reference to a sequence of characters. It provides a read-only view into string data without copying it.

**Key characteristics:**
- Specifically designed for character sequences (strings)
- Contains a pointer and a length
- Provides string-like operations (substr, find, compare, etc.)
- Works with `const char*`, `std::string`, and string literals
- Read-only - cannot modify the underlying data

**Common use cases:** Function parameters that accept string data without caring about the source type, avoiding unnecessary string copies.

```cpp
void process(std::string_view sv) {
    // Works with std::string, const char*, or string literals
}
```

## `std::span` (C++20)

`std::span` is a non-owning view over a contiguous sequence of objects. It's the generalized version of `string_view` for any type.

**Key characteristics:**
- Works with any contiguous data type (arrays, vectors, C-arrays)
- Contains a pointer and a size
- Can be mutable (`std::span<T>`) or const (`std::span<const T>`)
- Supports subscripting and iteration
- Fixed-size (`std::span<int, 5>`) or dynamic (`std::span<int>`)

**Common use cases:** Function parameters accepting array-like data, working with subranges of containers.

```cpp
void process(std::span<int> data) {
    for (auto& val : data) {
        val *= 2; // Can modify if not const
    }
}
```

## Ranges Views (C++20)

The Ranges library introduces a powerful system of composable views through `std::ranges::view` concept.

**Key types include:**

**`std::ranges::ref_view`** - Creates a view from a lvalue range

**`std::ranges::filter_view`** - Filters elements based on a predicate

**`std::ranges::transform_view`** - Transforms elements using a function

**`std::ranges::take_view`** - Takes first N elements

**`std::ranges::drop_view`** - Skips first N elements

**`std::ranges::reverse_view`** - Reverses iteration order

These views are lazy, composable, and don't own their data:

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6};
auto result = vec | std::views::filter([](int n) { return n % 2 == 0; })
                  | std::views::transform([](int n) { return n * n; });
// Result: 4, 16, 36 (computed lazily)
```

## Key Comparisons

**Ownership:** None of these types own their data. They're all non-owning views that must not outlive the data they reference.

**Mutability:** `std::string_view` is always read-only. `std::span` can be mutable or const. Ranges views depend on the underlying range.

**Specialization vs Generalization:** `std::string_view` is specialized for strings with string-specific operations. `std::span` is generalized for any contiguous sequence. Ranges views are the most flexible, working with any range (not just contiguous).

**When to use what:**
- Use `std::string_view` for string parameters and string manipulation
- Use `std::span` for array-like data of any type, especially when you need contiguous memory guarantees
- Use ranges views when you need composable transformations, lazy evaluation, or working with non-contiguous ranges

**Danger zone:** All views are susceptible to dangling references. Never return a view to local data:

```cpp
// DANGEROUS!
std::string_view bad() {
    std::string local = "temporary";
    return local; // Returns view to destroyed string
}
```

The common thread is that all these types provide efficient, zero-copy access to data without taking ownership, but they require careful lifetime management to avoid dangling references.

---

# C++ Views: Comprehensive Usage Examples

## **std::string_view Examples**

1. **Function parameters** - Accepting any string type without copying
2. **Efficient parsing** - Splitting strings without allocations (the tokens are views into the original string)
3. **Compile-time operations** - Using `constexpr` with string_view
4. **Substring operations** - Creating views into portions of strings

## **std::span Examples**

1. **Generic array processing** - Works with vectors, arrays, and C-arrays
2. **Read-only spans** - Using `std::span<const T>` for immutable access
3. **Fixed-size spans** - Compile-time size checking with `std::span<T, N>`
4. **Subspan operations** - Creating views into portions of arrays (sliding windows)

## **Ranges Views Examples**

1. **Filter & Transform** - Chain operations lazily
2. **Take/Drop/Reverse** - Manipulate range boundaries and direction
3. **Complex composition** - Combine multiple view operations
4. **Enumerate pattern** - Get index and value together using `zip` and `iota`
5. **Join (flatten)** - Flatten nested ranges
6. **Split** - Split ranges by delimiter
7. **Chunk** - Group elements into fixed-size chunks
8. **Practical pipeline** - Real-world data filtering and processing

## Key Takeaways from Examples

**Lazy evaluation**: Ranges views don't compute until you iterate. The pipeline `filter | transform | take` only processes elements as needed.

**Composability**: You can chain views with the pipe operator (`|`) for readable data transformations.

**Zero-copy**: All views reference existing data rather than copying it, making them very efficient.

**Type safety**: `std::span<T, N>` provides compile-time size checking.

The code is ready to compile with C++20 (use `-std=c++20` flag). Try modifying the examples to experiment with different view combinations!

```cpp
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <vector>
#include <array>
#include <ranges>
#include <algorithm>
#include <numeric>

// ============================================================================
// std::string_view Examples
// ============================================================================

// Example 1: Function parameters - avoid unnecessary copies
void printPrefix(std::string_view text, size_t len) {
    std::cout << "First " << len << " chars: " 
              << text.substr(0, len) << "\n";
}

// Example 2: Parsing without allocations
std::vector<std::string_view> split(std::string_view text, char delim) {
    std::vector<std::string_view> result;
    size_t start = 0;
    
    while (start < text.size()) {
        size_t end = text.find(delim, start);
        if (end == std::string_view::npos) {
            result.push_back(text.substr(start));
            break;
        }
        result.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    return result;
}

// Example 3: String literal optimization
constexpr bool startsWith(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() && 
           text.substr(0, prefix.size()) == prefix;
}

void stringViewExamples() {
    std::cout << "=== std::string_view Examples ===\n\n";
    
    // Works with different string types
    std::string str = "Hello, World!";
    const char* cstr = "C-style string";
    
    printPrefix(str, 5);           // std::string
    printPrefix(cstr, 7);          // const char*
    printPrefix("literal", 3);     // string literal
    
    // Efficient parsing
    std::string csv = "apple,banana,cherry,date";
    auto tokens = split(csv, ',');
    std::cout << "\nTokens: ";
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << "\n";
    
    // Compile-time operations
    constexpr std::string_view msg = "https://example.com";
    static_assert(startsWith(msg, "https://"));
    
    // Substring without allocation
    std::string_view sv = str;
    std::string_view sub = sv.substr(7, 5);  // "World"
    std::cout << "Substring: " << sub << "\n\n";
}

// ============================================================================
// std::span Examples
// ============================================================================

// Example 1: Function accepting array-like data
void processData(std::span<int> data) {
    std::cout << "Processing " << data.size() << " elements: ";
    for (auto& val : data) {
        val *= 2;  // Can modify elements
        std::cout << val << " ";
    }
    std::cout << "\n";
}

// Example 2: Read-only span
double average(std::span<const double> values) {
    if (values.empty()) return 0.0;
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

// Example 3: Fixed-size span for known dimensions
void process3DPoint(std::span<float, 3> point) {
    std::cout << "Point: (" << point[0] << ", " 
              << point[1] << ", " << point[2] << ")\n";
}

// Example 4: Subspan operations
void analyzeWindow(std::span<const int> data, size_t windowSize) {
    for (size_t i = 0; i + windowSize <= data.size(); ++i) {
        auto window = data.subspan(i, windowSize);
        int sum = std::accumulate(window.begin(), window.end(), 0);
        std::cout << "Window " << i << " sum: " << sum << "\n";
    }
}

void spanExamples() {
    std::cout << "=== std::span Examples ===\n\n";
    
    // Works with various container types
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::array<int, 4> arr = {10, 20, 30, 40};
    int carray[] = {100, 200, 300};
    
    processData(vec);
    processData(arr);
    processData(carray);
    
    // Read-only span
    std::vector<double> scores = {85.5, 92.0, 78.5, 88.0};
    std::cout << "Average: " << average(scores) << "\n";
    
    // Fixed-size span
    float coords[] = {1.0f, 2.0f, 3.0f};
    process3DPoint(coords);
    
    // Subspan operations
    std::vector<int> series = {1, 2, 3, 4, 5, 6, 7, 8};
    std::cout << "Moving window analysis:\n";
    analyzeWindow(series, 3);
    
    std::cout << "\n";
}

// ============================================================================
// Ranges Views Examples
// ============================================================================

void rangesFilterTransform() {
    std::cout << "=== Filter & Transform ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Filter even numbers, square them
    auto result = numbers 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; });
    
    std::cout << "Even squares: ";
    for (int n : result) {
        std::cout << n << " ";
    }
    std::cout << "\n\n";
}

void rangesTakeDropReverse() {
    std::cout << "=== Take, Drop, Reverse ===\n";
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Take first 5 elements
    auto first5 = data | std::views::take(5);
    std::cout << "First 5: ";
    for (int n : first5) std::cout << n << " ";
    std::cout << "\n";
    
    // Drop first 3 elements
    auto skip3 = data | std::views::drop(3);
    std::cout << "After dropping 3: ";
    for (int n : skip3) std::cout << n << " ";
    std::cout << "\n";
    
    // Reverse view
    auto reversed = data | std::views::reverse;
    std::cout << "Reversed: ";
    for (int n : reversed) std::cout << n << " ";
    std::cout << "\n\n";
}

void rangesComposition() {
    std::cout << "=== Complex Composition ===\n";
    
    std::vector<std::string> words = {
        "hello", "world", "cpp", "programming", "ranges", "views"
    };
    
    // Get first 4 words with length > 3, convert to uppercase
    auto result = words
        | std::views::filter([](const auto& s) { return s.length() > 3; })
        | std::views::take(4)
        | std::views::transform([](const auto& s) {
            std::string upper = s;
            std::ranges::transform(upper, upper.begin(), ::toupper);
            return upper;
        });
    
    std::cout << "Filtered & transformed: ";
    for (const auto& word : result) {
        std::cout << word << " ";
    }
    std::cout << "\n\n";
}

void rangesEnumerate() {
    std::cout << "=== Enumerate Pattern ===\n";
    
    std::vector<std::string> items = {"apple", "banana", "cherry"};
    
    // Enumerate using zip with iota
    auto indexed = std::views::zip(
        std::views::iota(0),
        items
    );
    
    for (const auto& [idx, item] : indexed) {
        std::cout << idx << ": " << item << "\n";
    }
    std::cout << "\n";
}

void rangesJoin() {
    std::cout << "=== Join (Flatten) ===\n";
    
    std::vector<std::vector<int>> nested = {
        {1, 2, 3},
        {4, 5},
        {6, 7, 8, 9}
    };
    
    // Flatten nested vectors
    auto flattened = nested | std::views::join;
    std::cout << "Flattened: ";
    for (int n : flattened) {
        std::cout << n << " ";
    }
    std::cout << "\n\n";
}

void rangesSplit() {
    std::cout << "=== Split View ===\n";
    
    std::string text = "one,two,three,four";
    
    // Split by comma
    auto parts = text | std::views::split(',');
    
    std::cout << "Parts: ";
    for (const auto& part : parts) {
        std::cout << "[";
        for (char c : part) {
            std::cout << c;
        }
        std::cout << "] ";
    }
    std::cout << "\n\n";
}

void rangesChunking() {
    std::cout << "=== Chunk View ===\n";
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    // Process in chunks of 3
    auto chunks = data | std::views::chunk(3);
    
    int chunkNum = 1;
    for (const auto& chunk : chunks) {
        std::cout << "Chunk " << chunkNum++ << ": ";
        for (int n : chunk) {
            std::cout << n << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void rangesPracticalExample() {
    std::cout << "=== Practical Example: Data Pipeline ===\n";
    
    // Simulate sensor readings
    std::vector<int> readings = {
        15, 102, 23, 105, 18, 200, 22, 98, 19, 150, 25, 101
    };
    
    // Find average of valid readings (20-100 range), excluding outliers
    auto validReadings = readings
        | std::views::filter([](int r) { return r >= 20 && r <= 100; });
    
    int count = std::ranges::distance(validReadings);
    int sum = std::accumulate(validReadings.begin(), validReadings.end(), 0);
    
    std::cout << "Valid readings: ";
    for (int r : validReadings) {
        std::cout << r << " ";
    }
    std::cout << "\n";
    std::cout << "Count: " << count << ", Average: " 
              << (count > 0 ? sum / count : 0) << "\n\n";
}

// ============================================================================
// Main
// ============================================================================

int main() {
    stringViewExamples();
    spanExamples();
    
    std::cout << "=== Ranges Views Examples ===\n\n";
    rangesFilterTransform();
    rangesTakeDropReverse();
    rangesComposition();
    rangesEnumerate();
    rangesJoin();
    rangesSplit();
    rangesChunking();
    rangesPracticalExample();
    
    return 0;
}
```