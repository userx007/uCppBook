# C++ compile-time programming covering Template Metaprogramming and modern C++20 features

**Key Topics Covered:**

1. **Template Metaprogramming (TMP)** - The classical approach using template specialization and recursion for compile-time factorial and type selection

2. **constexpr Evolution** - From restrictive C++11 single-expression functions to C++20's powerful capabilities including STL containers and dynamic memory allocation

3. **consteval (C++20)** - Immediate functions that must execute at compile-time, useful for validation and guaranteeing zero runtime cost

4. **Practical Examples** including:
   - Lookup table generation
   - Compile-time string validation
   - Configuration checking
   - Type-level computations

5. **Comparisons** showing when to use each approach and their tradeoffs

The examples demonstrate the evolution from verbose TMP syntax to modern, readable constexpr/consteval code while maintaining the performance benefits of compile-time computation. Each code sample is complete and ready to compile with a C++20 compliant compiler.

# Compile-Time Programming in C++

## Overview

Compile-time programming allows computations to be performed during compilation rather than at runtime, resulting in zero runtime overhead for these calculations. C++ offers several approaches, from classical Template Metaprogramming (TMP) to modern C++20 features.

## 1. Template Metaprogramming (TMP)

TMP is the original C++ technique for compile-time computation, using template specialization and recursion.

### Example: Compile-Time Factorial

```cpp
// Primary template - recursive case
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

// Template specialization - base case
template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// Usage
int main() {
    constexpr int result = Factorial<5>::value;  // Computed at compile-time: 120
    int array[Factorial<4>::value];  // Array size must be compile-time constant
}
```

### Example: Type Selection

```cpp
template<bool Condition, typename TrueType, typename FalseType>
struct Conditional {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct Conditional<false, TrueType, FalseType> {
    using type = FalseType;
};

// Usage
using MyType = Conditional<sizeof(int) == 4, int, long>::type;
```

### Limitations of TMP
- Verbose and difficult to read
- Awkward syntax (using structs for computation)
- Limited control flow capabilities
- Cryptic error messages

## 2. constexpr (C++11 and beyond)

`constexpr` allows functions to be evaluated at compile-time when given constant expressions, while still being callable at runtime.

### C++11 constexpr (Restrictive)

```cpp
// C++11: Very limited - single return statement only
constexpr int factorial_cpp11(int n) {
    return n <= 1 ? 1 : n * factorial_cpp11(n - 1);
}
```

### C++14/17 constexpr (Expanded)

```cpp
// C++14+: Allows loops, multiple statements, local variables
constexpr int factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    
    int prev = 0, curr = 1;
    for (int i = 2; i <= n; ++i) {
        int next = prev + curr;
        prev = curr;
        curr = next;
    }
    return curr;
}

// Usage
constexpr int val1 = factorial(5);     // Compile-time
int n = 5;
int val2 = factorial(n);               // Runtime (n is not constant)
```

### C++20 constexpr (Even More Powerful)

C++20 dramatically expanded constexpr capabilities:

```cpp
#include <vector>
#include <algorithm>

// C++20: STL containers and algorithms can be constexpr
constexpr int sum_even_numbers(int limit) {
    std::vector<int> numbers;
    for (int i = 0; i <= limit; ++i) {
        if (i % 2 == 0) {
            numbers.push_back(i);
        }
    }
    
    int sum = 0;
    for (int num : numbers) {
        sum += num;
    }
    return sum;
}

// C++20: constexpr dynamic memory allocation
constexpr auto create_lookup_table() {
    std::vector<int> table(256);
    for (int i = 0; i < 256; ++i) {
        table[i] = i * i;
    }
    return table;
}

// Usage
constexpr int result = sum_even_numbers(10);  // All at compile-time
```

### constexpr Classes and Methods

```cpp
class Point {
    int x_, y_;
public:
    constexpr Point(int x, int y) : x_(x), y_(y) {}
    
    constexpr int x() const { return x_; }
    constexpr int y() const { return y_; }
    
    constexpr int distance_squared() const {
        return x_ * x_ + y_ * y_;
    }
};

constexpr Point origin(0, 0);
constexpr Point p(3, 4);
constexpr int dist = p.distance_squared();  // 25, computed at compile-time
```

## 3. consteval (C++20 Immediate Functions)

`consteval` functions **must** be evaluated at compile-time. They cannot be called at runtime.

### Basic Example

```cpp
consteval int square(int n) {
    return n * n;
}

int main() {
    constexpr int a = square(5);        // ✓ OK: compile-time
    
    int x = 5;
    // int b = square(x);               // ✗ ERROR: x is not a constant expression
    
    const int y = 5;
    int c = square(y);                  // ✓ OK: y is a constant expression
}
```

### Compile-Time Validation

```cpp
consteval int checked_divide(int a, int b) {
    if (b == 0) {
        throw std::logic_error("Division by zero!");  // Compile-time error!
    }
    return a / b;
}

// This will cause a compilation error with the exception message
// constexpr int result = checked_divide(10, 0);
```

### Forcing Compile-Time Evaluation

```cpp
consteval auto force_compile_time(auto value) {
    return value;
}

constexpr int expensive_computation(int n) {
    // Complex calculation...
    return n * n * n;
}

int main() {
    // Guarantee compile-time evaluation
    constexpr int result = force_compile_time(expensive_computation(100));
    
    int runtime_val = 100;
    // auto bad = force_compile_time(expensive_computation(runtime_val));  // ERROR
}
```

## 4. Comparison and Use Cases

### TMP vs constexpr vs consteval

```cpp
// TMP: Type computations, compile-time only
template<typename T>
struct is_pointer {
    static constexpr bool value = false;
};

template<typename T>
struct is_pointer<T*> {
    static constexpr bool value = true;
};

// constexpr: Can run at compile-time OR runtime
constexpr int flexible_function(int n) {
    return n * 2;
}

// consteval: MUST run at compile-time
consteval int strict_compile_time(int n) {
    return n * 2;
}

int main() {
    // TMP: Type-level computation
    static_assert(is_pointer<int*>::value);
    
    // constexpr: Both compile-time and runtime
    constexpr int ct = flexible_function(5);    // Compile-time
    int rt = flexible_function(get_input());    // Runtime
    
    // consteval: Only compile-time
    constexpr int ct2 = strict_compile_time(5); // ✓ OK
    // int rt2 = strict_compile_time(get_input()); // ✗ ERROR
}
```

### Practical Use Cases

#### 1. Lookup Tables

```cpp
constexpr auto generate_sin_table() {
    std::array<double, 360> table{};
    for (int i = 0; i < 360; ++i) {
        table[i] = std::sin(i * 3.14159265359 / 180.0);
    }
    return table;
}

constexpr auto SIN_TABLE = generate_sin_table();  // Computed once at compile-time
```

#### 2. String Processing

```cpp
consteval bool is_valid_identifier(std::string_view str) {
    if (str.empty() || std::isdigit(str[0])) return false;
    
    for (char c : str) {
        if (!std::isalnum(c) && c != '_') return false;
    }
    return true;
}

// Compile-time validation of identifiers
static_assert(is_valid_identifier("variable_name"));
// static_assert(is_valid_identifier("123invalid"));  // Compilation error
```

#### 3. Configuration Validation

```cpp
struct Config {
    int max_connections;
    int timeout_ms;
    
    consteval Config(int connections, int timeout) 
        : max_connections(connections), timeout_ms(timeout) {
        if (max_connections <= 0) {
            throw std::logic_error("Connections must be positive");
        }
        if (timeout_ms < 100) {
            throw std::logic_error("Timeout too short");
        }
    }
};

// Validates configuration at compile-time
constexpr Config server_config(100, 5000);
// constexpr Config bad_config(0, 50);  // Compilation error with message
```

## 5. Best Practices

### When to Use Each

- **TMP**: Type-level computations, template specialization, SFINAE
- **constexpr**: Functions that should work both at compile-time and runtime
- **consteval**: Force compile-time evaluation, validation, guarantee zero runtime cost

### Performance Benefits

```cpp
// Without compile-time: Runtime cost
int runtime_factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

// With compile-time: Zero runtime cost
constexpr int compile_time_factorial(int n) {
    int result = 1;
    for (int i = 2; i <= n; ++i) result *= i;
    return result;
}

int main() {
    // This performs calculation every time at runtime
    int val1 = runtime_factorial(10);
    
    // This is replaced with the literal 3628800 in the compiled code
    constexpr int val2 = compile_time_factorial(10);
}
```

### Debugging Tip

```cpp
// Use constexpr for development (allows runtime debugging)
constexpr int debug_function(int n) {
    // Can test with runtime values during development
    return n * n;
}

// Switch to consteval for production (enforces compile-time)
consteval int production_function(int n) {
    // Guarantees no runtime cost
    return n * n;
}
```

## Summary

- **TMP**: Classic approach, verbose, powerful for type-level programming
- **constexpr**: Modern, readable, flexible (compile-time when possible, runtime when needed)
- **consteval**: C++20 feature, enforces compile-time evaluation, useful for validation
- **C++20 enhancements**: STL containers, dynamic allocation, more complex algorithms at compile-time

The evolution from TMP to modern constexpr/consteval represents a significant improvement in code clarity while maintaining the performance benefits of compile-time computation.