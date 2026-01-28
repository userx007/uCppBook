# Comprehensive guide to C++23 features with working examples

## Major Features

[**Core Language Features:**](#core-language-features)
- **`std::print`/`std::println`**: Modern formatted output replacing iostream for common cases
- **`std::expected<T, E>`**: Railway-oriented error handling without exceptions
- **Multidimensional `operator[]`**: Natural syntax for matrices like `m[i, j]`
- **`if consteval`**: Conditional code for compile-time vs runtime contexts
- **Deducing `this`**: Explicit object parameters for better CRTP and forwarding

[**Library Enhancements:**](#library-enhancements)
- **`std::mdspan`**: Non-owning multidimensional array views
- **`std::stacktrace`**: Built-in stack trace capture for debugging
- **String improvements**: `contains()` method for easier substring checks
- **`std::to_underlying`**: Clean enum to integer conversion
- **`std::unreachable()`**: Optimization hint for impossible code paths

[**Ranges & Views:**](#ranges-and-views)
- **`std::ranges::to<>`**: Direct conversion from ranges to containers
- **New views**: `zip`, `slide`, `chunk`, `enumerate`, `cartesian_product`

[**Other Improvements:**](#other-improvements)
- **`constexpr` for `<cmath>`**: Compile-time math computations
- **Monadic operations** for `std::optional`: `and_then`, `transform`, `or_else`
- **Size literal suffix** `uz`/`z` for `size_t`
- **`#warning` directive**: Standard warning messages

[**Containers & Data Structures:**](#containers-and-data-structures)
- **`std::flat_map` / `std::flat_set`**: Contiguous memory containers with better cache locality than tree-based containers

[**Coroutines:**](#coroutines)
- **`std::generator`**: Lazy sequence generation with `co_yield`, perfect for Fibonacci sequences and infinite streams

[**Formatting:**](#formatting)
- **`std::format` improvements**: Direct range formatting, nested container support, escaped string formatting

[**Range Views (the big batch!):**](#range-views)
- **`std::views::zip` / `zip_transform`**: Combine multiple ranges element-wise
- **`std::views::slide` / `adjacent`**: Sliding windows and adjacent element pairing
- **`std::views::chunk` / `chunk_by`**: Split ranges into fixed-size or predicate-based chunks
- **`std::views::enumerate`**: Python-style index + value iteration
- **`std::views::cartesian_product`**: Generate all combinations across multiple ranges

[**Language Features:**](#language-features)
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

---

## Core Language Features
[Back to top](#major-features)

### 1. `std::print` and `std::println` - Modern Formatted Output

C++23 introduces `std::print` and `std::println` in `<print>` as modern alternatives to iostream, offering Python-like formatting with better performance and type safety.

#### Key Features
- Format string syntax based on `std::format` (similar to Python's f-strings)
- Direct output without stream manipulators
- Compile-time format string checking
- Better performance than iostream
- Automatic newline handling with `println`

#### Examples

```cpp
#include <print>
#include <string>
#include <vector>

int main() {
    // Basic usage - replaces std::cout
    std::println("Hello, C++23!");
    
    // Formatted output with arguments
    int age = 25;
    std::string name = "Alice";
    std::println("Name: {}, Age: {}", name, age);
    
    // No newline version
    std::print("Loading");
    std::print(".");
    std::print(".");
    std::println("Done!");
    
    // Format specifications
    double pi = 3.14159265359;
    std::println("Pi to 2 decimals: {:.2f}", pi);  // 3.14
    std::println("Pi to 6 decimals: {:.6f}", pi);  // 3.141593
    
    // Alignment and width
    std::println("{:>10}", "right");      // Right-aligned in 10 chars
    std::println("{:<10}", "left");       // Left-aligned
    std::println("{:^10}", "center");     // Center-aligned
    
    // Numbers with different bases
    int num = 42;
    std::println("Decimal: {}, Hex: {:x}, Binary: {:b}", num, num, num);
    
    // Multiple arguments
    std::println("x={}, y={}, z={}", 10, 20, 30);
    
    // Containers (with std::format support)
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::println("Vector: {}", vec);  // Requires range formatting support
    
    return 0;
}
```

**Benefits over iostream:**
```cpp
// Old way (iostream)
std::cout << "Name: " << name << ", Age: " << age << std::endl;

// New way (more readable, faster)
std::println("Name: {}, Age: {}", name, age);
```

### 2. `std::expected<T, E>` - Railway-Oriented Error Handling

`std::expected<T, E>` from `<expected>` represents a value that can either contain a successful result (`T`) or an error (`E`), enabling functional error handling without exceptions.

#### Key Concepts
- Returns either a value or an error (like Rust's `Result<T, E>`)
- Explicit error handling in return types
- No performance overhead of exceptions
- Composable error handling (railway-oriented programming)

#### Examples

```cpp
#include <expected>
#include <string>
#include <cmath>

enum class MathError {
    DivisionByZero,
    NegativeSquareRoot,
    Overflow
};

// Function returning expected value or error
std::expected<double, MathError> safe_divide(double a, double b) {
    if (b == 0.0) {
        return std::unexpected(MathError::DivisionByZero);
    }
    return a / b;
}

std::expected<double, MathError> safe_sqrt(double x) {
    if (x < 0.0) {
        return std::unexpected(MathError::NegativeSquareRoot);
    }
    return std::sqrt(x);
}

// Chaining operations
std::expected<double, MathError> calculate(double a, double b, double c) {
    auto div_result = safe_divide(a, b);
    if (!div_result) {
        return div_result;  // Propagate error
    }
    
    return safe_sqrt(div_result.value() + c);
}

// Using and_then for monadic composition
std::expected<double, MathError> calculate_monadic(double a, double b, double c) {
    return safe_divide(a, b)
        .and_then([c](double result) { return safe_sqrt(result + c); });
}

int main() {
    // Success case
    auto result1 = safe_divide(10.0, 2.0);
    if (result1) {
        std::println("Result: {}", result1.value());  // or *result1
    }
    
    // Error case
    auto result2 = safe_divide(10.0, 0.0);
    if (!result2) {
        std::println("Error occurred!");
        // Access the error
        MathError err = result2.error();
        if (err == MathError::DivisionByZero) {
            std::println("Division by zero!");
        }
    }
    
    // Using value_or for default values
    double val = safe_divide(10.0, 0.0).value_or(0.0);
    std::println("Value or default: {}", val);
    
    // transform and transform_error
    auto result3 = safe_divide(10.0, 2.0)
        .transform([](double x) { return x * 2; })  // Transforms value if success
        .value_or(-1.0);
    
    // or_else for error recovery
    auto result4 = safe_divide(10.0, 0.0)
        .or_else([](MathError) { 
            return std::expected<double, MathError>(0.0);  // Recover from error
        });
    
    return 0;
}
```

**Practical Example: File I/O**
```cpp
enum class FileError {
    NotFound,
    PermissionDenied,
    InvalidFormat
};

std::expected<std::string, FileError> read_config(const std::string& path) {
    // ... file reading logic
    if (/* file not found */) {
        return std::unexpected(FileError::NotFound);
    }
    return "config_content";
}

// Usage
auto config = read_config("config.txt");
if (config) {
    std::println("Config: {}", *config);
} else {
    std::println("Failed to read config");
}
```

### 3. Multidimensional `operator[]` - Natural Matrix Syntax

C++23 allows `operator[]` to accept multiple arguments, enabling natural multidimensional indexing syntax.

#### Before vs After

```cpp
// C++20 and earlier - awkward
matrix(i, j)        // Using operator()
matrix[i][j]        // Chained single indices (error-prone)

// C++23 - natural and clean
matrix[i, j]        // Direct multidimensional indexing
```

#### Examples

```cpp
#include <vector>
#include <print>

template<typename T>
class Matrix {
    std::vector<T> data;
    size_t rows, cols;
    
public:
    Matrix(size_t r, size_t c) : rows(r), cols(c), data(r * c) {}
    
    // C++23: Multiple parameters in operator[]
    T& operator[](size_t i, size_t j) {
        return data[i * cols + j];
    }
    
    const T& operator[](size_t i, size_t j) const {
        return data[i * cols + j];
    }
    
    // Can also support bounds checking version
    T& at(size_t i, size_t j) {
        if (i >= rows || j >= cols) {
            throw std::out_of_range("Matrix indices out of bounds");
        }
        return (*this)[i, j];
    }
};

int main() {
    Matrix<int> m(3, 4);
    
    // Natural indexing syntax
    m[0, 0] = 1;
    m[0, 1] = 2;
    m[1, 2] = 42;
    
    std::println("Value at [1,2]: {}", m[1, 2]);
    
    // Works with more dimensions too
    class Tensor3D {
        std::vector<double> data;
        size_t d1, d2, d3;
        
    public:
        Tensor3D(size_t x, size_t y, size_t z) 
            : d1(x), d2(y), d3(z), data(x * y * z) {}
        
        double& operator[](size_t i, size_t j, size_t k) {
            return data[i * d2 * d3 + j * d3 + k];
        }
    };
    
    Tensor3D tensor(10, 10, 10);
    tensor[3, 5, 7] = 3.14;
    
    return 0;
}
```

**Comparison with Old Approaches:**
```cpp
// Approach 1: operator() - not as intuitive for indexing
m(i, j) = value;

// Approach 2: Chained operator[] - returns intermediate proxy objects
// m[i][j] requires complex proxy classes

// C++23: Direct and natural
m[i, j] = value;
```

### 4. `if consteval` - Compile-Time Context Detection

`if consteval` allows you to write different code paths for compile-time evaluation versus runtime execution, without using `std::is_constant_evaluated()`.

#### Key Features
- Cleaner syntax than `std::is_constant_evaluated()`
- Distinguishes between constant evaluation and runtime
- Useful for optimization and debugging
- No need for functions, works inline

#### Examples

```cpp
#include <print>
#include <array>

// Example 1: Different implementations for compile-time vs runtime
constexpr int fibonacci(int n) {
    if consteval {
        // Compile-time: Use simple recursion (will be optimized)
        if (n <= 1) return n;
        return fibonacci(n - 1) + fibonacci(n - 2);
    } else {
        // Runtime: Use iterative approach (more efficient)
        if (n <= 1) return n;
        int a = 0, b = 1;
        for (int i = 2; i <= n; ++i) {
            int temp = a + b;
            a = b;
            b = temp;
        }
        return b;
    }
}

// Example 2: Logging during compile-time debugging
constexpr int complex_calculation(int x) {
    if consteval {
        // This code only runs during constant evaluation
        // Can't use std::println here, but concept is clear
    } else {
        std::println("Runtime calculation with x={}", x);
    }
    
    return x * x + 2 * x + 1;
}

// Example 3: Bounds checking
template<typename T, size_t N>
constexpr T safe_get(const std::array<T, N>& arr, size_t index) {
    if consteval {
        // At compile-time, use at() which will cause compile error if out of bounds
        return arr.at(index);
    } else {
        // At runtime, provide custom error handling
        if (index >= N) {
            std::println("Warning: Index {} out of bounds, clamping to {}", 
                        index, N - 1);
            return arr[N - 1];
        }
        return arr[index];
    }
}

// Example 4: Different allocation strategies
constexpr auto create_buffer(size_t size) {
    if consteval {
        // Compile-time: use std::array
        return std::array<int, 100>{};  // Fixed size
    } else {
        // Runtime: could use std::vector or dynamic allocation
        return std::vector<int>(size);
    }
}

int main() {
    // Compile-time evaluation
    constexpr int fib10 = fibonacci(10);  // Uses recursive version
    
    // Runtime evaluation
    int n = 10;
    int fib_runtime = fibonacci(n);  // Uses iterative version
    
    std::println("Compile-time fib(10): {}", fib10);
    std::println("Runtime fib(10): {}", fib_runtime);
    
    // Compile-time array access
    constexpr std::array<int, 5> arr = {1, 2, 3, 4, 5};
    constexpr int val = safe_get(arr, 2);  // Compile-time check
    
    // Runtime array access
    size_t idx = 10;
    int val2 = safe_get(arr, idx);  // Runtime warning and clamping
    
    return 0;
}
```

**Comparison with `std::is_constant_evaluated()`:**
```cpp
// Old way (C++20)
constexpr int old_style(int x) {
    if (std::is_constant_evaluated()) {
        // Compile-time path
        return x * 2;
    } else {
        // Runtime path
        return x + x;
    }
}

// New way (C++23) - clearer intent
constexpr int new_style(int x) {
    if consteval {
        // Compile-time path
        return x * 2;
    } else {
        // Runtime path
        return x + x;
    }
}

// Also supports negation
constexpr int check_runtime(int x) {
    if !consteval {
        // Only runs at runtime
        std::println("This is runtime!");
    }
    return x;
}
```



### 5. Deducing `this` - Explicit Object Parameters

Deducing `this` allows member functions to have an explicit object parameter, simplifying CRTP, forwarding, and code deduplication between const/non-const overloads.

#### Key Benefits
- Eliminates CRTP boilerplate
- Perfect forwarding in member functions
- Single implementation for const/non-const/rvalue overloads
- More efficient recursive lambdas

#### Examples

**Example 1: Deduplicating const/non-const overloads**
```cpp
#include <print>
#include <string>

struct OldStyle {
    std::string data;
    
    // Before: Need two nearly identical functions
    std::string& get() { 
        return data; 
    }
    
    const std::string& get() const { 
        return data; 
    }
};

struct NewStyle {
    std::string data;
    
    // C++23: Single function handles both cases
    template<typename Self>
    auto&& get(this Self&& self) {
        return std::forward<Self>(self).data;
    }
    
    // Works for: non-const, const, rvalue references
};

int main() {
    NewStyle obj{"hello"};
    const NewStyle const_obj{"world"};
    
    obj.get() = "modified";              // Returns std::string&
    std::println("{}", const_obj.get()); // Returns const std::string&
    std::println("{}", NewStyle{"temp"}.get()); // Returns std::string&&
}
```

**Example 2: Simplified CRTP (Curiously Recurring Template Pattern)**
```cpp
// Old CRTP - Complex boilerplate
template<typename Derived>
struct OldBase {
    void interface() {
        static_cast<Derived*>(this)->implementation();
    }
};

struct OldDerived : OldBase<OldDerived> {
    void implementation() {
        std::println("Old implementation");
    }
};

// New style - No CRTP needed!
struct NewBase {
    void interface(this auto&& self) {
        self.implementation();
    }
};

struct NewDerived : NewBase {
    void implementation() {
        std::println("New implementation");
    }
};

int main() {
    NewDerived d;
    d.interface();  // Calls NewDerived::implementation
}
```

**Example 3: Recursive Lambdas**
```cpp
#include <print>

int main() {
    // Old way: Complex with std::function or Y-combinator
    // New way: Direct recursion
    
    auto factorial = [](this auto&& self, int n) -> int {
        if (n <= 1) return 1;
        return n * self(n - 1);  // self refers to the lambda itself
    };
    
    std::println("5! = {}", factorial(5));  // 120
    
    // Fibonacci with deducing this
    auto fib = [](this auto&& self, int n) -> int {
        if (n <= 1) return n;
        return self(n - 1) + self(n - 2);
    };
    
    std::println("fib(10) = {}", fib(10));
}
```

**Example 4: Builder Pattern with Perfect Forwarding**
```cpp
#include <string>
#include <print>

class Builder {
    std::string name;
    int age = 0;
    
public:
    // Supports both lvalue and rvalue usage
    template<typename Self>
    Self&& set_name(this Self&& self, std::string n) {
        self.name = std::move(n);
        return std::forward<Self>(self);
    }
    
    template<typename Self>
    Self&& set_age(this Self&& self, int a) {
        self.age = a;
        return std::forward<Self>(self);
    }
    
    void print() const {
        std::println("Name: {}, Age: {}", name, age);
    }
};

int main() {
    Builder b;
    b.set_name("Alice").set_age(30).print();
    
    // Also works with temporaries
    Builder{}.set_name("Bob").set_age(25).print();
}
```

**Example 5: Avoiding Code Duplication**
```cpp
template<typename T>
class Container {
    std::vector<T> data;
    
public:
    // Single function works for all reference types
    template<typename Self>
    auto&& front(this Self&& self) {
        return std::forward<Self>(self).data.front();
    }
    
    template<typename Self>
    auto&& back(this Self&& self) {
        return std::forward<Self>(self).data.back();
    }
    
    // This one function replaces 4 overloads:
    // T& front()
    // const T& front() const
    // T&& front() &&
    // const T&& front() const&&
};
```

### Summary: Key Points to Remember

#### `std::print` / `std::println`
- Modern formatted output with Python-like syntax
- Better performance than iostream
- Use `std::println("x={}, y={}", x, y)` instead of `std::cout << "x=" << x ...`
- Compile-time format string checking

#### `std::expected<T, E>`
- Represents either a value (T) or error (E)
- Explicit error handling without exceptions
- Check with `if (result)` or `if (!result)`
- Access value with `.value()` or `*result`, error with `.error()`
- Composable with `.and_then()`, `.transform()`, `.or_else()`

#### Multidimensional `operator[]`
- Natural syntax: `matrix[i, j]` instead of `matrix(i, j)` or `matrix[i][j]`
- Works with any number of dimensions: `tensor[x, y, z]`
- Simpler implementation than proxy-based approaches

#### `if consteval`
- Cleaner than `std::is_constant_evaluated()`
- Different code paths for compile-time vs runtime
- Syntax: `if consteval { ... } else { ... }`
- Use for optimization, debugging, or different strategies

#### Deducing `this`
- Explicit object parameter: `void func(this Self&& self)`
- Eliminates const/non-const overload duplication
- Simplifies CRTP patterns
- Enables recursive lambdas: `[](this auto&& self, int n) { return self(n-1); }`
- Perfect forwarding in member functions

---

## Library Enhancements
[Back to top](#major-features)

### 1. `std::mdspan` - Multidimensional Array Views

`std::mdspan` provides a non-owning view over contiguous or strided multidimensional data, similar to how `std::span` works for one-dimensional arrays.

#### Key Features:
- Zero-overhead abstraction for multidimensional data
- Works with existing memory (doesn't allocate)
- Supports custom layouts (row-major, column-major, strided)
- Type-safe indexing

#### Examples:

```cpp
#include <mdspan>
#include <vector>
#include <iostream>

void example_basic_mdspan() {
    // Create underlying 1D storage
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12};
    
    // Create a 3x4 view over this data
    std::mdspan<int, std::extents<size_t, 3, 4>> matrix(data.data());
    
    // Access elements using multidimensional indexing
    matrix[1, 2] = 99;
    
    std::cout << "Element at [1,2]: " << matrix[1, 2] << "\n";
}

void example_dynamic_extents() {
    std::vector<double> data(20);
    
    // Dynamic dimensions determined at runtime
    size_t rows = 4, cols = 5;
    std::mdspan<double, std::dextents<size_t, 2>> matrix(data.data(), rows, cols);
    
    // Fill the matrix
    for (size_t i = 0; i < rows; ++i) {
        for (size_t j = 0; j < cols; ++j) {
            matrix[i, j] = i * cols + j;
        }
    }
}

void process_3d_tensor(std::mdspan<float, std::dextents<size_t, 3>> tensor) {
    // Function accepts any 3D mdspan regardless of underlying storage
    auto [depth, height, width] = tensor.extents();
    
    for (size_t d = 0; d < depth; ++d) {
        for (size_t h = 0; h < height; ++h) {
            for (size_t w = 0; w < width; ++w) {
                tensor[d, h, w] *= 2.0f;
            }
        }
    }
}
```

### 2. `std::stacktrace` - Built-in Stack Trace Capture

Previously, capturing stack traces required platform-specific code or third-party libraries. C++23 standardizes this functionality.

#### Examples:

```cpp
#include <stacktrace>
#include <iostream>
#include <string>

void print_current_stacktrace() {
    auto trace = std::stacktrace::current();
    std::cout << "Stack trace:\n" << trace << "\n";
}

void level3() {
    print_current_stacktrace();
}

void level2() {
    level3();
}

void level1() {
    level2();
}

class Exception : public std::exception {
    std::stacktrace trace_;
    std::string msg_;
    
public:
    Exception(std::string msg) 
        : trace_(std::stacktrace::current())
        , msg_(std::move(msg)) {}
    
    const char* what() const noexcept override {
        return msg_.c_str();
    }
    
    const std::stacktrace& where() const noexcept {
        return trace_;
    }
};

void risky_function() {
    throw Exception("Something went wrong!");
}

void debug_example() {
    try {
        risky_function();
    } catch (const Exception& e) {
        std::cout << "Error: " << e.what() << "\n";
        std::cout << "Stack trace at exception:\n" << e.where() << "\n";
    }
}

// Skip frames from the trace
void analyze_caller() {
    auto trace = std::stacktrace::current(1, 5); // Skip 1 frame, get next 5
    std::cout << "Caller context:\n" << trace << "\n";
}
```

### 3. String Improvements - `contains()` Method

A simple but highly requested feature: checking if a string contains a substring without using `find()`.

#### Examples:

```cpp
#include <string>
#include <string_view>
#include <iostream>

void old_way_vs_new_way() {
    std::string text = "Hello, C++23 World!";
    
    // Old way (C++20 and earlier)
    if (text.find("C++") != std::string::npos) {
        std::cout << "Contains C++ (old way)\n";
    }
    
    // New way (C++23)
    if (text.contains("C++")) {
        std::cout << "Contains C++ (new way)\n";
    }
}

void various_contains_overloads() {
    std::string text = "Programming in C++";
    
    // Check for substring
    bool has_cpp = text.contains("C++");
    
    // Check for single character
    bool has_space = text.contains(' ');
    
    // Check using string_view
    std::string_view pattern = "gram";
    bool has_pattern = text.contains(pattern);
    
    // Check using C-string
    bool has_in = text.contains("in");
    
    std::cout << std::boolalpha;
    std::cout << "Has C++: " << has_cpp << "\n";
    std::cout << "Has space: " << has_space << "\n";
    std::cout << "Has 'gram': " << has_pattern << "\n";
    std::cout << "Has 'in': " << has_in << "\n";
}

bool is_valid_email(const std::string& email) {
    // More readable than find() != npos
    return email.contains('@') && email.contains('.');
}

void validation_example() {
    std::string email1 = "user@example.com";
    std::string email2 = "invalid.email";
    
    std::cout << email1 << " is valid: " << is_valid_email(email1) << "\n";
    std::cout << email2 << " is valid: " << is_valid_email(email2) << "\n";
}
```

### 4. `std::to_underlying` - Clean Enum to Integer Conversion

Converts an enum value to its underlying integral type in a type-safe, readable way.

#### Examples:

```cpp
#include <utility>
#include <iostream>
#include <cstdint>

enum class Color : uint8_t {
    Red = 1,
    Green = 2,
    Blue = 4,
    Yellow = Red | Green  // Bitwise operations
};

void old_vs_new_conversion() {
    Color c = Color::Red;
    
    // Old way - verbose and error-prone
    auto old_val = static_cast<std::underlying_type_t<Color>>(c);
    
    // New way - clean and readable
    auto new_val = std::to_underlying(c);
    
    std::cout << "Value: " << new_val << "\n";
}

enum class StatusCode {
    Success = 0,
    NotFound = 404,
    ServerError = 500
};

int get_http_code(StatusCode status) {
    return std::to_underlying(status);
}

enum class Flags : unsigned {
    None = 0,
    Read = 1,
    Write = 2,
    Execute = 4
};

void bitwise_operations() {
    Flags perms = Flags::Read;
    
    // Easy conversion for bitwise operations
    auto mask = std::to_underlying(Flags::Read) | 
                std::to_underlying(Flags::Write);
    
    std::cout << "Combined mask: " << mask << "\n";
}

// Useful for array indexing
enum class Direction { North, East, South, West };

void direction_example() {
    const char* names[] = {"North", "East", "South", "West"};
    Direction dir = Direction::South;
    
    // Clean conversion for indexing
    std::cout << "Direction: " << names[std::to_underlying(dir)] << "\n";
}

// Working with APIs that expect integers
enum class Priority : int { Low = 1, Medium = 5, High = 10 };

void set_task_priority(int priority_value);

void api_integration() {
    Priority p = Priority::High;
    set_task_priority(std::to_underlying(p));
}

void set_task_priority(int priority_value) {
    std::cout << "Setting priority to: " << priority_value << "\n";
}
```

### 5. `std::unreachable()` - Optimization Hint

Indicates that a code path should never be executed, allowing the compiler to optimize accordingly. If reached in practice, the behavior is undefined.

#### Examples:

```cpp
#include <utility>
#include <iostream>
#include <cassert>

enum class Animal { Dog, Cat, Bird };

std::string_view get_sound(Animal a) {
    switch (a) {
        case Animal::Dog:  return "Woof";
        case Animal::Cat:  return "Meow";
        case Animal::Bird: return "Tweet";
    }
    
    // Tell compiler this is unreachable
    // Avoids warning about missing return statement
    std::unreachable();
}

// Before C++23, you might have used:
// return ""; // Default return (wasteful)
// throw std::logic_error("Unreachable"); // Runtime overhead
// __builtin_unreachable(); // Compiler-specific

int safe_divide(int a, int b) {
    if (b == 0) {
        throw std::invalid_argument("Division by zero");
    }
    
    // If we reach here, b is guaranteed non-zero
    return a / b;
}

void optimization_example() {
    // Compiler can optimize knowing this branch never happens
    bool condition = true; // Compile-time constant
    
    if (condition) {
        std::cout << "Always taken\n";
    } else {
        std::unreachable(); // Helps optimizer eliminate dead code
    }
}

enum class ErrorCode { None, Warning, Error, Critical };

[[noreturn]] void handle_critical_error();

void process_error(ErrorCode code) {
    switch (code) {
        case ErrorCode::None:
            return;
        case ErrorCode::Warning:
            std::cout << "Warning\n";
            return;
        case ErrorCode::Error:
            std::cout << "Error\n";
            return;
        case ErrorCode::Critical:
            handle_critical_error(); // [[noreturn]] function
            std::unreachable(); // Helps compiler understand control flow
    }
    std::unreachable();
}

[[noreturn]] void handle_critical_error() {
    std::cout << "Critical error - terminating\n";
    std::terminate();
}

// State machine example
enum class State { Init, Running, Stopped };

class StateMachine {
    State state_ = State::Init;
    
public:
    void transition_to_running() {
        if (state_ != State::Init) {
            std::unreachable(); // Called only from Init state
        }
        state_ = State::Running;
    }
};
```

### Summary: Quick Reference

#### `std::mdspan`
- Non-owning view over multidimensional data
- Zero-overhead abstraction, no allocations
- Use `std::extents` for compile-time dimensions, `std::dextents` for runtime
- Access with `mdspan[i, j, k]` syntax

#### `std::stacktrace`
- Capture call stacks: `std::stacktrace::current()`
- Great for debugging and exception context
- Can skip/limit frames: `current(skip, max_depth)`
- Print directly or store in exceptions

#### String `contains()`
- Replaces `str.find(x) != npos` pattern
- Overloads: substring, character, string_view, C-string
- More readable: `if (text.contains("pattern"))`
- Works with `std::string` and `std::string_view`

#### `std::to_underlying`
- Converts enum to underlying integral type
- Replaces `static_cast<underlying_type_t<E>>(value)`
- Useful for: array indexing, bitwise ops, API integration
- Type-safe and concise

#### `std::unreachable()`
- Marks impossible code paths
- Enables compiler optimizations
- **UB if actually reached** - use carefully
- Replaces compiler-specific `__builtin_unreachable()`
- Common uses: exhaustive switches, after `[[noreturn]]` calls

---

## Ranges And Views
[Back to top](#major-features)

### Overview

C++23 significantly enhances the ranges library introduced in C++20, making range-based programming more practical and ergonomic. The two major additions are `std::ranges::to<>` for materializing ranges into containers, and several powerful new view types that enable sophisticated data transformations.

### `std::ranges::to<>`: Materializing Ranges

Before C++23, converting a range back into a container required verbose code. The `std::ranges::to<>` function template solves this elegantly.

#### Basic Usage

```cpp
#include <ranges>
#include <vector>
#include <list>
#include <set>

std::vector<int> vec = {1, 2, 3, 4, 5};

// Convert to different container types
auto result_vec = std::ranges::to<std::vector>(
    vec | std::views::filter([](int n) { return n % 2 == 0; })
);
// result_vec is std::vector<int>{2, 4}

auto result_list = std::ranges::to<std::list>(
    vec | std::views::transform([](int n) { return n * 2; })
);
// result_list is std::list<int>{2, 4, 6, 8, 10}

auto result_set = std::ranges::to<std::set>(vec);
// result_set is std::set<int>{1, 2, 3, 4, 5}
```

#### Nested Conversions

`std::ranges::to<>` excels at handling nested ranges:

```cpp
#include <ranges>
#include <vector>
#include <string>

std::vector<std::string> words = {"hello", "world", "cpp23"};

// Convert nested ranges
auto nested = words 
    | std::views::transform([](const auto& s) { 
        return s | std::views::filter([](char c) { return c != 'l'; });
    })
    | std::ranges::to<std::vector<std::string>>();
// Result: {"heo", "word", "cpp23"}
```

#### Deducing Container Type

You can also let the compiler deduce the container type:

```cpp
std::vector<int> vec = {1, 2, 3};
auto result = vec | std::views::transform([](int n) { return n * 2; })
                  | std::ranges::to<std::vector>();
// Deduces std::vector<int>
```

### New View Types

#### `std::views::zip`

Combines multiple ranges element-wise into tuples. This is useful for iterating over parallel sequences.

```cpp
#include <ranges>
#include <vector>
#include <string>

std::vector<int> ids = {1, 2, 3};
std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
std::vector<double> scores = {95.5, 88.0, 92.3};

for (auto [id, name, score] : std::views::zip(ids, names, scores)) {
    std::cout << id << ": " << name << " - " << score << '\n';
}
// Output:
// 1: Alice - 95.5
// 2: Bob - 88.0
// 3: Charlie - 92.3

// Combine with other views
auto result = std::views::zip(ids, names)
    | std::views::filter([](auto tuple) { 
        return std::get<0>(tuple) > 1; 
    })
    | std::ranges::to<std::vector>();
```

The zip operation stops at the shortest range's length.

#### `std::views::slide`

Creates a sliding window of fixed size over a range. Each element is a view of N consecutive elements.

```cpp
#include <ranges>
#include <vector>

std::vector<int> data = {1, 2, 3, 4, 5};

// Sliding window of size 3
for (auto window : data | std::views::slide(3)) {
    std::cout << "[";
    for (auto elem : window) {
        std::cout << elem << " ";
    }
    std::cout << "]\n";
}
// Output:
// [1 2 3 ]
// [2 3 4 ]
// [3 4 5 ]

// Calculate moving averages
auto moving_avg = data 
    | std::views::slide(3)
    | std::views::transform([](auto window) {
        int sum = 0;
        for (auto n : window) sum += n;
        return sum / 3.0;
    })
    | std::ranges::to<std::vector>();
// moving_avg: {2.0, 3.0, 4.0}
```

#### `std::views::chunk`

Divides a range into non-overlapping chunks of a specified size. The last chunk may be smaller.

```cpp
#include <ranges>
#include <vector>

std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};

// Split into chunks of 3
for (auto chunk : data | std::views::chunk(3)) {
    std::cout << "[";
    for (auto elem : chunk) {
        std::cout << elem << " ";
    }
    std::cout << "]\n";
}
// Output:
// [1 2 3 ]
// [4 5 6 ]
// [7 8 9 ]

// Process batches
std::vector<int> numbers(100);
std::iota(numbers.begin(), numbers.end(), 1);

auto batch_sums = numbers
    | std::views::chunk(10)
    | std::views::transform([](auto chunk) {
        return std::ranges::fold_left(chunk, 0, std::plus{});
    })
    | std::ranges::to<std::vector>();
```

#### `std::views::enumerate`

Pairs each element with its index, similar to Python's `enumerate()`.

```cpp
#include <ranges>
#include <vector>
#include <string>

std::vector<std::string> items = {"apple", "banana", "cherry"};

for (auto [idx, item] : items | std::views::enumerate) {
    std::cout << idx << ": " << item << '\n';
}
// Output:
// 0: apple
// 1: banana
// 2: cherry

// Find indices of specific elements
auto indices = items 
    | std::views::enumerate
    | std::views::filter([](auto pair) {
        auto [idx, value] = pair;
        return value.starts_with('b');
    })
    | std::views::transform([](auto pair) { return std::get<0>(pair); })
    | std::ranges::to<std::vector>();
// indices: {1}
```

#### `std::views::cartesian_product`

Generates the cartesian product of multiple ranges, producing tuples of all possible combinations.

```cpp
#include <ranges>
#include <vector>

std::vector<int> nums = {1, 2};
std::vector<char> letters = {'a', 'b', 'c'};

for (auto [num, letter] : std::views::cartesian_product(nums, letters)) {
    std::cout << "(" << num << ", " << letter << ") ";
}
// Output: (1, a) (1, b) (1, c) (2, a) (2, b) (2, c)

// Generate test cases
std::vector<bool> flags = {true, false};
std::vector<int> values = {0, 1, 2};

auto test_cases = std::views::cartesian_product(flags, values, values)
    | std::views::filter([](auto tuple) {
        auto [flag, v1, v2] = tuple;
        return v1 <= v2;  // Only ascending pairs
    })
    | std::ranges::to<std::vector>();
```

### Combining Multiple Views

The power of these views comes from composing them:

```cpp
#include <ranges>
#include <vector>
#include <string>

std::vector<std::string> words = {"the", "quick", "brown", "fox", "jumps"};

// Complex pipeline
auto result = words
    | std::views::enumerate
    | std::views::filter([](auto pair) {
        return std::get<1>(pair).length() > 3;
    })
    | std::views::transform([](auto pair) {
        auto [idx, word] = pair;
        return std::make_pair(idx, word.length());
    })
    | std::views::chunk(2)
    | std::ranges::to<std::vector>();

// Matrix operations with zip and slide
std::vector<int> row1 = {1, 2, 3, 4};
std::vector<int> row2 = {5, 6, 7, 8};

auto dot_products = std::views::zip(row1, row2)
    | std::views::transform([](auto pair) {
        auto [a, b] = pair;
        return a * b;
    })
    | std::views::slide(2)
    | std::views::transform([](auto window) {
        int sum = 0;
        for (auto n : window) sum += n;
        return sum;
    })
    | std::ranges::to<std::vector>();
```

### Performance Considerations

All these views are lazy and don't materialize data until needed. They compose efficiently without creating intermediate containers:

```cpp
// No intermediate vectors created until to<>
auto result = data
    | std::views::filter(predicate)
    | std::views::transform(func)
    | std::views::chunk(10)
    | std::ranges::to<std::vector>();  // Only here is memory allocated
```

### Key Points to Remember

- **`std::ranges::to<>`** materializes ranges into concrete containers, ending the pipeline and allocating memory
- **`zip`** combines multiple ranges element-wise; stops at the shortest range length
- **`slide`** creates overlapping windows of fixed size; useful for moving calculations
- **`chunk`** creates non-overlapping groups; the last chunk may be smaller
- **`enumerate`** adds index numbers to elements; indices start at 0
- **`cartesian_product`** generates all combinations of elements from multiple ranges
- All views are **lazy** and composable; no work is done until iteration or conversion
- Views can be freely **chained** with the pipe operator `|` for complex transformations
- Use `std::ranges::to<>()` to deduce container types or `std::ranges::to<Container>()` for explicit types
- These features require C++23 and appropriate compiler support (GCC 13+, Clang 16+, MSVC 19.37+)

---

## Other Improvements
[Back to top](#major-features)


### 1. `constexpr` for `<cmath>` Functions

C++23 makes many mathematical functions from `<cmath>` usable in `constexpr` contexts, enabling compile-time mathematical computations.

#### What's New

Previously, math functions like `std::abs`, `std::sqrt`, `std::sin`, etc., could only be evaluated at runtime. Now they can be used in constant expressions.

#### Examples

```cpp
#include <cmath>
#include <numbers>

// Compile-time calculations
constexpr double hypotenuse(double a, double b) {
    return std::sqrt(a * a + b * b);
}

constexpr double circle_area(double radius) {
    return std::numbers::pi * radius * radius;
}

// These are computed at compile time!
constexpr double diag = hypotenuse(3.0, 4.0);  // 5.0
constexpr double area = circle_area(2.0);       // ~12.566

// Using in array sizes (compile-time constant required)
constexpr int size = static_cast<int>(std::floor(std::sqrt(100.0)));
int arr[size];  // arr[10]

// More complex compile-time math
constexpr double physics_calc() {
    double velocity = 10.0;
    double angle = std::numbers::pi / 4.0;  // 45 degrees
    double x_component = velocity * std::cos(angle);
    double y_component = velocity * std::sin(angle);
    return std::sqrt(x_component * x_component + y_component * y_component);
}

constexpr double result = physics_calc();  // Computed at compile time
```

#### Benefits

- **Performance**: Complex calculations done during compilation, not at runtime
- **Optimization**: Enables better compiler optimizations
- **Type safety**: Compile-time validation of mathematical operations


### 2. Monadic Operations for `std::optional`

C++23 adds functional programming-style operations to `std::optional`, making it easier to chain operations without explicit checks.

#### The Three Operations

- **`and_then`**: Chains operations that return `optional`
- **`transform`**: Applies a function and wraps result in `optional`
- **`or_else`**: Provides alternative when `optional` is empty

#### Examples

##### Without Monadic Operations (C++17/20)

```cpp
std::optional<std::string> get_user_name(int id);
std::optional<int> get_user_age(const std::string& name);
std::optional<std::string> format_age(int age);

// Verbose nested checks
std::optional<std::string> result;
auto name = get_user_name(42);
if (name) {
    auto age = get_user_age(*name);
    if (age) {
        result = format_age(*age);
    }
}
```

##### With Monadic Operations (C++23)

```cpp
// Clean, functional-style chaining
auto result = get_user_name(42)
    .and_then(get_user_age)
    .and_then(format_age);
```

#### Detailed Examples

```cpp
#include <optional>
#include <string>
#include <cctype>

// and_then: chains optional-returning functions
std::optional<int> parse_int(const std::string& s) {
    try {
        return std::stoi(s);
    } catch (...) {
        return std::nullopt;
    }
}

std::optional<int> validate_positive(int n) {
    return n > 0 ? std::optional(n) : std::nullopt;
}

std::optional<std::string> input = "42";
auto valid_number = input
    .and_then(parse_int)           // std::optional<int>
    .and_then(validate_positive);  // std::optional<int>

// transform: applies function, wraps result in optional
std::optional<int> number = 5;
auto squared = number.transform([](int n) { return n * n; });  // optional<int>(25)
auto empty = std::optional<int>{};
auto still_empty = empty.transform([](int n) { return n * n; }); // nullopt

// Real-world transform example
std::optional<std::string> username = "john_doe";
auto uppercase = username.transform([](const std::string& s) {
    std::string result = s;
    for (char& c : result) c = std::toupper(c);
    return result;
});  // optional<string>("JOHN_DOE")

// or_else: provides alternative when empty
std::optional<int> maybe_value = std::nullopt;
auto with_default = maybe_value.or_else([]() { 
    return std::optional<int>(42); 
});  // optional<int>(42)

// Combining all three
struct User {
    std::string name;
    int age;
};

std::optional<std::string> get_config(const std::string& key);
std::optional<User> parse_user(const std::string& json);
std::string default_user_json();

auto user = get_config("user_settings")
    .or_else([]() { return std::optional(default_user_json()); })
    .and_then(parse_user)
    .transform([](const User& u) { return u.name; });
```

#### Benefits

- **Readability**: Linear flow instead of nested `if` statements
- **Composability**: Easy to build complex pipelines
- **Safety**: Automatic null handling throughout the chain


### 3. Size Literal Suffix `uz` and `z`

C++23 introduces literal suffixes for `size_t` and signed size types to avoid type conversion issues.

#### The Suffixes

- **`uz`** or **`zu`**: Creates `size_t` literals
- **`z`**: Creates `std::ptrdiff_t` or signed size literals

#### Examples

```cpp
#include <vector>
#include <cstddef>

std::vector<int> vec = {1, 2, 3, 4, 5};

// C++20 and earlier - warning: comparison between signed and unsigned
for (int i = 0; i < vec.size(); ++i) {  // Warning!
    // vec.size() is size_t (unsigned), i is int (signed)
}

// C++23 - clean comparison with uz suffix
for (size_t i = 0uz; i < vec.size(); ++i) {  // No warning!
    std::cout << vec[i] << '\n';
}

// Avoiding casts
std::vector<double> data(100);

// Old way - explicit cast needed
for (size_t i = 0; i < static_cast<size_t>(50); ++i) { }

// C++23 way - natural and clear
for (size_t i = 0uz; i < 50uz; ++i) { }

// Array indexing
int arr[10];
size_t index = 5uz;  // Clear intent: this is a size
arr[index] = 42;

// Arithmetic with sizes
size_t buffer_size = 1024uz;
size_t half_buffer = buffer_size / 2uz;

// Template arguments
std::array<int, 100uz> my_array;  // More explicit than just 100

// Avoiding narrowing conversion warnings
void process(size_t count);

process(42uz);  // Clear and correct
process(42);    // Might warn about implicit conversion
```

#### Why It Matters

```cpp
// Common pitfall avoided by uz
std::vector<int> v = {1, 2, 3};

// Dangerous - can underflow!
for (size_t i = v.size() - 1; i >= 0; --i) {  
    // size_t is unsigned, so i >= 0 is always true!
    // This is an infinite loop!
}

// With proper types and awareness
for (size_t i = v.size(); i > 0uz; ) {
    --i;
    std::cout << v[i] << '\n';
}
```

### 4. `#warning` Directive

C++23 standardizes the `#warning` preprocessor directive, which many compilers already supported as an extension.

#### What It Does

Emits a compiler warning with a custom message during compilation.

#### Examples

```cpp
// Basic usage
#warning "This code is deprecated and will be removed in v2.0"

// Conditional warnings
#ifdef USE_OLD_API
    #warning "USE_OLD_API is defined - consider migrating to new API"
#endif

// Version checks
#if __cplusplus < 202302L
    #warning "This code is optimized for C++23, consider upgrading"
#endif

// Feature detection
#ifndef ENABLE_OPTIMIZATION
    #warning "Optimizations disabled - performance may be reduced"
#endif

// Development reminders
#ifdef DEBUG
    #warning "Debug mode enabled - not for production use"
#endif

// API deprecation notice
#define OLD_FUNCTION_CALL() \
    _Pragma("GCC warning \"OLD_FUNCTION_CALL is deprecated\"") \
    old_function_implementation()

// Platform-specific warnings
#if defined(_WIN32) && !defined(UNICODE)
    #warning "Building without UNICODE support on Windows"
#endif

// Configuration validation
#if MAX_BUFFER_SIZE < 1024
    #warning "MAX_BUFFER_SIZE is very small, this may cause issues"
#endif
```

#### Practical Use Cases

```cpp
// Work-in-progress notification
#warning "TODO: Implement error handling for edge cases"

// Performance notification
#ifdef DISABLE_SIMD
    #warning "SIMD optimizations disabled - expect slower performance"
#endif

// Security warnings
#ifndef ENABLE_ENCRYPTION
    #warning "Building without encryption - data will be transmitted in plaintext"
#endif

// Build configuration validation
#if defined(FEATURE_A) && defined(FEATURE_B)
    #warning "FEATURE_A and FEATURE_B are mutually exclusive"
#endif
```

#### Difference from `#error`

```cpp
// #error stops compilation
#ifndef REQUIRED_DEFINE
    #error "REQUIRED_DEFINE must be set"  // Compilation fails
#endif

// #warning continues compilation
#ifndef OPTIONAL_FEATURE
    #warning "OPTIONAL_FEATURE not set, using defaults"  // Just warns
#endif
```

### Summary: Quick Reference

#### Things to Remember

1. **`constexpr <cmath>`**
   - Math functions now work at compile-time
   - Use for performance-critical constant calculations
   - Enables compile-time physics, geometry, and numerical computations
   - Functions like `sqrt`, `sin`, `cos`, `abs`, `floor`, etc. are now `constexpr`

2. **Monadic `std::optional`**
   - `and_then(f)` - chains optional-returning functions
   - `transform(f)` - applies function, wraps result in optional
   - `or_else(f)` - provides alternative when empty
   - Eliminates nested `if` checks for cleaner, functional-style code
   - All operations short-circuit on empty optionals

3. **Size Literal Suffixes**
   - `uz` or `zu` creates `size_t` literals
   - `z` creates signed size literals
   - Avoids signed/unsigned comparison warnings
   - Makes intent explicit in size-related code
   - Use when working with containers, arrays, and indices

4. **`#warning` Directive**
   - Now part of the standard (was compiler extension)
   - Emits custom warning messages at compile-time
   - Doesn't stop compilation (unlike `#error`)
   - Use for deprecation notices, TODOs, and configuration alerts
   - Helpful for development reminders and build validation

---

## Containers and Data Structures
[Back to top](#major-features)

### Overview

C++23 introduces `std::flat_map` and `std::flat_set` as new container adaptors that provide associative container interfaces backed by contiguous memory storage. These containers store their elements in sorted vectors rather than tree nodes, offering superior cache locality and memory efficiency compared to traditional tree-based containers like `std::map` and `std::set`.

These containers are defined in the `<flat_map>` and `<flat_set>` headers respectively.

### Key Characteristics

#### Memory Layout

Unlike `std::map` and `std::set` which use red-black trees with scattered node allocations, flat containers store elements in contiguous memory (typically using `std::vector` internally). For `std::flat_map`, keys and values are stored in separate contiguous sequences.

**Benefits:**
- Better cache locality leading to faster lookups in many scenarios
- Reduced memory overhead (no tree node pointers)
- Faster iteration through elements
- Better memory compaction

**Trade-offs:**
- Slower insertions and deletions (O(n) vs O(log n) for trees)
- Best suited for scenarios with infrequent modifications and frequent lookups

### `std::flat_set`

#### Basic Usage

```cpp
#include <flat_set>
#include <iostream>
#include <string>

int main() {
    // Creating a flat_set
    std::flat_set<int> numbers = {5, 2, 8, 1, 9, 3};
    
    // Elements are automatically sorted
    for (int n : numbers) {
        std::cout << n << " "; // Output: 1 2 3 5 8 9
    }
    std::cout << "\n";
    
    // Insertion
    numbers.insert(4);
    numbers.insert(2); // Duplicate, won't be added
    
    // Lookup
    if (numbers.contains(5)) {
        std::cout << "Found 5\n";
    }
    
    // Range-based operations
    auto it = numbers.lower_bound(3);
    std::cout << "First element >= 3: " << *it << "\n"; // Output: 3
    
    return 0;
}
```

#### Advanced Example: Custom Comparator

```cpp
#include <flat_set>
#include <string>
#include <iostream>

struct CaseInsensitiveCompare {
    bool operator()(const std::string& a, const std::string& b) const {
        return std::lexicographical_compare(
            a.begin(), a.end(),
            b.begin(), b.end(),
            [](char c1, char c2) { 
                return std::tolower(c1) < std::tolower(c2); 
            }
        );
    }
};

int main() {
    std::flat_set<std::string, CaseInsensitiveCompare> words = {
        "Apple", "banana", "Cherry", "DATE"
    };
    
    for (const auto& word : words) {
        std::cout << word << " ";
    }
    // Output: Apple banana Cherry DATE (sorted case-insensitively)
    
    return 0;
}
```

### `std::flat_map`

#### Basic Usage

```cpp
#include <flat_map>
#include <iostream>
#include <string>

int main() {
    // Creating a flat_map
    std::flat_map<std::string, int> ages = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35}
    };
    
    // Insertion
    ages["Diana"] = 28;
    ages.insert({"Eve", 32});
    
    // Access
    std::cout << "Alice's age: " << ages["Alice"] << "\n";
    
    // Safe access with at()
    try {
        std::cout << "Bob's age: " << ages.at("Bob") << "\n";
        std::cout << ages.at("Unknown"); // Throws exception
    } catch (const std::out_of_range& e) {
        std::cout << "Key not found\n";
    }
    
    // Iteration
    for (const auto& [name, age] : ages) {
        std::cout << name << ": " << age << "\n";
    }
    
    return 0;
}
```

#### Performance Comparison Example

```cpp
#include <flat_map>
#include <map>
#include <chrono>
#include <iostream>
#include <random>

template<typename Container>
void benchmark_lookups(Container& container, const std::vector<int>& keys) {
    auto start = std::chrono::high_resolution_clock::now();
    
    int sum = 0;
    for (int key : keys) {
        auto it = container.find(key);
        if (it != container.end()) {
            sum += it->second;
        }
    }
    
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Sum: " << sum << ", Time: " << duration.count() << " μs\n";
}

int main() {
    const int SIZE = 1000;
    std::map<int, int> tree_map;
    std::flat_map<int, int> flat_map;
    
    // Populate containers
    for (int i = 0; i < SIZE; ++i) {
        tree_map[i] = i * 2;
        flat_map[i] = i * 2;
    }
    
    // Generate random lookup keys
    std::vector<int> lookup_keys(10000);
    std::mt19937 gen(42);
    std::uniform_int_distribution<> dis(0, SIZE - 1);
    for (auto& key : lookup_keys) {
        key = dis(gen);
    }
    
    std::cout << "std::map lookups: ";
    benchmark_lookups(tree_map, lookup_keys);
    
    std::cout << "std::flat_map lookups: ";
    benchmark_lookups(flat_map, lookup_keys);
    
    // flat_map typically performs better for lookup-heavy workloads
    
    return 0;
}
```

### Construction from Sorted Ranges

A key optimization is constructing flat containers from already-sorted data using `std::sorted_unique`:

```cpp
#include <flat_map>
#include <vector>
#include <algorithm>

int main() {
    // Pre-sorted data
    std::vector<int> sorted_keys = {1, 2, 3, 4, 5};
    std::vector<std::string> sorted_values = {"one", "two", "three", "four", "five"};
    
    // Efficient construction without re-sorting
    std::flat_map<int, std::string> map(
        std::sorted_unique,
        std::move(sorted_keys),
        std::move(sorted_values)
    );
    
    // For flat_set
    std::vector<int> sorted_data = {1, 3, 5, 7, 9};
    std::flat_set<int> set(std::sorted_unique, std::move(sorted_data));
    
    return 0;
}
```

### Working with Underlying Containers

You can extract or replace the underlying containers:

```cpp
#include <flat_map>
#include <vector>

int main() {
    std::flat_map<int, std::string> map = {{1, "one"}, {2, "two"}};
    
    // Extract underlying containers
    auto containers = std::move(map).extract();
    std::vector<int>& keys = containers.keys;
    std::vector<std::string>& values = containers.values;
    
    // Modify them directly
    keys.push_back(3);
    values.push_back("three");
    
    // Re-create the map
    std::flat_map<int, std::string> new_map(
        std::sorted_unique,
        std::move(keys),
        std::move(values)
    );
    
    return 0;
}
```

### Use Case Examples

#### Configuration Cache

```cpp
#include <flat_map>
#include <string>

class ConfigurationManager {
    std::flat_map<std::string, std::string> config_;
    
public:
    void load_config() {
        // Load once, query many times
        config_ = {
            {"app.name", "MyApp"},
            {"app.version", "1.0"},
            {"db.host", "localhost"},
            {"db.port", "5432"}
        };
    }
    
    std::string get(const std::string& key) const {
        auto it = config_.find(key);
        return (it != config_.end()) ? it->second : "";
    }
};
```

#### Enum to String Mapping

```cpp
#include <flat_map>
#include <string_view>

enum class ErrorCode {
    Success = 0,
    NotFound = 404,
    ServerError = 500,
    BadRequest = 400
};

class ErrorMessages {
    inline static const std::flat_map<ErrorCode, std::string_view> messages_ = {
        {ErrorCode::Success, "Operation successful"},
        {ErrorCode::NotFound, "Resource not found"},
        {ErrorCode::ServerError, "Internal server error"},
        {ErrorCode::BadRequest, "Bad request"}
    };
    
public:
    static std::string_view get_message(ErrorCode code) {
        auto it = messages_.find(code);
        return (it != messages_.end()) ? it->second : "Unknown error";
    }
};
```

### When to Use Flat Containers

**Prefer `std::flat_map`/`std::flat_set` when:**
- Lookups are much more frequent than insertions/deletions
- The container size is relatively stable
- Memory efficiency is important
- Cache locality matters for performance
- You're working with small to medium-sized datasets

**Prefer `std::map`/`std::set` when:**
- Frequent insertions and deletions are needed
- Iterator stability is required (flat containers invalidate iterators on insertion/deletion)
- The dataset is very large and tree traversal patterns are beneficial

### Summary: Key Points to Remember

- **Contiguous storage**: Flat containers use vectors internally, providing better cache locality than tree-based containers
- **Performance trade-off**: Faster lookups (especially sequential access) but slower insertions/deletions (O(n) vs O(log n))
- **Memory efficient**: No per-element node overhead, reduced memory fragmentation
- **Iterator invalidation**: Insertions and deletions invalidate iterators and references
- **Sorted unique construction**: Use `std::sorted_unique` to efficiently construct from pre-sorted data
- **Direct container access**: Can extract and manipulate underlying containers with `.extract()`
- **Headers**: `<flat_map>` and `<flat_set>`
- **Best for**: Read-heavy workloads, configuration data, lookup tables, static or slowly-changing collections
- **API similarity**: Nearly identical API to `std::map` and `std::set`, making migration straightforward

---

## Coroutines
[Back to top](#major-features)

### Overview

`std::generator` is a standard library addition in C++23 that provides a simple, type-safe way to create lazy sequences using coroutines. It represents a coroutine that produces a sequence of values on-demand, yielding control back to the caller between each value generation.

### Core Concepts

#### What is `std::generator`?

`std::generator<T>` is a range that generates values lazily using the `co_yield` keyword. Unlike regular functions that compute all values upfront, generators compute values only when requested, making them memory-efficient for large or infinite sequences.

**Key characteristics:**
- **Lazy evaluation**: Values are computed only when needed
- **Stateful**: Maintains execution state between yields
- **Single-pass input range**: Can be iterated once
- **No `co_await` or `co_return` with values**: Only `co_yield` and empty `co_return`

### Basic Syntax

```cpp
#include <generator>

std::generator<int> simple_generator() {
    co_yield 1;
    co_yield 2;
    co_yield 3;
}

int main() {
    for (int value : simple_generator()) {
        std::cout << value << " "; // Outputs: 1 2 3
    }
}
```

### Practical Examples

#### Example 1: Fibonacci Sequence

The classic use case - generating Fibonacci numbers on demand:

```cpp
#include <generator>
#include <iostream>

std::generator<unsigned long long> fibonacci() {
    unsigned long long a = 0, b = 1;
    
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

int main() {
    int count = 0;
    for (auto fib : fibonacci()) {
        std::cout << fib << " ";
        if (++count == 10) break; // Get first 10 numbers
    }
    // Outputs: 0 1 1 2 3 5 8 13 21 34
}
```

#### Example 2: Infinite Stream with Filter

Generating prime numbers lazily:

```cpp
#include <generator>
#include <cmath>

bool is_prime(int n) {
    if (n < 2) return false;
    if (n == 2) return true;
    if (n % 2 == 0) return false;
    
    for (int i = 3; i <= std::sqrt(n); i += 2) {
        if (n % i == 0) return false;
    }
    return true;
}

std::generator<int> primes() {
    int candidate = 2;
    while (true) {
        if (is_prime(candidate)) {
            co_yield candidate;
        }
        candidate++;
    }
}

int main() {
    int count = 0;
    for (int prime : primes()) {
        std::cout << prime << " ";
        if (++count == 20) break; // First 20 primes
    }
    // Outputs: 2 3 5 7 11 13 17 19 23 29 31 37 41 43 47 53 59 61 67 71
}
```

#### Example 3: Range Generation

Creating custom ranges with parameters:

```cpp
#include <generator>

std::generator<int> range(int start, int end, int step = 1) {
    for (int i = start; i < end; i += step) {
        co_yield i;
    }
}

int main() {
    // Even numbers from 0 to 20
    for (int num : range(0, 20, 2)) {
        std::cout << num << " ";
    }
    // Outputs: 0 2 4 6 8 10 12 14 16 18
}
```

#### Example 4: Tree Traversal

Lazy traversal of data structures:

```cpp
#include <generator>
#include <memory>
#include <string>

struct TreeNode {
    int value;
    std::unique_ptr<TreeNode> left;
    std::unique_ptr<TreeNode> right;
    
    TreeNode(int v) : value(v) {}
};

std::generator<int> inorder_traversal(const TreeNode* node) {
    if (!node) co_return;
    
    // Traverse left subtree
    for (int value : inorder_traversal(node->left.get())) {
        co_yield value;
    }
    
    // Yield current node
    co_yield node->value;
    
    // Traverse right subtree
    for (int value : inorder_traversal(node->right.get())) {
        co_yield value;
    }
}

int main() {
    // Build tree:    4
    //               / \
    //              2   6
    //             / \ / \
    //            1  3 5  7
    auto root = std::make_unique<TreeNode>(4);
    root->left = std::make_unique<TreeNode>(2);
    root->right = std::make_unique<TreeNode>(6);
    root->left->left = std::make_unique<TreeNode>(1);
    root->left->right = std::make_unique<TreeNode>(3);
    root->right->left = std::make_unique<TreeNode>(5);
    root->right->right = std::make_unique<TreeNode>(7);
    
    for (int value : inorder_traversal(root.get())) {
        std::cout << value << " ";
    }
    // Outputs: 1 2 3 4 5 6 7
}
```

#### Example 5: Parsing and Token Generation

Lazy tokenization of input:

```cpp
#include <generator>
#include <string>
#include <string_view>

std::generator<std::string_view> tokenize(std::string_view str, char delimiter) {
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string_view::npos) {
        co_yield str.substr(start, end - start);
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    // Yield the last token
    if (start < str.length()) {
        co_yield str.substr(start);
    }
}

int main() {
    std::string data = "apple,banana,cherry,date";
    
    for (auto token : tokenize(data, ',')) {
        std::cout << token << "\n";
    }
    // Outputs:
    // apple
    // banana
    // cherry
    // date
}
```

#### Example 6: Combining Generators

Chaining multiple generators:

```cpp
#include <generator>

std::generator<int> take(std::generator<int> gen, int n) {
    int count = 0;
    for (int value : gen) {
        if (count >= n) break;
        co_yield value;
        count++;
    }
}

std::generator<int> filter_even(std::generator<int> gen) {
    for (int value : gen) {
        if (value % 2 == 0) {
            co_yield value;
        }
    }
}

std::generator<int> naturals() {
    int n = 0;
    while (true) {
        co_yield n++;
    }
}

int main() {
    // Get first 5 even natural numbers
    for (int value : take(filter_even(naturals()), 5)) {
        std::cout << value << " ";
    }
    // Outputs: 0 2 4 6 8
}
```

### Memory Efficiency

Generators shine when dealing with large datasets:

```cpp
#include <generator>
#include <fstream>
#include <string>

// Read file line-by-line without loading entire file into memory
std::generator<std::string> read_lines(const std::string& filename) {
    std::ifstream file(filename);
    std::string line;
    
    while (std::getline(file, line)) {
        co_yield line;
    }
}

int main() {
    // Process huge file without loading it all into memory
    for (const auto& line : read_lines("huge_file.txt")) {
        // Process one line at a time
        if (line.find("ERROR") != std::string::npos) {
            std::cout << "Found error: " << line << "\n";
        }
    }
}
```

### Important Considerations

#### Lifetime Management

```cpp
std::generator<int> dangerous_example() {
    int local = 42;
    co_yield local; // OK: value is copied
    
    int* ptr = &local;
    // co_yield *ptr; // DANGEROUS: yielding reference to local
}

std::generator<const int&> reference_generator() {
    static int value = 100;
    co_yield value; // OK: static lifetime
}
```

#### Single-Pass Limitation

```cpp
auto gen = fibonacci();

// First iteration works
for (auto fib : gen) {
    if (fib > 100) break;
}

// Second iteration won't work - generator is exhausted
// for (auto fib : gen) { } // Won't produce values
```

#### Exception Handling

```cpp
std::generator<int> may_throw() {
    co_yield 1;
    throw std::runtime_error("Error!");
    co_yield 2; // Never reached
}

int main() {
    try {
        for (int value : may_throw()) {
            std::cout << value << "\n";
        }
    } catch (const std::exception& e) {
        std::cout << "Caught: " << e.what() << "\n";
    }
}
```

### Summary: Key Points to Remember

**Core Concepts:**
- `std::generator<T>` creates lazy sequences that compute values on-demand
- Use `co_yield` to produce values; use `co_return` (without value) to end the sequence
- Generators are single-pass input ranges (iterate once only)

**Best Use Cases:**
- Infinite sequences (Fibonacci, primes, natural numbers)
- Large datasets where you need only partial results
- Memory-constrained scenarios (streaming file processing)
- Tree/graph traversal without materializing entire collections

**Important Limitations:**
- **Single-pass**: Once exhausted, cannot be restarted
- **No random access**: Sequential iteration only
- **Lifetime care**: Be cautious with references to local variables
- **No `co_await`**: Generators are synchronous, not for async operations

**Performance Benefits:**
- Zero overhead when not iterating
- Constant memory usage regardless of sequence length
- Enables early termination without computing unnecessary values
- Composes well with range algorithms and views

**Syntax Essentials:**
- `co_yield value;` - produce a value and suspend
- `co_return;` - end the sequence (optional if falling off the end)
- Return type must be `std::generator<T>` or `std::generator<T, V>` (with allocator)

---

## Formatting
[Back to top](#major-features)

### Overview

C++23 significantly enhances the `std::format` library (introduced in C++20) by adding three major features that make formatting more powerful and convenient: direct range formatting, nested container support, and escaped string formatting. These improvements reduce boilerplate code and provide safer, more intuitive ways to format complex data structures.

### 1. Direct Range Formatting

Prior to C++23, formatting ranges required custom formatters or manual iteration. C++23 adds built-in support for formatting ranges directly.

#### Basic Range Formatting

```cpp
#include <format>
#include <vector>
#include <list>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    std::list<std::string> words = {"hello", "world", "from", "C++23"};
    
    // Direct formatting of vectors
    std::cout << std::format("Numbers: {}\n", numbers);
    // Output: Numbers: [1, 2, 3, 4, 5]
    
    // Direct formatting of lists
    std::cout << std::format("Words: {}\n", words);
    // Output: Words: ["hello", "world", "from", "C++23"]
    
    // Works with various containers
    std::array<double, 3> coords = {1.5, 2.7, 3.9};
    std::cout << std::format("Coordinates: {}\n", coords);
    // Output: Coordinates: [1.5, 2.7, 3.9]
}
```

#### Custom Range Formatting

You can customize how ranges are displayed using format specifiers:

```cpp
#include <format>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> values = {10, 20, 30, 40};
    
    // Default formatting
    std::cout << std::format("{}\n", values);
    // Output: [10, 20, 30, 40]
    
    // Custom separator and delimiters
    std::cout << std::format("{:n}\n", values);
    // Output: 10, 20, 30, 40 (no brackets)
    
    // Nested formatting with precision for floats
    std::vector<double> decimals = {1.234567, 2.345678, 3.456789};
    std::cout << std::format("{:.2f}\n", decimals);
    // Output: [1.23, 2.35, 3.46]
}
```

#### Range of Tuples/Pairs

```cpp
#include <format>
#include <vector>
#include <map>
#include <iostream>

int main() {
    // Vector of pairs
    std::vector<std::pair<int, std::string>> pairs = {
        {1, "one"}, {2, "two"}, {3, "three"}
    };
    std::cout << std::format("{}\n", pairs);
    // Output: [(1, "one"), (2, "two"), (3, "three")]
    
    // Map formatting
    std::map<std::string, int> ages = {
        {"Alice", 30}, {"Bob", 25}, {"Charlie", 35}
    };
    std::cout << std::format("{}\n", ages);
    // Output: [("Alice", 30), ("Bob", 25), ("Charlie", 35)]
}
```

### 2. Nested Container Support

C++23 allows formatting of nested containers (containers of containers) without custom formatters.

#### Basic Nested Containers

```cpp
#include <format>
#include <vector>
#include <iostream>

int main() {
    // 2D vector
    std::vector<std::vector<int>> matrix = {
        {1, 2, 3},
        {4, 5, 6},
        {7, 8, 9}
    };
    
    std::cout << std::format("{}\n", matrix);
    // Output: [[1, 2, 3], [4, 5, 6], [7, 8, 9]]
    
    // Vector of lists
    std::vector<std::list<std::string>> nested_words = {
        {"alpha", "beta"},
        {"gamma", "delta", "epsilon"},
        {"zeta"}
    };
    
    std::cout << std::format("{}\n", nested_words);
    // Output: [["alpha", "beta"], ["gamma", "delta", "epsilon"], ["zeta"]]
}
```

#### Deeply Nested Structures

```cpp
#include <format>
#include <vector>
#include <map>
#include <iostream>

int main() {
    // 3D vector
    std::vector<std::vector<std::vector<int>>> tensor = {
        {{1, 2}, {3, 4}},
        {{5, 6}, {7, 8}}
    };
    
    std::cout << std::format("{}\n", tensor);
    // Output: [[[1, 2], [3, 4]], [[5, 6], [7, 8]]]
    
    // Map of vectors
    std::map<std::string, std::vector<int>> data = {
        {"scores", {85, 90, 78}},
        {"ages", {20, 22, 21}}
    };
    
    std::cout << std::format("{}\n", data);
    // Output: [("ages", [20, 22, 21]), ("scores", [85, 90, 78])]
}
```

#### Formatting Nested Containers with Precision

```cpp
#include <format>
#include <vector>
#include <iostream>

int main() {
    std::vector<std::vector<double>> data = {
        {1.23456, 2.34567, 3.45678},
        {4.56789, 5.67890, 6.78901}
    };
    
    // Apply precision to all nested elements
    std::cout << std::format("{:.2f}\n", data);
    // Output: [[1.23, 2.35, 3.46], [4.57, 5.68, 6.79]]
    
    // Width and alignment for nested elements
    std::vector<std::vector<int>> grid = {{1, 22, 333}, {4, 55, 666}};
    std::cout << std::format("{:>4}\n", grid);
    // Output: [[   1,   22,  333], [   4,   55,  666]]
}
```

### 3. Escaped String Formatting

C++23 introduces the `?` format specifier for escaped string output, which is crucial for debugging and displaying strings with special characters safely.

#### Basic Escaped Formatting

```cpp
#include <format>
#include <iostream>

int main() {
    std::string text = "Hello\nWorld\t!";
    
    // Normal formatting
    std::cout << std::format("{}\n", text);
    // Output: Hello
    //         World   !
    
    // Escaped formatting
    std::cout << std::format("{:?}\n", text);
    // Output: "Hello\nWorld\t!"
    
    // Shows the actual escape sequences as readable text
}
```

#### Handling Special Characters

```cpp
#include <format>
#include <iostream>

int main() {
    // String with various escape characters
    std::string complex = "Line1\nLine2\r\nTab:\tQuote:\"End";
    std::cout << std::format("Raw: {}\n", complex);
    std::cout << std::format("Escaped: {:?}\n", complex);
    // Escaped: "Line1\nLine2\r\nTab:\tQuote:\"End"
    
    // Unicode and special characters
    std::string unicode = "Hello 🌍 World";
    std::cout << std::format("{:?}\n", unicode);
    // Output depends on implementation but shows escaped form
    
    // Null character
    std::string with_null = std::string("Text\0More", 9);
    std::cout << std::format("{:?}\n", with_null);
    // Output: "Text\0More"
}
```

#### Escaped Formatting for Ranges

```cpp
#include <format>
#include <vector>
#include <iostream>

int main() {
    std::vector<std::string> strings = {
        "normal",
        "with\nnewline",
        "with\ttab",
        "with\"quotes\""
    };
    
    // Normal formatting
    std::cout << std::format("{}\n", strings);
    // Output: ["normal", "with
    //          newline", "with    tab", "with"quotes""]
    
    // Escaped formatting
    std::cout << std::format("{:?}\n", strings);
    // Output: ["normal", "with\nnewline", "with\ttab", "with\"quotes\""]
    
    // Useful for debugging and logging
}
```

#### Combining Escaped with Other Specifiers

```cpp
#include <format>
#include <iostream>

int main() {
    std::string debug_text = "Error:\nFile not found";
    
    // Escaped with width and alignment
    std::cout << std::format("{:?>30}\n", debug_text);
    // Right-aligned, escaped, width 30
    
    // Escaped in structured output
    std::cout << std::format("Log entry: {:?} at line {}\n", 
                            debug_text, 42);
    // Log entry: "Error:\nFile not found" at line 42
}
```

### Practical Combined Example

Here's a real-world example combining all three features:

```cpp
#include <format>
#include <vector>
#include <map>
#include <iostream>

struct LogEntry {
    std::string message;
    std::vector<std::string> tags;
};

int main() {
    // Nested containers with potentially problematic strings
    std::map<std::string, std::vector<LogEntry>> logs = {
        {"errors", {
            {"File\tnot\nfound", {"critical", "io"}},
            {"Access\"denied\"", {"security", "auth"}}
        }},
        {"warnings", {
            {"Deprecated\nAPI", {"compatibility"}},
            {"Slow\tresponse", {"performance"}}
        }}
    };
    
    // Format the entire structure with escaped strings
    for (const auto& [category, entries] : logs) {
        std::cout << std::format("Category: {}\n", category);
        
        for (const auto& entry : entries) {
            std::cout << std::format("  Message: {:?}\n", entry.message);
            std::cout << std::format("  Tags: {:?}\n", entry.tags);
        }
    }
    
    /* Output:
    Category: errors
      Message: "File\tnot\nfound"
      Tags: ["critical", "io"]
      Message: "Access\"denied\""
      Tags: ["security", "auth"]
    Category: warnings
      Message: "Deprecated\nAPI"
      Tags: ["compatibility"]
      Message: "Slow\tresponse"
      Tags: ["performance"]
    */
}
```

### Performance Considerations

```cpp
#include <format>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> large_vector(1000);
    
    // Efficient: single format call
    std::string result = std::format("{}", large_vector);
    
    // Less efficient: manual iteration (pre-C++23 approach)
    std::string manual = "[";
    for (size_t i = 0; i < large_vector.size(); ++i) {
        if (i > 0) manual += ", ";
        manual += std::to_string(large_vector[i]);
    }
    manual += "]";
    
    // C++23 approach is cleaner and often faster
}
```

### Summary: Key Things to Remember

- **Direct Range Formatting**: All standard containers (vector, list, array, map, etc.) can be formatted directly without custom code
- **Default Output**: Ranges format as `[elem1, elem2, elem3]` with automatic quoting for strings
- **Format Specifier `:n`**: Removes brackets to show just comma-separated values
- **Nested Container Support**: Works automatically with containers of containers (2D vectors, maps of vectors, etc.) to any depth
- **Precision Applies Recursively**: Format specifiers like `:.2f` apply to all nested elements in nested containers
- **Escaped String Formatting**: Use `:?` specifier to display strings with escape sequences visible (e.g., `\n`, `\t`, `\"`)
- **Debugging Aid**: Escaped formatting is invaluable for logging and debugging strings with special characters
- **Works with Ranges**: The `:?` specifier works with ranges of strings, escaping each element
- **Cleaner Code**: Eliminates need for manual loops and custom formatters for common formatting tasks
- **Type Safety**: Compile-time checked, unlike printf-style formatting

---

## Range Views
[Back to top](#major-features)

C++23 significantly expands the ranges library introduced in C++20, adding powerful new view adaptors that make working with sequences more expressive and efficient. Let's explore each of these new views in detail.

### 1. `std::views::zip` and `std::views::zip_transform`

#### `std::views::zip`

Combines multiple ranges element-wise into tuples, similar to Python's `zip()`. The resulting range has a length equal to the shortest input range.

```cpp
#include <ranges>
#include <vector>
#include <string>
#include <iostream>

int main() {
    std::vector<int> ids = {1, 2, 3, 4};
    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    std::vector<double> scores = {95.5, 87.3, 92.1, 88.0};
    
    // Zip three ranges together
    for (auto [id, name, score] : std::views::zip(ids, names, scores)) {
        std::cout << "ID: " << id << ", Name: " << name 
                  << ", Score: " << score << '\n';
    }
    // Output stops at 3 elements (shortest range)
    
    // Practical example: parallel iteration
    std::vector<int> x = {1, 2, 3};
    std::vector<int> y = {4, 5, 6};
    
    for (auto [a, b] : std::views::zip(x, y)) {
        std::cout << a << " + " << b << " = " << (a + b) << '\n';
    }
}
```

#### `std::views::zip_transform`

Applies a function to corresponding elements from multiple ranges, combining `zip` with `transform`.

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> a = {1, 2, 3, 4};
    std::vector<int> b = {10, 20, 30, 40};
    std::vector<int> c = {100, 200, 300, 400};
    
    // Multiply corresponding elements from three vectors
    auto result = std::views::zip_transform(
        [](int x, int y, int z) { return x * y * z; },
        a, b, c
    );
    
    for (auto val : result) {
        std::cout << val << ' '; // 1000 8000 27000 64000
    }
    std::cout << '\n';
    
    // Vector addition example
    auto sum = std::views::zip_transform(std::plus{}, a, b);
    for (auto val : sum) {
        std::cout << val << ' '; // 11 22 33 44
    }
}
```

### 2. `std::views::slide` and `std::views::adjacent`

#### `std::views::slide`

Creates overlapping sliding windows of a specified size over a range.

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    
    // Sliding window of size 3
    for (auto window : data | std::views::slide(3)) {
        std::cout << "[ ";
        for (auto val : window) {
            std::cout << val << ' ';
        }
        std::cout << "]\n";
    }
    // Output:
    // [ 1 2 3 ]
    // [ 2 3 4 ]
    // [ 3 4 5 ]
    // [ 4 5 6 ]
    
    // Moving average calculation
    std::vector<double> prices = {100.0, 102.5, 101.0, 105.0, 103.5};
    
    for (auto window : prices | std::views::slide(3)) {
        double sum = 0.0;
        int count = 0;
        for (auto price : window) {
            sum += price;
            count++;
        }
        std::cout << "3-day moving avg: " << (sum / count) << '\n';
    }
}
```

#### `std::views::adjacent`

A specialized version of `slide` for accessing N adjacent elements as tuples. Available as `adjacent<N>`, with convenience aliases like `adjacent<2>` (pairwise).

```cpp
#include <ranges>
#include <vector>
#include <iostream>
#include <cmath>

int main() {
    std::vector<int> nums = {1, 2, 3, 4, 5};
    
    // Get pairs of adjacent elements (pairwise)
    for (auto [a, b] : nums | std::views::adjacent<2>) {
        std::cout << a << " -> " << b << '\n';
    }
    // Output:
    // 1 -> 2
    // 2 -> 3
    // 3 -> 4
    // 4 -> 5
    
    // Triples
    for (auto [a, b, c] : nums | std::views::adjacent<3>) {
        std::cout << "(" << a << ", " << b << ", " << c << ")\n";
    }
    
    // Calculate differences between consecutive elements
    std::vector<double> values = {1.0, 1.5, 2.5, 5.0, 8.5};
    
    for (auto [prev, curr] : values | std::views::adjacent<2>) {
        std::cout << "Change: " << (curr - prev) << '\n';
    }
}
```

There's also `std::views::pairwise` as an alias for `adjacent<2>`:

```cpp
// Detect monotonically increasing sequences
std::vector<int> sequence = {1, 3, 5, 7, 9};
bool is_increasing = std::ranges::all_of(
    sequence | std::views::pairwise,
    [](auto pair) { 
        auto [a, b] = pair; 
        return a < b; 
    }
);
```

### 3. `std::views::chunk` and `std::views::chunk_by`

#### `std::views::chunk`

Splits a range into fixed-size chunks (non-overlapping).

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    // Split into chunks of 3
    for (auto chunk : data | std::views::chunk(3)) {
        std::cout << "[ ";
        for (auto val : chunk) {
            std::cout << val << ' ';
        }
        std::cout << "]\n";
    }
    // Output:
    // [ 1 2 3 ]
    // [ 4 5 6 ]
    // [ 7 8 9 ]
    
    // Last chunk may be smaller if range size isn't divisible
    std::vector<int> data2 = {1, 2, 3, 4, 5, 6, 7};
    for (auto chunk : data2 | std::views::chunk(3)) {
        std::cout << "Chunk size: " << std::ranges::distance(chunk) << '\n';
    }
    // Outputs: 3, 3, 1
    
    // Batch processing example
    std::vector<std::string> tasks = {
        "task1", "task2", "task3", "task4", "task5"
    };
    
    int batch_num = 1;
    for (auto batch : tasks | std::views::chunk(2)) {
        std::cout << "Processing batch " << batch_num++ << ":\n";
        for (const auto& task : batch) {
            std::cout << "  - " << task << '\n';
        }
    }
}
```

#### `std::views::chunk_by`

Groups consecutive elements where a binary predicate returns true. Unlike `chunk`, the size is dynamic based on the predicate.

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> nums = {1, 2, 2, 3, 3, 3, 1, 1, 4, 4, 4, 4};
    
    // Group consecutive equal elements
    for (auto chunk : nums | std::views::chunk_by(std::equal_to{})) {
        std::cout << "[ ";
        for (auto val : chunk) {
            std::cout << val << ' ';
        }
        std::cout << "]\n";
    }
    // Output:
    // [ 1 ]
    // [ 2 2 ]
    // [ 3 3 3 ]
    // [ 1 1 ]
    // [ 4 4 4 4 ]
    
    // Group by same parity (even/odd)
    std::vector<int> mixed = {2, 4, 6, 1, 3, 5, 8, 10, 7};
    
    for (auto chunk : mixed | std::views::chunk_by(
        [](int a, int b) { return (a % 2) == (b % 2); }
    )) {
        std::cout << "[ ";
        for (auto val : chunk) {
            std::cout << val << ' ';
        }
        std::cout << "]\n";
    }
    // Groups evens and odds separately
    
    // Group ascending sequences
    std::vector<int> data = {1, 2, 3, 5, 4, 5, 6, 2, 3};
    
    for (auto chunk : data | std::views::chunk_by(std::less_equal{})) {
        std::cout << "Ascending sequence: [ ";
        for (auto val : chunk) {
            std::cout << val << ' ';
        }
        std::cout << "]\n";
    }
}
```

### 4. `std::views::enumerate`

Provides Python-style enumeration: pairs each element with its index (starting at 0).

```cpp
#include <ranges>
#include <vector>
#include <string>
#include <iostream>

int main() {
    std::vector<std::string> fruits = {"apple", "banana", "cherry", "date"};
    
    // Basic enumeration
    for (auto [index, fruit] : fruits | std::views::enumerate) {
        std::cout << index << ": " << fruit << '\n';
    }
    // Output:
    // 0: apple
    // 1: banana
    // 2: cherry
    // 3: date
    
    // Combining with other views
    std::vector<int> numbers = {10, 20, 30, 40, 50};
    
    for (auto [idx, val] : numbers 
                           | std::views::filter([](int x) { return x > 20; })
                           | std::views::enumerate) {
        std::cout << "Filtered index " << idx << ": " << val << '\n';
    }
    // Note: idx is the index in the filtered range (0, 1, 2), not original
    
    // Find positions of elements meeting criteria
    std::vector<int> scores = {85, 92, 78, 95, 88, 91};
    
    std::cout << "Students with A grades (90+):\n";
    for (auto [pos, score] : scores | std::views::enumerate) {
        if (score >= 90) {
            std::cout << "  Position " << pos << ": " << score << '\n';
        }
    }
    
    // Creating a lookup table
    std::vector<std::string> commands = {"start", "stop", "pause", "resume"};
    
    for (auto [id, cmd] : commands | std::views::enumerate) {
        std::cout << "Command ID " << id << " = '" << cmd << "'\n";
    }
}
```

### 5. `std::views::cartesian_product`

Generates the Cartesian product of multiple ranges—all possible combinations of elements from each range.

```cpp
#include <ranges>
#include <vector>
#include <string>
#include <iostream>

int main() {
    std::vector<int> sizes = {1, 2, 3};
    std::vector<char> colors = {'R', 'G', 'B'};
    
    // All combinations of sizes and colors
    for (auto [size, color] : std::views::cartesian_product(sizes, colors)) {
        std::cout << "Size " << size << ", Color " << color << '\n';
    }
    // Produces 9 combinations (3 × 3)
    
    // Three ranges
    std::vector<std::string> shirts = {"T-shirt", "Polo"};
    std::vector<std::string> pants = {"Jeans", "Chinos"};
    std::vector<std::string> shoes = {"Sneakers", "Boots"};
    
    std::cout << "\nAll outfit combinations:\n";
    for (auto [shirt, pant, shoe] : 
         std::views::cartesian_product(shirts, pants, shoes)) {
        std::cout << shirt << " + " << pant << " + " << shoe << '\n';
    }
    // Produces 8 combinations (2 × 2 × 2)
    
    // Coordinate grid generation
    std::vector<int> x_coords = {0, 1, 2};
    std::vector<int> y_coords = {0, 1, 2};
    
    std::cout << "\n2D Grid coordinates:\n";
    for (auto [x, y] : std::views::cartesian_product(x_coords, y_coords)) {
        std::cout << "(" << x << ", " << y << ") ";
    }
    std::cout << '\n';
    
    // Combining with filters for constrained problems
    std::vector<int> dice1 = {1, 2, 3, 4, 5, 6};
    std::vector<int> dice2 = {1, 2, 3, 4, 5, 6};
    
    // Find all rolls that sum to 7
    auto sum_to_seven = std::views::cartesian_product(dice1, dice2)
                      | std::views::filter([](auto pair) {
                          auto [a, b] = pair;
                          return a + b == 7;
                      });
    
    std::cout << "\nDice rolls summing to 7:\n";
    for (auto [d1, d2] : sum_to_seven) {
        std::cout << d1 << " + " << d2 << " = 7\n";
    }
}
```

### Practical Combined Examples

These views become even more powerful when combined:

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    // Example 1: Moving average with enumeration
    std::vector<double> stock_prices = {100, 102, 98, 105, 103, 107, 109};
    
    for (auto [day, window] : stock_prices 
                             | std::views::slide(3) 
                             | std::views::enumerate) {
        double sum = 0;
        for (auto price : window) sum += price;
        std::cout << "Day " << (day + 2) << " 3-day avg: " 
                  << (sum / 3.0) << '\n';
    }
    
    // Example 2: Pairwise comparison with filtering
    std::vector<int> temps = {15, 18, 22, 20, 25, 28, 26};
    
    auto temp_increases = temps 
                        | std::views::pairwise
                        | std::views::filter([](auto pair) {
                            auto [prev, curr] = pair;
                            return curr > prev;
                          });
    
    std::cout << "\nTemperature increases:\n";
    for (auto [prev, curr] : temp_increases) {
        std::cout << prev << "°C → " << curr << "°C (+" 
                  << (curr - prev) << ")\n";
    }
    
    // Example 3: Batch processing with enumeration
    std::vector<std::string> data = {
        "a", "b", "c", "d", "e", "f", "g", "h", "i"
    };
    
    for (auto [batch_id, batch] : data 
                                 | std::views::chunk(3) 
                                 | std::views::enumerate) {
        std::cout << "Batch " << batch_id << ": ";
        for (const auto& item : batch) {
            std::cout << item << ' ';
        }
        std::cout << '\n';
    }
}
```

### Performance Considerations

All these views are **lazy** and **non-owning**:
- Elements are computed on-demand during iteration
- No memory allocation for intermediate results
- Original ranges must outlive the views
- Composable with zero-overhead abstraction

```cpp
// All views compose without intermediate allocations
auto result = data 
            | std::views::filter(predicate)
            | std::views::chunk(5)
            | std::views::enumerate
            | std::views::take(10);
// Nothing computed until you iterate over 'result'
```

### Quick Reference Summary

#### Key Things to Remember:

1. **`zip` / `zip_transform`**: Combine N ranges element-wise; length = shortest range; use `zip_transform` for applying functions

2. **`slide(n)`**: Overlapping windows of size n; use for moving averages, pattern detection

3. **`adjacent<N>` / `pairwise`**: Structured tuple access to N consecutive elements; perfect for comparisons and differences

4. **`chunk(n)`**: Fixed-size non-overlapping groups; last chunk may be smaller

5. **`chunk_by(pred)`**: Dynamic grouping based on predicate between consecutive elements; groups consecutive elements while predicate holds

6. **`enumerate`**: Adds index to each element as `(index, value)` pairs; index reflects position in the **view**, not original range

7. **`cartesian_product`**: All combinations across N ranges; size = product of all range sizes; useful for grids, configurations, combinations

8. **All views are lazy**: No computation until iteration; no memory overhead; original ranges must remain valid

9. **Views compose freely**: Chain multiple views with `|` operator for complex transformations

10. **Structured bindings**: Use `auto [...]` to destructure tuples from `zip`, `adjacent`, `enumerate`, and `cartesian_product`

---

## Language Features
[Back to top](#major-features)

### 1. Relaxed `constexpr` - Compile-Time Containers

C++23 significantly expands `constexpr` capabilities by allowing **non-transient allocations** at compile-time, enabling `std::vector` and `std::string` to be used in `constexpr` contexts.

#### Key Concept
Previously, all memory allocated during constant evaluation had to be deallocated before the evaluation completed. C++23 relaxes this, allowing containers to exist at compile-time as long as they're properly cleaned up.

#### Example: Compile-Time String Processing

```cpp
#include <string>
#include <vector>
#include <algorithm>

// Compile-time string reversal
constexpr std::string reverse_string(std::string_view input) {
    std::string result(input);
    std::ranges::reverse(result);
    return result;
}

// Use at compile-time
constexpr auto reversed = reverse_string("Hello, C++23!");
// reversed is computed entirely at compile-time

// Compile-time integer filtering
constexpr std::vector<int> get_evens(std::vector<int> nums) {
    std::vector<int> evens;
    for (int n : nums) {
        if (n % 2 == 0) {
            evens.push_back(n);
        }
    }
    return evens;
}

constexpr auto even_numbers = get_evens({1, 2, 3, 4, 5, 6, 7, 8});
// even_numbers = {2, 4, 6, 8} computed at compile-time
```

#### Example: Compile-Time Configuration Parser

```cpp
constexpr auto parse_config() {
    std::vector<std::string> settings;
    settings.push_back("debug_mode=true");
    settings.push_back("max_threads=8");
    settings.push_back("log_level=verbose");
    return settings;
}

constexpr auto config = parse_config();
// Entire configuration parsed at compile-time!

// Can even do compile-time validation
constexpr bool validate_config() {
    auto cfg = parse_config();
    return cfg.size() >= 3; // Ensure minimum settings present
}

static_assert(validate_config(), "Invalid configuration");
```

#### Benefits
- **Zero runtime overhead** for complex initialization
- **Compile-time validation** of data structures
- **Template metaprogramming** becomes more readable


### 2. `static operator()` and `operator[]` - Zero-Size Function Objects

C++23 allows marking `operator()` and `operator[]` as `static`, eliminating the need for an implicit `this` pointer and enabling **zero-size optimizations**.

#### Key Concept
Stateless function objects (lambdas with no captures) can now be truly zero-size by making their call operator `static`.

#### Example: Zero-Size Comparators

```cpp
#include <algorithm>
#include <vector>

// Traditional approach - has implicit 'this'
struct CompareTraditional {
    bool operator()(int a, int b) const {
        return a < b;
    }
};

// C++23 - truly stateless
struct CompareStatic {
    static bool operator()(int a, int b) {
        return a < b;
    }
};

// Size comparison
static_assert(sizeof(CompareTraditional) == 1); // Requires 1 byte
static_assert(sizeof(CompareStatic) == 1);      // Still 1 byte due to empty struct
// But CompareStatic can be optimized better by compiler

// More useful with lambdas
void example() {
    std::vector<int> numbers = {5, 2, 8, 1, 9};
    
    // C++23: static lambda
    auto compare = [](int a, int b) static { return a < b; };
    std::ranges::sort(numbers, compare);
}
```

#### Example: Static Index Operator

```cpp
template<typename T>
struct StaticLookup {
    static constexpr std::array<T, 4> data = {10, 20, 30, 40};
    
    // No need for object state - pure static lookup
    static T operator[](size_t index) {
        return data[index];
    }
};

// Usage - more intuitive than static member function
auto value = StaticLookup<int>{}[2]; // Returns 30
// Equivalent to: StaticLookup<int>::data[2]
```

#### Example: Algorithm Optimizations

```cpp
#include <ranges>

struct Multiplier {
    static int operator()(int x) {
        return x * 2;
    }
};

void process(std::vector<int>& data) {
    // Compiler can optimize better knowing no state is involved
    auto result = data | std::views::transform(Multiplier{});
}
```

#### Benefits
- **Better optimization** potential for algorithms
- **Clearer intent** - explicitly stateless
- **Improved lambda semantics** for pure functions


### 3. `auto(x)` and `auto{x}` - Explicit Decay-Copy

These new constructs provide **explicit decay-copy** semantics, converting references to values and stripping cv-qualifiers.

#### Key Concept
`auto(x)` creates a prvalue by performing decay-copy, useful in templates and perfect forwarding scenarios where you need to force a copy.

#### Example: Perfect Forwarding Edge Cases

```cpp
#include <utility>
#include <thread>
#include <iostream>

template<typename T>
void process(T&& arg) {
    // Problem: forwarding a reference to a thread
    // std::thread t([&arg]() { use(arg); }); // Dangling reference!
    
    // Solution: explicit decay-copy
    std::thread t([arg = auto(std::forward<T>(arg))]() {
        // arg is now a proper copy, safe to use
        std::cout << arg << '\n';
    });
    t.detach();
}

void example() {
    int x = 42;
    process(x); // Safe - x is copied into lambda
}
```

#### Example: Stripping References and Qualifiers

```cpp
void demonstrate_decay() {
    const int& ref = 42;
    
    // auto(ref) creates a non-const, non-reference copy
    auto copy1 = auto(ref);
    static_assert(std::is_same_v<decltype(copy1), int>);
    
    // Compare with traditional behavior
    auto copy2 = ref; // Also int due to reference stripping
    decltype(auto) copy3 = ref; // const int& - preserves everything
    
    // Useful for arrays
    int arr[3] = {1, 2, 3};
    auto decayed = auto(arr); // Decays to int*
    static_assert(std::is_same_v<decltype(decayed), int*>);
}
```

#### Example: Generic Lambda Return

```cpp
#include <string>
#include <vector>

auto process_strings(const std::vector<std::string>& vec) {
    // Want to return a copy, not a reference
    return [&vec]() {
        if (vec.empty()) {
            return auto(std::string("default")); // Explicit prvalue
        }
        return auto(vec[0]); // Force copy, not reference
    };
}
```

#### Example: SFINAE and Concept Compatibility

```cpp
template<typename T>
concept Copyable = requires(T t) {
    { auto(t) } -> std::same_as<T>; // Requires copy constructor
};

template<Copyable T>
void safe_copy(const T& source, T& dest) {
    dest = auto(source); // Guaranteed copy
}
```

#### Benefits
- **Explicit control** over copy semantics
- **Safer forwarding** in async contexts
- **Clearer intent** in generic code


### 4. `#elifdef` / `#elifndef` - Cleaner Preprocessor Conditionals

C++23 adds `#elifdef` and `#elifndef` as shorthand for common preprocessor patterns, improving readability.

#### Key Concept
Replace verbose `#elif defined(...)` / `#elif !defined(...)` patterns with cleaner equivalents.

#### Example: Traditional vs C++23

```cpp
// Traditional approach - verbose
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    void platform_init() { /* Windows code */ }
#elif defined(PLATFORM_LINUX)
    #include <unistd.h>
    void platform_init() { /* Linux code */ }
#elif defined(PLATFORM_MACOS)
    #include <mach/mach.h>
    void platform_init() { /* macOS code */ }
#else
    #error "Unknown platform"
#endif

// C++23 - cleaner
#ifdef PLATFORM_WINDOWS
    #include <windows.h>
    void platform_init() { /* Windows code */ }
#elifdef PLATFORM_LINUX
    #include <unistd.h>
    void platform_init() { /* Linux code */ }
#elifdef PLATFORM_MACOS
    #include <mach/mach.h>
    void platform_init() { /* macOS code */ }
#else
    #error "Unknown platform"
#endif
```

#### Example: Feature Detection

```cpp
// Checking for various feature support
#ifdef HAS_CUDA
    #include <cuda_runtime.h>
    #define ACCELERATOR "CUDA"
#elifdef HAS_OPENCL
    #include <CL/cl.h>
    #define ACCELERATOR "OpenCL"
#elifdef HAS_METAL
    #include <Metal/Metal.h>
    #define ACCELERATOR "Metal"
#else
    #define ACCELERATOR "CPU"
#endif

const char* get_accelerator() {
    return ACCELERATOR;
}
```

#### Example: Debug Level Configuration

```cpp
#ifdef DEBUG_LEVEL_VERBOSE
    #define LOG(msg) std::cout << "[VERBOSE] " << msg << '\n'
#elifdef DEBUG_LEVEL_INFO
    #define LOG(msg) std::cout << "[INFO] " << msg << '\n'
#elifdef DEBUG_LEVEL_WARNING
    #define LOG(msg) std::cerr << "[WARN] " << msg << '\n'
#else
    #define LOG(msg) // No logging in release
#endif
```

#### Example: `#elifndef` Usage

```cpp
// Fallback definitions
#ifndef USE_CUSTOM_ALLOCATOR
    using Allocator = std::allocator<int>;
#elifndef USE_POOL_ALLOCATOR
    using Allocator = CustomAllocator<int>;
#else
    using Allocator = PoolAllocator<int>;
#endif
```

#### Benefits
- **Improved readability** - less visual noise
- **Consistency** with `#ifdef`/`#ifndef` patterns
- **Reduced errors** - simpler syntax means fewer typos

### Summary: Key Takeaways

#### ✓ Relaxed `constexpr`
- `std::vector` and `std::string` work at compile-time
- Enables complex compile-time computations
- Zero runtime cost for initialization

#### ✓ `static operator()` / `operator[]`
- Marks function objects as truly stateless
- Better optimization opportunities
- Clearer semantic intent

#### ✓ `auto(x)` / `auto{x}`
- Explicit decay-copy for references and values
- Critical for safe perfect forwarding
- Strips cv-qualifiers and converts to prvalue

#### ✓ `#elifdef` / `#elifndef`
- Shorthand for `#elif defined(...)` patterns
- Cleaner preprocessor conditionals
- Reduces verbosity in platform/feature detection

**Bottom Line**: C++23's language features focus on making existing patterns cleaner, safer, and more expressive while enabling new compile-time programming capabilities.
