# Lambda Functions in C++

Lambda functions, introduced in C++11, are anonymous function objects that can be defined inline. They provide a concise way to create small, single-use functions without the overhead of declaring a separate named function.

## Basic Syntax

The general syntax of a lambda function is:

```cpp
[capture_clause](parameters) -> return_type { function_body }
```

The return type is optional and can be omitted if the compiler can deduce it.

## Capture Clauses

The capture clause determines what variables from the surrounding scope the lambda can access:

- `[]` - Captures nothing
- `[=]` - Captures all variables by value
- `[&]` - Captures all variables by reference
- `[x]` - Captures variable x by value
- `[&x]` - Captures variable x by reference
- `[=, &x]` - Captures all by value except x by reference
- `[&, x]` - Captures all by reference except x by value
- `[this]` - Captures the this pointer (for use in member functions)

## Detailed Examples

### Example 1: Simple Lambda Without Captures

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> numbers = {5, 2, 8, 1, 9, 3};
    
    // Sort in ascending order
    std::sort(numbers.begin(), numbers.end(), 
              [](int a, int b) { return a < b; });
    
    std::cout << "Sorted: ";
    for(int n : numbers) std::cout << n << " ";
    std::cout << std::endl;
    
    return 0;
}
```

### Example 2: Capturing Variables by Value

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    int threshold = 5;
    
    // Count numbers greater than threshold
    int count = std::count_if(numbers.begin(), numbers.end(),
                              [threshold](int n) { return n > threshold; });
    
    std::cout << "Numbers greater than " << threshold << ": " << count << std::endl;
    
    return 0;
}
```

### Example 3: Capturing Variables by Reference

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    int sum = 0;
    
    // Calculate sum using lambda with reference capture
    std::for_each(numbers.begin(), numbers.end(),
                  [&sum](int n) { sum += n; });
    
    std::cout << "Sum: " << sum << std::endl;  // Output: 15
    
    return 0;
}
```

### Example 4: Mutable Lambdas

By default, variables captured by value are const. The `mutable` keyword allows modification:

```cpp
#include <iostream>

int main() {
    int x = 10;
    
    auto lambda = [x]() mutable {
        x += 5;  // Modifies the captured copy, not the original
        return x;
    };
    
    std::cout << "Lambda result: " << lambda() << std::endl;  // 15
    std::cout << "Original x: " << x << std::endl;            // 10 (unchanged)
    
    return 0;
}
```

### Example 5: Explicit Return Type

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // Lambda with explicit return type
    auto divide = [](double a, double b) -> double {
        if (b == 0) return 0.0;
        return a / b;
    };
    
    std::cout << "10 / 3 = " << divide(10, 3) << std::endl;
    
    return 0;
}
```

### Example 6: Generic Lambda (C++14)

C++14 introduced generic lambdas using `auto` parameters:

```cpp
#include <iostream>
#include <string>

int main() {
    // Generic lambda that works with any type
    auto print = [](const auto& value) {
        std::cout << value << std::endl;
    };
    
    print(42);              // int
    print(3.14);            // double
    print("Hello");         // const char*
    print(std::string("World"));  // std::string
    
    return 0;
}
```

### Example 7: Storing Lambdas

```cpp
#include <iostream>
#include <functional>
#include <vector>

int main() {
    // Store lambda in std::function
    std::function<int(int, int)> add = [](int a, int b) { return a + b; };
    
    // Store lambdas in a vector
    std::vector<std::function<int(int)>> operations;
    
    operations.push_back([](int x) { return x * 2; });      // double
    operations.push_back([](int x) { return x * x; });      // square
    operations.push_back([](int x) { return x + 10; });     // add 10
    
    int value = 5;
    for(const auto& op : operations) {
        std::cout << "Result: " << op(value) << std::endl;
    }
    
    return 0;
}
```

### Example 8: Lambda with Standard Algorithms

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Remove even numbers
    numbers.erase(
        std::remove_if(numbers.begin(), numbers.end(),
                      [](int n) { return n % 2 == 0; }),
        numbers.end()
    );
    
    // Transform: multiply each by 3
    std::transform(numbers.begin(), numbers.end(), numbers.begin(),
                   [](int n) { return n * 3; });
    
    std::cout << "Result: ";
    for(int n : numbers) std::cout << n << " ";
    std::cout << std::endl;
    
    return 0;
}
```

### Example 9: Recursive Lambda (C++14+)

```cpp
#include <iostream>
#include <functional>

int main() {
    // Recursive lambda for factorial
    std::function<int(int)> factorial = [&factorial](int n) -> int {
        return (n <= 1) ? 1 : n * factorial(n - 1);
    };
    
    std::cout << "Factorial of 5: " << factorial(5) << std::endl;  // 120
    
    return 0;
}
```

### Example 10: Lambda in Class (this capture)

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

class DataProcessor {
private:
    int multiplier;
    
public:
    DataProcessor(int m) : multiplier(m) {}
    
    void process(std::vector<int>& data) {
        // Capture 'this' to access member variables
        std::transform(data.begin(), data.end(), data.begin(),
                      [this](int n) { return n * multiplier; });
    }
};

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    DataProcessor processor(10);
    
    processor.process(numbers);
    
    for(int n : numbers) std::cout << n << " ";  // 10 20 30 40 50
    std::cout << std::endl;
    
    return 0;
}
```

## Key Benefits

Lambda functions offer several advantages: they eliminate the need for separate function objects, keep code localized where it's used, enable cleaner syntax with standard algorithms, and can capture context from surrounding scope. They're particularly useful for callbacks, event handlers, and short operations passed to standard library algorithms. Modern C++ code extensively uses lambdas for more expressive and maintainable code.