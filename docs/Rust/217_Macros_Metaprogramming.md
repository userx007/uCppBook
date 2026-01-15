# Macros & Metaprogramming: C++ vs. Rust

## Overview

Both C++ and Rust provide powerful metaprogramming capabilities, but they take fundamentally different approaches. C++ offers preprocessor macros, template metaprogramming, and compile-time evaluation through `constexpr`. Rust provides hygienic macros through `macro_rules!` and procedural macros, designed with safety and predictability in mind.

## C++ Metaprogramming Approaches

### 1. Preprocessor Macros

C++ preprocessor macros are text-substitution mechanisms that operate before compilation. They're powerful but dangerous, as they don't respect scope, types, or namespaces.

```cpp
#include <iostream>

// Simple macro
#define PI 3.14159

// Function-like macro (dangerous - no type safety)
#define SQUARE(x) ((x) * (x))

// Multi-line macro
#define LOG_ERROR(msg) \
    std::cerr << "[ERROR] " << __FILE__ << ":" << __LINE__ \
              << " - " << msg << std::endl

// Conditional compilation
#ifdef DEBUG
    #define DEBUG_PRINT(x) std::cout << x << std::endl
#else
    #define DEBUG_PRINT(x)
#endif

int main() {
    std::cout << "PI = " << PI << std::endl;
    
    int x = 5;
    std::cout << "Square of " << x << " = " << SQUARE(x) << std::endl;
    
    // Dangerous case - macro expansion issues
    int y = SQUARE(x + 1);  // Expands to ((x + 1) * (x + 1))
    std::cout << "y = " << y << std::endl;  // 36, as expected
    
    // But this can be problematic:
    int z = SQUARE(++x);  // Expands to ((++x) * (++x)) - undefined behavior!
    std::cout << "z = " << z << std::endl;  // Unpredictable result
    
    LOG_ERROR("Something went wrong");
    DEBUG_PRINT("Debug information");
    
    return 0;
}
```

**Problems with preprocessor macros:**
- No type checking
- No scope respect
- Difficult to debug
- Can cause unexpected side effects
- Pollution of namespace

### 2. Template Metaprogramming

Templates allow compile-time computation and code generation with type safety.

```cpp
#include <iostream>
#include <type_traits>

// Compile-time factorial using templates
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// Type traits and SFINAE (Substitution Failure Is Not An Error)
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
multiply_by_two(T value) {
    return value * 2;
}

template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
multiply_by_two(T value) {
    std::cout << "(floating point version) ";
    return value * 2.0;
}

// Variadic templates for compile-time operations
template<typename... Args>
struct Count;

template<>
struct Count<> {
    static constexpr size_t value = 0;
};

template<typename T, typename... Rest>
struct Count<T, Rest...> {
    static constexpr size_t value = 1 + Count<Rest...>::value;
};

// Template specialization for code generation
template<typename T>
class Container {
public:
    void process() {
        std::cout << "Generic container processing" << std::endl;
    }
};

template<>
class Container<int> {
public:
    void process() {
        std::cout << "Specialized int container processing" << std::endl;
    }
};

int main() {
    // Compile-time computation
    std::cout << "Factorial of 5: " << Factorial<5>::value << std::endl;
    
    // SFINAE-based function selection
    std::cout << multiply_by_two(10) << std::endl;        // int version
    std::cout << multiply_by_two(3.14) << std::endl;      // float version
    
    // Variadic template
    std::cout << "Argument count: " << Count<int, double, char, float>::value << std::endl;
    
    // Template specialization
    Container<double> generic;
    Container<int> specialized;
    generic.process();
    specialized.process();
    
    return 0;
}
```

### 3. constexpr - Compile-Time Evaluation

Modern C++ (C++11 and later) provides `constexpr` for guaranteed compile-time evaluation.

```cpp
#include <iostream>
#include <array>

// constexpr function
constexpr int fibonacci(int n) {
    if (n <= 1) return n;
    return fibonacci(n - 1) + fibonacci(n - 2);
}

// constexpr variable
constexpr int compile_time_value = fibonacci(10);

// constexpr with classes (C++14+)
class Rectangle {
private:
    int width_;
    int height_;
public:
    constexpr Rectangle(int w, int h) : width_(w), height_(h) {}
    
    constexpr int area() const {
        return width_ * height_;
    }
};

// Compile-time array generation
template<size_t N>
constexpr auto generate_squares() {
    std::array<int, N> result{};
    for (size_t i = 0; i < N; ++i) {
        result[i] = i * i;
    }
    return result;
}

int main() {
    // Computed at compile time
    std::cout << "Fibonacci(10) = " << compile_time_value << std::endl;
    
    constexpr Rectangle rect(5, 3);
    constexpr int area = rect.area();
    std::cout << "Rectangle area: " << area << std::endl;
    
    // Generate array at compile time
    constexpr auto squares = generate_squares<10>();
    std::cout << "Square of 7: " << squares[7] << std::endl;
    
    return 0;
}
```

## Rust Metaprogramming Approaches

### 1. Declarative Macros (macro_rules!)

Rust's declarative macros are hygienic and pattern-based, operating on syntax trees rather than raw text.

```rust
// Simple macro
macro_rules! say_hello {
    () => {
        println!("Hello, world!");
    };
}

// Macro with arguments
macro_rules! create_function {
    ($func_name:ident) => {
        fn $func_name() {
            println!("Function {:?} was called", stringify!($func_name));
        }
    };
}

// Macro with multiple patterns
macro_rules! calculate {
    (add $a:expr, $b:expr) => {
        $a + $b
    };
    (mul $a:expr, $b:expr) => {
        $a * $b
    };
}

// Variadic macro (repetition)
macro_rules! vec_of_strings {
    ($($element:expr),* $(,)?) => {
        vec![$($element.to_string()),*]
    };
}

// Complex macro with guards
macro_rules! hashmap {
    ($($key:expr => $value:expr),* $(,)?) => {{
        let mut map = ::std::collections::HashMap::new();
        $(map.insert($key, $value);)*
        map
    }};
}

// Hygiene demonstration
macro_rules! unhygienic_example {
    () => {
        let x = 10;  // This x doesn't interfere with outer scope
    };
}

fn main() {
    say_hello!();
    
    create_function!(foo);
    foo();
    
    let sum = calculate!(add 5, 3);
    let product = calculate!(mul 5, 3);
    println!("Sum: {}, Product: {}", sum, product);
    
    let strings = vec_of_strings!["hello", "world", "rust"];
    println!("{:?}", strings);
    
    let map = hashmap! {
        "name" => "Alice",
        "age" => "30",
    };
    println!("{:?}", map);
    
    // Hygiene test
    let x = 5;
    unhygienic_example!();
    println!("x is still: {}", x);  // x is 5, not affected by macro
}
```

### 2. Procedural Macros

Procedural macros operate on Rust's abstract syntax tree (AST) and can generate arbitrary code. They come in three types: function-like, derive, and attribute macros.

```rust
// Note: Procedural macros must be in a separate crate with proc-macro = true
// This is a demonstration of how they're used

// Function-like procedural macro (similar usage to macro_rules!)
// sql!(SELECT * FROM users WHERE id = 1)

// Derive macro (most common)
use serde::{Serialize, Deserialize};

#[derive(Debug, Clone, Serialize, Deserialize)]
struct User {
    id: u32,
    name: String,
    email: String,
}

// Custom derive example
#[derive(Debug)]
struct Point {
    x: i32,
    y: i32,
}

// Attribute macro
// #[route(GET, "/users")]
// fn get_users() -> Response { ... }

fn main() {
    let user = User {
        id: 1,
        name: "Alice".to_string(),
        email: "alice@example.com".to_string(),
    };
    
    // Serialize (generated by derive macro)
    let json = serde_json::to_string(&user).unwrap();
    println!("Serialized: {}", json);
    
    // Deserialize (generated by derive macro)
    let deserialized: User = serde_json::from_str(&json).unwrap();
    println!("Deserialized: {:?}", deserialized);
}
```

### 3. Compile-Time Evaluation with const fn

Rust provides `const fn` for compile-time computation, similar to C++'s `constexpr`.

```rust
// const function for compile-time computation
const fn fibonacci(n: u32) -> u32 {
    match n {
        0 => 0,
        1 => 1,
        _ => fibonacci(n - 1) + fibonacci(n - 2),
    }
}

// const evaluation
const FIB_10: u32 = fibonacci(10);

// const generic parameters
struct Array<T, const N: usize> {
    data: [T; N],
}

impl<T, const N: usize> Array<T, N> {
    const fn len(&self) -> usize {
        N
    }
}

// Complex const fn
const fn factorial(n: u32) -> u32 {
    let mut result = 1;
    let mut i = 1;
    while i <= n {
        result *= i;
        i += 1;
    }
    result
}

fn main() {
    println!("Fibonacci(10) = {}", FIB_10);
    println!("Factorial(5) = {}", factorial(5));
    
    let arr: Array<i32, 5> = Array { data: [1, 2, 3, 4, 5] };
    println!("Array length: {}", arr.len());
}
```

## Hygiene and Safety Differences

### C++ Macro Hygiene Issues

```cpp
#include <iostream>

#define SWAP(a, b) { auto temp = a; a = b; b = temp; }

void demonstrate_hygiene_problem() {
    int temp = 5;  // Variable named 'temp'
    int x = 10;
    
    SWAP(temp, x);  // BUG! Macro's 'temp' shadows our variable
    
    std::cout << "temp: " << temp << ", x: " << x << std::endl;
}

// Macro parameter evaluation issues
#define MAX(a, b) ((a) > (b) ? (a) : (b))

int main() {
    int i = 5;
    // This evaluates ++i twice!
    int result = MAX(++i, 10);  // Undefined behavior
    std::cout << "i: " << i << ", result: " << result << std::endl;
    
    return 0;
}
```

### Rust Macro Hygiene (Safe)

```rust
macro_rules! swap {
    ($a:expr, $b:expr) => {{
        let temp = $a;  // This 'temp' is hygienic
        $a = $b;
        $b = temp;
    }};
}

fn demonstrate_hygiene() {
    let mut temp = 5;  // Our variable
    let mut x = 10;
    
    swap!(temp, x);  // Safe! Macro's 'temp' doesn't interfere
    
    println!("temp: {}, x: {}", temp, x);  // temp=10, x=5 (correctly swapped)
}

// Rust prevents multiple evaluation
macro_rules! max {
    ($a:expr, $b:expr) => {{
        let a_val = $a;  // Evaluated once
        let b_val = $b;  // Evaluated once
        if a_val > b_val { a_val } else { b_val }
    }};
}

fn main() {
    demonstrate_hygiene();
    
    let mut i = 5;
    let result = max!(i + 1, 10);  // Safe! Expression evaluated only once
    println!("i: {}, result: {}", i, result);
}
```

## Practical Examples: Building a Mini DSL

### C++ Template-Based DSL

```cpp
#include <iostream>
#include <string>
#include <vector>

// Builder pattern using templates
template<typename T>
class QueryBuilder {
private:
    std::string table_;
    std::vector<std::string> conditions_;
    
public:
    QueryBuilder& from(const std::string& table) {
        table_ = table;
        return *this;
    }
    
    QueryBuilder& where(const std::string& condition) {
        conditions_.push_back(condition);
        return *this;
    }
    
    std::string build() const {
        std::string query = "SELECT * FROM " + table_;
        if (!conditions_.empty()) {
            query += " WHERE ";
            for (size_t i = 0; i < conditions_.size(); ++i) {
                if (i > 0) query += " AND ";
                query += conditions_[i];
            }
        }
        return query;
    }
};

int main() {
    auto query = QueryBuilder<void>()
        .from("users")
        .where("age > 18")
        .where("active = true")
        .build();
    
    std::cout << query << std::endl;
    return 0;
}
```

### Rust Macro-Based DSL

```rust
// Define a simple SQL-like DSL using macros
macro_rules! sql {
    (SELECT * FROM $table:ident WHERE $($condition:expr),+ $(,)?) => {{
        let table = stringify!($table);
        let conditions = vec![$( $condition.to_string() ),+];
        format!("SELECT * FROM {} WHERE {}", table, conditions.join(" AND "))
    }};
    
    (SELECT * FROM $table:ident) => {{
        format!("SELECT * FROM {}", stringify!($table))
    }};
}

fn main() {
    let query1 = sql!(SELECT * FROM users WHERE "age > 18", "active = true");
    println!("{}", query1);
    
    let query2 = sql!(SELECT * FROM products);
    println!("{}", query2);
}
```

## Summary Table

| Feature | C++ | Rust |
|---------|-----|------|
| **Preprocessor Macros** | Text-based substitution, runs before compilation | Not available (replaced by hygienic macros) |
| **Hygiene** | Non-hygienic: macros can capture/shadow variables | Fully hygienic: macro expansions don't interfere with scope |
| **Type Safety** | Preprocessor macros have no type checking | Macros operate on typed AST, preserving type safety |
| **Template Metaprogramming** | Turing-complete, complex syntax, long compile times | Not applicable (different approach) |
| **Declarative Macros** | Not available | `macro_rules!` with pattern matching on syntax |
| **Procedural Macros** | Not available (closest: external code generators) | Full AST manipulation in separate crate |
| **Compile-Time Computation** | `constexpr` functions and variables | `const fn` with similar capabilities |
| **Error Messages** | Often cryptic for template errors, no location for macro errors | Clear errors with span information, helpful diagnostics |
| **Debugging** | Difficult to debug macro expansions | `cargo expand` shows macro expansions, better tooling |
| **Multiple Evaluation** | Common bug with function-like macros | Prevented by design (bindings in macro body) |
| **Code Generation** | Templates generate code per type instantiation | Macros generate code at expansion sites |
| **Variadic Support** | Variadic templates (C++11+) | Repetition syntax in `macro_rules!` |
| **Namespace Pollution** | Macros can pollute global namespace | Macros respect module system and hygiene |
| **Conditional Compilation** | `#ifdef`, `#ifndef`, `#if defined` | `cfg!` macro, `#[cfg]` attributes |
| **Recursion** | Template recursion with specialization | Macro recursion with pattern matching |
| **Safety Guarantees** | No safety guarantees in macros | Hygiene and type safety preserved |
| **Learning Curve** | High (especially template metaprogramming) | Moderate (declarative), High (procedural) |
| **Performance** | Zero-cost abstractions, compile-time heavy | Zero-cost abstractions, generally faster compilation |

## Key Takeaways

**C++ Strengths:**
- Extremely powerful template metaprogramming
- `constexpr` allows complex compile-time computation
- Mature ecosystem with decades of metaprogramming patterns
- Can achieve almost anything at compile time

**C++ Weaknesses:**
- Preprocessor macros are dangerous and error-prone
- Non-hygienic macros lead to subtle bugs
- Template error messages can be incomprehensible
- Long compilation times for heavy template usage
- Multiple evaluation bugs are common

**Rust Strengths:**
- Hygienic macros prevent entire classes of bugs
- Clear separation between declarative and procedural macros
- Excellent error messages with span information
- Type safety preserved through macro expansion
- Modern design avoids legacy macro pitfalls

**Rust Weaknesses:**
- Procedural macros require separate crate setup
- Learning curve for macro syntax
- Less mature ecosystem compared to C++ templates
- Some compile-time computations require workarounds

Both languages provide powerful metaprogramming, but Rust's approach prioritizes safety and predictability, while C++ offers maximum flexibility at the cost of complexity and potential pitfalls.