#include <iostream>
#include <vector>
#include <string>
#include <type_traits>
#include <cmath>

// ============================================================================
// 1. FUNCTION TEMPLATES
// ============================================================================

// Basic function template - works with any type supporting operator<
template<typename T>
T max(T a, T b) {
    return (a > b) ? a : b;
}

// Multiple template parameters
template<typename T, typename U>
auto multiply(T a, U b) -> decltype(a * b) {
    return a * b;
}

// Non-type template parameter (compile-time constant)
template<typename T, int N>
T arraySum(T (&arr)[N]) {
    T sum = T();
    for (int i = 0; i < N; ++i) {
        sum += arr[i];
    }
    return sum;
}

// Template with constraints (C++20 concepts style, shown with SFINAE for compatibility)
template<typename T>
typename std::enable_if<std::is_arithmetic<T>::value, T>::type
square(T value) {
    return value * value;
}

// ============================================================================
// 2. CLASS TEMPLATES
// ============================================================================

// Basic class template - generic container
template<typename T>
class Stack {
private:
    std::vector<T> elements;

public:
    void push(const T& elem) {
        elements.push_back(elem);
    }

    void pop() {
        if (!elements.empty()) {
            elements.pop_back();
        }
    }

    T top() const {
        if (!elements.empty()) {
            return elements.back();
        }
        throw std::runtime_error("Stack is empty");
    }

    bool empty() const {
        return elements.empty();
    }

    size_t size() const {
        return elements.size();
    }
};

// Class template with multiple parameters
template<typename Key, typename Value>
class Pair {
private:
    Key key;
    Value value;

public:
    Pair(const Key& k, const Value& v) : key(k), value(v) {}

    Key getKey() const { return key; }
    Value getValue() const { return value; }

    void setValue(const Value& v) { value = v; }
};

// Template specialization - optimized version for bool
template<>
class Stack<bool> {
private:
    std::vector<bool> elements;  // Uses specialized vector<bool>

public:
    void push(bool elem) {
        elements.push_back(elem);
        std::cout << "Using specialized Stack<bool>\n";
    }

    void pop() {
        if (!elements.empty()) {
            elements.pop_back();
        }
    }

    bool top() const {
        if (!elements.empty()) {
            return elements.back();
        }
        throw std::runtime_error("Stack is empty");
    }

    bool empty() const {
        return elements.empty();
    }
};

// Partial specialization - for pointer types
template<typename T>
class SmartContainer<T*> {
private:
    std::vector<T*> pointers;

public:
    void add(T* ptr) {
        pointers.push_back(ptr);
    }

    ~SmartContainer() {
        // Clean up all pointers
        for (auto ptr : pointers) {
            delete ptr;
        }
    }
};

// ============================================================================
// 3. VARIABLE TEMPLATES (C++14)
// ============================================================================

// Mathematical constant with appropriate precision for each type
template<typename T>
constexpr T pi = T(3.1415926535897932385);

// Type-dependent constant
template<typename T>
constexpr T max_value = std::numeric_limits<T>::max();

// Helper variable template for type traits
template<typename T>
constexpr bool is_pointer_v = std::is_pointer<T>::value;

// ============================================================================
// 4. TEMPLATE METAPROGRAMMING EXAMPLES
// ============================================================================

// Compile-time factorial using recursion
template<int N>
struct Factorial {
    static constexpr int value = N * Factorial<N - 1>::value;
};

template<>
struct Factorial<0> {
    static constexpr int value = 1;
};

// Type traits - check if type has a specific method
template<typename T>
class has_toString {
private:
    template<typename U>
    static auto test(int) -> decltype(std::declval<U>().toString(), std::true_type());

    template<typename>
    static std::false_type test(...);

public:
    static constexpr bool value = decltype(test<T>(0))::value;
};

// Conditional type selection
template<bool Condition, typename TrueType, typename FalseType>
struct Conditional {
    using type = TrueType;
};

template<typename TrueType, typename FalseType>
struct Conditional<false, TrueType, FalseType> {
    using type = FalseType;
};

// ============================================================================
// DEMONSTRATION
// ============================================================================

class Person {
public:
    std::string name;
    Person(const std::string& n) : name(n) {}
    std::string toString() const { return "Person: " + name; }
};

int main() {
    std::cout << "=== FUNCTION TEMPLATES ===\n";

    // Basic function template
    std::cout << "max(10, 20) = " << max(10, 20) << "\n";
    std::cout << "max(3.14, 2.71) = " << max(3.14, 2.71) << "\n";
    std::cout << "max(std::string(\"apple\"), std::string(\"banana\")) = "
              << max(std::string("apple"), std::string("banana")) << "\n\n";

    // Multiple parameters
    std::cout << "multiply(5, 3.5) = " << multiply(5, 3.5) << "\n\n";

    // Non-type parameter
    int arr[] = {1, 2, 3, 4, 5};
    std::cout << "Sum of array: " << arraySum(arr) << "\n\n";

    // Constrained template
    std::cout << "square(4) = " << square(4) << "\n";
    std::cout << "square(2.5) = " << square(2.5) << "\n\n";

    std::cout << "=== CLASS TEMPLATES ===\n";

    // Stack of integers
    Stack<int> intStack;
    intStack.push(1);
    intStack.push(2);
    intStack.push(3);
    std::cout << "Int stack top: " << intStack.top() << "\n";
    std::cout << "Int stack size: " << intStack.size() << "\n\n";

    // Stack of strings
    Stack<std::string> stringStack;
    stringStack.push("Hello");
    stringStack.push("World");
    std::cout << "String stack top: " << stringStack.top() << "\n\n";

    // Specialized stack for bool
    Stack<bool> boolStack;
    boolStack.push(true);  // Shows specialization message

    // Pair template
    Pair<std::string, int> person("Alice", 30);
    std::cout << person.getKey() << " is " << person.getValue() << " years old\n\n";

    std::cout << "=== VARIABLE TEMPLATES ===\n";

    // Pi with different precisions
    std::cout << "pi<float>: " << pi<float> << "\n";
    std::cout << "pi<double>: " << pi<double> << "\n";
    std::cout << "pi<long double>: " << pi<long double> << "\n\n";

    // Type-specific max values
    std::cout << "max_value<int>: " << max_value<int> << "\n";
    std::cout << "max_value<char>: " << (int)max_value<char> << "\n\n";

    std::cout << "=== TEMPLATE METAPROGRAMMING ===\n";

    // Compile-time factorial
    std::cout << "Factorial<5>::value = " << Factorial<5>::value << "\n";
    std::cout << "Factorial<10>::value = " << Factorial<10>::value << "\n\n";

    // Type trait detection
    std::cout << "Person has toString: " << has_toString<Person>::value << "\n";
    std::cout << "int has toString: " << has_toString<int>::value << "\n\n";

    // Conditional type
    using MyType = typename Conditional<true, int, double>::type;
    std::cout << "Conditional<true, int, double> is int: "
              << std::is_same<MyType, int>::value << "\n";

    return 0;
}