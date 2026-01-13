# Class Template Argument Deduction (CTAD) and Deduction Guides

This is about **Class Template Argument Deduction (CTAD)**, introduced in C++17, which allows you to omit template parameters when they can be deduced from constructor arguments.

## The Problem Before C++17

```cpp
template<typename T>
class MyVector {
    T* data;
    size_t size;
public:
    MyVector(size_t n, T val) {
        size = n;
        data = new T[n];
        for(size_t i = 0; i < n; ++i) {
            data[i] = val;
        }
    }
    ~MyVector() { delete[] data; }
};

int main() {
    // Before C++17: MUST specify template argument explicitly
    MyVector<double> vec1(10, 3.14);  // Verbose!
    MyVector<int> vec2(5, 42);
    
    // This would be an ERROR before C++17:
    // MyVector vec(10, 3.14);  // Error: missing template arguments
}
```

## The Solution: CTAD with Deduction Guides

```cpp
template<typename T>
class MyVector {
    T* data;
    size_t size;
public:
    MyVector(size_t n, T val) {
        size = n;
        data = new T[n];
        for(size_t i = 0; i < n; ++i) {
            data[i] = val;
        }
    }
    ~MyVector() { delete[] data; }
};

// Deduction Guide: tells compiler how to deduce T
template<typename T>
MyVector(size_t, T) -> MyVector<T>;
//       ^^^^^^^^^^    ^^^^^^^^^^^^
//       constructor   what type to
//       arguments     deduce

int main() {
    // C++17 and later: template argument deduced!
    MyVector vec(10, 3.14);  // Deduces MyVector<double>
    
    // How it works:
    // - Constructor takes (size_t, T)
    // - We pass (10, 3.14)
    // - 3.14 is double, so T = double
    // - Result: MyVector<double>
}
```

## Breaking Down the Deduction Guide

```cpp
template<typename T>
MyVector(size_t, T) -> MyVector<T>;
//  ^1^  ^2^  ^3^     ^4^
```

1. **Template parameter**: What type we're deducing
2. **Constructor signature**: Pattern to match (argument types)
3. **Type to deduce from**: `T` comes from the second argument
4. **Resulting type**: The class template instantiation

## Without Deduction Guide vs With

```cpp
template<typename T>
class Container {
    T value;
public:
    Container(T v) : value(v) {}
};

// NO deduction guide needed here - compiler can figure it out!
Container c1(42);      // ✓ Container<int> - implicit deduction works
Container c2(3.14);    // ✓ Container<double>


// But sometimes you need explicit guides:
template<typename T>
class Wrapper {
    T value;
public:
    // Problem: T can't be deduced from size_t alone!
    Wrapper(size_t n) : value(n) {}
};

// This WON'T work without a guide:
// Wrapper w(10);  // Error: can't deduce T

// Solution: provide a deduction guide
template<typename T = int>
Wrapper(size_t) -> Wrapper<T>;

Wrapper w(10);  // ✓ Wrapper<int> (using default)
```

## Real-World Examples

### Example 1: std::pair

```cpp
#include <utility>

// Before C++17
std::pair<int, double> p1(42, 3.14);
auto p2 = std::make_pair(42, 3.14);  // Workaround using helper function

// C++17 and later (CTAD)
std::pair p3(42, 3.14);  // ✓ std::pair<int, double> automatically!
```

### Example 2: Custom deduction guide for arrays

```cpp
#include <iostream>

template<typename T>
class Array {
    T* data;
    size_t len;
public:
    Array(std::initializer_list<T> init) {
        len = init.size();
        data = new T[len];
        size_t i = 0;
        for(const auto& val : init) {
            data[i++] = val;
        }
    }
    
    ~Array() { delete[] data; }
    
    void print() {
        for(size_t i = 0; i < len; ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << "\n";
    }
};

// Deduction guide for initializer_list
template<typename T>
Array(std::initializer_list<T>) -> Array<T>;

int main() {
    Array arr{1, 2, 3, 4, 5};  // ✓ Array<int>
    arr.print();
    
    Array arr2{1.5, 2.5, 3.5};  // ✓ Array<double>
    arr2.print();
}
```

### Example 3: Complex deduction - choosing between types

```cpp
#include <string>
#include <iostream>

template<typename T>
class Storage {
    T value;
public:
    Storage(T v) : value(v) {}
    
    void print() {
        std::cout << "Value: " << value << "\n";
    }
};

// Deduction guide: convert C-strings to std::string
Storage(const char*) -> Storage<std::string>;

int main() {
    Storage s1(42);         // Storage<int>
    Storage s2(3.14);       // Storage<double>
    Storage s3("hello");    // Storage<std::string> (via deduction guide!)
    //                         Without guide: Storage<const char*>
    
    s1.print();
    s2.print();
    s3.print();
}
```

## When Deduction Guides Are Needed

### Case 1: Implicit deduction works (no guide needed)

```cpp
template<typename T>
class Simple {
    T value;
public:
    Simple(T v) : value(v) {}  // T directly from constructor argument
};

Simple s(42);  // ✓ Simple<int> - works without guide
```

### Case 2: Deduction guide required

```cpp
template<typename T>
class Complex {
    T* ptr;
public:
    // Can't deduce T from size_t!
    Complex(size_t size) : ptr(new T[size]) {}
};

// Need this:
template<typename T = int>
Complex(size_t) -> Complex<T>;

Complex c(10);  // ✓ Complex<int>
```

### Case 3: Customizing deduction behavior

```cpp
template<typename T>
class SmartPointer {
    T* ptr;
public:
    SmartPointer(T* p) : ptr(p) {}
};

// Guide to strip pointer from deduction
template<typename T>
SmartPointer(T*) -> SmartPointer<T>;

int x = 42;
SmartPointer sp(&x);  // ✓ SmartPointer<int>, not SmartPointer<int*>
```

## Complete Working Example

```cpp
#include <iostream>
#include <string>

template<typename T>
class MyVector {
    T* data;
    size_t size;
    
public:
    MyVector(size_t n, T val) : size(n), data(new T[n]) {
        for(size_t i = 0; i < size; ++i) {
            data[i] = val;
        }
    }
    
    ~MyVector() { delete[] data; }
    
    void print() const {
        for(size_t i = 0; i < size; ++i) {
            std::cout << data[i] << " ";
        }
        std::cout << "\n";
    }
};

// Deduction guide
template<typename T>
MyVector(size_t, T) -> MyVector<T>;

int main() {
    MyVector v1(5, 42);           // MyVector<int>
    MyVector v2(3, 3.14);         // MyVector<double>
    MyVector v3(4, std::string("hi"));  // MyVector<std::string>
    
    v1.print();  // 42 42 42 42 42
    v2.print();  // 3.14 3.14 3.14
    v3.print();  // hi hi hi hi
    
    return 0;
}
```

## Key Takeaways

1. **CTAD** lets you omit template arguments when constructing objects (C++17+)
2. **Deduction guides** tell the compiler how to deduce template parameters from constructor arguments
3. Syntax: `ClassName(arg_types...) -> ClassName<deduced_types>;`
4. Not always needed - compiler can often deduce automatically
5. Useful for: complex deduction, type conversions, setting defaults

This feature makes C++ templates much more ergonomic and reduces boilerplate!