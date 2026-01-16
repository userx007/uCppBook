# Operator Overloading in C++ vs. Rust

## Overview

Operator overloading allows programmers to define custom behavior for operators (like `+`, `-`, `*`, etc.) when used with user-defined types. Both C++ and Rust support operator overloading, but their approaches reflect fundamentally different design philosophies: C++ emphasizes flexibility and programmer freedom, while Rust prioritizes safety, explicitness, and trait-based design.

## C++ Operator Overloading

### Design Philosophy

C++ adopts a **permissive and flexible** approach to operator overloading. The language allows:
- Overloading as **member functions** or **free functions** (non-member functions)
- Overloading almost any operator, including some unusual ones
- Asymmetric overloading (different types on left and right sides)
- Multiple overload versions for the same operator

### Syntax and Implementation

C++ provides three ways to overload operators:

**1. Member Functions:** The left operand becomes the implicit `this` parameter
**2. Free Functions:** Both operands are explicit parameters
**3. Friend Functions:** Free functions with access to private members

### C++ Examples

```cpp
#include <iostream>
#include <string>

// Example 1: Complex number class with member function overloading
class Complex {
private:
    double real, imag;

public:
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}

    // Member function: operator+ (binary)
    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }

    // Member function: operator+= (compound assignment)
    Complex& operator+=(const Complex& other) {
        real += other.real;
        imag += other.imag;
        return *this;
    }

    // Member function: unary operator-
    Complex operator-() const {
        return Complex(-real, -imag);
    }

    // Member function: operator==
    bool operator==(const Complex& other) const {
        return real == other.real && imag == other.imag;
    }

    // Member function: operator[] (for accessing components)
    double operator[](int index) const {
        return index == 0 ? real : imag;
    }

    void display() const {
        std::cout << real << " + " << imag << "i";
    }

    // Friend declaration for free function
    friend Complex operator*(const Complex& lhs, const Complex& rhs);
    friend std::ostream& operator<<(std::ostream& os, const Complex& c);
};

// Free function: operator* (demonstrates friend function)
Complex operator*(const Complex& lhs, const Complex& rhs) {
    return Complex(
        lhs.real * rhs.real - lhs.imag * rhs.imag,
        lhs.real * rhs.imag + lhs.imag * rhs.real
    );
}

// Free function: operator<< (stream insertion)
std::ostream& operator<<(std::ostream& os, const Complex& c) {
    os << c.real << " + " << c.imag << "i";
    return os;
}

// Example 2: Vector class with extensive overloading
class Vector3D {
private:
    double x, y, z;

public:
    Vector3D(double x = 0, double y = 0, double z = 0) : x(x), y(y), z(z) {}

    // Arithmetic operators
    Vector3D operator+(const Vector3D& v) const {
        return Vector3D(x + v.x, y + v.y, z + v.z);
    }

    Vector3D operator-(const Vector3D& v) const {
        return Vector3D(x - v.x, y - v.y, z - v.z);
    }

    // Scalar multiplication (member function)
    Vector3D operator*(double scalar) const {
        return Vector3D(x * scalar, y * scalar, z * scalar);
    }

    // Comparison operators
    bool operator<(const Vector3D& v) const {
        // Compare by magnitude
        return (x*x + y*y + z*z) < (v.x*v.x + v.y*v.y + v.z*v.z);
    }

    // Increment operator
    Vector3D& operator++() {  // Prefix
        ++x; ++y; ++z;
        return *this;
    }

    Vector3D operator++(int) {  // Postfix
        Vector3D temp = *this;
        ++(*this);
        return temp;
    }

    // Function call operator
    double operator()(int index) const {
        if (index == 0) return x;
        if (index == 1) return y;
        return z;
    }

    friend Vector3D operator*(double scalar, const Vector3D& v);
    friend std::ostream& operator<<(std::ostream& os, const Vector3D& v);
};

// Free function: reverse scalar multiplication (double * Vector3D)
Vector3D operator*(double scalar, const Vector3D& v) {
    return Vector3D(v.x * scalar, v.y * scalar, v.z * scalar);
}

// Free function: stream output
std::ostream& operator<<(std::ostream& os, const Vector3D& v) {
    os << "(" << v.x << ", " << v.y << ", " << v.z << ")";
    return os;
}

// Example 3: String class with diverse operator overloading
class MyString {
private:
    std::string data;

public:
    MyString(const std::string& s = "") : data(s) {}

    // Concatenation
    MyString operator+(const MyString& other) const {
        return MyString(data + other.data);
    }

    // Subscript operator (non-const version)
    char& operator[](size_t index) {
        return data[index];
    }

    // Subscript operator (const version)
    const char& operator[](size_t index) const {
        return data[index];
    }

    // Conversion operator (implicit conversion to std::string)
    operator std::string() const {
        return data;
    }

    // Boolean operator (check if string is non-empty)
    explicit operator bool() const {
        return !data.empty();
    }

    friend std::ostream& operator<<(std::ostream& os, const MyString& str);
};

std::ostream& operator<<(std::ostream& os, const MyString& str) {
    os << str.data;
    return os;
}

int main() {
    std::cout << "=== Complex Number Examples ===" << std::endl;
    Complex c1(3, 4);
    Complex c2(1, 2);
    
    Complex c3 = c1 + c2;      // Member function
    Complex c4 = c1 * c2;      // Friend function
    Complex c5 = -c1;          // Unary operator
    
    std::cout << "c1 = " << c1 << std::endl;
    std::cout << "c2 = " << c2 << std::endl;
    std::cout << "c1 + c2 = " << c3 << std::endl;
    std::cout << "c1 * c2 = " << c4 << std::endl;
    std::cout << "-c1 = " << c5 << std::endl;
    std::cout << "c1[0] = " << c1[0] << std::endl;

    std::cout << "\n=== Vector3D Examples ===" << std::endl;
    Vector3D v1(1, 2, 3);
    Vector3D v2(4, 5, 6);
    
    Vector3D v3 = v1 + v2;
    Vector3D v4 = v1 * 2;      // Member function
    Vector3D v5 = 3 * v2;      // Free function (reverse order)
    Vector3D v6 = ++v1;        // Prefix increment
    
    std::cout << "v1 = " << v1 << std::endl;
    std::cout << "v2 = " << v2 << std::endl;
    std::cout << "v1 + v2 = " << v3 << std::endl;
    std::cout << "v1 * 2 = " << v4 << std::endl;
    std::cout << "3 * v2 = " << v5 << std::endl;
    std::cout << "Function call v2(1) = " << v2(1) << std::endl;

    std::cout << "\n=== MyString Examples ===" << std::endl;
    MyString s1("Hello");
    MyString s2(" World");
    MyString s3 = s1 + s2;
    
    std::cout << "s3 = " << s3 << std::endl;
    std::cout << "s3[0] = " << s3[0] << std::endl;
    
    if (s3) {
        std::cout << "s3 is not empty" << std::endl;
    }

    return 0;
}
```

### Key C++ Features

**Member vs. Free Functions:**
- **Member functions:** Used when the operator naturally belongs to the class (e.g., `+=`, unary operators)
- **Free functions:** Used for symmetric operations or when the left operand is not of the class type (e.g., `2 * vector`)

**Extensive Overloading Options:**
- Arithmetic operators: `+`, `-`, `*`, `/`, `%`
- Comparison operators: `==`, `!=`, `<`, `>`, `<=`, `>=`
- Logical operators: `&&`, `||`, `!`
- Bitwise operators: `&`, `|`, `^`, `~`, `<<`, `>>`
- Assignment operators: `=`, `+=`, `-=`, `*=`, etc.
- Increment/decrement: `++`, `--` (both prefix and postfix)
- Subscript: `[]`
- Function call: `()`
- Member access: `->`, `->*`
- Memory management: `new`, `delete`, `new[]`, `delete[]`
- Conversion operators
- Comma operator: `,`

## Rust Operator Overloading

### Design Philosophy

Rust takes a **trait-based, explicit, and safety-focused** approach:
- All operator overloading is done through **traits** (interfaces)
- Operators must be explicitly imported from `std::ops`
- More restricted set of overloadable operators
- Emphasis on preventing confusion and ensuring clarity
- No implicit conversions

### Syntax and Implementation

In Rust, operators are overloaded by implementing specific traits from the `std::ops` module. Each operator corresponds to a trait with an associated method.

### Rust Examples

```rust
use std::ops::{Add, Sub, Mul, Neg, AddAssign, Index, IndexMut};
use std::fmt;

// Example 1: Complex number with trait-based overloading
#[derive(Debug, Clone, Copy, PartialEq)]
struct Complex {
    real: f64,
    imag: f64,
}

impl Complex {
    fn new(real: f64, imag: f64) -> Self {
        Complex { real, imag }
    }
}

// Implement Add trait for Complex + Complex
impl Add for Complex {
    type Output = Complex;

    fn add(self, other: Complex) -> Complex {
        Complex {
            real: self.real + other.real,
            imag: self.imag + other.imag,
        }
    }
}

// Implement AddAssign trait for +=
impl AddAssign for Complex {
    fn add_assign(&mut self, other: Complex) {
        self.real += other.real;
        self.imag += other.imag;
    }
}

// Implement Neg trait for unary -
impl Neg for Complex {
    type Output = Complex;

    fn neg(self) -> Complex {
        Complex {
            real: -self.real,
            imag: -self.imag,
        }
    }
}

// Implement Mul trait for Complex * Complex
impl Mul for Complex {
    type Output = Complex;

    fn mul(self, other: Complex) -> Complex {
        Complex {
            real: self.real * other.real - self.imag * other.imag,
            imag: self.real * other.imag + self.imag * other.real,
        }
    }
}

// Custom Display implementation
impl fmt::Display for Complex {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "{} + {}i", self.real, self.imag)
    }
}

// Example 2: Vector3D with multiple trait implementations
#[derive(Debug, Clone, Copy, PartialEq)]
struct Vector3D {
    x: f64,
    y: f64,
    z: f64,
}

impl Vector3D {
    fn new(x: f64, y: f64, z: f64) -> Self {
        Vector3D { x, y, z }
    }

    fn magnitude_squared(&self) -> f64 {
        self.x * self.x + self.y * self.y + self.z * self.z
    }
}

// Implement Add for Vector3D + Vector3D
impl Add for Vector3D {
    type Output = Vector3D;

    fn add(self, other: Vector3D) -> Vector3D {
        Vector3D {
            x: self.x + other.x,
            y: self.y + other.y,
            z: self.z + other.z,
        }
    }
}

// Implement Sub for Vector3D - Vector3D
impl Sub for Vector3D {
    type Output = Vector3D;

    fn sub(self, other: Vector3D) -> Vector3D {
        Vector3D {
            x: self.x - other.x,
            y: self.y - other.y,
            z: self.z - other.z,
        }
    }
}

// Implement Mul for Vector3D * f64 (scalar multiplication)
impl Mul<f64> for Vector3D {
    type Output = Vector3D;

    fn mul(self, scalar: f64) -> Vector3D {
        Vector3D {
            x: self.x * scalar,
            y: self.y * scalar,
            z: self.z * scalar,
        }
    }
}

// Implement Mul for f64 * Vector3D (reverse scalar multiplication)
impl Mul<Vector3D> for f64 {
    type Output = Vector3D;

    fn mul(self, vec: Vector3D) -> Vector3D {
        Vector3D {
            x: vec.x * self,
            y: vec.y * self,
            z: vec.z * self,
        }
    }
}

// Implement PartialOrd for comparison (by magnitude)
impl PartialOrd for Vector3D {
    fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
        self.magnitude_squared().partial_cmp(&other.magnitude_squared())
    }
}

impl fmt::Display for Vector3D {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        write!(f, "({}, {}, {})", self.x, self.y, self.z)
    }
}

// Example 3: Custom Matrix type with indexing
#[derive(Debug, Clone)]
struct Matrix {
    data: Vec<Vec<f64>>,
    rows: usize,
    cols: usize,
}

impl Matrix {
    fn new(rows: usize, cols: usize) -> Self {
        Matrix {
            data: vec![vec![0.0; cols]; rows],
            rows,
            cols,
        }
    }

    fn from_vec(data: Vec<Vec<f64>>) -> Self {
        let rows = data.len();
        let cols = if rows > 0 { data[0].len() } else { 0 };
        Matrix { data, rows, cols }
    }
}

// Implement Index trait for read access: matrix[i]
impl Index<usize> for Matrix {
    type Output = Vec<f64>;

    fn index(&self, index: usize) -> &Self::Output {
        &self.data[index]
    }
}

// Implement IndexMut trait for write access: matrix[i][j] = value
impl IndexMut<usize> for Matrix {
    fn index_mut(&mut self, index: usize) -> &mut Self::Output {
        &mut self.data[index]
    }
}

// Implement Add for Matrix + Matrix
impl Add for Matrix {
    type Output = Matrix;

    fn add(self, other: Matrix) -> Matrix {
        assert_eq!(self.rows, other.rows);
        assert_eq!(self.cols, other.cols);

        let mut result = Matrix::new(self.rows, self.cols);
        for i in 0..self.rows {
            for j in 0..self.cols {
                result.data[i][j] = self.data[i][j] + other.data[i][j];
            }
        }
        result
    }
}

// Implement Mul for Matrix * f64 (scalar multiplication)
impl Mul<f64> for Matrix {
    type Output = Matrix;

    fn mul(self, scalar: f64) -> Matrix {
        let mut result = Matrix::new(self.rows, self.cols);
        for i in 0..self.rows {
            for j in 0..self.cols {
                result.data[i][j] = self.data[i][j] * scalar;
            }
        }
        result
    }
}

impl fmt::Display for Matrix {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        for row in &self.data {
            writeln!(f, "{:?}", row)?;
        }
        Ok(())
    }
}

fn main() {
    println!("=== Complex Number Examples ===");
    let c1 = Complex::new(3.0, 4.0);
    let c2 = Complex::new(1.0, 2.0);
    
    let c3 = c1 + c2;           // Add trait
    let c4 = c1 * c2;           // Mul trait
    let c5 = -c1;               // Neg trait
    
    println!("c1 = {}", c1);
    println!("c2 = {}", c2);
    println!("c1 + c2 = {}", c3);
    println!("c1 * c2 = {}", c4);
    println!("-c1 = {}", c5);
    
    let mut c6 = Complex::new(5.0, 6.0);
    c6 += c2;                   // AddAssign trait
    println!("c6 after += c2: {}", c6);

    println!("\n=== Vector3D Examples ===");
    let v1 = Vector3D::new(1.0, 2.0, 3.0);
    let v2 = Vector3D::new(4.0, 5.0, 6.0);
    
    let v3 = v1 + v2;           // Add trait
    let v4 = v1 - v2;           // Sub trait
    let v5 = v1 * 2.0;          // Mul<f64> trait
    let v6 = 3.0 * v2;          // f64's Mul<Vector3D> trait
    
    println!("v1 = {}", v1);
    println!("v2 = {}", v2);
    println!("v1 + v2 = {}", v3);
    println!("v1 - v2 = {}", v4);
    println!("v1 * 2.0 = {}", v5);
    println!("3.0 * v2 = {}", v6);
    
    if v1 < v2 {
        println!("v1 has smaller magnitude than v2");
    }

    println!("\n=== Matrix Examples ===");
    let mut m1 = Matrix::from_vec(vec![
        vec![1.0, 2.0, 3.0],
        vec![4.0, 5.0, 6.0],
    ]);
    
    let m2 = Matrix::from_vec(vec![
        vec![7.0, 8.0, 9.0],
        vec![10.0, 11.0, 12.0],
    ]);
    
    println!("m1 =\n{}", m1);
    println!("m2 =\n{}", m2);
    
    // Using Index and IndexMut traits
    println!("m1[0][1] = {}", m1[0][1]);
    m1[1][2] = 99.0;
    println!("After m1[1][2] = 99.0:\n{}", m1);
    
    let m3 = m1 + m2;           // Add trait
    println!("m1 + m2 =\n{}", m3);
}
```

### Key Rust Features

**Trait-Based System:**
Each operator corresponds to a specific trait in `std::ops`:

- **Arithmetic:** `Add`, `Sub`, `Mul`, `Div`, `Rem`
- **Compound assignment:** `AddAssign`, `SubAssign`, `MulAssign`, etc.
- **Unary:** `Neg`, `Not`
- **Bitwise:** `BitAnd`, `BitOr`, `BitXor`, `Shl`, `Shr`
- **Comparison:** `PartialEq`, `Eq`, `PartialOrd`, `Ord`
- **Indexing:** `Index`, `IndexMut`
- **Dereference:** `Deref`, `DerefMut`

**Associated Types:**
Traits use associated types (like `type Output`) to specify the return type, providing type safety and flexibility.

**Generic Implementations:**
Rust allows implementing operators for different type combinations:
```rust
impl Mul<f64> for Vector3D { ... }  // Vector * scalar
impl Mul<Vector3D> for f64 { ... }  // scalar * Vector
```

## Design Philosophy Differences

### C++ Philosophy: Maximum Flexibility

**Advantages:**
- Extremely expressive and concise syntax
- Can overload almost any operator
- Allows implicit conversions through conversion operators
- Member and free function options provide design flexibility
- Can create highly intuitive domain-specific languages (DSLs)

**Disadvantages:**
- Easy to abuse and create confusing code
- Implicit conversions can lead to unexpected behavior
- No compile-time enforcement of semantic meaning
- Can hide expensive operations behind simple syntax
- Difficult to understand without seeing implementation

### Rust Philosophy: Explicit Safety and Clarity

**Advantages:**
- Trait system makes overloading explicit and discoverable
- No implicit conversions prevent surprise behavior
- Type safety enforced at compile time
- Clear separation between operations and types
- Easier to reason about code behavior

**Disadvantages:**
- More verbose trait implementations required
- Cannot overload all operators (e.g., `&&`, `||` for short-circuiting)
- Less syntactic flexibility compared to C++
- May require more boilerplate code
- Learning curve for trait system

### Specific Differences

**1. Conversion Operators:**
- **C++:** Supports implicit and explicit conversion operators
  ```cpp
  operator std::string() const { return data; }
  ```
- **Rust:** No conversion operators; use `From` and `Into` traits explicitly

**2. Assignment Operator:**
- **C++:** Can overload `operator=`
- **Rust:** Cannot overload assignment; use `Clone` trait instead

**3. Function Call Operator:**
- **C++:** Can overload `operator()` for functors
- **Rust:** Use `Fn`, `FnMut`, `FnOnce` traits for callable types

**4. Symmetry Handling:**
- **C++:** Requires separate free functions for symmetric operations
- **Rust:** Can implement trait on different types naturally

**5. Short-Circuit Operators:**
- **C++:** Can overload `&&` and `||` (loses short-circuit behavior)
- **Rust:** Cannot overload these to preserve short-circuit semantics

## Operator Overloading: C++ vs. Rust - Summary Tables

### Core Comparison

| Feature | C++ | Rust |
|---------|-----|------|
| **Implementation Method** | Member functions, free functions, friend functions | Trait-based only (from `std::ops`) |
| **Syntax Complexity** | Simple, direct operator definition | Requires trait implementation with associated types |
| **Number of Overloadable Operators** | Almost all operators (~40+) | Limited set (~20-25 main traits) |
| **Assignment Operator** | Can overload `operator=` | Cannot overload; use `Clone` trait |
| **Conversion Operators** | Implicit and explicit conversion operators | No conversion operators; use `From`/`Into` traits |
| **Function Call Operator** | `operator()` for functors | `Fn`, `FnMut`, `FnOnce` traits |
| **Indexing Operator** | `operator[]` (single parameter) | `Index` and `IndexMut` traits |
| **Increment/Decrement** | `operator++` and `operator--` (prefix/postfix) | Not available; manual implementation needed |
| **Logical Operators** | Can overload `&&` and `||` (loses short-circuit) | Cannot overload to preserve short-circuit behavior |
| **Return Type Flexibility** | Any return type allowed | Defined by associated `Output` type in trait |
| **Symmetry Handling** | Requires separate free functions (e.g., `2 * vec`) | Implement trait on different types naturally |
| **Const Correctness** | Explicit `const` qualification needed | Ownership system handles mutability |
| **Memory Safety** | Programmer responsibility | Enforced by borrow checker |
| **Discoverability** | Low; must read header files or documentation | High; traits are explicit and searchable |

### Operator Availability

| Operator Category | C++ | Rust |
|-------------------|-----|------|
| **Arithmetic** | `+`, `-`, `*`, `/`, `%` ✓ | `Add`, `Sub`, `Mul`, `Div`, `Rem` ✓ |
| **Compound Assignment** | `+=`, `-=`, `*=`, etc. ✓ | `AddAssign`, `SubAssign`, `MulAssign`, etc. ✓ |
| **Comparison** | `==`, `!=`, `<`, `>`, `<=`, `>=` ✓ | `PartialEq`, `Eq`, `PartialOrd`, `Ord` ✓ |
| **Bitwise** | `&`, `|`, `^`, `~`, `<<`, `>>` ✓ | `BitAnd`, `BitOr`, `BitXor`, `Not`, `Shl`, `Shr` ✓ |
| **Unary** | `-`, `+`, `!`, `~` ✓ | `Neg`, `Not` ✓ (`+` not overloadable) |
| **Increment/Decrement** | `++`, `--` (prefix/postfix) ✓ | ✗ Not available |
| **Subscript** | `[]` ✓ | `Index`, `IndexMut` ✓ |
| **Function Call** | `()` ✓ | `Fn`, `FnMut`, `FnOnce` ✓ |
| **Dereference** | `*`, `->` ✓ | `Deref`, `DerefMut` ✓ |
| **Logical Short-Circuit** | `&&`, `||` ✓ (loses short-circuit) | ✗ Not overloadable |
| **Member Access** | `->*`, `.*` ✓ | ✗ Not available |
| **Comma** | `,` ✓ | ✗ Not overloadable |
| **Memory Management** | `new`, `delete`, `new[]`, `delete[]` ✓ | ✗ Handled by allocator API |

### Design Philosophy

| Aspect | C++ | Rust |
|--------|-----|------|
| **Core Philosophy** | Maximum flexibility and expressiveness | Safety, explicitness, and clarity |
| **Implicit Behavior** | Embraces implicit conversions and operations | Avoids implicit behavior; requires explicit intent |
| **Compile-Time Checks** | Type checking; limited semantic verification | Strong type and lifetime checking; borrow checker |
| **Abuse Potential** | High; easy to create confusing code | Lower; trait system provides guardrails |
| **Learning Curve** | Easy to start, complex rules to master | Steeper initial curve; trait system must be understood |
| **Code Readability** | Can be very concise or very obscure | More verbose but clearer intent |

### Pros and Cons Summary

| Language | Advantages | Disadvantages |
|----------|------------|---------------|
| **C++** | ✓ Maximum flexibility<br>✓ Highly expressive syntax<br>✓ Can overload almost any operator<br>✓ Great for DSLs<br>✓ Minimal boilerplate | ✗ Easy to abuse<br>✗ Can hide expensive operations<br>✗ Implicit conversions can surprise<br>✗ Poor discoverability<br>✗ No semantic enforcement |
| **Rust** | ✓ Explicit and discoverable<br>✓ Type-safe with borrow checker<br>✓ Prevents common pitfalls<br>✓ Clear trait boundaries<br>✓ Better maintainability | ✗ More verbose<br>✗ Limited operator set<br>✗ Steeper learning curve<br>✗ More boilerplate required<br>✗ Less syntactic freedom |

## Key Takeaway

**C++** prioritizes *programmer freedom* and *expressiveness*, allowing nearly unlimited operator overloading with minimal restrictions. This enables elegant, domain-specific syntax but requires discipline to avoid abuse.

**Rust** prioritizes *safety*, *explicitness*, and *maintainability* through its trait-based system. While more restrictive and verbose, it prevents common pitfalls and makes code behavior more predictable and discoverable.

## Conclusion

The differences in operator overloading between C++ and Rust reflect their fundamental design philosophies. C++ empowers programmers with maximum flexibility to create elegant, expressive code but requires significant discipline to avoid creating unmaintainable or confusing systems. Rust constrains programmers through its trait system, trading some expressiveness for guaranteed safety, clarity, and maintainability.

**Choose C++ operator overloading when:**
- Building mathematical or scientific libraries requiring natural notation
- Creating domain-specific languages with intuitive syntax
- Working with legacy code that relies on extensive operator overloading
- Expressiveness and minimal syntax are top priorities

**Choose Rust operator overloading when:**
- Safety and correctness are paramount
- Code maintainability and clarity are priorities
- You want discoverable, self-documenting interfaces
- Building systems where unexpected behavior would be catastrophic

Both approaches are valid within their respective ecosystems, and understanding these differences helps developers write idiomatic code in each language while appreciating the trade-offs inherent in language design.