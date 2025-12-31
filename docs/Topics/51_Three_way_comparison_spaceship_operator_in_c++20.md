# Three-Way Comparison (Spaceship) Operator in C++20

The **three-way comparison operator** `<=>`, nicknamed the "spaceship operator" due to its visual resemblance to a spaceship, is one of the most significant additions to C++20. It provides a concise way to define all six comparison operations (`==`, `!=`, `<`, `>`, `<=`, `>=`) for a class by implementing just one or two operators.

## Why Was It Introduced?

Before C++20, defining comparison operators for a custom class was repetitive and error-prone. You typically needed to implement all six comparison operators manually, which led to code duplication and potential inconsistencies:

```cpp
// Pre-C++20 approach - tedious and error-prone
class Point {
    int x, y;
public:
    bool operator==(const Point& other) const { return x == other.x && y == other.y; }
    bool operator!=(const Point& other) const { return !(*this == other); }
    bool operator<(const Point& other) const { return x < other.x || (x == other.x && y < other.y); }
    bool operator>(const Point& other) const { return other < *this; }
    bool operator<=(const Point& other) const { return !(other < *this); }
    bool operator>=(const Point& other) const { return !(*this < other); }
};
```

The spaceship operator solves this problem elegantly.

## How It Works

The spaceship operator performs a three-way comparison and returns an object that indicates whether the left operand is less than, equal to, or greater than the right operand. The compiler can automatically generate the other comparison operators based on this single implementation.

## Return Types

The spaceship operator returns one of three ordering types from the `<compare>` header:

1. **`std::strong_ordering`**: Indicates a total order where equality implies substitutability (like integers)
   - Possible values: `strong_ordering::less`, `strong_ordering::equal`, `strong_ordering::greater`

2. **`std::weak_ordering`**: Indicates a total order where equivalent values may not be substitutable (like case-insensitive strings)
   - Possible values: `weak_ordering::less`, `weak_ordering::equivalent`, `weak_ordering::greater`

3. **`std::partial_ordering`**: Indicates that some values may be incomparable (like floating-point with NaN)
   - Possible values: `partial_ordering::less`, `partial_ordering::equivalent`, `partial_ordering::greater`, `partial_ordering::unordered`

## Practical Examples

### Example 1: Basic Usage with Default Implementation

The simplest approach is to use the compiler-generated default implementation:

```cpp
#include <compare>
#include <iostream>

class Point {
public:
    int x, y;
    
    // Compiler generates all six comparison operators from this
    auto operator<=>(const Point&) const = default;
};

int main() {
    Point p1{1, 2};
    Point p2{1, 3};
    Point p3{1, 2};
    
    std::cout << std::boolalpha;
    std::cout << (p1 < p2) << '\n';   // true
    std::cout << (p1 == p3) << '\n';  // true
    std::cout << (p2 >= p1) << '\n';  // true
}
```

### Example 2: Custom Implementation

Sometimes you need custom comparison logic:

```cpp
#include <compare>
#include <string>
#include <iostream>

class Person {
    std::string name;
    int age;
    
public:
    Person(std::string n, int a) : name(std::move(n)), age(a) {}
    
    // Custom three-way comparison - compare by age, then by name
    std::strong_ordering operator<=>(const Person& other) const {
        if (auto cmp = age <=> other.age; cmp != 0)
            return cmp;
        return name <=> other.name;
    }
    
    // Note: == is not automatically generated from <=>, so we define it
    bool operator==(const Person& other) const {
        return age == other.age && name == other.name;
    }
};

int main() {
    Person alice{"Alice", 30};
    Person bob{"Bob", 25};
    Person charlie{"Charlie", 30};
    
    std::cout << std::boolalpha;
    std::cout << (bob < alice) << '\n';      // true (25 < 30)
    std::cout << (alice < charlie) << '\n';  // true (same age, "Alice" < "Charlie")
    std::cout << (charlie > alice) << '\n';  // true
}
```

### Example 3: Working with Different Ordering Types

```cpp
#include <compare>
#include <iostream>
#include <string>
#include <algorithm>

class CaseInsensitiveString {
    std::string data;
    
public:
    CaseInsensitiveString(std::string s) : data(std::move(s)) {}
    
    // Weak ordering because "Hello" and "HELLO" are equivalent but not equal
    std::weak_ordering operator<=>(const CaseInsensitiveString& other) const {
        std::string lower1 = data, lower2 = other.data;
        std::transform(lower1.begin(), lower1.end(), lower1.begin(), ::tolower);
        std::transform(lower2.begin(), lower2.end(), lower2.begin(), ::tolower);
        return lower1 <=> lower2;
    }
    
    bool operator==(const CaseInsensitiveString& other) const {
        return (*this <=> other) == 0;
    }
    
    const std::string& str() const { return data; }
};

int main() {
    CaseInsensitiveString s1{"Hello"};
    CaseInsensitiveString s2{"HELLO"};
    CaseInsensitiveString s3{"World"};
    
    std::cout << std::boolalpha;
    std::cout << (s1 == s2) << '\n';  // true (equivalent in comparison)
    std::cout << (s1 < s3) << '\n';   // true
}
```

### Example 4: Partial Ordering with Floating Point

```cpp
#include <compare>
#include <iostream>
#include <cmath>

class Temperature {
    double celsius;
    
public:
    Temperature(double c) : celsius(c) {}
    
    // Partial ordering because NaN is unordered
    std::partial_ordering operator<=>(const Temperature& other) const {
        return celsius <=> other.celsius;
    }
    
    bool operator==(const Temperature& other) const {
        return celsius == other.celsius;
    }
};

int main() {
    Temperature t1{25.0};
    Temperature t2{30.0};
    Temperature t3{NAN};
    
    std::cout << std::boolalpha;
    std::cout << (t1 < t2) << '\n';     // true
    std::cout << (t3 < t1) << '\n';     // false
    std::cout << (t3 > t1) << '\n';     // false
    std::cout << (t3 == t3) << '\n';    // false (NaN != NaN)
}
```

## Key Advantages

The spaceship operator significantly reduces boilerplate code. With a single defaulted `<=>` operator, the compiler generates all six comparison operators automatically. For custom implementations, you typically only need to define `<=>` and `==`, and the compiler derives the rest. This leads to more maintainable code with fewer opportunities for bugs and inconsistencies between related comparison operations.