# Static Polymorphism in C++

Static polymorphism, also known as **compile-time polymorphism**, is a form of polymorphism where the function call is resolved at compile time rather than runtime. This contrasts with dynamic polymorphism (using virtual functions), which resolves calls at runtime.

## Main Mechanisms

Static polymorphism in C++ is achieved through:

1. **Function Overloading**
2. **Operator Overloading**
3. **Templates**
4. **CRTP (Curiously Recurring Template Pattern)**

Let me demonstrate each with detailed examples:

---

## 1. Function Overloading

Function overloading allows multiple functions with the same name but different parameters:---

```cpp
#include <iostream>
#include <string>
using namespace std;

class Calculator {
public:
    // Overloaded add functions for different types
    int add(int a, int b) {
        cout << "Adding two integers" << endl;
        return a + b;
    }
    
    double add(double a, double b) {
        cout << "Adding two doubles" << endl;
        return a + b;
    }
    
    string add(const string& a, const string& b) {
        cout << "Concatenating two strings" << endl;
        return a + b;
    }
    
    // Overloaded with different number of parameters
    int add(int a, int b, int c) {
        cout << "Adding three integers" << endl;
        return a + b + c;
    }
};

int main() {
    Calculator calc;
    
    // Compiler determines which function to call at compile time
    cout << calc.add(5, 3) << endl;                    // Calls int version
    cout << calc.add(5.5, 3.2) << endl;                // Calls double version
    cout << calc.add("Hello, ", "World!") << endl;     // Calls string version
    cout << calc.add(1, 2, 3) << endl;                 // Calls 3-parameter version
    
    return 0;
}
```

## 2. Operator Overloading

Operator overloading allows you to redefine operators for user-defined types:---

```cpp
#include <iostream>
using namespace std;

class Complex {
private:
    double real, imag;
    
public:
    Complex(double r = 0, double i = 0) : real(r), imag(i) {}
    
    // Overload + operator
    Complex operator+(const Complex& other) const {
        return Complex(real + other.real, imag + other.imag);
    }
    
    // Overload - operator
    Complex operator-(const Complex& other) const {
        return Complex(real - other.real, imag - other.imag);
    }
    
    // Overload * operator
    Complex operator*(const Complex& other) const {
        return Complex(
            real * other.real - imag * other.imag,
            real * other.imag + imag * other.real
        );
    }
    
    // Overload == operator
    bool operator==(const Complex& other) const {
        return (real == other.real && imag == other.imag);
    }
    
    // Overload << operator for output
    friend ostream& operator<<(ostream& os, const Complex& c) {
        os << c.real;
        if (c.imag >= 0) os << " + " << c.imag << "i";
        else os << " - " << -c.imag << "i";
        return os;
    }
};

int main() {
    Complex c1(3, 4);
    Complex c2(1, 2);
    
    // Using overloaded operators - resolved at compile time
    Complex c3 = c1 + c2;
    Complex c4 = c1 - c2;
    Complex c5 = c1 * c2;
    
    cout << "c1 = " << c1 << endl;
    cout << "c2 = " << c2 << endl;
    cout << "c1 + c2 = " << c3 << endl;
    cout << "c1 - c2 = " << c4 << endl;
    cout << "c1 * c2 = " << c5 << endl;
    cout << "c1 == c2: " << (c1 == c2) << endl;
    
    return 0;
}
```

## 3. Templates (Generic Programming)

Templates are the most powerful form of static polymorphism, allowing you to write code that works with any type:---

```cpp
#include <iostream>
#include <vector>
#include <string>
using namespace std;

// Template function - works with any type
template <typename T>
T maximum(T a, T b) {
    return (a > b) ? a : b;
}

// Template class
template <typename T>
class Container {
private:
    T value;
    
public:
    Container(T val) : value(val) {}
    
    T getValue() const { return value; }
    void setValue(T val) { value = val; }
    
    void print() const {
        cout << "Value: " << value << endl;
    }
};

// Template with multiple parameters
template <typename T, typename U>
class Pair {
private:
    T first;
    U second;
    
public:
    Pair(T f, U s) : first(f), second(s) {}
    
    void display() const {
        cout << "(" << first << ", " << second << ")" << endl;
    }
};

// Function template with template template parameter
template <typename T>
void printContainer(const vector<T>& vec) {
    cout << "Container contents: ";
    for (const auto& elem : vec) {
        cout << elem << " ";
    }
    cout << endl;
}

int main() {
    // Template functions - type resolved at compile time
    cout << "Max of 10 and 20: " << maximum(10, 20) << endl;
    cout << "Max of 3.5 and 2.1: " << maximum(3.5, 2.1) << endl;
    cout << "Max of 'a' and 'z': " << maximum('a', 'z') << endl;
    
    // Template classes
    Container<int> intContainer(42);
    Container<string> strContainer("Hello");
    Container<double> dblContainer(3.14);
    
    intContainer.print();
    strContainer.print();
    dblContainer.print();
    
    // Template with multiple types
    Pair<int, string> p1(1, "First");
    Pair<double, char> p2(3.14, 'A');
    
    p1.display();
    p2.display();
    
    // Vector with different types
    vector<int> intVec = {1, 2, 3, 4, 5};
    vector<string> strVec = {"apple", "banana", "cherry"};
    
    printContainer(intVec);
    printContainer(strVec);
    
    return 0;
}
```

## 4. CRTP (Curiously Recurring Template Pattern)

CRTP is an advanced template technique where a class inherits from a template instantiated with itself. It provides static polymorphism with zero runtime overhead:---

```cpp
#include <iostream>
using namespace std;

// Base template class using CRTP
template <typename Derived>
class Shape {
public:
    // Interface method that calls derived class
    void draw() const {
        static_cast<const Derived*>(this)->drawImpl();
    }
    
    double area() const {
        return static_cast<const Derived*>(this)->areaImpl();
    }
    
    void describe() const {
        cout << "This is a shape with area: " << area() << endl;
        draw();
    }
};

// Derived classes
class Circle : public Shape<Circle> {
private:
    double radius;
    
public:
    Circle(double r) : radius(r) {}
    
    void drawImpl() const {
        cout << "Drawing a circle with radius " << radius << endl;
    }
    
    double areaImpl() const {
        return 3.14159 * radius * radius;
    }
};

class Rectangle : public Shape<Rectangle> {
private:
    double width, height;
    
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    
    void drawImpl() const {
        cout << "Drawing a rectangle " << width << "x" << height << endl;
    }
    
    double areaImpl() const {
        return width * height;
    }
};

class Triangle : public Shape<Triangle> {
private:
    double base, height;
    
public:
    Triangle(double b, double h) : base(b), height(h) {}
    
    void drawImpl() const {
        cout << "Drawing a triangle with base " << base 
             << " and height " << height << endl;
    }
    
    double areaImpl() const {
        return 0.5 * base * height;
    }
};

// Template function that works with any Shape
template <typename T>
void processShape(const Shape<T>& shape) {
    shape.describe();
    cout << "---" << endl;
}

int main() {
    Circle circle(5.0);
    Rectangle rectangle(4.0, 6.0);
    Triangle triangle(3.0, 8.0);
    
    // All resolved at compile time - no virtual function overhead!
    cout << "Circle:" << endl;
    circle.describe();
    cout << endl;
    
    cout << "Rectangle:" << endl;
    rectangle.describe();
    cout << endl;
    
    cout << "Triangle:" << endl;
    triangle.describe();
    cout << endl;
    
    // Using template function
    cout << "Processing shapes:" << endl;
    processShape(circle);
    processShape(rectangle);
    processShape(triangle);
    
    return 0;
}
```

## 5. Comparison: Static vs Dynamic Polymorphism

Here's a side-by-side comparison to illustrate the differences:---

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
using namespace std;

// ============================================
// DYNAMIC POLYMORPHISM (Runtime - Virtual Functions)
// ============================================
class AnimalDynamic {
public:
    virtual void makeSound() const = 0;
    virtual string getType() const = 0;
    virtual ~AnimalDynamic() = default;
};

class DogDynamic : public AnimalDynamic {
public:
    void makeSound() const override {
        cout << "Woof!" << endl;
    }
    string getType() const override {
        return "Dog";
    }
};

class CatDynamic : public AnimalDynamic {
public:
    void makeSound() const override {
        cout << "Meow!" << endl;
    }
    string getType() const override {
        return "Cat";
    }
};

// ============================================
// STATIC POLYMORPHISM (Compile-time - Templates/CRTP)
// ============================================
template <typename Derived>
class AnimalStatic {
public:
    void makeSound() const {
        static_cast<const Derived*>(this)->makeSoundImpl();
    }
    
    string getType() const {
        return static_cast<const Derived*>(this)->getTypeImpl();
    }
};

class DogStatic : public AnimalStatic<DogStatic> {
public:
    void makeSoundImpl() const {
        cout << "Woof!" << endl;
    }
    string getTypeImpl() const {
        return "Dog";
    }
};

class CatStatic : public AnimalStatic<CatStatic> {
public:
    void makeSoundImpl() const {
        cout << "Meow!" << endl;
    }
    string getTypeImpl() const {
        return "Cat";
    }
};

// ============================================
// DEMONSTRATION
// ============================================

// Function using dynamic polymorphism
void useDynamicPolymorphism() {
    cout << "=== DYNAMIC POLYMORPHISM ===" << endl;
    
    // Can store different types in same container
    vector<unique_ptr<AnimalDynamic>> animals;
    animals.push_back(make_unique<DogDynamic>());
    animals.push_back(make_unique<CatDynamic>());
    animals.push_back(make_unique<DogDynamic>());
    
    for (const auto& animal : animals) {
        cout << animal->getType() << " says: ";
        animal->makeSound();
    }
    cout << endl;
}

// Function using static polymorphism
template <typename T>
void processAnimal(const AnimalStatic<T>& animal) {
    cout << animal.getType() << " says: ";
    animal.makeSound();
}

void useStaticPolymorphism() {
    cout << "=== STATIC POLYMORPHISM ===" << endl;
    
    DogStatic dog;
    CatStatic cat;
    
    // Type known at compile time
    processAnimal(dog);
    processAnimal(cat);
    processAnimal(dog);
    cout << endl;
}

// Performance comparison
void performanceTest() {
    cout << "=== PERFORMANCE TEST ===" << endl;
    const int iterations = 10000000;
    
    // Dynamic polymorphism test
    auto start = chrono::high_resolution_clock::now();
    {
        unique_ptr<AnimalDynamic> animal = make_unique<DogDynamic>();
        for (int i = 0; i < iterations; ++i) {
            animal->getType(); // Virtual function call
        }
    }
    auto end = chrono::high_resolution_clock::now();
    auto dynamicTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    // Static polymorphism test
    start = chrono::high_resolution_clock::now();
    {
        DogStatic dog;
        for (int i = 0; i < iterations; ++i) {
            dog.getType(); // Direct call, inlined
        }
    }
    end = chrono::high_resolution_clock::now();
    auto staticTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << "Dynamic polymorphism: " << dynamicTime << "ms" << endl;
    cout << "Static polymorphism:  " << staticTime << "ms" << endl;
    cout << "Speedup: " << (double)dynamicTime / staticTime << "x" << endl;
}

int main() {
    useDynamicPolymorphism();
    useStaticPolymorphism();
    performanceTest();
    
    cout << "\nKey Differences:" << endl;
    cout << "- Dynamic: Runtime resolution, virtual table overhead, " 
         << "supports heterogeneous containers" << endl;
    cout << "- Static: Compile-time resolution, no overhead, " 
         << "faster but less flexible" << endl;
    
    return 0;
}

```

## Key Characteristics

### **Advantages of Static Polymorphism:**

1. **Performance**: No runtime overhead - function calls are resolved at compile time and can be inlined
2. **Type Safety**: Errors are caught at compile time rather than runtime
3. **No Virtual Table**: No vtable pointer overhead in objects
4. **Optimization**: Compiler can perform better optimizations (inlining, dead code elimination)
5. **Zero-cost abstraction**: You pay nothing for what you don't use

### **Disadvantages of Static Polymorphism:**

1. **Code Bloat**: Templates generate code for each type, increasing binary size
2. **Compilation Time**: Template instantiation can slow down compilation
3. **Less Flexible**: Cannot have heterogeneous containers (e.g., `vector<Shape*>` with different shapes)
4. **Error Messages**: Template errors can be cryptic and hard to debug
5. **Cannot use with runtime-determined types**: Type must be known at compile time

---

## When to Use Static Polymorphism

- **Performance-critical code** (game engines, numerical computations, embedded systems)
- When working with **generic algorithms** (STL-style programming)
- When types are **known at compile time**
- When you want **zero runtime overhead**
- **Library code** that should work with many types

## When to Use Dynamic Polymorphism

- Need **heterogeneous collections** (storing different types in same container)
- **Plugin systems** or runtime type determination
- When flexibility is more important than performance
- **Interface-based design** where implementation details are hidden

---

## Summary

Static polymorphism in C++ provides compile-time resolution through function/operator overloading, templates, and CRTP. It offers excellent performance with zero runtime overhead but at the cost of flexibility. Modern C++ heavily favors static polymorphism for generic programming, as evidenced by the STL and other high-performance libraries.

The choice between static and dynamic polymorphism depends on your specific requirements: choose static for performance and compile-time guarantees, choose dynamic for flexibility and runtime behavior.

