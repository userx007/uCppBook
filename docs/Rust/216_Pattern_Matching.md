# Pattern Matching in C++ vs. Rust

Pattern matching is a powerful programming language feature that allows you to check a value against a pattern and execute code based on which pattern matches. While C++ has evolved to include some pattern matching capabilities, Rust's pattern matching system is far more comprehensive and central to the language design.

## C++ Pattern Matching

C++ provides several mechanisms for pattern matching, though they are more limited and less integrated than Rust's approach.

### Traditional Switch Statements

The classic C++ `switch` statement works only with integral types and enums:

```cpp
#include <iostream>
#include <string>

enum class Color { Red, Green, Blue };

void processColor(Color color) {
    switch (color) {
        case Color::Red:
            std::cout << "Red color\n";
            break;
        case Color::Green:
            std::cout << "Green color\n";
            break;
        case Color::Blue:
            std::cout << "Blue color\n";
            break;
        // No exhaustiveness checking - forgetting a case won't cause error
    }
}
```

### Structured Bindings (C++17)

Structured bindings allow decomposing tuple-like objects:

```cpp
#include <tuple>
#include <map>
#include <string>
#include <iostream>

struct Point {
    int x, y;
};

void structuredBindingExamples() {
    // With structs
    Point p{10, 20};
    auto [x, y] = p;
    std::cout << "x: " << x << ", y: " << y << "\n";
    
    // With pairs/tuples
    std::pair<int, std::string> data{42, "answer"};
    auto [num, str] = data;
    
    // With maps
    std::map<std::string, int> ages{{"Alice", 30}, {"Bob", 25}};
    for (const auto& [name, age] : ages) {
        std::cout << name << " is " << age << " years old\n";
    }
}
```

### std::visit with std::variant (C++17)

The most sophisticated pattern matching in C++ uses `std::variant` with `std::visit`:

```cpp
#include <variant>
#include <iostream>
#include <string>

// Define a variant type
using Value = std::variant<int, double, std::string>;

// Visitor using overloaded lambdas
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

void processValue(const Value& v) {
    std::visit(overloaded{
        [](int i) { std::cout << "Integer: " << i << "\n"; },
        [](double d) { std::cout << "Double: " << d << "\n"; },
        [](const std::string& s) { std::cout << "String: " << s << "\n"; }
    }, v);
}

// More complex example with nested variants
struct Circle { double radius; };
struct Rectangle { double width, height; };
struct Triangle { double base, height; };

using Shape = std::variant<Circle, Rectangle, Triangle>;

double calculateArea(const Shape& shape) {
    return std::visit(overloaded{
        [](const Circle& c) { return 3.14159 * c.radius * c.radius; },
        [](const Rectangle& r) { return r.width * r.height; },
        [](const Triangle& t) { return 0.5 * t.base * t.height; }
    }, shape);
}

int main() {
    Value v1 = 42;
    Value v2 = 3.14;
    Value v3 = std::string("hello");
    
    processValue(v1);
    processValue(v2);
    processValue(v3);
    
    Shape circle = Circle{5.0};
    Shape rect = Rectangle{4.0, 6.0};
    
    std::cout << "Circle area: " << calculateArea(circle) << "\n";
    std::cout << "Rectangle area: " << calculateArea(rect) << "\n";
}
```

## Rust Pattern Matching

Rust's pattern matching is built into the language at a fundamental level through the `match` expression, providing exhaustiveness checking, destructuring, and guards.

### Basic Match Expressions

```rust
enum Color {
    Red,
    Green,
    Blue,
}

fn process_color(color: Color) {
    match color {
        Color::Red => println!("Red color"),
        Color::Green => println!("Green color"),
        Color::Blue => println!("Blue color"),
        // Compiler enforces exhaustiveness - must handle all cases
    }
}

// Match with values
fn describe_number(n: i32) {
    match n {
        0 => println!("Zero"),
        1 | 2 => println!("One or two"),
        3..=9 => println!("Between 3 and 9"),
        _ => println!("Something else"),
    }
}
```

### Destructuring with Match

Rust allows powerful destructuring in match patterns:

```rust
struct Point {
    x: i32,
    y: i32,
}

fn process_point(point: Point) {
    match point {
        Point { x: 0, y: 0 } => println!("Origin"),
        Point { x: 0, y } => println!("On Y-axis at {}", y),
        Point { x, y: 0 } => println!("On X-axis at {}", x),
        Point { x, y } => println!("At ({}, {})", x, y),
    }
}

// Tuple destructuring
fn process_tuple(t: (i32, i32, i32)) {
    match t {
        (0, y, z) => println!("First is zero, y: {}, z: {}", y, z),
        (x, 0, z) => println!("Second is zero, x: {}, z: {}", x, z),
        (x, y, 0) => println!("Third is zero, x: {}, y: {}", x, y),
        _ => println!("All non-zero"),
    }
}
```

### Enum Matching with Data

```rust
enum Message {
    Quit,
    Move { x: i32, y: i32 },
    Write(String),
    ChangeColor(u8, u8, u8),
}

fn process_message(msg: Message) {
    match msg {
        Message::Quit => println!("Quit message"),
        Message::Move { x, y } => println!("Move to ({}, {})", x, y),
        Message::Write(text) => println!("Text: {}", text),
        Message::ChangeColor(r, g, b) => println!("Change color to RGB({}, {}, {})", r, g, b),
    }
}
```

### Match Guards and Advanced Patterns

```rust
fn categorize_number(n: i32) {
    match n {
        x if x < 0 => println!("Negative: {}", x),
        x if x % 2 == 0 => println!("Positive even: {}", x),
        x => println!("Positive odd: {}", x),
    }
}

// Nested patterns
enum Option<T> {
    Some(T),
    None,
}

fn process_nested(opt: Option<Option<i32>>) {
    match opt {
        Option::Some(Option::Some(x)) => println!("Nested value: {}", x),
        Option::Some(Option::None) => println!("Inner None"),
        Option::None => println!("Outer None"),
    }
}

// Reference patterns
fn process_reference(opt: &Option<i32>) {
    match opt {
        Some(x) => println!("Has value: {}", x),
        None => println!("No value"),
    }
}
```

### If Let and While Let

Rust provides convenient syntax for matching single patterns:

```rust
fn if_let_example(value: Option<i32>) {
    // Instead of full match for single case
    if let Some(x) = value {
        println!("Got value: {}", x);
    } else {
        println!("No value");
    }
}

fn while_let_example() {
    let mut stack = vec![1, 2, 3];
    
    while let Some(top) = stack.pop() {
        println!("Popped: {}", top);
    }
}
```

### Complex Shape Example in Rust

```rust
enum Shape {
    Circle { radius: f64 },
    Rectangle { width: f64, height: f64 },
    Triangle { base: f64, height: f64 },
}

fn calculate_area(shape: &Shape) -> f64 {
    match shape {
        Shape::Circle { radius } => std::f64::consts::PI * radius * radius,
        Shape::Rectangle { width, height } => width * height,
        Shape::Triangle { base, height } => 0.5 * base * height,
    }
}

fn describe_shape(shape: &Shape) -> String {
    match shape {
        Shape::Circle { radius } if *radius > 10.0 => {
            format!("Large circle with radius {}", radius)
        }
        Shape::Circle { radius } => {
            format!("Small circle with radius {}", radius)
        }
        Shape::Rectangle { width, height } if width == height => {
            format!("Square with side {}", width)
        }
        Shape::Rectangle { width, height } => {
            format!("Rectangle {}x{}", width, height)
        }
        Shape::Triangle { base, height } => {
            format!("Triangle with base {} and height {}", base, height)
        }
    }
}

fn main() {
    let shapes = vec![
        Shape::Circle { radius: 5.0 },
        Shape::Rectangle { width: 4.0, height: 4.0 },
        Shape::Triangle { base: 3.0, height: 6.0 },
    ];
    
    for shape in &shapes {
        println!("{}: area = {}", 
                 describe_shape(shape), 
                 calculate_area(shape));
    }
}
```

## Comparison Summary

| Feature | C++ | Rust | Winner |
|---------|-----|------|--------|
| **Basic Switch/Match** | `switch` for integral types only | `match` for all types | Rust |
| **Exhaustiveness Checking** | None (warnings with `-Wall`) | Compiler-enforced | Rust |
| **Destructuring** | Structured bindings (limited) | Full destructuring in patterns | Rust |
| **Type Variants** | `std::variant` with `std::visit` | `enum` with `match` | Rust |
| **Pattern Guards** | Not available | `if` guards in patterns | Rust |
| **Nested Patterns** | Not supported | Fully supported | Rust |
| **Range Patterns** | Not available | Supported (`1..=10`) | Rust |
| **Reference Patterns** | Manual dereferencing needed | Built-in syntax | Rust |
| **Syntax Ergonomics** | Verbose (visitor pattern) | Concise and readable | Rust |
| **Compile-Time Safety** | Runtime errors possible | Guaranteed exhaustiveness | Rust |
| **Or Patterns** | Not available | Supported (`1 | 2 | 3`) | Rust |
| **If Let/While Let** | Not available | Convenient shortcuts | Rust |
| **Performance** | Zero-cost with variants | Zero-cost | Tie |
| **Learning Curve** | Complex (visitor pattern) | Intuitive | Rust |

## Key Advantages of Rust's Pattern Matching

**Exhaustiveness Checking**: The Rust compiler guarantees you handle all possible cases, preventing bugs at compile time. In C++, forgetting a case leads to runtime issues.

**Integrated Design**: Pattern matching is central to Rust's design philosophy, making it natural and consistent across the language. C++'s solutions feel bolted on.

**Destructuring Power**: Rust allows deep destructuring of complex data structures in match arms, while C++ requires manual decomposition or complex visitor patterns.

**Guards and Conditions**: Rust's match guards allow inline conditions without breaking the pattern matching flow, something C++ cannot easily replicate.

**Ergonomics**: Rust's syntax is significantly cleaner and more readable, especially compared to C++'s verbose `std::visit` with lambda overloading.

**Safety**: The combination of exhaustiveness checking, move semantics integration, and borrow checker interaction makes Rust's pattern matching not just powerful but safe by default.

While C++ has made strides with C++17's structured bindings and variants, Rust's pattern matching remains significantly more powerful, safer, and more ergonomic. It's one of Rust's standout features that genuinely improves code quality and developer experience.