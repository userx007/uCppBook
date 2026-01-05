## What are Concepts?

**Concepts** are named sets of requirements (constraints) on template parameters. They allow you to specify what operations and properties a type must support, making templates more expressive, readable, and producing better error messages.

Before C++20, template constraints were implicit - you'd get cryptic error messages deep in template instantiation when a type didn't meet requirements. Concepts make these constraints explicit and checked at the point of use.

## Basic Concept Syntax

```cpp
// Define a concept
template<typename T>
concept Integral = std::is_integral_v<T>;

// Use it to constrain a template
template<Integral T>
T add(T a, T b) {
    return a + b;
}

// This works
int result = add(5, 10);

// This won't compile - clear error message
// double x = add(5.5, 10.5); // Error: double doesn't satisfy Integral
```

## Requires-Clauses

A **requires-clause** is a boolean expression that specifies constraints. There are several ways to use them:

### 1. Trailing requires-clause

```cpp
template<typename T>
T add(T a, T b) requires std::is_integral_v<T> {
    return a + b;
}
```

### 2. Leading requires-clause (with template<>)

```cpp
template<typename T>
    requires std::is_integral_v<T>
T add(T a, T b) {
    return a + b;
}
```

### 3. Using concepts directly

```cpp
template<std::integral T>  // std::integral is from <concepts>
T add(T a, T b) {
    return a + b;
}
```

### 4. Abbreviated function templates (terse syntax)

```cpp
auto add(std::integral auto a, std::integral auto b) {
    return a + b;
}
```

## Requires-Expression

A **requires-expression** is a prvalue expression that yields a boolean indicating whether requirements are satisfied. It has this syntax:

```cpp
requires (parameter-list) {
    requirement-seq
}
```

### Types of Requirements in Requires-Expressions

#### 1. Simple Requirements
Checks that an expression is valid (compiles):

```cpp
template<typename T>
concept HasSize = requires(T t) {
    t.size();  // T must have a size() member
};
```

#### 2. Type Requirements
Checks that a type is valid:

```cpp
template<typename T>
concept HasValueType = requires {
    typename T::value_type;  // T must have a nested value_type
};
```

#### 3. Compound Requirements
Checks expression validity AND properties (type, noexcept):

```cpp
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;  // a+b must be convertible to T
    { a + b } noexcept -> std::same_as<T>; // Can check noexcept too
};
```

The syntax is: `{ expression } noexcept(optional) -> constraint;`

#### 4. Nested Requirements
Boolean constraints:

```cpp
template<typename T>
concept SignedIntegral = requires {
    requires std::integral<T>;
    requires std::signed_integral<T>;
};
```

## Detailed Examples

### Example 1: Creating a Container Concept

```cpp
template<typename C>
concept Container = requires(C c) {
    typename C::value_type;
    typename C::iterator;
    { c.begin() } -> std::same_as<typename C::iterator>;
    { c.end() } -> std::same_as<typename C::iterator>;
    { c.size() } -> std::convertible_to<std::size_t>;
};

template<Container C>
void print_container(const C& container) {
    for (const auto& elem : container) {
        std::cout << elem << " ";
    }
}
```

### Example 2: Multiple Constraints with Logical Operators

```cpp
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

template<typename T>
concept Arithmetic = Numeric<T> && requires(T a, T b) {
    { a + b } -> std::same_as<T>;
    { a - b } -> std::same_as<T>;
    { a * b } -> std::same_as<T>;
    { a / b } -> std::same_as<T>;
};
```

### Example 3: Concepts with Multiple Template Parameters

```cpp
template<typename T, typename U>
concept Comparable = requires(T t, U u) {
    { t == u } -> std::convertible_to<bool>;
    { t != u } -> std::convertible_to<bool>;
    { t < u } -> std::convertible_to<bool>;
    { t > u } -> std::convertible_to<bool>;
};

template<typename T, typename U>
    requires Comparable<T, U>
bool is_less(T a, U b) {
    return a < b;
}
```

## Function Overloading with Concepts

Concepts enable overload resolution based on constraints:

```cpp
#include <concepts>
#include <iostream>

// More constrained version for integral types
void process(std::integral auto x) {
    std::cout << "Processing integer: " << x << "\n";
}

// Less constrained version for other types
void process(auto x) {
    std::cout << "Processing other type\n";
}

int main() {
    process(42);      // Calls integral version
    process(3.14);    // Calls generic version
}
```

The more constrained function is preferred during overload resolution.

## Standard Library Concepts

C++20 provides many standard concepts in `<concepts>`:

### Core language concepts:
- `std::same_as<T, U>` - T and U are the same type
- `std::derived_from<T, U>` - T is derived from U
- `std::convertible_to<T, U>` - T is convertible to U
- `std::common_reference_with<T, U>` - T and U share a common reference type

### Comparison concepts:
- `std::equality_comparable<T>`
- `std::totally_ordered<T>`

### Object concepts:
- `std::movable<T>`
- `std::copyable<T>`
- `std::semiregular<T>`
- `std::regular<T>`

### Callable concepts:
- `std::invocable<F, Args...>`
- `std::predicate<F, Args...>`

### Arithmetic concepts:
- `std::integral<T>`
- `std::signed_integral<T>`
- `std::unsigned_integral<T>`
- `std::floating_point<T>`

## Subsumption Rules

When multiple constraints apply, C++ uses **subsumption** to determine which is more specialized:

```cpp
template<typename T>
concept Integral = std::is_integral_v<T>;

template<typename T>
concept SignedIntegral = Integral<T> && std::is_signed_v<T>;

void foo(Integral auto x) {
    std::cout << "Integral\n";
}

void foo(SignedIntegral auto x) {
    std::cout << "SignedIntegral\n";
}

int main() {
    foo(42);   // Calls SignedIntegral version (more constrained)
    foo(42u);  // Calls Integral version
}
```

`SignedIntegral` subsumes `Integral` because every `SignedIntegral` is also `Integral`.

## Advanced Patterns

### Concept Refinement

```cpp
template<typename T>
concept Incrementable = requires(T t) {
    { ++t } -> std::same_as<T&>;
    { t++ } -> std::same_as<T>;
};

template<typename T>
concept Iterator = Incrementable<T> && requires(T t) {
    { *t };
};

template<typename T>
concept RandomAccessIterator = Iterator<T> && requires(T t, std::ptrdiff_t n) {
    { t + n } -> std::same_as<T>;
    { t - n } -> std::same_as<T>;
    { t[n] };
};
```

### SFINAE Replacement

Concepts replace the complex SFINAE patterns:

```cpp
// Old SFINAE way
template<typename T,
         typename = std::enable_if_t<std::is_integral_v<T>>>
void process(T value) { }

// Modern concepts way
template<std::integral T>
void process(T value) { }
```

### Constraining Class Templates

```cpp
template<typename T>
    requires std::integral<T>
class Calculator {
public:
    T add(T a, T b) { return a + b; }
};

// Or with concept directly
template<std::integral T>
class Calculator {
public:
    T add(T a, T b) { return a + b; }
};
```

## Checking Concepts

You can check if a type satisfies a concept at compile time:

```cpp
static_assert(std::integral<int>);
static_assert(!std::integral<double>);

if constexpr (std::integral<T>) {
    // Do something for integral types
}
```

## Benefits of Concepts

1. **Better error messages**: Errors at the point of call, not deep in template instantiation
2. **Self-documenting**: Template requirements are explicit
3. **Function overloading**: Enable overload resolution based on semantic requirements
4. **Faster compilation**: Early constraint checking
5. **Cleaner syntax**: Replace SFINAE complexity

## Common Pitfalls

1. **Don't over-constrain**: Only check what you actually need
2. **Watch subsumption**: Understand which overload will be selected
3. **Concept vs. requires-expression**: Concepts are named and reusable, requires-expressions are inline checks
4. **Compound requirement syntax**: Remember the `->` for return type constraints

