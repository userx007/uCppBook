 # C++ type traits and meta-programming 

**Core Concepts:**
- What type traits are and why they matter
- Standard library type traits for querying and transforming types
- Custom type trait implementation using template specialization

**Key Techniques:**
- SFINAE (Substitution Failure Is Not An Error)
- `if constexpr` for cleaner compile-time branching (C++17)
- Concepts for template constraints (C++20)
- Tag dispatch pattern for algorithm optimization

**Practical Examples:**
- Type-based function overloading
- Smart container selection based on type properties
- Iterator optimization using tag dispatch
- Compile-time factorial computation


# C++ Type Traits & Meta-Programming

## Overview

**Meta-programming** in C++ is the practice of writing code that generates or manipulates other code at compile-time. **Type traits** are a specific meta-programming technique that allows you to query and manipulate type information at compile-time, enabling you to write more generic, efficient, and type-safe code.

## What Are Type Traits?

Type traits are template classes that provide compile-time information about types. They allow you to:
- Query properties of types (is it a pointer? is it const? is it integral?)
- Transform types (remove const, add pointer, etc.)
- Make compile-time decisions based on type properties

## Basic Type Trait Example

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void print_info(T value) {
    if constexpr (std::is_integral<T>::value) {
        std::cout << "Integer type: " << value << std::endl;
    } else if constexpr (std::is_floating_point<T>::value) {
        std::cout << "Floating-point type: " << value << std::endl;
    } else {
        std::cout << "Other type" << std::endl;
    }
}

int main() {
    print_info(42);        // Integer type: 42
    print_info(3.14);      // Floating-point type: 3.14
    print_info("hello");   // Other type
}
```

## Common Standard Library Type Traits

### Type Properties

```cpp
#include <type_traits>

// Primary type categories
std::is_integral<int>::value           // true
std::is_floating_point<double>::value  // true
std::is_pointer<int*>::value           // true
std::is_array<int[]>::value            // true
std::is_class<std::string>::value      // true

// Type properties
std::is_const<const int>::value        // true
std::is_volatile<volatile int>::value  // true
std::is_reference<int&>::value         // true
std::is_signed<int>::value             // true
std::is_unsigned<unsigned int>::value  // true

// Supported operations
std::is_copy_constructible<std::string>::value  // true
std::is_move_constructible<std::string>::value  // true
std::is_trivially_copyable<int>::value          // true
```

### Type Transformations

```cpp
#include <type_traits>

// Remove qualifiers
using T1 = std::remove_const<const int>::type;      // int
using T2 = std::remove_reference<int&>::type;       // int
using T3 = std::remove_pointer<int*>::type;         // int
using T4 = std::remove_cv<const volatile int>::type; // int

// Add qualifiers
using T5 = std::add_const<int>::type;               // const int
using T6 = std::add_pointer<int>::type;             // int*
using T7 = std::add_lvalue_reference<int>::type;    // int&

// Conditional types
using T8 = std::conditional<true, int, double>::type;  // int
using T9 = std::conditional<false, int, double>::type; // double
```

## Custom Type Traits

You can create your own type traits using template specialization:

```cpp
// Check if a type has a specific member function
template<typename T, typename = void>
struct has_size : std::false_type {};

template<typename T>
struct has_size<T, std::void_t<decltype(std::declval<T>().size())>> 
    : std::true_type {};

// Usage
#include <vector>
#include <string>

static_assert(has_size<std::vector<int>>::value, "vector has size()");
static_assert(has_size<std::string>::value, "string has size()");
static_assert(!has_size<int>::value, "int doesn't have size()");
```

## SFINAE (Substitution Failure Is Not An Error)

SFINAE is a key principle in C++ meta-programming that allows template instantiation to fail without causing compilation errors:

```cpp
#include <iostream>
#include <type_traits>

// Function enabled only for integral types
template<typename T>
typename std::enable_if<std::is_integral<T>::value, void>::type
process(T value) {
    std::cout << "Processing integer: " << value << std::endl;
}

// Function enabled only for floating-point types
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, void>::type
process(T value) {
    std::cout << "Processing float: " << value << std::endl;
}

int main() {
    process(42);      // Calls integer version
    process(3.14);    // Calls floating-point version
}
```

## Modern C++17/20 Techniques

### if constexpr (C++17)

```cpp
template<typename T>
auto get_value(T container) {
    if constexpr (std::is_pointer<T>::value) {
        return *container;  // Dereference if pointer
    } else {
        return container[0];  // Index if container
    }
}
```

### Concepts (C++20)

```cpp
#include <concepts>

// Define a concept
template<typename T>
concept Numeric = std::is_integral_v<T> || std::is_floating_point_v<T>;

// Use the concept to constrain templates
template<Numeric T>
T add(T a, T b) {
    return a + b;
}

// Alternative syntax
template<typename T>
requires Numeric<T>
T multiply(T a, T b) {
    return a * b;
}
```

## Practical Example: Smart Container Selection

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <type_traits>

// Choose the best container based on type properties
template<typename T>
using optimal_container = typename std::conditional<
    std::is_trivially_copyable<T>::value,
    std::vector<T>,  // Use vector for trivially copyable types
    std::list<T>     // Use list for complex types
>::type;

struct Simple {
    int x, y;
};

struct Complex {
    std::string name;
    std::vector<int> data;
    
    Complex(const Complex&) { /* expensive copy */ }
    Complex& operator=(const Complex&) { return *this; }
};

int main() {
    optimal_container<int> numbers;          // std::vector<int>
    optimal_container<Simple> simples;       // std::vector<Simple>
    optimal_container<Complex> complexes;    // std::list<Complex>
}
```

## Tag Dispatch Pattern

```cpp
#include <iostream>
#include <iterator>

// Implementation for random access iterators
template<typename Iter>
void advance_impl(Iter& it, int n, std::random_access_iterator_tag) {
    std::cout << "Using random access (O(1))" << std::endl;
    it += n;
}

// Implementation for bidirectional iterators
template<typename Iter>
void advance_impl(Iter& it, int n, std::bidirectional_iterator_tag) {
    std::cout << "Using bidirectional (O(n))" << std::endl;
    if (n >= 0) {
        while (n--) ++it;
    } else {
        while (n++) --it;
    }
}

// Public interface
template<typename Iter>
void advance(Iter& it, int n) {
    advance_impl(it, n, 
        typename std::iterator_traits<Iter>::iterator_category{});
}
```

## Advanced Example: Compile-Time Computation

```cpp
// Compile-time factorial using template meta-programming
template<unsigned int N>
struct Factorial {
    static constexpr unsigned int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr unsigned int value = 1;
};

// Usage
constexpr unsigned int fact5 = Factorial<5>::value;  // 120 at compile-time

// Modern alternative using constexpr
constexpr unsigned int factorial(unsigned int n) {
    return (n <= 1) ? 1 : n * factorial(n - 1);
}

constexpr unsigned int fact6 = factorial(6);  // 720 at compile-time
```

## Key Benefits

1. **Zero Runtime Overhead**: All decisions made at compile-time
2. **Type Safety**: Catch errors during compilation
3. **Generic Programming**: Write code that works with multiple types
4. **Optimization**: Enable compiler to generate optimized code paths
5. **Code Reusability**: Single implementation works for many types

## Best Practices

- Use `std::is_*_v` shortcuts in C++17+ instead of `std::is_*<T>::value`
- Prefer `if constexpr` over SFINAE when possible (C++17+)
- Use concepts for cleaner template constraints (C++20+)
- Keep meta-programming logic simple and well-documented
- Use standard library type traits before writing custom ones

## Common Pitfalls

- **Compilation Time**: Excessive meta-programming can slow compilation
- **Error Messages**: Template errors can be difficult to debug
- **Readability**: Complex meta-programming reduces code clarity
- **Overuse**: Not everything needs to be compile-time

Type traits and meta-programming are powerful tools that enable highly optimized, generic C++ code while maintaining type safety and zero runtime overhead.