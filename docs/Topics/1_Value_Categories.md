# C++ Value Categories

C++ has a sophisticated value category system that helps determine how expressions can be used and what operations are valid. Let me break down each category with clear examples.

## Overview

There are 5 value categories organized in a hierarchy
- glvalue (generalized lvalue) has identity
  - lvalue has identity, can't be moved from
  - xvalue has identity, can be moved from
- rvalue can be moved from
  - xvalue has identity, can be moved from
  - prvalue doesn't have identity, can be moved from

## 1. lvalue (left value)

An lvalue is an expression that has an identity (memory address) and cannot be moved from. You can take its address with `&`.

```cpp
int x = 10;            x is an lvalue
int ptr = &x;         OK can take address of lvalue

int& getRef() { static int a = 5; return a; }
getRef() = 20;         OK function returning lvalue reference is lvalue

stdstring name = Alice;
name[0] = 'B';         OK subscript operator returns lvalue
```

Key characteristics
- Has persistent memory location
- Can appear on left side of assignment
- Cannot bind to rvalue references (without const)

## 2. prvalue (pure rvalue)

A prvalue is an expression that doesn't have identity but can be moved from. It's typically a temporary value.

```cpp
int a = 42;            42 is a prvalue
int b = a + 5;         (a + 5) is a prvalue

stdstring getName() { return Bob; }
stdstring s = getName();   getName() returns prvalue

 Literals are prvalues (except string literals)
double d = 3.14;       3.14 is prvalue
bool flag = true;      true is prvalue
```

Key characteristics
- Temporary, no persistent address
- Result of most operators
- Can initialize objects

## 3. xvalue (expiring value)

An xvalue has identity but is about to expire (can be moved from). Created primarily through `stdmove` or functions returning rvalue references.

```cpp
stdstring str = Hello;
stdstring moved = stdmove(str);   stdmove(str) is xvalue
 str now in valid but unspecified state

stdvectorint getVector() { return {1, 2, 3}; }
stdvectorint v = stdmove(getVector());   stdmove(...) is xvalue

 Casting to rvalue reference creates xvalue
stdstring s = World;
stdstring s2 = static_caststdstring&&(s);   cast result is xvalue
```

Key characteristics
- Has identity but resources can be stolen
- Result of `stdmove`
- Can bind to rvalue references

## 4. glvalue (generalized lvalue)

A glvalue is either an lvalue or xvalue. It's an expression that has identity.

```cpp
int x = 5;                     x is glvalue (specifically lvalue)
int&& ref = stdmove(x);      stdmove(x) is glvalue (specifically xvalue)

 Both can be used to determine object identity
int p1 = &x;                  OK has identity
 int p2 = &42;              Error 42 is prvalue, no identity
```

Key characteristics
- Has memory locationidentity
- Encompasses both lvalues and xvalues
- Can have incomplete type

## 5. rvalue

An rvalue is either a prvalue or xvalue. It can be moved from.

```cpp
void process(stdstring&& s) {   accepts rvalues
    stdcout  s  'n';
}

process(temporary);              OK string literal converts to prvalue
process(stdstring(temp));      OK prvalue
process(stdmove(str));           OK xvalue

 rvalue references bind to rvalues
stdstring&& rref = stdstring(movable);   OK
 stdstring&& rref2 = str;      Error str is lvalue
```

## Practical Example Move Semantics

```cpp
class Buffer {
    int data;
    size_t size;
public
     Copy constructor (accepts lvalue)
    Buffer(const Buffer& other)  size(other.size) {
        data = new int[size];
        stdcopy(other.data, other.data + size, data);
        stdcout  Copiedn;
    }

     Move constructor (accepts rvalue)
    Buffer(Buffer&& other) noexcept  data(other.data), size(other.size) {
        other.data = nullptr;
        other.size = 0;
        stdcout  Movedn;
    }

    ~Buffer() { delete[] data; }
};

Buffer createBuffer() { return Buffer(); }

Buffer b1;
Buffer b2 = b1;                    Copy b1 is lvalue
Buffer b3 = stdmove(b1);         Move stdmove(b1) is xvalue
Buffer b4 = createBuffer();        Move return value is prvalue
```

## Quick Reference Table

 Category  Has Identity  Can Move From  Examples
--------------------------------------------------
 lvalue  Yes  No  variables, `ptr`, `str[0]`
 prvalue  No  Yes  `42`, `x+y`, `true`, temp objects
 xvalue  Yes  Yes  `stdmove(x)`, `stdforwardT(x)`
 glvalue  Yes  Maybe  lvalues + xvalues
 rvalue  Maybe  Yes  prvalues + xvalues

Understanding these categories is crucial for
- Writing efficient move semantics
- Perfect forwarding with templates
- Understanding compiler errors about reference binding
- Optimizing performance by avoiding unnecessary copies