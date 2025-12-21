I'll create a comprehensive guide on CRTP with detailed examples and explanations.I've created a comprehensive guide on **CRTP (Curiously Recurring Template Pattern)** with 7 practical examples covering different use cases.

## What is CRTP?

CRTP is a C++ idiom where a class template takes a derived class as its template parameter:
```cpp
class Derived : public Base<Derived>
```

## Key Features Demonstrated:

1. **Basic Pattern** - Shows the fundamental structure
2. **Static Polymorphism** - Compile-time dispatch without virtual functions (Shape example)
3. **Mixin Classes** - Injecting functionality like `Printable` interface
4. **Method Chaining** - Fluent interfaces that return the derived type
5. **Object Counting** - Per-class instance tracking
6. **Operator Generation** - Implementing comparison operators from just `==` and `<`
7. **Performance Comparison** - CRTP vs virtual functions

## Why Use CRTP?

**Advantages:**
- Zero runtime overhead (no vtable lookups)
- Compile-time type safety
- Can add functionality to derived classes
- Perfect for performance-critical code

**Disadvantages:**
- Cannot use polymorphic base pointers
- More complex syntax
- Less flexible than runtime polymorphism

The guide includes a working `main()` function that demonstrates all patterns and detailed comments explaining when to use CRTP versus traditional virtual functions.

```cpp
/*
 * CRTP (Curiously Recurring Template Pattern)
 * A comprehensive guide with examples
 */

#include <iostream>
#include <string>
#include <cmath>

// ============================================================================
// 1. BASIC CRTP PATTERN
// ============================================================================
// The base class is a template that takes the derived class as a parameter

template <typename Derived>
class Base {
public:
    void interface() {
        // Call the derived class implementation through static polymorphism
        static_cast<Derived*>(this)->implementation();
    }
    
    // Default implementation (optional)
    void implementation() {
        std::cout << "Base implementation" << std::endl;
    }
};

class Derived1 : public Base<Derived1> {
public:
    void implementation() {
        std::cout << "Derived1 implementation" << std::endl;
    }
};

class Derived2 : public Base<Derived2> {
public:
    void implementation() {
        std::cout << "Derived2 implementation" << std::endl;
    }
};

// ============================================================================
// 2. STATIC POLYMORPHISM - Compile-time dispatch without virtual functions
// ============================================================================

template <typename Shape>
class ShapeBase {
public:
    double area() const {
        return static_cast<const Shape*>(this)->calculateArea();
    }
    
    void printInfo() const {
        std::cout << "Area: " << area() << std::endl;
    }
};

class Circle : public ShapeBase<Circle> {
    double radius;
public:
    Circle(double r) : radius(r) {}
    
    double calculateArea() const {
        return 3.14159 * radius * radius;
    }
};

class Rectangle : public ShapeBase<Rectangle> {
    double width, height;
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    
    double calculateArea() const {
        return width * height;
    }
};

// ============================================================================
// 3. ADDING FUNCTIONALITY TO DERIVED CLASSES
// ============================================================================
// CRTP can be used to inject common functionality

template <typename T>
class Printable {
public:
    void print() const {
        const T& derived = static_cast<const T&>(*this);
        std::cout << derived.toString() << std::endl;
    }
};

class Point : public Printable<Point> {
    int x, y;
public:
    Point(int x, int y) : x(x), y(y) {}
    
    std::string toString() const {
        return "Point(" + std::to_string(x) + ", " + std::to_string(y) + ")";
    }
};

class Vector : public Printable<Vector> {
    double x, y, z;
public:
    Vector(double x, double y, double z) : x(x), y(y), z(z) {}
    
    std::string toString() const {
        return "Vector(" + std::to_string(x) + ", " + 
               std::to_string(y) + ", " + std::to_string(z) + ")";
    }
};

// ============================================================================
// 4. METHOD CHAINING (FLUENT INTERFACE)
// ============================================================================

template <typename Derived>
class FluentBase {
public:
    Derived& setName(const std::string& name) {
        static_cast<Derived*>(this)->name = name;
        return static_cast<Derived&>(*this);
    }
    
    Derived& setId(int id) {
        static_cast<Derived*>(this)->id = id;
        return static_cast<Derived&>(*this);
    }
};

class Product : public FluentBase<Product> {
public:
    std::string name;
    int id;
    double price;
    
    Product& setPrice(double p) {
        price = p;
        return *this;
    }
    
    void display() const {
        std::cout << "Product: " << name << " (ID: " << id 
                  << ", Price: $" << price << ")" << std::endl;
    }
};

// ============================================================================
// 5. COMPILE-TIME COUNTER (Advanced usage)
// ============================================================================

template <typename Derived>
class Counted {
    static inline int count = 0;
    
protected:
    Counted() { ++count; }
    Counted(const Counted&) { ++count; }
    ~Counted() { --count; }
    
public:
    static int getCount() { return count; }
};

class Widget : public Counted<Widget> {
public:
    Widget() = default;
};

class Gadget : public Counted<Gadget> {
public:
    Gadget() = default;
};

// ============================================================================
// 6. COMPARISON OPERATORS MIXIN
// ============================================================================

template <typename T>
class Comparable {
public:
    bool operator!=(const T& other) const {
        return !(static_cast<const T&>(*this) == other);
    }
    
    bool operator>(const T& other) const {
        return other < static_cast<const T&>(*this);
    }
    
    bool operator<=(const T& other) const {
        return !(static_cast<const T&>(*this) > other);
    }
    
    bool operator>=(const T& other) const {
        return !(static_cast<const T&>(*this) < other);
    }
};

class Number : public Comparable<Number> {
    int value;
public:
    Number(int v) : value(v) {}
    
    // Only need to implement == and <
    bool operator==(const Number& other) const {
        return value == other.value;
    }
    
    bool operator<(const Number& other) const {
        return value < other.value;
    }
    
    int getValue() const { return value; }
};

// ============================================================================
// 7. PERFORMANCE COMPARISON: CRTP vs Virtual Functions
// ============================================================================

// Virtual function approach
class VirtualBase {
public:
    virtual int compute(int x) const = 0;
    virtual ~VirtualBase() = default;
};

class VirtualDerived : public VirtualBase {
public:
    int compute(int x) const override {
        return x * x;
    }
};

// CRTP approach
template <typename T>
class CRTPBase {
public:
    int compute(int x) const {
        return static_cast<const T*>(this)->computeImpl(x);
    }
};

class CRTPDerived : public CRTPBase<CRTPDerived> {
public:
    int computeImpl(int x) const {
        return x * x;
    }
};

// ============================================================================
// DEMONSTRATION
// ============================================================================

int main() {
    std::cout << "=== 1. Basic CRTP Pattern ===" << std::endl;
    Derived1 d1;
    Derived2 d2;
    d1.interface();
    d2.interface();
    
    std::cout << "\n=== 2. Static Polymorphism ===" << std::endl;
    Circle circle(5.0);
    Rectangle rect(4.0, 6.0);
    circle.printInfo();
    rect.printInfo();
    
    std::cout << "\n=== 3. Printable Mixin ===" << std::endl;
    Point p(10, 20);
    Vector v(1.5, 2.5, 3.5);
    p.print();
    v.print();
    
    std::cout << "\n=== 4. Method Chaining ===" << std::endl;
    Product product;
    product.setName("Laptop")
           .setId(101)
           .setPrice(999.99)
           .display();
    
    std::cout << "\n=== 5. Object Counting ===" << std::endl;
    {
        Widget w1, w2, w3;
        Gadget g1, g2;
        std::cout << "Widgets created: " << Widget::getCount() << std::endl;
        std::cout << "Gadgets created: " << Gadget::getCount() << std::endl;
    }
    std::cout << "After scope - Widgets: " << Widget::getCount() << std::endl;
    std::cout << "After scope - Gadgets: " << Gadget::getCount() << std::endl;
    
    std::cout << "\n=== 6. Comparison Operators ===" << std::endl;
    Number n1(10), n2(20), n3(10);
    std::cout << "n1 == n3: " << (n1 == n3) << std::endl;
    std::cout << "n1 != n2: " << (n1 != n2) << std::endl;
    std::cout << "n1 < n2: " << (n1 < n2) << std::endl;
    std::cout << "n1 >= n3: " << (n1 >= n3) << std::endl;
    
    std::cout << "\n=== 7. Performance Note ===" << std::endl;
    std::cout << "CRTP: No vtable lookup, resolved at compile-time" << std::endl;
    std::cout << "Virtual: Runtime dispatch through vtable" << std::endl;
    
    return 0;
}

/*
 * KEY CONCEPTS:
 * 
 * 1. WHAT IS CRTP?
 *    - Template pattern where a class inherits from a template base class
 *    - The derived class passes itself as a template parameter to the base
 *    - Syntax: class Derived : public Base<Derived>
 * 
 * 2. HOW IT WORKS:
 *    - Base class uses static_cast to call derived class methods
 *    - Type information is known at compile-time
 *    - No virtual function overhead
 * 
 * 3. ADVANTAGES:
 *    - Zero runtime overhead (no vtable)
 *    - Compile-time polymorphism
 *    - Can add functionality to derived classes
 *    - Type-safe without runtime checks
 * 
 * 4. DISADVANTAGES:
 *    - Cannot use base class pointers polymorphically
 *    - More complex template syntax
 *    - Code bloat (each derived class creates new template instantiation)
 *    - Compile-time only, no runtime flexibility
 * 
 * 5. COMMON USE CASES:
 *    - Static polymorphism for performance
 *    - Mixin classes (adding functionality)
 *    - Method chaining (fluent interfaces)
 *    - Counting instances per class
 *    - Operator generation
 * 
 * 6. WHEN TO USE:
 *    - Performance-critical code
 *    - When compile-time dispatch is sufficient
 *    - Adding common functionality to multiple classes
 *    - When you don't need runtime polymorphism
 * 
 * 7. WHEN NOT TO USE:
 *    - Need heterogeneous containers
 *    - Require runtime polymorphism
 *    - Code simplicity is more important than performance
 */
```