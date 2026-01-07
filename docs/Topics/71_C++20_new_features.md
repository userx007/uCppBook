# Comprehensive code artifact demonstrating C++20's major features

## Major C++20 Features:

**1. Concepts**
- Type constraints for templates that make requirements explicit
- Replaces complex SFINAE with readable syntax
- Example: `template<Numeric T>` ensures T is a numeric type

**2. Ranges**
- Composable, lazy-evaluated algorithms
- Pipeline syntax using `|` operator
- Views are lightweight and don't copy data
- Much more expressive than traditional iterators

**3. Coroutines**
- Functions that can suspend and resume execution
- Enable generators, async operations, and lazy sequences
- Uses `co_yield`, `co_await`, and `co_return` keywords

**4. Three-Way Comparison (Spaceship Operator `<=>`)**
- Single operator generates all six comparison operators
- Returns ordering category (strong, weak, partial)
- `= default` makes it trivial to implement

**5. Modules**
- Modern alternative to header files
- Faster compilation and better encapsulation
- No more header guards needed

**6. Designated Initializers**
- Initialize struct members by name
- More readable and maintainable
- Borrowed from C99

**7. `std::format`**
- Type-safe Python-style string formatting
- Replaces printf and iostreams for many uses
- Supports positional and named arguments

**8. `std::span`**
- Non-owning view over contiguous memory
- Works with arrays, vectors, and C-style arrays
- Safer than raw pointers

**9. Constexpr Enhancements**
- Virtual functions can be constexpr
- Dynamic memory allocation at compile time
- `std::vector` and `std::string` in constexpr contexts

**10. Lambda Improvements**
- Template parameters in lambdas
- `[=, this]` capture allowed
- Lambdas in unevaluated contexts

**11. `std::source_location`**
- Better alternative to `__FILE__`, `__LINE__`, `__func__`
- Automatic propagation through function calls
- Useful for logging and debugging

**12. Calendar and Time Zones**
- Comprehensive date/time library
- Time zone support built-in
- Date literals like `2024y/January/15`

**13. Additional Features:**
- `[[likely]]` and `[[unlikely]]` attributes for optimization hints
- `constinit` for guaranteed compile-time initialization
- `std::bit_cast` for safe type punning
- Mathematical constants in `std::numbers`
- Init-statements in range-based for loops
- Abbreviated function templates with `auto` parameters

The code artifact includes working examples of all these features that you can compile with a C++20-compliant compiler (gcc 10+, clang 10+, or MSVC 2019+).

```cpp
#include <iostream>
#include <vector>
#include <ranges>
#include <concepts>
#include <compare>
#include <coroutine>
#include <format>
#include <span>
#include <bit>
#include <numbers>

// ============================================================================
// 1. CONCEPTS - Type constraints for template parameters
// ============================================================================

// Define a custom concept
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// Use concept to constrain template
template<Numeric T>
T add(T a, T b) {
    return a + b;
}

// Concept with requires clause
template<typename T>
concept Comparable = requires(T a, T b) {
    { a < b } -> std::convertible_to<bool>;
    { a > b } -> std::convertible_to<bool>;
};

// ============================================================================
// 2. RANGES - Composable algorithms
// ============================================================================

void demonstrateRanges() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Filter even numbers, square them, and take first 3
    auto result = numbers 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; })
        | std::views::take(3);
    
    std::cout << "Ranges example: ";
    for (int n : result) {
        std::cout << n << " ";  // Output: 4 16 36
    }
    std::cout << "\n";
}

// ============================================================================
// 3. COROUTINES - Suspendable functions
// ============================================================================

struct Generator {
    struct promise_type {
        int current_value;
        
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void unhandled_exception() { std::terminate(); }
        
        std::suspend_always yield_value(int value) {
            current_value = value;
            return {};
        }
        void return_void() {}
    };
    
    std::coroutine_handle<promise_type> h;
    
    Generator(std::coroutine_handle<promise_type> handle) : h(handle) {}
    ~Generator() { if (h) h.destroy(); }
    
    bool move_next() {
        h.resume();
        return !h.done();
    }
    
    int current_value() {
        return h.promise().current_value;
    }
};

Generator counter(int start, int end) {
    for (int i = start; i < end; ++i) {
        co_yield i;
    }
}

// ============================================================================
// 4. THREE-WAY COMPARISON (Spaceship Operator <=>)
// ============================================================================

struct Point {
    int x, y;
    
    // Compiler generates all six comparison operators
    auto operator<=>(const Point&) const = default;
};

// ============================================================================
// 5. MODULES - Better alternative to header files
// ============================================================================

// Note: Modules require special compiler setup
// Example syntax:
// export module math;
// export int add(int a, int b) { return a + b; }
// 
// import math;  // Instead of #include

// ============================================================================
// 6. DESIGNATED INITIALIZERS
// ============================================================================

struct Config {
    int width;
    int height;
    bool fullscreen;
    std::string title;
};

void demonstrateDesignatedInit() {
    Config cfg = {
        .width = 1920,
        .height = 1080,
        .fullscreen = true,
        .title = "My Window"
    };
    
    std::cout << "Window: " << cfg.width << "x" << cfg.height << "\n";
}

// ============================================================================
// 7. std::format - Type-safe string formatting
// ============================================================================

void demonstrateFormat() {
    int age = 25;
    std::string name = "Alice";
    
    // Python-style formatting
    std::string msg = std::format("Hello {}, you are {} years old", name, age);
    std::cout << msg << "\n";
    
    // With positional arguments
    std::string msg2 = std::format("{1} is {0} years old", age, name);
    std::cout << msg2 << "\n";
}

// ============================================================================
// 8. std::span - Non-owning view over contiguous memory
// ============================================================================

void processData(std::span<const int> data) {
    std::cout << "Processing " << data.size() << " elements: ";
    for (int val : data) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

void demonstrateSpan() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    int arr[] = {6, 7, 8};
    
    processData(vec);  // Works with vector
    processData(arr);  // Works with array
}

// ============================================================================
// 9. CONSTEXPR IMPROVEMENTS - More contexts support constexpr
// ============================================================================

constexpr int factorial(int n) {
    if (n <= 1) return 1;
    return n * factorial(n - 1);
}

// Constexpr vectors and strings
constexpr auto computeAtCompileTime() {
    std::vector<int> vec = {1, 2, 3};
    vec.push_back(4);
    return vec.size();
}

// ============================================================================
// 10. LAMBDA IMPROVEMENTS
// ============================================================================

void demonstrateLambdas() {
    // Template lambdas
    auto add = []<typename T>(T a, T b) {
        return a + b;
    };
    
    std::cout << "Lambda: " << add(5, 3) << "\n";
    std::cout << "Lambda: " << add(2.5, 1.5) << "\n";
    
    // Capture [=, this] allowed
    int x = 10;
    auto lambda = [=, this]() { return x; };
}

// ============================================================================
// 11. std::source_location - Better than __FILE__ and __LINE__
// ============================================================================

#include <source_location>

void log(const std::string& msg, 
         const std::source_location& loc = std::source_location::current()) {
    std::cout << loc.file_name() << ":" 
              << loc.line() << " [" 
              << loc.function_name() << "] " 
              << msg << "\n";
}

// ============================================================================
// 12. RANGES ALGORITHMS
// ============================================================================

void demonstrateRangesAlgorithms() {
    std::vector<int> data = {5, 2, 8, 1, 9};
    
    // Range-based sort (no begin/end needed)
    std::ranges::sort(data);
    
    // Projections
    std::vector<std::string> words = {"apple", "cat", "elephant"};
    std::ranges::sort(words, {}, &std::string::length);
    
    for (const auto& w : words) {
        std::cout << w << " ";  // cat apple elephant
    }
    std::cout << "\n";
}

// ============================================================================
// 13. CONSTINIT - Constant initialization guarantee
// ============================================================================

constinit int global_value = 42;  // Guaranteed compile-time initialization

// ============================================================================
// 14. std::bit_cast - Type-safe bit reinterpretation
// ============================================================================

void demonstrateBitCast() {
    float f = 3.14f;
    auto bits = std::bit_cast<uint32_t>(f);
    std::cout << "Float as bits: " << bits << "\n";
}

// ============================================================================
// 15. MATHEMATICAL CONSTANTS
// ============================================================================

void demonstrateMathConstants() {
    std::cout << "Pi: " << std::numbers::pi << "\n";
    std::cout << "e: " << std::numbers::e << "\n";
    std::cout << "sqrt(2): " << std::numbers::sqrt2 << "\n";
}

// ============================================================================
// 16. CALENDAR AND TIMEZONE SUPPORT
// ============================================================================

#include <chrono>

void demonstrateChrono() {
    using namespace std::chrono;
    
    // Date literals
    auto date = 2024y/January/15;
    std::cout << "Date: " << date << "\n";
    
    // Time zones
    auto now = system_clock::now();
    auto local = zoned_time{current_zone(), now};
}

// ============================================================================
// 17. [[likely]] and [[unlikely]] attributes
// ============================================================================

int processValue(int x) {
    if (x > 0) [[likely]] {
        return x * 2;
    } else [[unlikely]] {
        return -1;
    }
}

// ============================================================================
// 18. INIT-STATEMENT IN RANGE-BASED FOR
// ============================================================================

void demonstrateRangeForInit() {
    for (std::vector v = {1, 2, 3}; auto val : v) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

// ============================================================================
// 19. ABBREVIATED FUNCTION TEMPLATES (auto parameters)
// ============================================================================

auto multiply(auto a, auto b) {
    return a * b;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=== C++20 Features Demo ===\n\n";
    
    // 1. Concepts
    std::cout << "1. Concepts: " << add(5, 10) << "\n";
    
    // 2. Ranges
    std::cout << "2. ";
    demonstrateRanges();
    
    // 3. Coroutines
    std::cout << "3. Coroutines: ";
    auto gen = counter(1, 5);
    while (gen.move_next()) {
        std::cout << gen.current_value() << " ";
    }
    std::cout << "\n";
    
    // 4. Three-way comparison
    Point p1{1, 2}, p2{1, 3};
    std::cout << "4. Spaceship: p1 < p2 = " << (p1 < p2) << "\n";
    
    // 6. Designated initializers
    std::cout << "6. ";
    demonstrateDesignatedInit();
    
    // 7. std::format
    std::cout << "7. ";
    demonstrateFormat();
    
    // 8. std::span
    std::cout << "8. ";
    demonstrateSpan();
    
    // 9. Constexpr
    std::cout << "9. Constexpr factorial(5) = " << factorial(5) << "\n";
    
    // 10. Lambdas
    std::cout << "10. ";
    demonstrateLambdas();
    
    // 11. source_location
    std::cout << "11. ";
    log("This is a test message");
    
    // 12. Ranges algorithms
    std::cout << "12. ";
    demonstrateRangesAlgorithms();
    
    // 14. bit_cast
    std::cout << "14. ";
    demonstrateBitCast();
    
    // 15. Math constants
    std::cout << "15. ";
    demonstrateMathConstants();
    
    // 17. likely/unlikely
    std::cout << "17. Process value: " << processValue(10) << "\n";
    
    // 18. Range-for init
    std::cout << "18. ";
    demonstrateRangeForInit();
    
    // 19. Abbreviated templates
    std::cout << "19. Multiply: " << multiply(5, 3) << "\n";
    
    return 0;
}

```