# C++ initialization methods

## Key Topics Covered:

**1. Copy Initialization** (`T obj = value;`)
- Uses assignment-like syntax
- May involve implicit conversions
- Cannot use explicit constructors

**2. Direct Initialization** (`T obj(args);`)
- Directly calls constructors
- Works with explicit constructors
- Traditional parentheses syntax

**3. List Initialization** (`T obj{args};`)
- Brace initialization introduced in C++11
- Prevents dangerous narrowing conversions
- Safer and more modern

**4. Uniform Initialization**
- Consistent brace syntax `{}` for all types
- Solves the "most vexing parse" problem
- Preferred in modern C++

**5. Aggregate Initialization**
- For simple structs/classes without constructors
- Direct member initialization
- Supports nested structures

## Key Differences:

The code demonstrates important distinctions like:
- `std::vector<int> v(10)` creates 10 elements
- `std::vector<int> v{10}` creates 1 element with value 10
- Uniform initialization prevents narrowing: `int x{3.14}` won't compile
- Direct initialization allows explicit constructors where copy init doesn't

```cpp
/*
 * C++ INITIALIZATION METHODS - COMPREHENSIVE GUIDE
 * ================================================
 */

#include <iostream>
#include <string>
#include <vector>

// =============================================================================
// 1. COPY INITIALIZATION
// =============================================================================
// Syntax: T obj = value;
// Uses copy constructor or conversion constructor
// May involve implicit conversions

void copy_initialization_examples() {
    std::cout << "=== COPY INITIALIZATION ===\n";
    
    // Basic types
    int x = 5;                    // Copy initialization with literal
    double d = 3.14;              // Copy initialization
    
    // With classes
    std::string s = "hello";      // Implicit conversion from const char*
    std::vector<int> v = {1,2,3}; // Copy list-initialization
    
    // Note: Copy initialization doesn't allow explicit constructors
    // This would NOT compile if constructor is marked explicit:
    // std::vector<int> v2 = 10;  // Error if explicit constructor
}

// =============================================================================
// 2. DIRECT INITIALIZATION
// =============================================================================
// Syntax: T obj(args);
// Calls constructor directly without copy
// Allows explicit constructors

void direct_initialization_examples() {
    std::cout << "\n=== DIRECT INITIALIZATION ===\n";
    
    // Basic types
    int x(5);                     // Direct initialization
    double d(3.14);
    
    // With classes
    std::string s("hello");       // Direct call to constructor
    std::vector<int> v(10);       // Creates vector with 10 elements
    
    // Works with explicit constructors
    std::vector<int> v2(5, 100);  // 5 elements, each initialized to 100
    
    std::cout << "Vector size: " << v.size() << "\n";
}

// =============================================================================
// 3. LIST INITIALIZATION (C++11)
// =============================================================================
// Syntax: T obj{args}; or T obj = {args};
// Also called brace initialization
// Prevents narrowing conversions

void list_initialization_examples() {
    std::cout << "\n=== LIST INITIALIZATION ===\n";
    
    // Direct list initialization
    int x{5};
    double d{3.14};
    
    // Copy list initialization
    int y = {10};
    
    // Prevents narrowing (compile error)
    // int narrow{3.14};  // ERROR: narrowing conversion
    
    // With containers
    std::vector<int> v{1, 2, 3, 4, 5};  // Initializer list
    
    // Empty initialization
    std::string empty{};  // Default constructed
    
    for(int val : v) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}

// =============================================================================
// 4. UNIFORM INITIALIZATION (C++11)
// =============================================================================
// Using braces {} for consistent initialization syntax
// Works for all types: built-in, aggregate, class types
// Prevents narrowing and most-vexing-parse problem

void uniform_initialization_examples() {
    std::cout << "\n=== UNIFORM INITIALIZATION ===\n";
    
    // Built-in types
    int a{42};
    double b{3.14};
    char c{'A'};
    
    // Arrays
    int arr[]{1, 2, 3, 4, 5};
    
    // Containers
    std::vector<int> vec{10, 20, 30};
    
    // Prevents narrowing
    // int narrow{3.14};  // Compile error
    
    // Solves "most vexing parse"
    std::vector<int> v1();    // Function declaration (vexing parse)
    std::vector<int> v2{};    // Empty vector initialization (uniform)
    
    std::cout << "Uniform init works for all types consistently\n";
}

// =============================================================================
// 5. AGGREGATE INITIALIZATION
// =============================================================================
// For aggregate types: arrays, structs/classes with:
// - No private/protected non-static data members
// - No user-declared constructors
// - No base classes (C++17: can have base classes)
// - No virtual functions

struct Point {
    int x;
    int y;
};

struct Rectangle {
    Point topLeft;
    Point bottomRight;
    std::string name;
};

void aggregate_initialization_examples() {
    std::cout << "\n=== AGGREGATE INITIALIZATION ===\n";
    
    // C-style aggregate initialization
    Point p1 = {10, 20};
    
    // C++11 uniform syntax
    Point p2{30, 40};
    
    // Nested aggregates
    Rectangle rect = {{0, 0}, {100, 200}, "MyRect"};
    
    // Designated initializers (C++20)
    // Point p3{.x = 5, .y = 10};
    
    std::cout << "Point: (" << p1.x << ", " << p1.y << ")\n";
    std::cout << "Rectangle: " << rect.name << "\n";
    
    // Arrays (aggregates)
    int numbers[] = {1, 2, 3, 4, 5};
    int matrix[][3] = {{1,2,3}, {4,5,6}};
}

// =============================================================================
// 6. COMPARISON AND BEST PRACTICES
// =============================================================================

class MyClass {
private:
    int value;
    std::string name;
    
public:
    // Regular constructor
    MyClass(int v, std::string n) : value(v), name(n) {
        std::cout << "Constructor called: " << name << "\n";
    }
    
    // Explicit constructor (cannot be used with copy initialization)
    explicit MyClass(int v) : value(v), name("unnamed") {
        std::cout << "Explicit constructor called\n";
    }
    
    void display() const {
        std::cout << "Value: " << value << ", Name: " << name << "\n";
    }
};

void comparison_examples() {
    std::cout << "\n=== COMPARISON OF METHODS ===\n";
    
    // Copy initialization
    MyClass obj1 = MyClass(10, "copy");  // May optimize away copy
    
    // Direct initialization
    MyClass obj2(20, "direct");
    
    // Uniform initialization (preferred modern C++)
    MyClass obj3{30, "uniform"};
    
    // This would NOT work (explicit constructor)
    // MyClass obj4 = MyClass(40);  // Error with explicit constructor
    MyClass obj5(50);  // OK - direct initialization with explicit
    MyClass obj6{60};  // OK - list initialization with explicit
    
    std::cout << "\n";
    obj1.display();
    obj2.display();
    obj3.display();
}

// =============================================================================
// KEY DIFFERENCES SUMMARY
// =============================================================================

/*
 * COPY INITIALIZATION (T obj = value;)
 * - May involve implicit conversions
 * - Cannot use explicit constructors
 * - Traditional C++ syntax
 * - May be optimized (copy elision)
 *
 * DIRECT INITIALIZATION (T obj(args);)
 * - Directly calls constructor
 * - Can use explicit constructors
 * - No implicit conversions
 * - Suffers from "most vexing parse"
 *
 * LIST INITIALIZATION (T obj{args};)
 * - Prevents narrowing conversions (safer)
 * - Can use explicit constructors
 * - Works with initializer lists
 * - Modern C++ preferred syntax
 *
 * UNIFORM INITIALIZATION (using {} consistently)
 * - One syntax for all types
 * - Prevents narrowing
 * - Prevents most-vexing-parse
 * - Preferred in modern C++
 *
 * AGGREGATE INITIALIZATION
 * - For simple aggregate types
 * - Direct member initialization
 * - No constructors involved
 * - Supports nested initialization
 *
 * BEST PRACTICES:
 * 1. Prefer uniform initialization {} for consistency
 * 2. Use () when you need to avoid initializer_list constructor
 * 3. Use = for simple copy semantics
 * 4. Mark single-argument constructors as explicit
 * 5. Use auto with brace initialization carefully
 */

// =============================================================================
// COMMON PITFALLS
// =============================================================================

void pitfall_examples() {
    std::cout << "\n=== COMMON PITFALLS ===\n";
    
    // Pitfall 1: Most vexing parse
    // std::vector<int> v();  // Function declaration, not object!
    std::vector<int> v{};     // Correct: empty vector
    
    // Pitfall 2: Narrowing with copy/direct init
    int i1 = 3.14;   // OK but truncates (warning)
    int i2(3.14);    // OK but truncates (warning)
    // int i3{3.14}; // ERROR: narrowing conversion not allowed
    
    // Pitfall 3: Initializer list vs size constructor
    std::vector<int> v1(10);    // 10 elements with value 0
    std::vector<int> v2{10};    // 1 element with value 10
    
    std::cout << "v1 size: " << v1.size() << "\n";
    std::cout << "v2 size: " << v2.size() << "\n";
    
    // Pitfall 4: auto with braces
    auto a = {1, 2, 3};   // std::initializer_list<int>
    auto b{42};           // int (C++17), initializer_list (C++11/14)
}

// =============================================================================
// MAIN FUNCTION
// =============================================================================

int main() {
    copy_initialization_examples();
    direct_initialization_examples();
    list_initialization_examples();
    uniform_initialization_examples();
    aggregate_initialization_examples();
    comparison_examples();
    pitfall_examples();
    
    return 0;
}
```