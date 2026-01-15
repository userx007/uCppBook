# Templates vs. Generics: C++ and Rust

## Overview

Both C++ and Rust provide mechanisms for writing generic code that works with multiple types, but they take fundamentally different philosophical approaches. C++ templates are a Turing-complete metaprogramming system that performs textual substitution and type checking after instantiation. Rust generics use trait bounds for compile-time polymorphism with type checking before monomorphization, providing stronger guarantees about generic code correctness.

## C++ Templates

C++ templates are a powerful metaprogramming facility that allows code generation at compile time. Templates are instantiated on-demand and type-checked only when instantiated with concrete types.

### Basic Template Syntax

```cpp
// Function template
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

// Class template
template<typename T>
class Container {
    T value;
public:
    Container(T v) : value(v) {}
    T get() const { return value; }
};

// Usage
int main() {
    auto x = max(5, 10);           // T = int
    auto y = max(3.14, 2.71);      // T = double
    Container<std::string> c("hello");
}
```

### Template Specialization

C++ allows full and partial specialization to provide custom implementations for specific types:

```cpp
#include <iostream>
#include <cstring>

// Primary template
template<typename T>
class Printer {
public:
    void print(const T& value) {
        std::cout << "Generic: " << value << std::endl;
    }
};

// Full specialization for C-strings
template<>
class Printer<const char*> {
public:
    void print(const char* value) {
        std::cout << "String: " << value << " (length: " 
                  << strlen(value) << ")" << std::endl;
    }
};

// Partial specialization for pointers
template<typename T>
class Printer<T*> {
public:
    void print(T* value) {
        std::cout << "Pointer to: " << *value << std::endl;
    }
};
```

### SFINAE (Substitution Failure Is Not An Error)

SFINAE is a technique that allows template instantiation to fail gracefully, enabling compile-time function overload resolution:

```cpp
#include <type_traits>
#include <iostream>

// Enabled only for integral types
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process(T value) {
    std::cout << "Processing integer: " << value << std::endl;
    return value * 2;
}

// Enabled only for floating-point types
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
process(T value) {
    std::cout << "Processing float: " << value << std::endl;
    return value * 3.14;
}

int main() {
    process(42);      // Calls integer version
    process(3.14);    // Calls floating-point version
}
```

### C++20 Concepts

Concepts provide a cleaner, more expressive way to constrain templates with compile-time requirements:

```cpp
#include <concepts>
#include <iostream>

// Define a concept
template<typename T>
concept Numeric = std::integral<T> || std::floating_point<T>;

// Concept for types that support arithmetic
template<typename T>
concept Arithmetic = requires(T a, T b) {
    { a + b } -> std::convertible_to<T>;
    { a - b } -> std::convertible_to<T>;
    { a * b } -> std::convertible_to<T>;
    { a / b } -> std::convertible_to<T>;
};

// Function constrained by concept
template<Numeric T>
T multiply(T a, T b) {
    return a * b;
}

// Multiple constraints
template<typename T>
requires Arithmetic<T> && std::copyable<T>
class Calculator {
    T value;
public:
    Calculator(T v) : value(v) {}
    
    T add(T other) { return value + other; }
    T multiply(T other) { return value * other; }
};

int main() {
    std::cout << multiply(5, 10) << std::endl;
    std::cout << multiply(3.14, 2.0) << std::endl;
    
    Calculator<int> calc(100);
    std::cout << calc.add(50) << std::endl;
}
```

### Template Metaprogramming

Templates enable compile-time computation:

```cpp
#include <iostream>

// Compile-time factorial
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// Compile-time type selection
template<bool Condition, typename TrueType, typename FalseType>
struct Conditional {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct Conditional<false, TrueType, FalseType> {
    using type = FalseType;
};

int main() {
    std::cout << "5! = " << Factorial<5>::value << std::endl;
    
    using ResultType = Conditional<sizeof(int) >= 4, int, long>::type;
    std::cout << "Selected type size: " << sizeof(ResultType) << std::endl;
}
```

## Rust Generics

Rust generics use trait bounds to constrain type parameters. The type checker validates generic code against trait requirements before monomorphization, catching errors earlier.

### Basic Generic Syntax

```rust
// Generic function
fn max<T: PartialOrd>(a: T, b: T) -> T {
    if a > b { a } else { b }
}

// Generic struct
struct Container<T> {
    value: T,
}

impl<T> Container<T> {
    fn new(value: T) -> Self {
        Container { value }
    }
    
    fn get(&self) -> &T {
        &self.value
    }
}

fn main() {
    let x = max(5, 10);
    let y = max(3.14, 2.71);
    let c = Container::new(String::from("hello"));
}
```

### Trait Bounds

Traits define behavior contracts that types must implement. Generic functions can require types to implement specific traits:

```rust
use std::fmt::Display;

// Single trait bound
fn print_value<T: Display>(value: T) {
    println!("Value: {}", value);
}

// Multiple trait bounds
fn compare_and_print<T: PartialOrd + Display>(a: T, b: T) {
    if a > b {
        println!("{} is greater", a);
    } else {
        println!("{} is greater", b);
    }
}

// Where clause for complex bounds
fn complex_function<T, U>(t: T, u: U) -> i32
where
    T: Display + Clone,
    U: Clone + std::fmt::Debug,
{
    println!("T: {}, U: {:?}", t, u);
    42
}

fn main() {
    print_value(100);
    compare_and_print(5, 10);
    complex_function("hello", vec![1, 2, 3]);
}
```

### Associated Types

Associated types allow traits to define placeholder types that implementers must specify:

```rust
// Iterator trait with associated type
trait Iterator {
    type Item;
    
    fn next(&mut self) -> Option<Self::Item>;
}

// Custom iterator
struct Counter {
    count: u32,
    max: u32,
}

impl Iterator for Counter {
    type Item = u32;  // Associated type defined
    
    fn next(&mut self) -> Option<Self::Item> {
        if self.count < self.max {
            self.count += 1;
            Some(self.count)
        } else {
            None
        }
    }
}

// Generic function using associated types
fn print_items<I>(mut iter: I)
where
    I: Iterator,
    I::Item: std::fmt::Display,
{
    while let Some(item) = iter.next() {
        println!("{}", item);
    }
}

fn main() {
    let counter = Counter { count: 0, max: 5 };
    print_items(counter);
}
```

### Advanced Trait Bounds with Associated Types

```rust
use std::ops::Add;

// Generic container with arithmetic operations
struct Pair<T> {
    first: T,
    second: T,
}

impl<T> Pair<T>
where
    T: Add<Output = T> + Copy,
{
    fn sum(&self) -> T {
        self.first + self.second
    }
}

// Trait with associated type for conversion
trait FromPair {
    type Output;
    
    fn from_pair<T>(pair: Pair<T>) -> Self::Output
    where
        T: Add<Output = T> + Copy;
}

struct SumResult;

impl FromPair for SumResult {
    type Output = i32;
    
    fn from_pair<T>(pair: Pair<T>) -> Self::Output
    where
        T: Add<Output = T> + Copy + Into<i32>,
    {
        (pair.first + pair.second).into()
    }
}

fn main() {
    let pair = Pair { first: 10, second: 20 };
    println!("Sum: {}", pair.sum());
}
```

### Const Generics

Rust supports compile-time constant parameters:

```rust
// Array wrapper with compile-time size
struct Array<T, const N: usize> {
    data: [T; N],
}

impl<T, const N: usize> Array<T, N>
where
    T: Default + Copy,
{
    fn new() -> Self {
        Array {
            data: [T::default(); N],
        }
    }
    
    fn len(&self) -> usize {
        N
    }
}

// Generic function with const parameter
fn create_array<T: Default + Copy, const N: usize>() -> [T; N] {
    [T::default(); N]
}

fn main() {
    let arr1: Array<i32, 5> = Array::new();
    let arr2: Array<f64, 10> = Array::new();
    
    println!("arr1 length: {}", arr1.len());
    println!("arr2 length: {}", arr2.len());
    
    let arr3 = create_array::<u8, 100>();
    println!("arr3 length: {}", arr3.len());
}
```

### Trait Objects vs Monomorphization

Rust offers both static (monomorphization) and dynamic (trait objects) dispatch:

```rust
use std::fmt::Display;

// Static dispatch - monomorphization
fn print_static<T: Display>(value: &T) {
    println!("Static: {}", value);
}

// Dynamic dispatch - trait object
fn print_dynamic(value: &dyn Display) {
    println!("Dynamic: {}", value);
}

// Return trait objects
fn get_displayable(choice: bool) -> Box<dyn Display> {
    if choice {
        Box::new(42)
    } else {
        Box::new("hello")
    }
}

fn main() {
    print_static(&100);      // Generates specific code for i32
    print_static(&"world");  // Generates specific code for &str
    
    print_dynamic(&100);     // Uses vtable at runtime
    print_dynamic(&"world"); // Uses vtable at runtime
    
    let obj = get_displayable(true);
    println!("{}", obj);
}
```

## Comparison Summary

| **Aspect** | **C++ Templates** | **Rust Generics** |
|------------|-------------------|-------------------|
| **Type Checking** | Duck typing - checked after instantiation | Trait-based - checked before monomorphization |
| **Error Messages** | Can be cryptic due to late checking | Generally clearer due to early checking |
| **Constraints** | SFINAE, concepts (C++20) | Trait bounds (always required) |
| **Specialization** | Full and partial specialization supported | Limited: only for trait implementations |
| **Metaprogramming** | Turing-complete template metaprogramming | Limited to const generics and macros |
| **Code Generation** | Instantiated on-demand per type | Monomorphization generates code per type |
| **Associated Types** | Not directly supported (use nested types) | First-class feature via traits |
| **Syntax Complexity** | Can become very complex (angle bracket hell) | Generally cleaner with where clauses |
| **Compile-Time Computation** | Extensive via template metaprogramming | Limited to const fn and const generics |
| **Dynamic Dispatch** | Virtual functions (separate from templates) | Trait objects (dyn Trait) |
| **Zero-Cost Abstraction** | Yes, but may increase binary size | Yes, with same binary size consideration |
| **Type Inference** | Strong, especially with auto/decltype | Strong, often doesn't need explicit types |
| **Default Parameters** | Supported for template parameters | Not supported for generic parameters |
| **Variadic Generics** | Variadic templates supported | Not yet supported (future feature) |
| **Coherence** | No coherence rules | Strict coherence prevents conflicts |

## Key Philosophical Differences

**C++ Philosophy**: Templates provide maximum flexibility and power through a Turing-complete compile-time programming system. The trade-off is complexity and potentially confusing error messages when constraints are violated.

**Rust Philosophy**: Generics prioritize safety and clarity through explicit trait bounds. The type system enforces constraints upfront, catching errors earlier but with less metaprogramming flexibility.

## Performance Considerations

Both approaches use monomorphization (generating separate code for each type), resulting in similar runtime performance but potentially larger binary sizes. C++ concepts and Rust trait bounds have zero runtime cost—they only affect compile time.

The choice between C++ templates and Rust generics often comes down to whether you prioritize metaprogramming power and flexibility (C++) or safety, clarity, and guaranteed correctness of generic code (Rust).