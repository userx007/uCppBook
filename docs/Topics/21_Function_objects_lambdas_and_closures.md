# Function Objects, Lambdas, and Closures in C++

These three concepts are closely related mechanisms in C++ for creating callable objects with associated state. Let me explain each and how they interconnect.

## Function Objects (Functors)

A function object, or functor, is any object that can be called as if it were a function. In C++, this is achieved by overloading the function call operator `operator()`. The most common form is a class or struct that defines this operator, allowing instances to be invoked with function call syntax.

The key advantage of function objects over regular function pointers is that they can maintain state between calls. When you create a functor, it can have member variables that persist across invocations, and different instances of the same functor class can have different internal states. For example, you might create a counter functor that increments an internal variable each time it's called, or a comparison functor that uses a stored threshold value to make decisions.

Function objects are particularly useful with STL algorithms. Before C++11 introduced lambdas, functors were the primary way to pass custom behavior to algorithms like `std::sort`, `std::find_if`, or `std::transform`. You would define a class with `operator()` implementing your logic, instantiate it, and pass that instance to the algorithm. The STL even provides predefined functors like `std::less`, `std::greater`, and `std::plus` for common operations.

## Lambda Expressions

Lambda expressions, introduced in C++11, provide a concise syntax for creating anonymous function objects inline. Rather than defining a separate class with `operator()`, you can write a lambda directly where you need it. The basic syntax consists of a capture clause in square brackets, a parameter list in parentheses, and a body in curly braces.

The real power of lambdas comes from their capture clause, which specifies what variables from the surrounding scope should be accessible inside the lambda body. You can capture by value using `[=]`, by reference using `[&]`, or explicitly list specific variables like `[x, &y]` to capture `x` by value and `y` by reference. This capture mechanism is what enables lambdas to access and work with local variables from their defining scope.

Under the hood, the compiler transforms each lambda expression into an unnamed functor class. The captured variables become member variables of this generated class, and the lambda body becomes the implementation of `operator()`. This means lambdas are essentially syntactic sugar for creating function objects, but the convenience they provide is substantial, especially for simple operations that would be tedious to write as full class definitions.

## Closures

A closure is the runtime object created when a lambda expression is evaluated. While "lambda" refers to the compile-time expression in your code, "closure" refers to the actual callable object that exists at runtime. When you write a lambda that captures variables, the resulting closure contains copies or references to those captured variables, forming a self-contained package of code and data.

The distinction between lambdas and closures parallels the distinction between classes and objects. A lambda expression is like a class definition—it describes what the callable entity looks like and how it behaves. A closure is like an object instance—it's the actual thing created in memory that holds specific values for captured variables and can be invoked.

Closures can be stored, copied, and passed around just like any other object. When you assign a lambda to a variable, you're actually storing the closure object. If the lambda captures variables by value, each copy of the closure has its own independent copies of those captured values. If it captures by reference, you need to be careful about lifetime issues—the closure holds references that will dangle if the referenced variables go out of scope.

## The Relationship and Practical Usage

These three concepts form a progression in C++'s evolution. Function objects were the original mechanism for creating callable entities with state. Lambdas provide a more convenient syntax for the common case of creating small, single-use functors. Closures are what actually execute when you call a lambda, representing the concrete instantiation of the abstract lambda expression.

In modern C++, you'll typically use lambdas for most scenarios where you need a small callable with captured state, such as when passing predicates to algorithms or defining callbacks. You might still use traditional function objects when you need a more complex callable that benefits from explicit class structure, when you want to give the functor a meaningful type name, or when the callable needs multiple member functions beyond just `operator()`.

The type system treats all of these consistently through the lens of function objects. A lambda's type is an unnamed functor type, and you can store closures in variables using `auto`, or in containers using `std::function` for type erasure when you need to store functors of different types together. Generic code that accepts callable objects through template parameters works seamlessly with function pointers, function objects, and lambdas alike, thanks to the uniform function call syntax they all support.

Understanding the connection between these concepts helps clarify why lambdas behave as they do, particularly around capture semantics and lifetime management. It also reveals that the apparently different constructs are really variations on the same fundamental theme: creating objects that bundle together code to execute with the state that code needs to operate on.

## Key Examples Included:

**1. Basic Function Objects**
- Simple functors without state
- Functors with internal state (Counter)
- Functors designed for STL algorithms (GreaterThan)

**2. Lambda Basics**
- Various lambda syntaxes
- Parameter passing
- Explicit return types
- Immediately invoked lambdas

**3. Capture Modes**
- Capture by value `[x]`
- Capture by reference `[&x]`
- Capture all by value `[=]`
- Capture all by reference `[&]`
- Mixed captures `[x, &y]`

**4. Mutable Lambdas**
- Why `mutable` is needed for modifying captured values
- Difference between mutable copies and reference captures

**5. Generic Lambdas (C++14)**
- Auto parameters for type-independent code
- Working with multiple types

**6. Init Captures (C++14)**
- Initializing variables in capture clause
- Move captures for unique_ptr and move-only types

**7. Closure Lifetime**
- Dangerous dangling reference captures
- Safe patterns with value captures
- Using shared_ptr for extended lifetime

**8. STL Integration**
- Using functors vs lambdas with algorithms
- `find_if`, `transform`, `count_if`, `sort`
- Practical comparisons

**9. Type Erasure with std::function**
- Storing different callable types
- Building callback systems
- Vector of heterogeneous callables

**10. Practical Event System**
- Real-world button click handler example
- Multiple handlers with different captures

**11. Recursive Lambdas**
- Using `std::function` for recursion
- Factorial and Fibonacci examples

**12. Stateful Comparisons**
- When to use functors vs lambdas
- Trade-offs between approaches

You can compile and run this code with C++14 or later to see all the concepts in action. Each section is independent and clearly demonstrates specific features.

