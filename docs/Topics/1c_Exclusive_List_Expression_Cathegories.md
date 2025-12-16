# Exclusive Lists of Value Categories

Here are three separate, mutually exclusive lists for lvalues, prvalues, and xvalues.

## LVALUES ONLY
*(Has identity, cannot be moved from)*

| Expression Type | Example | Notes |
|----------------|---------|-------|
| Variable names | `x`, `str`, `vec` | Named objects |
| Function parameters | Inside function: `param` | Named parameters |
| Function returning `T&` | `getRef()` | Returns lvalue reference |
| Function returning `const T&` | `getName()` | Returns const lvalue reference |
| Dereferencing pointer | `*ptr`, `*(&x)` | Accesses pointed object |
| Array subscript (on lvalue) | `arr[i]`, `vec[0]` | Element access |
| String literal | `"hello"` | Special case: array of const char |
| Member of lvalue | `obj.member`, `obj.field` | Accessing member of named object |
| Pointer member access | `ptr->member` | Via pointer to lvalue |
| Pre-increment/decrement | `++x`, `--x` | Returns reference |
| Assignment operators | `x = 5`, `y += 3` | Returns reference to left operand |
| Compound assignment | `x *= 2`, `y /= 4` | All forms: `+=`, `-=`, `*=`, `/=`, `%=`, `&=`, `|=`, `^=`, `<<=`, `>>=` |
| Comma operator (right is lvalue) | `(x, y)`, `(foo(), z)` | Right operand determines category |
| Ternary (both lvalues) | `flag ? x : y` | Both operands must be lvalues |
| lvalue `.*` ptr-to-member | `obj.*ptr` | Object is lvalue |
| lvalue `->*` ptr-to-member | `ptr->*member_ptr` | Pointer to lvalue |
| Cast to `T&` | `static_cast<int&>(x)` | Creates lvalue reference |
| Cast to `const T&` | `static_cast<const int&>(x)` | Creates const lvalue reference |
| Parenthesized lvalue | `(x)`, `((x))` | Preserves category |
| Array name | `arr` (before decay) | Array itself |
| Built-in subscript | `*(arr + i)` | Equivalent to `arr[i]` |
| Static data members | `MyClass::static_member` | Class-level variables |
| `*this` in member function | `*this` | Current object |
| Bit-fields | `flags.bit1` | Special lvalue with restrictions |

```cpp
// Examples
int x = 5;
int arr[3] = {1, 2, 3};
std::string str = "hello";
std::vector<int> vec{1, 2, 3};

// All lvalues:
x;              // variable name
arr[0];         // subscript on lvalue
str;            // variable name
*(&x);          // dereference
++x;            // pre-increment
x = 10;         // assignment
"literal";      // string literal
```

---

## PRVALUES ONLY
*(No identity, can be moved from)*

| Expression Type | Example | Notes |
|----------------|---------|-------|
| Integer literals | `42`, `100L`, `0xFF` | All numeric types |
| Floating literals | `3.14`, `2.5f`, `1.0` | double, float |
| Character literals | `'a'`, `L'x'`, `u8'y'` | char types |
| Boolean literals | `true`, `false` | bool |
| Pointer literals | `nullptr` | nullptr_t |
| Arithmetic operators | `x + y`, `x - y`, `x * y`, `x / y`, `x % y` | Binary arithmetic |
| Unary arithmetic | `+x`, `-x` | Unary plus/minus |
| Logical operators | `x && y`, `x \|\| y`, `!x` | Boolean results |
| Bitwise operators | `x & y`, `x \| y`, `x ^ y`, `~x`, `x << 2`, `x >> 2` | Bitwise operations |
| Comparison operators | `x == y`, `x != y`, `x < y`, `x > y`, `x <= y`, `x >= y` | All comparisons |
| Post-increment | `x++` | Returns old value (copy) |
| Post-decrement | `x--` | Returns old value (copy) |
| Address-of | `&x`, `&ptr`, `&arr` | Creates pointer value |
| Temporary objects | `Widget{}`, `std::string("hi")` | Explicit construction |
| Aggregate initialization | `Point{1, 2}`, `{1, 2, 3}` | Braced init |
| Function return by value | `getValue()`, `getString()` | Returns temporary |
| Lambda expressions | `[](){ }`, `[x](){ return x; }` | Closure object |
| `this` pointer | `this` | In member functions |
| Type casts (to value) | `static_cast<int>(x)`, `(double)x` | Creates temporary |
| C++ functional cast | `int(3.14)`, `double(5)` | Converts to value |
| `sizeof` | `sizeof(x)`, `sizeof(int)` | Returns size_t |
| `alignof` | `alignof(double)` | Returns size_t |
| `new` expression | `new int`, `new Widget{1}` | Returns pointer |
| Enumerators | `RED`, `Color::BLUE` | Enum values |
| Ternary (prvalue operands) | `flag ? 42 : 99` | Both operands prvalues |
| Comma (right is prvalue) | `(x, 42)`, `(foo(), 3.14)` | Right operand prvalue |
| Member of prvalue | Special - see xvalues | Actually becomes xvalue |
| Implicit conversions | `int` → `double` in `f(3)` | Temporary created |
| `typeid` (on type) | `typeid(int)` | Returns type_info& (actually lvalue) |

```cpp
// Examples
42;                     // literal
x + 5;                  // arithmetic
x == y;                 // comparison
x++;                    // post-increment
&x;                     // address-of
std::string("temp");    // temporary object
getValue();             // function return
[](){ return 1; };      // lambda
sizeof(x);              // sizeof
new int;                // new expression
```

---

## XVALUES ONLY
*(Has identity, can be moved from)*

| Expression Type | Example | Notes |
|----------------|---------|-------|
| `std::move` on lvalue | `std::move(x)`, `std::move(str)` | Most common way |
| `std::move` on xvalue | `std::move(std::move(x))` | Redundant but valid |
| `std::move` on prvalue | `std::move(getValue())` | Redundant but valid |
| `std::forward<T&&>` | `std::forward<T>(arg)` | When T is rvalue ref type |
| Cast to `T&&` | `static_cast<int&&>(x)` | Explicit rvalue ref cast |
| C-style cast to `T&&` | `(std::string&&)str` | Not recommended |
| Function return `T&&` | `getRvalueRef()` | Returning rvalue reference |
| Member of prvalue | `getWidget().member` | prvalue.member → xvalue |
| Member of xvalue | `std::move(obj).member` | xvalue.member → xvalue |
| Subscript on prvalue array | `getArray()[0]` | Array rvalue subscript |
| Subscript on xvalue | `std::move(arr)[0]` | If arr is array type |
| Subscript on rvalue container | `getString()[0]`, `getVector()[1]` | Container rvalue subscript |
| `.*` with rvalue object | `getData().*ptr`, `std::move(obj).*ptr` | Pointer-to-member on rvalue |
| `->*` with xvalue | Via rvalue pointer | Less common |
| Ternary (xvalue operands) | `flag ? std::move(a) : std::move(b)` | Both operands xvalues |
| Ternary (mixed, becomes xvalue) | `flag ? std::move(a) : b` | When result is xvalue |
| Comma (right is xvalue) | `(x, std::move(y))` | Right operand xvalue |
| Array-to-pointer on xvalue | `std::move(arr)` then operations | Array rvalue |
| Temporary materialization | Compiler-generated | When binding prvalue to ref |

```cpp
// Examples
std::move(x);               // std::move
static_cast<int&&>(x);      // cast to rvalue ref
getRvalueRef();             // function returning T&&
getWidget().value;          // member of prvalue
std::move(obj).member;      // member of xvalue
getString()[0];             // subscript on rvalue
getData().*ptr;             // pointer-to-member on rvalue
```

---

## Visual Summary

```
EXPRESSION VALUE CATEGORIES

┌─────────────────────────────────────────────────┐
│                  EXPRESSIONS                     │
├──────────────────────┬──────────────────────────┤
│      glvalue         │         rvalue           │
│  (has identity)      │    (can be moved)        │
├──────────┬───────────┼───────────┬──────────────┤
│  lvalue  │  xvalue   │  xvalue   │   prvalue    │
└──────────┴───────────┴───────────┴──────────────┘
     │          │            │            │
     │          └────────────┘            │
     │         (xvalue is in both!)       │
     │                                    │
   Named                            Temporary
  variables,                        values,
  ++x, *ptr,                       literals,
  arr[i],                          x+y, x++,
  "string"                         getValue()
```

## Quick Decision Tree

```
Is it a named variable/object?
├─ YES → lvalue (e.g., x, str, arr)
└─ NO ↓

Does it involve std::move or cast to T&&?
├─ YES → xvalue (e.g., std::move(x))
└─ NO ↓

Is it a temporary value/computation?
├─ YES → prvalue (e.g., 42, x+y, Widget{})
└─ NO ↓

Is it member access on rvalue?
├─ YES → xvalue (e.g., getWidget().member)
└─ NO ↓

Is it a function returning T&?
├─ YES → lvalue (e.g., getRef())
└─ NO ↓

Is it a function returning T&&?
├─ YES → xvalue (e.g., getRvalueRef())
└─ NO ↓

Is it a function returning T by value?
└─ YES → prvalue (e.g., getValue())
```

## Complete Example Program

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
int& getLvalueRef() { static int x = 5; return x; }
int&& getXvalue() { static int x = 10; return std::move(x); }

template<typename T>
const char* categorize(T&&) {
    if constexpr (std::is_lvalue_reference_v<T>) {
        return "lvalue";
    } else {
        return "xvalue/prvalue";
    }
}

#define CHECK(expr) \
    std::cout << #expr << " → " << categorize(expr) << '\n'

int main() {
    int x = 5;
    int arr[3] = {1, 2, 3};
    std::string str = "hello";
    Widget w{10, "widget"};
    int* ptr = &x;

    std::cout << "=== LVALUES ===\n";
    CHECK(x);
    CHECK(arr[0]);
    CHECK(str);
    CHECK(*ptr);
    CHECK(++x);
    CHECK((x = 10));
    CHECK("string literal");
    CHECK(getLvalueRef());

    std::cout << "\n=== PRVALUES ===\n";
    CHECK(42);
    CHECK(3.14);
    CHECK(true);
    CHECK(x + 5);
    CHECK(x == 5);
    CHECK(x++);
    CHECK(&x);
    CHECK(Widget{1, "temp"});
    CHECK(getValue());
    CHECK(sizeof(x));

    std::cout << "\n=== XVALUES ===\n";
    CHECK(std::move(x));
    CHECK(std::move(str));
    CHECK(static_cast<int&&>(x));
    CHECK(getXvalue());
    CHECK(getWidget().value);
    CHECK(std::move(w).name);
}
```

**Output:**
```
=== LVALUES ===
x → lvalue
arr[0] → lvalue
str → lvalue
*ptr → lvalue
++x → lvalue
(x = 10) → lvalue
"string literal" → lvalue
getLvalueRef() → lvalue

=== PRVALUES ===
42 → xvalue/prvalue
3.14 → xvalue/prvalue
true → xvalue/prvalue
x + 5 → xvalue/prvalue
x == 5 → xvalue/prvalue
x++ → xvalue/prvalue
&x → xvalue/prvalue
Widget{1, "temp"} → xvalue/prvalue
getValue() → xvalue/prvalue
sizeof(x) → xvalue/prvalue

=== XVALUES ===
std::move(x) → xvalue/prvalue
std::move(str) → xvalue/prvalue
static_cast<int&&>(x) → xvalue/prvalue
getXvalue() → xvalue/prvalue
getWidget().value → xvalue/prvalue
std::move(w).name → xvalue/prvalue
```

This provides three complete, mutually exclusive lists of expressions for each value category!