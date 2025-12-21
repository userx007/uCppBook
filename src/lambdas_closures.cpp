#include <iostream>
#include <vector>
#include <algorithm>
#include <functional>
#include <string>
#include <memory>

// ============================================================================
// 1. BASIC FUNCTION OBJECTS (FUNCTORS)
// ============================================================================

// Simple functor without state
struct Multiply {
    int operator()(int a, int b) const {
        return a * b;
    }
};

// Functor with state
class Counter {
private:
    int count;
public:
    Counter() : count(0) {}
    
    int operator()() {
        return ++count;
    }
    
    int getCount() const { return count; }
};

// Functor for use with STL algorithms
class GreaterThan {
private:
    int threshold;
public:
    explicit GreaterThan(int t) : threshold(t) {}
    
    bool operator()(int value) const {
        return value > threshold;
    }
};

// ============================================================================
// 2. COMPARISON: Functor vs Function Pointer
// ============================================================================

bool isEven(int n) { return n % 2 == 0; }

struct IsEven {
    bool operator()(int n) const { return n % 2 == 0; }
};

// ============================================================================
// 3. LAMBDA BASICS
// ============================================================================

void demonstrateLambdaBasics() {
    std::cout << "\n=== Lambda Basics ===\n";
    
    // Simplest lambda - no captures, no parameters
    auto hello = []() { std::cout << "Hello from lambda!\n"; };
    hello();
    
    // Lambda with parameters
    auto add = [](int a, int b) { return a + b; };
    std::cout << "5 + 3 = " << add(5, 3) << "\n";
    
    // Lambda with explicit return type
    auto divide = [](double a, double b) -> double {
        if (b == 0) return 0.0;
        return a / b;
    };
    std::cout << "10.0 / 3.0 = " << divide(10.0, 3.0) << "\n";
    
    // Immediately invoked lambda
    int result = [](int x) { return x * x; }(5);
    std::cout << "Square of 5 = " << result << "\n";
}

// ============================================================================
// 4. CAPTURE MODES
// ============================================================================

void demonstrateCaptures() {
    std::cout << "\n=== Capture Modes ===\n";
    
    int x = 10;
    int y = 20;
    int z = 30;
    
    // Capture by value
    auto byValue = [x]() { 
        // x is copied, cannot modify original
        std::cout << "Captured x by value: " << x << "\n";
    };
    byValue();
    
    // Capture by reference
    auto byReference = [&x]() { 
        x += 5;  // Modifies original x
        std::cout << "Modified x by reference: " << x << "\n";
    };
    byReference();
    std::cout << "x after lambda: " << x << "\n";
    
    // Capture all by value
    auto captureAllValue = [=]() {
        std::cout << "All by value - x: " << x << ", y: " << y << ", z: " << z << "\n";
    };
    captureAllValue();
    
    // Capture all by reference
    auto captureAllRef = [&]() {
        x++; y++; z++;
        std::cout << "All by ref - incremented all\n";
    };
    captureAllRef();
    
    // Mixed captures
    auto mixed = [x, &y, z]() {
        // x and z by value, y by reference
        y += 10;
        std::cout << "Mixed - x: " << x << ", y: " << y << ", z: " << z << "\n";
    };
    mixed();
    
    std::cout << "Final values - x: " << x << ", y: " << y << ", z: " << z << "\n";
}

// ============================================================================
// 5. MUTABLE LAMBDAS
// ============================================================================

void demonstrateMutableLambdas() {
    std::cout << "\n=== Mutable Lambdas ===\n";
    
    int counter = 0;
    
    // Without mutable - cannot modify captured-by-value variable
    // auto increment = [counter]() { counter++; };  // ERROR!
    
    // With mutable - can modify the copy
    auto increment = [counter]() mutable {
        counter++;  // Modifies the lambda's copy, not original
        std::cout << "Lambda's counter: " << counter << "\n";
        return counter;
    };
    
    increment();  // Lambda's counter: 1
    increment();  // Lambda's counter: 2
    increment();  // Lambda's counter: 3
    std::cout << "Original counter: " << counter << "\n";  // Still 0
    
    // Mutable with reference - modifies original
    auto incrementRef = [&counter]() {  // No mutable needed with reference
        counter++;
        std::cout << "Reference counter: " << counter << "\n";
    };
    
    incrementRef();  // Reference counter: 1
    incrementRef();  // Reference counter: 2
}

// ============================================================================
// 6. GENERIC LAMBDAS (C++14)
// ============================================================================

void demonstrateGenericLambdas() {
    std::cout << "\n=== Generic Lambdas ===\n";
    
    // Lambda with auto parameters - works with any type
    auto print = [](const auto& value) {
        std::cout << "Value: " << value << "\n";
    };
    
    print(42);
    print(3.14);
    print(std::string("Hello"));
    
    // Generic lambda for comparison
    auto max = [](const auto& a, const auto& b) {
        return (a > b) ? a : b;
    };
    
    std::cout << "Max(10, 20): " << max(10, 20) << "\n";
    std::cout << "Max(3.14, 2.71): " << max(3.14, 2.71) << "\n";
    std::cout << "Max('a', 'z'): " << max('a', 'z') << "\n";
}

// ============================================================================
// 7. INIT CAPTURES (C++14)
// ============================================================================

void demonstrateInitCaptures() {
    std::cout << "\n=== Init Captures ===\n";
    
    // Initialize a variable in the capture clause
    auto lambda1 = [value = 42]() {
        std::cout << "Initialized value: " << value << "\n";
    };
    lambda1();
    
    // Move capture - useful for move-only types
    auto ptr = std::make_unique<int>(100);
    auto lambda2 = [p = std::move(ptr)]() {
        std::cout << "Moved unique_ptr value: " << *p << "\n";
    };
    lambda2();
    // ptr is now null
    
    // Complex initialization
    std::vector<int> vec{1, 2, 3, 4, 5};
    auto lambda3 = [v = std::move(vec), sum = 0]() mutable {
        for (int n : v) sum += n;
        std::cout << "Sum of moved vector: " << sum << "\n";
    };
    lambda3();
}

// ============================================================================
// 8. CLOSURES AND LIFETIME
// ============================================================================

// Dangerous: Returning lambda with dangling reference
auto createDanglingLambda() {
    int local = 42;
    // DANGEROUS: captures local by reference, which will be destroyed
    return [&local]() { return local; };
}

// Safe: Capturing by value
auto createSafeLambda() {
    int local = 42;
    // Safe: captures by value
    return [local]() { return local; };
}

void demonstrateClosureLifetime() {
    std::cout << "\n=== Closure Lifetime ===\n";
    
    // Safe usage
    auto safe = createSafeLambda();
    std::cout << "Safe lambda result: " << safe() << "\n";
    
    // Dangerous usage (undefined behavior)
    // auto dangerous = createDanglingLambda();
    // std::cout << dangerous() << "\n";  // UB: accesses destroyed variable
    
    // Proper pattern: capture by value or use shared_ptr
    auto shared = std::make_shared<int>(100);
    auto lambda = [shared]() { return *shared; };
    std::cout << "Shared pointer capture: " << lambda() << "\n";
}

// ============================================================================
// 9. STL ALGORITHMS WITH FUNCTORS AND LAMBDAS
// ============================================================================

void demonstrateSTLUsage() {
    std::cout << "\n=== STL Algorithms ===\n";
    
    std::vector<int> numbers{1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Using functor
    GreaterThan gt5(5);
    auto it1 = std::find_if(numbers.begin(), numbers.end(), gt5);
    if (it1 != numbers.end()) {
        std::cout << "First number > 5 (functor): " << *it1 << "\n";
    }
    
    // Using lambda
    auto it2 = std::find_if(numbers.begin(), numbers.end(), 
                            [](int n) { return n > 5; });
    if (it2 != numbers.end()) {
        std::cout << "First number > 5 (lambda): " << *it2 << "\n";
    }
    
    // Transform with lambda
    std::vector<int> squares;
    std::transform(numbers.begin(), numbers.end(), 
                   std::back_inserter(squares),
                   [](int n) { return n * n; });
    
    std::cout << "Squares: ";
    for (int n : squares) std::cout << n << " ";
    std::cout << "\n";
    
    // Count with capture
    int threshold = 5;
    int count = std::count_if(numbers.begin(), numbers.end(),
                              [threshold](int n) { return n > threshold; });
    std::cout << "Numbers > " << threshold << ": " << count << "\n";
    
    // Custom sorting
    std::vector<std::string> words{"apple", "zoo", "cat", "dog", "elephant"};
    std::sort(words.begin(), words.end(), 
              [](const std::string& a, const std::string& b) {
                  return a.length() < b.length();
              });
    
    std::cout << "Sorted by length: ";
    for (const auto& word : words) std::cout << word << " ";
    std::cout << "\n";
}

// ============================================================================
// 10. std::function - TYPE ERASURE
// ============================================================================

void demonstrateStdFunction() {
    std::cout << "\n=== std::function ===\n";
    
    // std::function can hold any callable
    std::function<int(int, int)> operation;
    
    // Assign a lambda
    operation = [](int a, int b) { return a + b; };
    std::cout << "Lambda: 5 + 3 = " << operation(5, 3) << "\n";
    
    // Assign a functor
    operation = Multiply();
    std::cout << "Functor: 5 * 3 = " << operation(5, 3) << "\n";
    
    // Assign a function pointer
    operation = [](int a, int b) { return a - b; };
    std::cout << "Subtraction: 5 - 3 = " << operation(5, 3) << "\n";
    
    // Store different lambdas in a vector
    std::vector<std::function<void()>> callbacks;
    
    for (int i = 0; i < 3; ++i) {
        callbacks.push_back([i]() {
            std::cout << "Callback " << i << " called\n";
        });
    }
    
    std::cout << "Executing callbacks:\n";
    for (auto& cb : callbacks) {
        cb();
    }
}

// ============================================================================
// 11. PRACTICAL EXAMPLE: Event System
// ============================================================================

class Button {
private:
    std::vector<std::function<void()>> clickHandlers;
public:
    void onClick(std::function<void()> handler) {
        clickHandlers.push_back(handler);
    }
    
    void click() {
        std::cout << "Button clicked!\n";
        for (auto& handler : clickHandlers) {
            handler();
        }
    }
};

void demonstrateEventSystem() {
    std::cout << "\n=== Event System Example ===\n";
    
    Button button;
    int clickCount = 0;
    
    // Add lambda handlers with captures
    button.onClick([&clickCount]() {
        clickCount++;
        std::cout << "Handler 1: Click count = " << clickCount << "\n";
    });
    
    button.onClick([]() {
        std::cout << "Handler 2: Button was clicked!\n";
    });
    
    std::string message = "Custom message";
    button.onClick([message]() {
        std::cout << "Handler 3: " << message << "\n";
    });
    
    button.click();
    std::cout << "\n";
    button.click();
}

// ============================================================================
// 12. RECURSIVE LAMBDAS
// ============================================================================

void demonstrateRecursiveLambdas() {
    std::cout << "\n=== Recursive Lambdas ===\n";
    
    // Recursive lambda using std::function
    std::function<int(int)> factorial = [&factorial](int n) -> int {
        return (n <= 1) ? 1 : n * factorial(n - 1);
    };
    
    std::cout << "Factorial of 5: " << factorial(5) << "\n";
    
    // Generic recursive lambda for Fibonacci
    std::function<long long(int)> fib = [&fib](int n) -> long long {
        if (n <= 1) return n;
        return fib(n - 1) + fib(n - 2);
    };
    
    std::cout << "Fibonacci of 10: " << fib(10) << "\n";
}

// ============================================================================
// 13. STATEFUL FUNCTORS VS LAMBDAS
// ============================================================================

class Accumulator {
private:
    int sum;
public:
    Accumulator() : sum(0) {}
    
    void operator()(int value) {
        sum += value;
    }
    
    int getSum() const { return sum; }
};

void demonstrateStatefulComparison() {
    std::cout << "\n=== Stateful: Functor vs Lambda ===\n";
    
    std::vector<int> numbers{1, 2, 3, 4, 5};
    
    // Using functor
    Accumulator acc1;
    acc1 = std::for_each(numbers.begin(), numbers.end(), acc1);
    std::cout << "Functor sum: " << acc1.getSum() << "\n";
    
    // Using mutable lambda
    int sum = 0;
    std::for_each(numbers.begin(), numbers.end(), 
                  [&sum](int n) { sum += n; });
    std::cout << "Lambda sum: " << sum << "\n";
    
    // Using mutable lambda with captured state
    auto accumulate = [sum = 0](int n) mutable {
        sum += n;
        return sum;
    };
    
    for (int n : numbers) {
        std::cout << "Running sum: " << accumulate(n) << "\n";
    }
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "=== C++ Function Objects, Lambdas, and Closures ===\n";
    
    // Basic functor usage
    std::cout << "\n=== Basic Functors ===\n";
    Multiply mult;
    std::cout << "3 * 4 = " << mult(3, 4) << "\n";
    
    Counter counter;
    std::cout << "Counter: " << counter() << ", " << counter() << ", " << counter() << "\n";
    
    // Run all demonstrations
    demonstrateLambdaBasics();
    demonstrateCaptures();
    demonstrateMutableLambdas();
    demonstrateGenericLambdas();
    demonstrateInitCaptures();
    demonstrateClosureLifetime();
    demonstrateSTLUsage();
    demonstrateStdFunction();
    demonstrateEventSystem();
    demonstrateRecursiveLambdas();
    demonstrateStatefulComparison();
    
    return 0;
}