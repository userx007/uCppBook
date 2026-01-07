# Comprehensive guide to C++23 features with working examples

**Core Language Features:**
- **`std::print`/`std::println`**: Modern formatted output replacing iostream for common cases
- **`std::expected<T, E>`**: Railway-oriented error handling without exceptions
- **Multidimensional `operator[]`**: Natural syntax for matrices like `m[i, j]`
- **`if consteval`**: Conditional code for compile-time vs runtime contexts
- **Deducing `this`**: Explicit object parameters for better CRTP and forwarding

**Library Enhancements:**
- **`std::mdspan`**: Non-owning multidimensional array views
- **`std::stacktrace`**: Built-in stack trace capture for debugging
- **String improvements**: `contains()` method for easier substring checks
- **`std::to_underlying`**: Clean enum to integer conversion
- **`std::unreachable()`**: Optimization hint for impossible code paths

**Ranges & Views:**
- **`std::ranges::to<>`**: Direct conversion from ranges to containers
- **New views**: `zip`, `slide`, `chunk`, `enumerate`, `cartesian_product`

**Other Improvements:**
- **`constexpr` for `<cmath>`**: Compile-time math computations
- **Monadic operations** for `std::optional`: `and_then`, `transform`, `or_else`
- **Size literal suffix** `uz`/`z` for `size_t`
- **`#warning` directive**: Standard warning messages

**Containers & Data Structures:**
- **`std::flat_map` / `std::flat_set`**: Contiguous memory containers with better cache locality than tree-based containers

**Coroutines:**
- **`std::generator`**: Lazy sequence generation with `co_yield`, perfect for Fibonacci sequences and infinite streams

**Formatting:**
- **`std::format` improvements**: Direct range formatting, nested container support, escaped string formatting

**Range Views (the big batch!):**
- **`std::views::zip` / `zip_transform`**: Combine multiple ranges element-wise
- **`std::views::slide` / `adjacent`**: Sliding windows and adjacent element pairing
- **`std::views::chunk` / `chunk_by`**: Split ranges into fixed-size or predicate-based chunks
- **`std::views::enumerate`**: Python-style index + value iteration
- **`std::views::cartesian_product`**: Generate all combinations across multiple ranges

**Language Features:**
- **Relaxed `constexpr`**: Now supports `std::vector`, `std::string` at compile-time
- **`static operator()` and `operator[]`**: Zero-size function objects
- **`auto(x)` / `auto{x}`**: Explicit decay-copy for perfect forwarding scenarios
- **`#elifdef` / `#elifndef`**: Cleaner preprocessor conditionals



```cpp
// ============================================================================
// C++23 NEW FEATURES - COMPREHENSIVE GUIDE WITH EXAMPLES
// ============================================================================

#include <iostream>
#include <expected>
#include <print>
#include <stacktrace>
#include <optional>
#include <string>
#include <vector>
#include <ranges>
#include <span>
#include <mdspan>
#include <generator>

// ============================================================================
// 1. std::print and std::println - Modern Formatted Output
// ============================================================================
void demo_print() {
    std::println("=== std::print and std::println ===");
    
    // Basic printing with automatic newline
    std::println("Hello, C++23!");
    
    // Formatted output (like Python's f-strings)
    int x = 42;
    std::string name = "Alice";
    std::println("Name: {}, Value: {}", name, x);
    
    // Custom formatting
    double pi = 3.14159265359;
    std::println("Pi: {:.2f}", pi);  // 2 decimal places
    
    // std::print without newline
    std::print("Loading");
    std::print(".");
    std::print(".");
    std::println("Done!");
}

// ============================================================================
// 2. std::expected - Modern Error Handling
// ============================================================================
std::expected<int, std::string> divide(int a, int b) {
    if (b == 0) {
        return std::unexpected("Division by zero!");
    }
    return a / b;
}

void demo_expected() {
    std::println("\n=== std::expected ===");
    
    auto result1 = divide(10, 2);
    if (result1) {
        std::println("Result: {}", *result1);
    } else {
        std::println("Error: {}", result1.error());
    }
    
    auto result2 = divide(10, 0);
    if (result2) {
        std::println("Result: {}", *result2);
    } else {
        std::println("Error: {}", result2.error());
    }
    
    // Can use value_or for default values
    std::println("With default: {}", result2.value_or(-1));
    
    // Transform operations
    auto doubled = divide(20, 4).transform([](int val) { return val * 2; });
    std::println("Doubled: {}", doubled.value_or(0));
}

// ============================================================================
// 3. Multidimensional Subscript Operator
// ============================================================================
class Matrix {
    std::vector<std::vector<int>> data;
public:
    Matrix(size_t rows, size_t cols) : data(rows, std::vector<int>(cols, 0)) {}
    
    // C++23: Multiple arguments in operator[]
    int& operator[](size_t row, size_t col) {
        return data[row][col];
    }
    
    const int& operator[](size_t row, size_t col) const {
        return data[row][col];
    }
};

void demo_multidim_subscript() {
    std::println("\n=== Multidimensional Subscript ===");
    
    Matrix m(3, 3);
    
    // C++23: Can now use m[i, j] instead of m[i][j]
    m[0, 0] = 1;
    m[1, 1] = 5;
    m[2, 2] = 9;
    
    std::println("m[1, 1] = {}", m[1, 1]);
}

// ============================================================================
// 4. if consteval - Compile-time Conditional
// ============================================================================
constexpr int compute(int x) {
    if consteval {
        // This code runs only at compile-time
        return x * x;
    } else {
        // This code runs only at runtime
        return x * x * x;
    }
}

void demo_if_consteval() {
    std::println("\n=== if consteval ===");
    
    constexpr int compile_time = compute(5);  // Returns 25 (square)
    int runtime = compute(5);                  // Returns 125 (cube)
    
    std::println("Compile-time: {}", compile_time);
    std::println("Runtime: {}", runtime);
}

// ============================================================================
// 5. Deducing this (Explicit Object Parameters)
// ============================================================================
struct Counter {
    int value = 0;
    
    // C++23: Can deduce type of 'this' parameter
    void increment(this Counter& self) {
        self.value++;
    }
    
    // Works with perfect forwarding
    auto get_value(this auto&& self) {
        return std::forward<decltype(self)>(self).value;
    }
    
    // Can handle derived classes properly
    void print(this const auto& self) {
        std::println("Value: {}", self.value);
    }
};

void demo_deducing_this() {
    std::println("\n=== Deducing this ===");
    
    Counter c;
    c.increment();
    c.increment();
    c.print();
}

// ============================================================================
// 6. std::string::contains and string_view::contains
// ============================================================================
void demo_string_contains() {
    std::println("\n=== string::contains ===");
    
    std::string text = "Hello, C++23 World!";
    
    // Check if string contains substring
    if (text.contains("C++23")) {
        std::println("Text contains 'C++23'");
    }
    
    // Check for character
    if (text.contains('!')) {
        std::println("Text contains '!'");
    }
    
    // Works with string_view too
    std::string_view sv = "Programming";
    if (sv.contains("gram")) {
        std::println("Found 'gram' in '{}'", sv);
    }
}

// ============================================================================
// 7. std::to_underlying - Convert enum to underlying type
// ============================================================================
enum class Color : uint8_t {
    Red = 1,
    Green = 2,
    Blue = 3
};

void demo_to_underlying() {
    std::println("\n=== std::to_underlying ===");
    
    Color c = Color::Green;
    
    // C++23: Easy conversion to underlying type
    auto value = std::to_underlying(c);
    std::println("Color::Green as integer: {}", value);
    std::println("Type: uint8_t");
}

// ============================================================================
// 8. std::unreachable() - Optimization hint
// ============================================================================
int categorize(int value) {
    if (value < 0) return -1;
    if (value > 0) return 1;
    if (value == 0) return 0;
    
    // Tell compiler this point is unreachable
    std::unreachable();
}

void demo_unreachable() {
    std::println("\n=== std::unreachable ===");
    std::println("categorize(5) = {}", categorize(5));
    std::println("categorize(-3) = {}", categorize(-3));
}

// ============================================================================
// 9. std::mdspan - Multidimensional Array View
// ============================================================================
void demo_mdspan() {
    std::println("\n=== std::mdspan ===");
    
    // Create a flat array
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    // View it as a 3x3 matrix
    std::mdspan mat(data.data(), 3, 3);
    
    // Access elements with multiple indices
    std::println("mat[1, 1] = {}", mat[1, 1]);  // Prints 5
    
    // Modify through the view
    mat[0, 0] = 99;
    std::println("Modified mat[0, 0] = {}", data[0]);  // Prints 99
}

// ============================================================================
// 10. Range Enhancements - std::ranges::to
// ============================================================================
void demo_ranges_to() {
    std::println("\n=== std::ranges::to ===");
    
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // Convert range directly to container
    auto doubled = numbers 
        | std::views::transform([](int x) { return x * 2; })
        | std::ranges::to<std::vector>();
    
    std::print("Doubled: ");
    for (int n : doubled) {
        std::print("{} ", n);
    }
    std::println("");
}

// ============================================================================
// 11. constexpr improvements for <cmath> and <cstdlib>
// ============================================================================
constexpr double compute_at_compile_time() {
    // C++23: Many math functions are now constexpr
    return std::abs(-42.5) + std::sqrt(16.0);
}

void demo_constexpr_math() {
    std::println("\n=== constexpr math ===");
    constexpr double result = compute_at_compile_time();
    std::println("Computed at compile-time: {}", result);
}

// ============================================================================
// 12. std::stacktrace - Stack Trace Support
// ============================================================================
void function_c() {
    auto trace = std::stacktrace::current();
    std::println("\n=== std::stacktrace ===");
    std::println("Stack trace from function_c:");
    std::println("{}", std::to_string(trace));
}

void function_b() {
    function_c();
}

void function_a() {
    function_b();
}

// ============================================================================
// 13. Literal suffix for size_t
// ============================================================================
void demo_size_literal() {
    std::println("\n=== size_t literal (uz/z suffix) ===");
    
    auto size = 42uz;  // size_t literal
    std::println("Type of 42uz is size_t");
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    for (size_t i = 0uz; i < vec.size(); ++i) {
        std::print("{} ", vec[i]);
    }
    std::println("");
}

// ============================================================================
// 14. #warning directive
// ============================================================================
#warning "This is a C++23 warning directive example"

// ============================================================================
// 15. Monadic operations for std::optional
// ============================================================================
std::optional<int> get_value() {
    return 42;
}

void demo_optional_monadic() {
    std::println("\n=== Optional monadic operations ===");
    
    auto result = get_value()
        .and_then([](int x) -> std::optional<int> { return x * 2; })
        .transform([](int x) { return x + 10; })
        .or_else([]() -> std::optional<int> { return 0; });
    
    std::println("Result: {}", result.value_or(-1));
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================
int main() {
    std::println("========================================");
    std::println("   C++23 NEW FEATURES DEMONSTRATION    ");
    std::println("========================================");
    
    demo_print();
    demo_expected();
    demo_multidim_subscript();
    demo_if_consteval();
    demo_deducing_this();
    demo_string_contains();
    demo_to_underlying();
    demo_unreachable();
    demo_mdspan();
    demo_ranges_to();
    demo_constexpr_math();
    function_a();
    demo_size_literal();
    demo_optional_monadic();
    
    std::println("\n========================================");
    std::println("         DEMO COMPLETE!                 ");
    std::println("========================================");
    
    return 0;
}

// ============================================================================
// 16. std::flat_map and std::flat_set - Contiguous Memory Containers
// ============================================================================
void demo_flat_containers() {
    std::println("\n=== std::flat_map and std::flat_set ===");
    
    // flat_map uses contiguous memory (better cache locality than std::map)
    std::flat_map<std::string, int> scores;
    scores["Alice"] = 95;
    scores["Bob"] = 87;
    scores["Charlie"] = 92;
    
    std::println("Scores:");
    for (const auto& [name, score] : scores) {
        std::println("  {}: {}", name, score);
    }
    
    // flat_set also uses contiguous memory
    std::flat_set<int> unique_nums = {5, 2, 8, 2, 1, 8, 3};
    std::print("Unique numbers: ");
    for (int n : unique_nums) {
        std::print("{} ", n);
    }
    std::println("");
    
    // Better performance for small/medium datasets due to cache locality
    std::println("Benefits: Better cache locality, lower memory overhead");
}

// ============================================================================
// 17. std::generator - Coroutine-based Lazy Sequences
// ============================================================================
std::generator<int> fibonacci(int max) {
    int a = 0, b = 1;
    while (a <= max) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

std::generator<int> infinite_sequence(int start) {
    int value = start;
    while (true) {
        co_yield value++;
    }
}

void demo_generator() {
    std::println("\n=== std::generator ===");
    
    // Lazy evaluation - values generated on demand
    std::println("Fibonacci numbers up to 100:");
    for (int n : fibonacci(100)) {
        std::print("{} ", n);
    }
    std::println("");
    
    // Can work with infinite sequences
    std::println("First 10 numbers from infinite sequence:");
    int count = 0;
    for (int n : infinite_sequence(1)) {
        std::print("{} ", n);
        if (++count >= 10) break;
    }
    std::println("");
}

// ============================================================================
// 18. std::format Improvements
// ============================================================================
void demo_format_improvements() {
    std::println("\n=== std::format improvements ===");
    
    // Range formatting (new in C++23)
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::println("Vector: {}", numbers);
    
    // Nested containers
    std::vector<std::vector<int>> matrix = {{1, 2}, {3, 4}, {5, 6}};
    std::println("Matrix: {}", matrix);
    
    // Format specs for ranges
    std::println("Custom separator: {:n}", numbers);  // No brackets
    
    // Escaped string formatting
    std::string path = "C:\\Users\\Name";
    std::println("Path: {}", path);
    std::println("Escaped: {:?}", path);  // Shows escape sequences
    
    // Center, left, right alignment improvements
    std::println("Centered: |{:^20}|", "Hello");
    std::println("Left:     |{:<20}|", "Hello");
    std::println("Right:    |{:>20}|", "Hello");
}

// ============================================================================
// 19. std::views::zip and zip_transform
// ============================================================================
void demo_zip_views() {
    std::println("\n=== std::views::zip and zip_transform ===");
    
    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    std::vector<int> ages = {25, 30, 35};
    std::vector<std::string> cities = {"NYC", "LA", "Chicago"};
    
    // Zip multiple ranges together
    std::println("Using zip:");
    for (auto [name, age, city] : std::views::zip(names, ages, cities)) {
        std::println("  {} is {} years old and lives in {}", name, age, city);
    }
    
    // Zip with transformation
    std::println("\nUsing zip_transform:");
    auto formatted = std::views::zip_transform(
        [](const auto& name, int age) {
            return std::format("{} ({})", name, age);
        },
        names, ages
    );
    
    for (const auto& info : formatted) {
        std::println("  {}", info);
    }
}

// ============================================================================
// 20. std::views::slide and adjacent
// ============================================================================
void demo_slide_views() {
    std::println("\n=== std::views::slide and adjacent ===");
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    
    // Sliding window of size 3
    std::println("Sliding window (size 3):");
    for (auto window : data | std::views::slide(3)) {
        std::print("  [");
        for (int val : window) {
            std::print("{} ", val);
        }
        std::println("]");
    }
    
    // Adjacent pairs (equivalent to slide(2))
    std::println("\nAdjacent pairs:");
    for (auto [a, b] : data | std::views::adjacent<2>) {
        std::println("  ({}, {})", a, b);
    }
    
    // Adjacent triples
    std::println("\nAdjacent triples:");
    for (auto [a, b, c] : data | std::views::adjacent<3>) {
        std::println("  ({}, {}, {})", a, b, c);
    }
}

// ============================================================================
// 21. std::views::chunk and chunk_by
// ============================================================================
void demo_chunk_views() {
    std::println("\n=== std::views::chunk and chunk_by ===");
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Split into fixed-size chunks
    std::println("Chunks of 3:");
    for (auto chunk : numbers | std::views::chunk(3)) {
        std::print("  [");
        for (int n : chunk) {
            std::print("{} ", n);
        }
        std::println("]");
    }
    
    // Chunk by predicate (group consecutive equal elements)
    std::vector<int> data = {1, 1, 2, 2, 2, 3, 1, 1, 1};
    std::println("\nChunk by equality:");
    for (auto chunk : data | std::views::chunk_by(std::equal_to{})) {
        std::print("  [");
        for (int n : chunk) {
            std::print("{} ", n);
        }
        std::println("]");
    }
}

// ============================================================================
// 22. std::views::enumerate
// ============================================================================
void demo_enumerate() {
    std::println("\n=== std::views::enumerate ===");
    
    std::vector<std::string> fruits = {"apple", "banana", "cherry", "date"};
    
    // Get index and value together (Python-style enumerate)
    std::println("Fruits with indices:");
    for (auto [idx, fruit] : fruits | std::views::enumerate) {
        std::println("  {}: {}", idx, fruit);
    }
    
    // Works with any range
    std::println("\nEnumerate with transform:");
    auto upper_fruits = fruits 
        | std::views::transform([](auto s) { 
            for (auto& c : s) c = std::toupper(c); 
            return s; 
        });
    
    for (auto [idx, fruit] : upper_fruits | std::views::enumerate) {
        std::println("  {}: {}", idx, fruit);
    }
}

// ============================================================================
// 23. std::views::cartesian_product
// ============================================================================
void demo_cartesian_product() {
    std::println("\n=== std::views::cartesian_product ===");
    
    std::vector<int> numbers = {1, 2, 3};
    std::vector<char> letters = {'A', 'B'};
    std::vector<std::string> colors = {"Red", "Blue"};
    
    // Generate all combinations
    std::println("Cartesian product of numbers × letters:");
    for (auto [num, letter] : std::views::cartesian_product(numbers, letters)) {
        std::println("  ({}, {})", num, letter);
    }
    
    // Three-way cartesian product
    std::println("\nThree-way cartesian product:");
    int count = 0;
    for (auto [num, letter, color] : std::views::cartesian_product(numbers, letters, colors)) {
        std::println("  ({}, {}, {})", num, letter, color);
        if (++count >= 6) {
            std::println("  ... (showing first 6 of {} combinations)", 
                        numbers.size() * letters.size() * colors.size());
            break;
        }
    }
}

// ============================================================================
// 24. Relaxed constexpr Restrictions
// ============================================================================
constexpr std::vector<int> create_vector() {
    // C++23: Can now use std::vector in constexpr context
    std::vector<int> vec;
    vec.push_back(1);
    vec.push_back(2);
    vec.push_back(3);
    return vec;
}

constexpr int sum_vector() {
    auto vec = create_vector();
    int sum = 0;
    for (int n : vec) {
        sum += n;
    }
    return sum;
}

void demo_relaxed_constexpr() {
    std::println("\n=== Relaxed constexpr restrictions ===");
    
    // Vectors can now be used at compile time
    constexpr auto vec = create_vector();
    constexpr int sum = sum_vector();
    
    std::println("Vector created at compile-time, sum: {}", sum);
    std::println("First element: {}", vec[0]);
    
    // More standard library types are now constexpr-friendly
    constexpr std::string str = "Compile-time string!";
    std::println("Constexpr string: {}", str);
}

// ============================================================================
// 25. static operator() and operator[]
// ============================================================================
struct StaticOps {
    // Static call operator
    static int operator()(int x, int y) {
        return x + y;
    }
    
    // Static subscript operator
    static int operator[](int index) {
        return index * 10;
    }
};

void demo_static_operators() {
    std::println("\n=== static operator() and operator[] ===");
    
    // Can use without instance
    int result1 = StaticOps{}(5, 3);
    std::println("StaticOps()(5, 3) = {}", result1);
    
    int result2 = StaticOps{}[7];
    std::println("StaticOps[7] = {}", result2);
    
    // Useful for stateless function objects and policies
    std::println("Benefits: Zero-size objects, better optimization");
}

// ============================================================================
// 26. auto(x) and auto{x} - Decay Copy
// ============================================================================
void demo_decay_copy() {
    std::println("\n=== auto(x) and auto{x} decay-copy ===");
    
    const std::string& ref = "Hello";
    
    // auto(x) creates a prvalue (decay copy)
    auto decayed1 = auto(ref);  // Removes const and reference
    std::println("Original type: const string&");
    std::println("After auto(x): string (prvalue)");
    
    // auto{x} does the same with brace initialization
    auto decayed2 = auto{ref};
    
    // Useful for perfect forwarding and template code
    int arr[3] = {1, 2, 3};
    auto decayed_arr = auto(arr);  // Decays to pointer
    std::println("Array decayed to pointer");
    
    // Practical use: explicit move
    std::vector<int> vec = {1, 2, 3};
    auto moved = auto(std::move(vec));
    std::println("Vector moved, original size: {}", vec.size());
}

// ============================================================================
// 27. #elifdef and #elifndef Preprocessor Directives
// ============================================================================
#define FEATURE_A
// #define FEATURE_B
#define FEATURE_C

void demo_elifdef() {
    std::println("\n=== #elifdef and #elifndef ===");
    
#ifdef FEATURE_A
    std::println("Feature A is enabled");
#elifdef FEATURE_B
    std::println("Feature B is enabled");
#elifdef FEATURE_C
    std::println("Feature C is enabled (but FEATURE_A took precedence)");
#else
    std::println("No features enabled");
#endif

#ifndef FEATURE_B
    std::println("Feature B is NOT defined");
#elifndef FEATURE_C
    std::println("Feature C is NOT defined");
#else
    std::println("Both B and C are defined");
#endif
    
    std::println("Benefits: More readable than #elif defined()");
}

// ============================================================================
// ADDITIONAL C++23 FEATURES:
// ============================================================================
// - std::byteswap: Byte order reversal
// - std::start_lifetime_as: Explicit lifetime management
// - Attributes: [[assume]], [[indeterminate]]
// - Extended floating-point types (std::float16_t, std::bfloat16_t, etc.)
// - std::is_scoped_enum: Type trait for scoped enums
// - std::forward_like: Forward based on value category of another object
// ============================================================================
```