# C++ Operator Overloading: Best Practices & Pitfalls

## Overview

Operator overloading in C++ allows you to redefine how operators (like `+`, `-`, `*`, `==`, etc.) work with user-defined types. When done well, it creates intuitive, readable code that feels natural. When done poorly, it creates confusion and maintenance nightmares.

## Core Concepts

Operator overloading lets you write expressions like `a + b` where `a` and `b` are custom objects, not just built-in types. The compiler translates `a + b` into either `a.operator+(b)` (member function) or `operator+(a, b)` (free function).

## Best Practices

**Follow the Principle of Least Astonishment**: Your overloaded operators should behave intuitively, matching how users expect the operator to work. If `+` doesn't mean "addition-like" behavior, don't overload it. The classic bad example is using `<<` for something other than output or bit-shifting.

**Maintain Mathematical Consistency**: If you overload one operator in a family, overload the related ones too. For example, if you implement `==`, also implement `!=`. If you have `<`, consider implementing `<=`, `>`, `>=` as well. With C++20, you can use the spaceship operator `<=>` to generate all comparison operators automatically.

**Preserve Semantics**: Operators should maintain their expected properties. If `a + b` works, users expect `b + a` to work for commutative operations. If you increment with `++`, the value should actually increase in some meaningful way.

**Return Types Matter**:
- Arithmetic operators (`+`, `-`, `*`) should return by value
- Compound assignment operators (`+=`, `-=`) should return `*this` by reference
- Comparison operators should return `bool`
- Increment/decrement should return appropriately (prefix returns reference, postfix returns value)

**Member vs. Free Function**: Use member functions when the left operand is always your class type. Use free functions (often as friends) when you need symmetry or when the left operand might be a different type. For example, binary arithmetic operators are typically free functions to allow implicit conversions on both sides.

```cpp
// Member function - only allows MyClass + something
MyClass MyClass::operator+(const MyClass& other) const;

// Free function - allows symmetrical operations
MyClass operator+(const MyClass& lhs, const MyClass& rhs);
```

**Always Mark Read-Only Operators as const**: Operators that don't modify the object (arithmetic, comparison) should be `const` member functions.

**Implement Compound Assignment First**: When implementing arithmetic operators, define the compound assignment version first (`+=`, `-=`, etc.), then implement the simple operator in terms of it:

```cpp
MyClass& MyClass::operator+=(const MyClass& rhs) {
    // Do the actual work here
    return *this;
}

MyClass operator+(MyClass lhs, const MyClass& rhs) {
    lhs += rhs;  // Reuse compound assignment
    return lhs;
}
```

**Handle Self-Assignment**: For assignment operators and compound assignments, ensure your code handles `x = x` or `x += x` correctly.

**Follow the Rule of Five/Three/Zero**: If you overload the assignment operator, you likely need to follow the Rule of Five (destructor, copy constructor, copy assignment, move constructor, move assignment) or use the Rule of Zero (rely on compiler-generated defaults with smart pointers).

## Common Pitfalls

**Overloading `&&` and `||`**: Don't do this. These operators have special short-circuit evaluation semantics in C++ that you lose when overloading them. Your overloaded version will always evaluate both operands.

**Inconsistent Operator Sets**: Implementing `<` but not `==`, or `+` but not `+=`. Users expect these to exist together.

**Breaking Commutativity**: If multiplication should be commutative but `a * b` works while `b * a` doesn't (perhaps because you only defined a member function), that's confusing.

**Surprising Return Types**: Returning references when you should return values, or vice versa. For instance, returning a reference to a local variable is undefined behavior.

**Ignoring `operator[]` Const-Correctness**: Provide both const and non-const versions of `operator[]` for container-like classes:

```cpp
T& operator[](size_t index);              // non-const version
const T& operator[](size_t index) const;  // const version
```

**Overusing Operator Overloading**: Just because you can doesn't mean you should. If the operation isn't naturally operator-like, use a named function instead. Don't overload `+` to mean "merge databases" just to save typing.

**Forgetting Symmetry**: If you overload `operator==`, always overload `operator!=`. With C++20, you can define only `operator<=>` and `operator==`, and the compiler generates the rest.

**Implicit Conversions Gone Wrong**: Single-argument constructors create implicit conversions that interact with operator overloading. Use `explicit` to prevent unwanted conversions unless implicit conversion is truly desired.

**Postfix vs. Prefix Increment**: The postfix version takes a dummy `int` parameter to distinguish it from prefix. Remember that postfix returns the old value (by value), while prefix returns the new value (by reference):

```cpp
MyClass& operator++();        // prefix: ++obj
MyClass operator++(int);      // postfix: obj++
```

**Memory Management in Assignment**: Forgetting to check for self-assignment or properly clean up existing resources before assignment can lead to memory leaks or crashes.

**Stream Operators Must Be Free Functions**: `operator<<` and `operator>>` for streams must be free functions because `ostream` is on the left side:

```cpp
std::ostream& operator<<(std::ostream& os, const MyClass& obj);
```

## Special Cases

**Function Call Operator**: `operator()` makes objects callable and is essential for functors, lambda-like behavior, and implementing custom iterators.

**Smart Pointers**: Overloading `operator*` and `operator->` allows you to create smart pointer types that manage memory automatically.

**Comma Operator**: Can be overloaded but almost never should be. Its use in C++ is already minimal and overloading it creates confusion.

**Address-of Operator**: Overloading `operator&` is possible but highly discouraged as it breaks fundamental assumptions about pointer operations.

## Modern C++ Improvements

C++20's spaceship operator (`<=>`) dramatically simplifies comparison operator overloading. By defining just `operator<=>` and `operator==`, the compiler can generate all six comparison operators. This is now the preferred approach for new code.

The key to good operator overloading is making your code more readable and intuitive, not more clever. When in doubt, use a named member function instead.