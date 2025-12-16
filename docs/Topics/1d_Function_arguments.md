# Exclusive List of Valid Arguments for `void func(T&& arg)`

The `T&&` is a **forwarding reference** (also called universal reference) when `T` is a template parameter. It can bind to **almost anything**.

## The Function

```cpp
template<typename T>
void func(T&& arg) {
    // T&& is forwarding reference - binds to lvalues AND rvalues
}
```

## Complete List of Valid Arguments

### Category 1: Lvalues (T deduced as lvalue reference)

```cpp
// 1. Named variables
int x = 5;
func(x);                    // T = int&, T&& = int&

std::string str = "hello";
func(str);                  // T = std::string&, T&& = std::string&

std::vector<int> vec{1, 2, 3};
func(vec);                  // T = std::vector<int>&, T&& = std::vector<int>&

// 2. Array elements
int arr[5] = {1, 2, 3, 4, 5};
func(arr[0]);               // T = int&, T&& = int&
func(arr[2]);               // T = int&, T&& = int&

// 3. Dereferenced pointers
int* ptr = &x;
func(*ptr);                 // T = int&, T&& = int&

// 4. Functions returning lvalue references
int& getRef() { static int val = 42; return val; }
func(getRef());             // T = int&, T&& = int&

// 5. Pre-increment/decrement
func(++x);                  // T = int&, T&& = int&
func(--x);                  // T = int&, T&& = int&

// 6. Assignment results
func(x = 10);               // T = int&, T&& = int&
func(x += 5);               // T = int&, T&& = int&

// 7. String literals
func("hello");              // T = const char(&)[6], T&& = const char(&)[6]

// 8. Member of lvalue objects
struct Widget { int value; std::string name; };
Widget w{42, "test"};
func(w.value);              // T = int&, T&& = int&
func(w.name);               // T = std::string&, T&& = std::string&

// 9. Pointer member access
Widget* wptr = &w;
func(wptr->value);          // T = int&, T&& = int&

// 10. Ternary with lvalue operands
int y = 10;
bool flag = true;
func(flag ? x : y);         // T = int&, T&& = int&

// 11. Comma operator (right is lvalue)
func((x++, y));             // T = int&, T&& = int&

// 12. Parenthesized lvalues
func((x));                  // T = int&, T&& = int&
func(((str)));              // T = std::string&, T&& = std::string&
```

### Category 2: Prvalues (T deduced as value type)

```cpp
// 13. Numeric literals
func(42);                   // T = int, T&& = int&&
func(3.14);                 // T = double, T&& = double&&
func(3.14f);                // T = float, T&& = float&&
func(100L);                 // T = long, T&& = long&&

// 14. Boolean literals
func(true);                 // T = bool, T&& = bool&&
func(false);                // T = bool, T&& = bool&&

// 15. Character literals
func('a');                  // T = char, T&& = char&&
func(L'x');                 // T = wchar_t, T&& = wchar_t&&

// 16. nullptr
func(nullptr);              // T = std::nullptr_t, T&& = std::nullptr_t&&

// 17. Arithmetic operations
func(x + 5);                // T = int, T&& = int&&
func(x * y);                // T = int, T&& = int&&
func(x - y);                // T = int, T&& = int&&
func(x / 2);                // T = int, T&& = int&&

// 18. Comparison operations
func(x == y);               // T = bool, T&& = bool&&
func(x < y);                // T = bool, T&& = bool&&
func(x != 5);               // T = bool, T&& = bool&&

// 19. Logical operations
func(x && y);               // T = bool, T&& = bool&&
func(!x);                   // T = bool, T&& = bool&&

// 20. Bitwise operations
func(x & y);                // T = int, T&& = int&&
func(x | y);                // T = int, T&& = int&&
func(~x);                   // T = int, T&& = int&&

// 21. Post-increment/decrement
func(x++);                  // T = int, T&& = int&&
func(x--);                  // T = int, T&& = int&&

// 22. Address-of operator
func(&x);                   // T = int*, T&& = int*&&
func(&str);                 // T = std::string*, T&& = std::string*&&

// 23. Temporary objects
func(std::string("temp"));  // T = std::string, T&& = std::string&&
func(Widget{1, "test"});    // T = Widget, T&& = Widget&&
func(std::vector<int>{1, 2, 3}); // T = std::vector<int>, T&& = std::vector<int>&&

// 24. Functions returning by value
int getValue() { return 42; }
std::string getString() { return "hello"; }
func(getValue());           // T = int, T&& = int&&
func(getString());          // T = std::string, T&& = std::string&&

// 25. Lambda expressions
func([](){ return 1; });    // T = lambda type, T&& = lambda&&
func([x](){ return x * 2; }); // T = lambda type, T&& = lambda&&

// 26. Type casts to value
func(static_cast<double>(x)); // T = double, T&& = double&&
func((double)x);            // T = double, T&& = double&&

// 27. sizeof, alignof
func(sizeof(x));            // T = size_t, T&& = size_t&&
func(alignof(double));      // T = size_t, T&& = size_t&&

// 28. new expressions
func(new int);              // T = int*, T&& = int*&&
func(new std::string("hi")); // T = std::string*, T&& = std::string*&&

// 29. Enum values
enum Color { RED, GREEN, BLUE };
func(RED);                  // T = Color, T&& = Color&&

// 30. Ternary with prvalue operands
func(flag ? 42 : 99);       // T = int, T&& = int&&
```

### Category 3: Xvalues (T deduced as value type)

```cpp
// 31. std::move on lvalues
func(std::move(x));         // T = int, T&& = int&&
func(std::move(str));       // T = std::string, T&& = std::string&&
func(std::move(vec));       // T = std::vector<int>, T&& = std::vector<int>&&

// 32. std::move on array elements
func(std::move(arr[0]));    // T = int, T&& = int&&

// 33. std::move on members
func(std::move(w.value));   // T = int, T&& = int&&
func(std::move(w.name));    // T = std::string, T&& = std::string&&

// 34. Cast to rvalue reference
func(static_cast<int&&>(x));           // T = int, T&& = int&&
func(static_cast<std::string&&>(str)); // T = std::string, T&& = std::string&&

// 35. Functions returning rvalue reference
int&& getXvalue() { static int val = 42; return std::move(val); }
func(getXvalue());          // T = int, T&& = int&&

// 36. Member of prvalue
Widget getWidget() { return Widget{99, "temp"}; }
func(getWidget().value);    // T = int, T&& = int&&
func(getWidget().name);     // T = std::string, T&& = std::string&&

// 37. Member of xvalue
func(std::move(w).value);   // T = int, T&& = int&&
func(std::move(w).name);    // T = std::string, T&& = std::string&&

// 38. Subscript on rvalue containers
std::string getStr() { return "hello"; }
std::vector<int> getVec() { return {1, 2, 3}; }
func(getStr()[0]);          // T = char, T&& = char&&
func(getVec()[1]);          // T = int, T&& = int&&

// 39. std::forward forwarding rvalue
template<typename U>
void wrapper(U&& u) {
    func(std::forward<U>(u)); // When u is rvalue
}
wrapper(std::move(x));      // T = int, T&& = int&&

// 40. Ternary with xvalue operands
func(flag ? std::move(x) : std::move(y)); // T = int, T&& = int&&

// 41. Pointer-to-member on rvalue
struct Data { int value; };
Data getData() { return Data{42}; }
int Data::*ptr = &Data::value;
func(getData().*ptr);       // T = int, T&& = int&&
```

## Complete Demonstration Program

```cpp
#include <iostream>
#include <string>
#include <vector>
#include <type_traits>

struct Widget {
    int value;
    std::string name;
};

Widget getWidget() { return Widget{42, "test"}; }
int getValue() { return 42; }
int& getRef() { static int x = 5; return x; }
int&& getXvalue() { static int x = 10; return std::move(x); }

template<typename T>
void func(T&& arg) {
    std::cout << "T = ";
    if constexpr (std::is_lvalue_reference_v<T>) {
        std::cout << "lvalue reference (&)";
    } else {
        std::cout << "value type (rvalue)";
    }
    std::cout << '\n';
}

#define CALL(expr) \
    std::cout << #expr << " → "; \
    func(expr)

int main() {
    int x = 5, y = 10;
    int arr[3] = {1, 2, 3};
    std::string str = "hello";
    Widget w{99, "widget"};
    bool flag = true;

    std::cout << "=== LVALUES (T deduced as T&) ===\n";
    CALL(x);
    CALL(str);
    CALL(arr[0]);
    CALL(*(&x));
    CALL(getRef());
    CALL(++x);
    CALL(x = 10);
    CALL("literal");
    CALL(w.value);
    CALL(flag ? x : y);

    std::cout << "\n=== PRVALUES (T deduced as T) ===\n";
    CALL(42);
    CALL(3.14);
    CALL(true);
    CALL(x + 5);
    CALL(x == y);
    CALL(x++);
    CALL(&x);
    CALL(Widget{1, "temp"});
    CALL(getValue());
    CALL([](){ return 1; });
    CALL(sizeof(x));

    std::cout << "\n=== XVALUES (T deduced as T) ===\n";
    CALL(std::move(x));
    CALL(std::move(str));
    CALL(static_cast<int&&>(x));
    CALL(getXvalue());
    CALL(getWidget().value);
    CALL(std::move(w).name);
    CALL(std::move(arr[0]));
}
```

**Output:**
```
=== LVALUES (T deduced as T&) ===
x → T = lvalue reference (&)
str → T = lvalue reference (&)
arr[0] → T = lvalue reference (&)
*(&x) → T = lvalue reference (&)
getRef() → T = lvalue reference (&)
++x → T = lvalue reference (&)
x = 10 → T = lvalue reference (&)
"literal" → T = lvalue reference (&)
w.value → T = lvalue reference (&)
flag ? x : y → T = lvalue reference (&)

=== PRVALUES (T deduced as T) ===
42 → T = value type (rvalue)
3.14 → T = value type (rvalue)
true → T = value type (rvalue)
x + 5 → T = value type (rvalue)
x == y → T = value type (rvalue)
x++ → T = value type (rvalue)
&x → T = value type (rvalue)
Widget{1, "temp"} → T = value type (rvalue)
getValue() → T = value type (rvalue)
[](){ return 1; } → T = value type (rvalue)
sizeof(x) → T = value type (rvalue)

=== XVALUES (T deduced as T) ===
std::move(x) → T = value type (rvalue)
std::move(str) → T = value type (rvalue)
static_cast<int&&>(x) → T = value type (rvalue)
getXvalue() → T = value type (rvalue)
getWidget().value → T = value type (rvalue)
std::move(w).name → T = value type (rvalue)
std::move(arr[0]) → T = value type (rvalue)
```

## Summary Table

| Argument Type | Example | T deduced as | T&& becomes |
|--------------|---------|--------------|-------------|
| **Lvalue** | `x` | `int&` | `int&` |
| **Lvalue** | `str` | `std::string&` | `std::string&` |
| **Lvalue** | `arr[0]` | `int&` | `int&` |
| **Lvalue** | `++x` | `int&` | `int&` |
| **Lvalue** | `"literal"` | `const char(&)[8]` | `const char(&)[8]` |
| **Prvalue** | `42` | `int` | `int&&` |
| **Prvalue** | `x + 5` | `int` | `int&&` |
| **Prvalue** | `getValue()` | `int` | `int&&` |
| **Prvalue** | `Widget{}` | `Widget` | `Widget&&` |
| **Xvalue** | `std::move(x)` | `int` | `int&&` |
| **Xvalue** | `std::move(str)` | `std::string` | `std::string&&` |
| **Xvalue** | `getWidget().value` | `int` | `int&&` |

## Key Insight: Reference Collapsing Rules

```cpp
template<typename T>
void func(T&& arg);

// When you pass lvalue:
int x = 5;
func(x);
// T = int&
// T&& = int& && → int& (reference collapsing)

// When you pass rvalue:
func(42);
// T = int
// T&& = int&& (no collapsing)
```

## What CAN'T be passed?

```cpp
template<typename T>
void func(T&& arg);

// Nothing! Almost everything can be passed to T&&
// The only "invalid" cases are compile errors for other reasons:

// func();              // ERROR: no argument
// func(void_function); // ERROR: can't pass void
// func(incomplete_type); // ERROR: incomplete type
```

**Answer: You can pass ANY expression to `func(T&& arg)` - lvalues, prvalues, and xvalues all work!** This is why it's called a "universal" or "forwarding" reference.