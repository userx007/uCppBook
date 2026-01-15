# Move Semantics & Ownership: C++ vs Rust

## Introduction

Move semantics and ownership are fundamental concepts that determine how both languages manage resources, prevent memory leaks, and ensure memory safety. While C++ introduced move semantics in C++11 as an optimization feature, Rust built its entire memory model around ownership from the ground up.

## C++ Move Semantics

### The Problem Move Semantics Solves

Before C++11, copying large objects was expensive. Consider this pre-C++11 scenario:

```cpp
std::vector<int> createLargeVector() {
    std::vector<int> v(1000000);
    // ... fill vector
    return v; // Expensive copy!
}
```

### Move Constructors and Move Assignment

C++ introduced move semantics to transfer ownership of resources without copying:

```cpp
#include <iostream>
#include <vector>
#include <string>

class Resource {
private:
    int* data;
    size_t size;

public:
    // Constructor
    Resource(size_t s) : size(s), data(new int[s]) {
        std::cout << "Constructor: allocated " << size << " ints\n";
    }

    // Copy constructor (expensive)
    Resource(const Resource& other) : size(other.size), data(new int[other.size]) {
        std::cout << "Copy constructor: deep copy of " << size << " ints\n";
        std::copy(other.data, other.data + size, data);
    }

    // Move constructor (cheap)
    Resource(Resource&& other) noexcept : size(other.size), data(other.data) {
        std::cout << "Move constructor: transferred ownership\n";
        other.data = nullptr;
        other.size = 0;
    }

    // Copy assignment
    Resource& operator=(const Resource& other) {
        std::cout << "Copy assignment\n";
        if (this != &other) {
            delete[] data;
            size = other.size;
            data = new int[size];
            std::copy(other.data, other.data + size, data);
        }
        return *this;
    }

    // Move assignment
    Resource& operator=(Resource&& other) noexcept {
        std::cout << "Move assignment\n";
        if (this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
        }
        return *this;
    }

    ~Resource() {
        std::cout << "Destructor: freeing " << size << " ints\n";
        delete[] data;
    }
};

int main() {
    Resource r1(100);
    Resource r2 = std::move(r1); // Move constructor
    
    Resource r3(50);
    r3 = std::move(r2); // Move assignment
    
    // r1 and r2 are now in valid but unspecified state
    return 0;
}
```

### std::move and Perfect Forwarding

`std::move` is a cast that converts an lvalue to an rvalue reference:

```cpp
#include <iostream>
#include <vector>
#include <string>

void process(std::string&& s) {
    std::cout << "Processing rvalue: " << s << "\n";
}

void process(const std::string& s) {
    std::cout << "Processing lvalue: " << s << "\n";
}

template<typename T>
void wrapper(T&& arg) {
    // Perfect forwarding
    process(std::forward<T>(arg));
}

int main() {
    std::string str = "Hello";
    
    process(str);              // Calls lvalue version
    process(std::move(str));   // Calls rvalue version
    
    wrapper(str);              // Forwards as lvalue
    wrapper(std::string("World")); // Forwards as rvalue
    
    return 0;
}
```

### Copy Elision and RVO

C++ compilers can optimize away copies through Return Value Optimization (RVO):

```cpp
#include <iostream>

class Widget {
public:
    Widget() { std::cout << "Constructor\n"; }
    Widget(const Widget&) { std::cout << "Copy constructor\n"; }
    Widget(Widget&&) noexcept { std::cout << "Move constructor\n"; }
};

Widget createWidget() {
    return Widget(); // RVO: no copy, no move
}

Widget createConditional(bool flag) {
    Widget w1, w2;
    return flag ? w1 : w2; // NRVO may not apply, might move
}

int main() {
    Widget w = createWidget(); // Direct construction, no moves
    return 0;
}
```

## Rust Ownership System

### Ownership Rules

Rust enforces three ownership rules at compile time:

1. Each value has a single owner
2. When the owner goes out of scope, the value is dropped
3. Values are moved by default (not copied)

```rust
fn main() {
    let s1 = String::from("hello");
    let s2 = s1; // s1 is moved to s2
    
    // println!("{}", s1); // ERROR: value borrowed after move
    println!("{}", s2); // OK
    
    let s3 = s2;
    // s2 is now invalid, s3 owns the string
}
```

### Moves by Default

Unlike C++, Rust moves by default for types that don't implement `Copy`:

```rust
#[derive(Debug)]
struct Resource {
    data: Vec<i32>,
    name: String,
}

impl Resource {
    fn new(name: &str, size: usize) -> Self {
        println!("Creating resource: {}", name);
        Resource {
            data: vec![0; size],
            name: name.to_string(),
        }
    }
}

impl Drop for Resource {
    fn drop(&mut self) {
        println!("Dropping resource: {}", self.name);
    }
}

fn take_ownership(r: Resource) {
    println!("Took ownership of: {}", r.name);
    // r is dropped here
}

fn main() {
    let r1 = Resource::new("R1", 100);
    take_ownership(r1); // r1 is moved
    
    // println!("{:?}", r1); // ERROR: value borrowed after move
    
    let r2 = Resource::new("R2", 50);
    let r3 = r2; // r2 is moved to r3
    
    // r2 is invalid here, r3 owns the resource
    println!("Active resource: {}", r3.name);
}
```

### The Copy Trait

Types that implement `Copy` are duplicated instead of moved:

```rust
fn main() {
    // Primitive types implement Copy
    let x = 5;
    let y = x; // x is copied, not moved
    println!("x = {}, y = {}", x, y); // Both are valid
    
    // Custom type with Copy
    #[derive(Copy, Clone, Debug)]
    struct Point {
        x: i32,
        y: i32,
    }
    
    let p1 = Point { x: 10, y: 20 };
    let p2 = p1; // p1 is copied
    println!("p1: {:?}, p2: {:?}", p1, p2); // Both valid
    
    // Types with heap allocation cannot implement Copy
    // #[derive(Copy, Clone)] // ERROR!
    #[derive(Clone, Debug)]
    struct BoxedPoint {
        data: Box<i32>,
    }
    
    let bp1 = BoxedPoint { data: Box::new(42) };
    let bp2 = bp1; // bp1 is moved, not copied
    // println!("{:?}", bp1); // ERROR
}
```

### Borrowing: References Without Ownership Transfer

Rust allows borrowing data without transferring ownership:

```rust
fn main() {
    let s1 = String::from("hello");
    
    // Immutable borrow
    let len = calculate_length(&s1);
    println!("Length of '{}' is {}", s1, len); // s1 still valid
    
    // Mutable borrow
    let mut s2 = String::from("hello");
    change_string(&mut s2);
    println!("Modified: {}", s2);
}

fn calculate_length(s: &String) -> usize {
    s.len()
} // s goes out of scope but doesn't drop the String (no ownership)

fn change_string(s: &mut String) {
    s.push_str(", world");
}
```

### Borrowing Rules

Rust enforces these rules at compile time:

```rust
fn main() {
    let mut s = String::from("hello");
    
    // Rule 1: Multiple immutable borrows are OK
    let r1 = &s;
    let r2 = &s;
    println!("{} and {}", r1, r2);
    
    // Rule 2: Only one mutable borrow at a time
    let r3 = &mut s;
    // let r4 = &mut s; // ERROR: cannot borrow as mutable more than once
    r3.push_str(" world");
    
    // Rule 3: Cannot have mutable and immutable borrows simultaneously
    let r5 = &s;
    // let r6 = &mut s; // ERROR: cannot borrow as mutable
    println!("{}", r5);
    
    // Non-lexical lifetimes (NLL): borrows end when last used
    let r7 = &s;
    println!("{}", r7); // r7's lifetime ends here
    
    let r8 = &mut s; // OK: r7 is no longer used
    r8.push_str("!");
}
```

## Comparative Examples

### Example 1: Vector Operations

**C++:**
```cpp
#include <vector>
#include <iostream>

std::vector<int> create_vector() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    return v; // Move or RVO
}

void use_vector(std::vector<int> v) {
    // v is moved here, original is invalidated
    std::cout << "Size: " << v.size() << "\n";
}

int main() {
    std::vector<int> v1 = create_vector();
    
    // Explicit move needed to transfer ownership
    use_vector(std::move(v1));
    
    // v1 is in valid but unspecified state
    // Using it is undefined behavior (usually)
    std::cout << "v1 size after move: " << v1.size() << "\n"; // Might be 0
    
    return 0;
}
```

**Rust:**
```rust
fn create_vector() -> Vec<i32> {
    let v = vec![1, 2, 3, 4, 5];
    v // Automatically moved out
}

fn use_vector(v: Vec<i32>) {
    // v is moved here
    println!("Size: {}", v.len());
    // v is dropped here
}

fn main() {
    let v1 = create_vector();
    
    // Automatic move, no std::move needed
    use_vector(v1);
    
    // println!("{}", v1.len()); // ERROR: value borrowed after move
    
    // If we want to keep using it, we borrow instead
    let v2 = create_vector();
    use_vector_borrowed(&v2);
    println!("v2 still valid: {}", v2.len()); // OK
}

fn use_vector_borrowed(v: &Vec<i32>) {
    println!("Size: {}", v.len());
}
```

### Example 2: Smart Pointers

**C++:**
```cpp
#include <memory>
#include <iostream>

class Data {
public:
    int value;
    Data(int v) : value(v) { std::cout << "Created " << value << "\n"; }
    ~Data() { std::cout << "Destroyed " << value << "\n"; }
};

void take_ownership(std::unique_ptr<Data> ptr) {
    std::cout << "Took ownership: " << ptr->value << "\n";
} // ptr is destroyed here

int main() {
    auto ptr1 = std::make_unique<Data>(42);
    
    // Must explicitly move unique_ptr
    take_ownership(std::move(ptr1));
    
    if (!ptr1) {
        std::cout << "ptr1 is now null\n";
    }
    
    // Shared ownership
    auto shared1 = std::make_shared<Data>(100);
    auto shared2 = shared1; // Reference count increases
    std::cout << "Use count: " << shared1.use_count() << "\n";
    
    return 0;
}
```

**Rust:**
```rust
use std::rc::Rc;

struct Data {
    value: i32,
}

impl Data {
    fn new(v: i32) -> Self {
        println!("Created {}", v);
        Data { value: v }
    }
}

impl Drop for Data {
    fn drop(&mut self) {
        println!("Destroyed {}", self.value);
    }
}

fn take_ownership(data: Box<Data>) {
    println!("Took ownership: {}", data.value);
} // data is dropped here

fn main() {
    let boxed = Box::new(Data::new(42));
    
    // Automatic move, no explicit move needed
    take_ownership(boxed);
    
    // println!("{}", boxed.value); // ERROR: value borrowed after move
    
    // Shared ownership with Rc
    let shared1 = Rc::new(Data::new(100));
    let shared2 = Rc::clone(&shared1); // Reference count increases
    println!("Strong count: {}", Rc::strong_count(&shared1));
    
    // Both shared1 and shared2 can access the data
    println!("Values: {}, {}", shared1.value, shared2.value);
}
```

### Example 3: Preventing Use-After-Move

**C++:**
```cpp
#include <string>
#include <iostream>

int main() {
    std::string s1 = "Hello";
    std::string s2 = std::move(s1);
    
    // Using s1 here is undefined behavior
    // The compiler won't stop you!
    std::cout << s1 << "\n"; // Might work, might crash, might print garbage
    std::cout << s1.size() << "\n"; // Probably 0, but not guaranteed
    
    // This is dangerous and relies on programmer discipline
    return 0;
}
```

**Rust:**
```rust
fn main() {
    let s1 = String::from("Hello");
    let s2 = s1;
    
    // Using s1 here is a compile-time error
    // println!("{}", s1); // ERROR: borrow of moved value: `s1`
    
    println!("{}", s2); // OK
    
    // Rust prevents use-after-move at compile time!
}
```

## Summary Table

| Aspect | C++ | Rust |
|--------|-----|------|
| **Default Behavior** | Copy by default (if copyable) | Move by default (unless `Copy` trait) |
| **Move Syntax** | Explicit `std::move()` required | Automatic, implicit moves |
| **Move Safety** | Runtime concern, undefined behavior possible | Compile-time enforced, no use-after-move |
| **Copy Semantics** | Implicitly copyable if copy constructor exists | Must explicitly implement `Copy` trait |
| **Ownership Model** | Manual management, programmer responsibility | Enforced by compiler through ownership rules |
| **Multiple Ownership** | `shared_ptr`, reference counting at runtime | `Rc`/`Arc`, reference counting with compile-time checks |
| **Borrowed References** | Raw pointers and references, no safety guarantees | Borrowing with compile-time lifetime checking |
| **Mutable Aliasing** | Allowed, can lead to data races | Prevented at compile time (one mutable or many immutable) |
| **Performance** | Zero-cost abstraction with manual optimization | Zero-cost abstraction with automatic safety |
| **Compiler Optimizations** | RVO, NRVO, copy elision | Similar optimizations plus ownership analysis |
| **Learning Curve** | Must understand when to use `std::move` | Must understand ownership, borrowing, lifetimes |
| **Error Detection** | Runtime errors, debugging needed | Compile-time errors, immediate feedback |
| **Memory Safety** | Requires careful programming | Guaranteed by type system (in safe Rust) |
| **Use After Move** | Undefined behavior, valid but unspecified state | Compile-time error, prevented entirely |
| **Flexibility** | Can circumvent rules with casts and pointers | Cannot circumvent without `unsafe` blocks |

## Key Takeaways

**C++ Move Semantics:**
- Optimization feature added to improve performance
- Requires explicit use of `std::move` to indicate intent
- Compiler may optimize with RVO/NRVO
- After move, objects remain in valid but unspecified state
- Programmer must ensure moved-from objects aren't used incorrectly
- More flexible but more error-prone

**Rust Ownership:**
- Core language feature, not an optimization
- Moves happen automatically without explicit syntax
- Compiler enforces ownership rules at compile time
- After move, variables become completely invalid
- Impossible to use moved values due to borrow checker
- Less flexible but memory-safe by design

The fundamental difference is that C++ treats move semantics as a performance optimization that programmers must opt into and manage carefully, while Rust builds its entire memory model around ownership, making safety automatic but requiring developers to think differently about resource management from the start.