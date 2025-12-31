# What is an Expression in C++?

Great fundamental question! Let me clarify the difference between **statements**, **declarations**, and **expressions**.

## Definition: Expression

An **expression** is a sequence of operators and operands that:
1. **Computes a value**, OR
2. **Designates an object or function**, OR
3. **Generates side effects**, OR
4. **Does a combination of the above**

Most importantly: **An expression has a type and a value category** (lvalue, prvalue, xvalue).

## Your Examples

```cpp
int arr[5] = {1, 2, 3, 4, 5};  // NOT an expression - it's a DECLARATION
arr[0];                         // YES, this IS an expression
```

### Why the difference?

```cpp
// This is a DECLARATION (statement)
int arr[5] = {1, 2, 3, 4, 5};
//  └─┬──┘   └──────┬──────┘
//  type    initializer
// Introduces a new name into scope
// NOT an expression - it's a declaration statement

// This is an EXPRESSION
arr[0];
// └─┬─┘
//   └─ Has type: int
//      Has value category: lvalue
//      Evaluates to the first element of arr
```

## Breaking Down Your First Example

```cpp
int arr[5] = {1, 2, 3, 4, 5};
```

This line contains:
- **Declaration**: `int arr[5]` - introduces `arr`
- **Initializer**: `{1, 2, 3, 4, 5}` - this part IS an expression (braced-init-list)

But the **whole line** is a **declaration statement**, not an expression.

## What ARE Expressions?

```cpp
#include <iostream>

int main() {
    int x = 5;          // Declaration (not expression)
    int y = 10;         // Declaration (not expression)

    // All of these ARE expressions:
    x;                  // Expression (lvalue, type: int)
    42;                 // Expression (prvalue, type: int)
    x + y;              // Expression (prvalue, type: int)
    x = 20;             // Expression (lvalue, type: int&)
    ++x;                // Expression (lvalue, type: int&)
    x++;                // Expression (prvalue, type: int)
    x > y;              // Expression (prvalue, type: bool)
    x * 2 + y / 3;      // Expression (prvalue, type: int)
    &x;                 // Expression (prvalue, type: int*)
    *(&x);              // Expression (lvalue, type: int)
    sizeof(x);          // Expression (prvalue, type: size_t)

    // Function calls are expressions
    std::cout << "hi";  // Expression (returns ostream&)

    // Even this whole thing is an expression:
    (x = 5, y = 10, x + y);  // Expression (evaluates to 15)
}
```

## What are NOT Expressions?

```cpp
// DECLARATIONS - NOT expressions
int x;
int y = 5;
double arr[10];
std::string str = "hello";
void func();
struct Point { int x, y; };
class MyClass { };
using IntPtr = int*;
typedef int Integer;

// STATEMENTS - NOT expressions (but may contain expressions)
if (x > 5) { }          // if-statement (contains expression: x > 5)
while (x < 10) { }      // while-statement (contains expression: x < 10)
for (int i = 0; i < 5; ++i) { }  // for-statement
return x;               // return-statement (contains expression: x)
break;                  // jump-statement
continue;               // jump-statement

// PREPROCESSOR DIRECTIVES - NOT expressions
#include <iostream>
#define MAX 100
#ifdef DEBUG
#endif
```

## Key Test: Can You Ask "What is its type and value category?"

```cpp
int arr[5] = {1, 2, 3, 4, 5};

// Question: What is the type of "int arr[5] = {1, 2, 3, 4, 5};" ?
// Answer: This question doesn't make sense - it's a declaration, not an expression!

// Question: What is the type of "arr[0]" ?
// Answer: int, and it's an lvalue ✓
// This makes sense - arr[0] IS an expression!

// Question: What is the type of "arr" ?
// Answer: int[5], and it's an lvalue ✓
// arr (by itself) IS an expression!
```

## Detailed Examples

### Example 1: Declaration vs Expression

```cpp
// Declaration (NOT an expression)
int x = 42;
//      └┬┘
//       └─ But 42 IS an expression (prvalue, type: int)

// After declaration, using the name is an expression:
x;          // Expression (lvalue, type: int)
x + 1;      // Expression (prvalue, type: int)
```

### Example 2: Array Declaration vs Array Usage

```cpp
// Declaration (NOT an expression)
int arr[5] = {1, 2, 3, 4, 5};

// Using the array - these ARE expressions:
arr;        // Expression (lvalue, type: int[5])
arr[0];     // Expression (lvalue, type: int)
arr[1];     // Expression (lvalue, type: int)
&arr;       // Expression (prvalue, type: int(*)[5])
arr + 2;    // Expression (prvalue, type: int*)
```

### Example 3: In Context

```cpp
#include <iostream>

int main() {
    // This whole line is a DECLARATION STATEMENT
    int x = 5;
    //      └┬┘
    //       └─ This part (5) is an expression used as initializer

    // This is an EXPRESSION STATEMENT
    x = 10;
    // └──┬─┘
    //    └─ Expression (the assignment)

    // if-statement contains an EXPRESSION
    if (x > 5) {  // "x > 5" is an expression
        std::cout << x;  // "std::cout << x" is an expression
    }

    // return-statement contains an EXPRESSION
    return 0;  // "0" is an expression
}
```

## Expression Categories

```cpp
int x = 5;
int y = 10;
int arr[3] = {1, 2, 3};

// Primary expressions (simplest form)
x               // identifier
42              // literal
"hello"         // string literal
this            // in member function
(x + y)         // parenthesized expression

// Postfix expressions
arr[0]          // subscript
x++             // post-increment
x--             // post-decrement
func()          // function call
obj.member      // member access
ptr->member     // member access through pointer

// Unary expressions
++x             // pre-increment
--x             // pre-decrement
&x              // address-of
*ptr            // dereference
+x              // unary plus
-x              // unary minus
!x              // logical NOT
~x              // bitwise NOT
sizeof(x)       // sizeof

// Binary expressions
x + y           // arithmetic
x - y
x * y
x / y
x % y
x == y          // comparison
x != y
x < y
x > y
x <= y
x >= y
x && y          // logical
x || y
x & y           // bitwise
x | y
x ^ y
x << y
x >> y
x = y           // assignment
x += y          // compound assignment

// Ternary expression
x > y ? x : y   // conditional

// Other expressions
throw x         // throw expression
x, y            // comma expression
new int         // new expression
delete ptr      // delete expression
static_cast<int>(x)  // cast expression
```

## Can Expressions Be Statements?

**Yes!** An **expression statement** is an expression followed by a semicolon:

```cpp
int x = 5;

x = 10;         // Expression statement (expression: x = 10)
x++;            // Expression statement (expression: x++)
func();         // Expression statement (expression: func())
x + 5;          // Expression statement (but result discarded - unusual)
42;             // Expression statement (legal but useless)
```

## Practical Test Examples

```cpp
#include <iostream>
#include <type_traits>

template<typename T>
void analyzeExpression(T&& expr, const char* desc) {
    std::cout << desc << ":\n";
    std::cout << "  Type: " << typeid(T).name() << '\n';
    std::cout << "  Category: "
              << (std::is_lvalue_reference_v<T> ? "lvalue" : "rvalue")
              << '\n';
}

int main() {
    int arr[5] = {1, 2, 3, 4, 5};  // Declaration, not expression
    int x = 10;                     // Declaration, not expression

    // Now we use expressions:
    analyzeExpression(arr, "arr");           // arr is expression
    analyzeExpression(arr[0], "arr[0]");     // arr[0] is expression
    analyzeExpression(x, "x");               // x is expression
    analyzeExpression(x + 5, "x + 5");       // x + 5 is expression
    analyzeExpression(&x, "&x");             // &x is expression

    // Can't do this - not expressions:
    // analyzeExpression(int y = 5, "declaration");  // ERROR
    // analyzeExpression(int arr2[3], "declaration"); // ERROR
}
```

## Summary

| Code | Is it an Expression? | Why? |
|------|---------------------|------|
| `int arr[5] = {1, 2, 3, 4, 5};` | ❌ NO | Declaration statement |
| `arr` | ✅ YES | Identifier (type: int[5], lvalue) |
| `arr[0]` | ✅ YES | Subscript (type: int, lvalue) |
| `{1, 2, 3, 4, 5}` | ✅ YES | Braced-init-list (initializer) |
| `int x;` | ❌ NO | Declaration |
| `x` | ✅ YES | Identifier (type: int, lvalue) |
| `x = 5` | ✅ YES | Assignment (type: int&, lvalue) |
| `x + 5` | ✅ YES | Addition (type: int, prvalue) |
| `if (x > 5)` | ❌ NO | Statement (contains expression `x > 5`) |
| `x > 5` | ✅ YES | Comparison (type: bool, prvalue) |

**Key Insight**:
- **Declarations** introduce names → NOT expressions
- **Using those names** → ARE expressions
- `int arr[5] = {1, 2, 3, 4, 5};` → Declaration
- `arr[0]` → Expression

Expressions have **type** and **value category**. Declarations don't!