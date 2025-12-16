#include <iostream>
#include <array>
#include <string>
#include <cmath>

// ============================================================================
// C++11: INTRODUCTION OF CONSTEXPR
// ============================================================================

// constexpr variables - must be initialized with constant expression
constexpr int MAX_SIZE = 100;
constexpr double PI = 3.14159265359;

// C++11 constexpr functions had strict limitations:
// - Must have single return statement
// - No loops, no local variables
// - Recursion was the only way to iterate
constexpr int factorial_cpp11(int n) {
    return n <= 1 ? 1 : n * factorial_cpp11(n - 1);
}

// Can be used in compile-time contexts
constexpr int result = factorial_cpp11(5); // Computed at compile time
std::array<int, factorial_cpp11(4)> compile_time_array; // Size = 24

// ============================================================================
// C++14: RELAXED CONSTEXPR RESTRICTIONS
// ============================================================================

// C++14 allows:
// - Multiple statements
// - Loops (for, while, do-while)
// - Local variables (but not static or thread_local)
// - Mutation of local variables
constexpr int factorial_cpp14(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

// More complex example with conditions and loops
constexpr bool is_prime(int n) {
    if (n <= 1) return false;
    if (n <= 3) return true;
    if (n % 2 == 0 || n % 3 == 0) return false;

    for (int i = 5; i * i <= n; i += 6) {
        if (n % i == 0 || n % (i + 2) == 0)
            return false;
    }
    return true;
}

// Count primes up to N at compile time
constexpr int count_primes(int n) {
    int count = 0;
    for (int i = 2; i <= n; ++i) {
        if (is_prime(i)) ++count;
    }
    return count;
}

// ============================================================================
// C++17: IF CONSTEXPR AND CONSTEXPR LAMBDAS
// ============================================================================

// if constexpr - compile-time conditional compilation
template<typename T>
constexpr auto get_value(T t) {
    if constexpr (std::is_pointer_v<T>) {
        return *t;  // Dereference if pointer
    } else {
        return t;   // Return as-is otherwise
    }
}

// constexpr lambdas (C++17)
constexpr auto square = [](int x) { return x * x; };
constexpr int squared = square(5); // = 25 at compile time

// More complex example
template<typename T>
constexpr T sum_array(const T* arr, size_t size) {
    T sum = 0;
    for (size_t i = 0; i < size; ++i) {
        sum += arr[i];
    }
    return sum;
}

// ============================================================================
// C++20: MAJOR ENHANCEMENTS
// ============================================================================

// C++20 allows:
// - constexpr destructors
// - constexpr virtual functions
// - constexpr dynamic_cast and typeid
// - constexpr try-catch blocks
// - constexpr std::string, std::vector (in constexpr contexts)

// constexpr class with destructor (C++20)
class Point {
    int x_, y_;
public:
    constexpr Point(int x, int y) : x_(x), y_(y) {}
    constexpr ~Point() = default;  // C++20: constexpr destructor

    constexpr int x() const { return x_; }
    constexpr int y() const { return y_; }
    constexpr int distance_squared() const {
        return x_ * x_ + y_ * y_;
    }
};

// constexpr virtual functions (C++20)
class Shape {
public:
    constexpr virtual ~Shape() = default;
    constexpr virtual int area() const = 0;
};

class Rectangle : public Shape {
    int width_, height_;
public:
    constexpr Rectangle(int w, int h) : width_(w), height_(h) {}
    constexpr int area() const override { return width_ * height_; }
};

// C++20: constexpr std::string in constexpr context
constexpr int string_length_example() {
    std::string s = "Hello";
    s += " World";
    return s.length();  // Returns 11 at compile time
}

// C++20: constexpr try-catch
constexpr int safe_divide(int a, int b) {
    if (b == 0) {
        // In C++20, we can use try-catch in constexpr
        // But throwing in constexpr context still makes it non-constexpr
        return -1;
    }
    return a / b;
}

// ============================================================================
// PRACTICAL EXAMPLES
// ============================================================================

// Compile-time string hashing
constexpr unsigned int hash_string(const char* str) {
    unsigned int hash = 5381;
    while (*str) {
        hash = ((hash << 5) + hash) + static_cast<unsigned char>(*str++);
    }
    return hash;
}

// Use in switch statements!
constexpr unsigned int HASH_START = hash_string("start");
constexpr unsigned int HASH_STOP = hash_string("stop");
constexpr unsigned int HASH_PAUSE = hash_string("pause");

void process_command(const char* cmd) {
    switch (hash_string(cmd)) {
        case HASH_START: std::cout << "Starting...\n"; break;
        case HASH_STOP:  std::cout << "Stopping...\n"; break;
        case HASH_PAUSE: std::cout << "Pausing...\n"; break;
        default: std::cout << "Unknown command\n";
    }
}

// Compile-time computation for lookup tables
constexpr auto generate_squares() {
    std::array<int, 10> squares{};
    for (size_t i = 0; i < squares.size(); ++i) {
        squares[i] = i * i;
    }
    return squares;
}

constexpr auto SQUARES = generate_squares();

// ============================================================================
// DEMONSTRATION
// ============================================================================

int main() {
    std::cout << "=== CONSTEXPR EVOLUTION DEMO ===\n\n";

    // C++11 style
    std::cout << "C++11:\n";
    std::cout << "  Factorial(5) = " << factorial_cpp11(5) << "\n";
    std::cout << "  Array size = " << compile_time_array.size() << "\n\n";

    // C++14 style
    std::cout << "C++14:\n";
    std::cout << "  Factorial(6) = " << factorial_cpp14(6) << "\n";
    std::cout << "  Is 17 prime? " << (is_prime(17) ? "Yes" : "No") << "\n";
    std::cout << "  Primes up to 20: " << count_primes(20) << "\n\n";

    // C++17 style
    std::cout << "C++17:\n";
    int value = 42;
    std::cout << "  get_value(42) = " << get_value(value) << "\n";
    std::cout << "  get_value(&42) = " << get_value(&value) << "\n";
    std::cout << "  Lambda square(7) = " << square(7) << "\n\n";

    // C++20 style
    std::cout << "C++20:\n";
    constexpr Point p(3, 4);
    std::cout << "  Point distance² = " << p.distance_squared() << "\n";

    constexpr Rectangle rect(5, 10);
    std::cout << "  Rectangle area = " << rect.area() << "\n";
    std::cout << "  String length = " << string_length_example() << "\n\n";

    // Practical examples
    std::cout << "Practical:\n";
    std::cout << "  Hash('start') = " << HASH_START << "\n";
    process_command("start");

    std::cout << "  Squares lookup: SQUARES[5] = " << SQUARES[5] << "\n\n";

    // Runtime vs compile-time
    std::cout << "Runtime vs Compile-time:\n";
    int n;
    std::cout << "  Enter a number: ";
    std::cin >> n;
    std::cout << "  Runtime factorial(" << n << ") = "
              << factorial_cpp14(n) << "\n";
    std::cout << "  (Same function can run at compile-time OR runtime!)\n";

    return 0;
}

// KEY TAKEAWAYS:
// 1. constexpr enables compile-time computation, reducing runtime overhead
// 2. C++11: Basic support, very restrictive (single return, recursion only)
// 3. C++14: Relaxed (loops, local variables, multiple statements)
// 4. C++17: if constexpr, constexpr lambdas
// 5. C++20: Virtual functions, destructors, std::string/vector, try-catch
// 6. Same function can execute at compile-time OR runtime depending on context
// 7. Use for: lookup tables, hashing, validation, compile-time checks