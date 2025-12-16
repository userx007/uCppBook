
## **What is `constexpr`?**

`constexpr` tells the compiler that a value or function *can* be evaluated at **compile-time**. This enables:
- Zero runtime overhead for computations
- Values usable in constant expressions (array sizes, template parameters)
- Compile-time validation and optimization

## **Key Evolution Points:**

### **C++11 - Introduction**
- Very restrictive: only single `return` statement
- No loops, no local variables
- Had to use recursion for iteration
- Still powerful for simple calculations

### **C++14 - Relaxation**
- Multiple statements allowed
- Loops (`for`, `while`) permitted
- Local variables and mutation allowed
- Made `constexpr` practical for real algorithms

### **C++17 - Compile-Time Branching**
- `if constexpr` - branches removed at compile-time
- `constexpr` lambdas
- More standard library functions became `constexpr`

### **C++20 - Major Expansion**
- Virtual functions can be `constexpr`
- Destructors can be `constexpr`
- `std::string`, `std::vector` work in `constexpr` contexts
- `try-catch` blocks allowed (though throwing still prevents compile-time evaluation)

## **Important Concepts:**

1. **Dual nature**: A `constexpr` function can run at compile-time *or* runtime depending on whether arguments are constant expressions

2. **Compile-time guarantee**: Use `constexpr` variables to force compile-time evaluation:
   ```cpp
   constexpr int x = factorial(5);  // MUST compute at compile-time
   ```

3. **Practical uses**: Hash functions, lookup tables, validation, compile-time unit conversions

