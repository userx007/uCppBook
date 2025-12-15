# Copy Elision & Return Value Optimization in C++

Copy elision is a compiler optimization that eliminates unnecessary copying/moving of objects. RVO is the most common form of copy elision. Let me break down everything you need to know as a C++ expert.

## Core Concepts

**Copy Elision** allows the compiler to omit copy/move constructors even if they have side effects. This is unique in C++ - it's an optimization that changes observable behavior, yet it's explicitly allowed by the standard.

**Two Main Forms:**
1. **Named Return Value Optimization (NRVO)** - when returning a named local variable
2. **Return Value Optimization (RVO) / Unnamed RVO** - when returning a temporary

## Historical Evolution

**C++98/03:** Copy elision was permitted but optional
**C++11:** Move semantics provided fallback when elision didn't occur
**C++17:** **Mandatory copy elision** for temporaries (prvalues) - this is guaranteed by the standard
**C++20/23:** Further refinements to value categories

## Mandatory vs Optional Elision (C++17+)

### Guaranteed Elision (C++17)
```cpp
Widget makeWidget() {
    return Widget();  // Guaranteed elision - no copy/move
}

Widget w = makeWidget();  // Guaranteed elision
```

The temporary is constructed directly in the destination. The copy/move constructor **doesn't even need to exist**.

### Optional Elision (NRVO)
```cpp
Widget makeWidget() {
    Widget w;
    // ... do stuff ...
    return w;  // NRVO - optional but likely
}
```

NRVO is still **optional** and not guaranteed. The copy/move constructor must be accessible.

## Conditions for Copy Elision

### When RVO Applies (Guaranteed in C++17)
- Returning a prvalue (temporary) of the same type as function return type
- The expression in the return statement is not a function parameter

### When NRVO Applies (Optional)
- Returning a local automatic variable
- Same type as function return type (ignoring cv-qualifiers)
- Not a function parameter
- All return paths return the same variable

## Cases Where Elision Doesn't Occur

### Multiple Return Paths with Different Variables
```cpp
Widget makeWidget(bool flag) {
    Widget w1;
    Widget w2;
    return flag ? w1 : w2;  // Can't elide - different objects
}
```

### Returning Function Parameters
```cpp
Widget process(Widget w) {
    return w;  // No elision - parameter, not local
    // But move is applied automatically
}
```

### Returning Global/Static Variables
```cpp
static Widget global;
Widget getGlobal() {
    return global;  // No elision - not a local
}
```

### Explicit std::move Prevents NRVO
```cpp
Widget makeWidget() {
    Widget w;
    return std::move(w);  // DON'T DO THIS - prevents NRVO
                          // Falls back to move, but worse than NRVO
}
```

## Implicit Move from Return (C++11+)

When copy elision doesn't occur, C++ **automatically** treats local variables in return statements as rvalues:

```cpp
Widget makeWidget() {
    Widget w;
    return w;  // If NRVO fails: automatic move, not copy
}
```

The compiler tries:
1. Copy elision first
2. Move constructor if elision fails
3. Copy constructor as last resort

## Value Categories and Elision

Understanding value categories is crucial:

- **prvalue** (pure rvalue): Temporary, no identity → guaranteed elision in C++17
- **xvalue** (expiring value): Has identity but can be moved from
- **lvalue**: Has identity and can't be implicitly moved from

```cpp
Widget makeWidget() {
    return Widget();     // prvalue - guaranteed elision
}

Widget w1 = Widget();    // prvalue - guaranteed elision
Widget w2 = std::move(w1); // xvalue - no elision, uses move ctor
Widget w3 = w2;          // lvalue - no elision, uses copy ctor
```

## Practical Guidelines

### Do's
```cpp
// ✓ Return by value for optimal elision
Widget makeWidget() {
    return Widget(args);  // Guaranteed elision
}

// ✓ Return named locals directly
Widget makeWidget() {
    Widget w;
    w.init();
    return w;  // NRVO likely
}

// ✓ Use uniform return for best NRVO chance
Widget makeWidget(bool flag) {
    Widget w = flag ? Widget(1) : Widget(2);
    return w;  // Single variable - NRVO possible
}
```

### Don'ts
```cpp
// ✗ Don't use std::move on return
Widget makeWidget() {
    Widget w;
    return std::move(w);  // Prevents NRVO!
}

// ✗ Don't return different variables
Widget makeWidget(bool flag) {
    Widget w1, w2;
    return flag ? w1 : w2;  // NRVO impossible
}

// ✗ Don't unnecessarily return by const value
const Widget makeWidget() {  // Prevents move!
    return Widget();
}
```

## Checking for Copy Elision

### Compiler-Specific Flags
- **GCC/Clang:** `-fno-elide-constructors` to disable (for testing)
- **MSVC:** `/d1reportSingleClassLayout` for layout info
- **Guaranteed elision:** Cannot be disabled in C++17+

### Testing Pattern
```cpp
struct Tracker {
    Tracker() { std::cout << "ctor\n"; }
    Tracker(const Tracker&) { std::cout << "copy\n"; }
    Tracker(Tracker&&) noexcept { std::cout << "move\n"; }
};

Tracker make() { return Tracker(); }
int main() {
    Tracker t = make();  // C++17: only "ctor" printed
}
```

## Advanced Considerations

### Exception Safety
Copy elision improves exception safety by eliminating potential throw points from copy/move constructors.

### Performance Critical Code
```cpp
// Modern C++: return by value is often fastest
std::vector<int> getData() {
    std::vector<int> v(1000000);
    // ... populate ...
    return v;  // Elided or moved - very cheap
}
```

### ABI and Binary Compatibility
Mandatory copy elision in C++17 changed calling conventions for some functions, potentially breaking ABI compatibility with C++14 code.

### Optimization Levels
Even optional copy elision (NRVO) usually occurs at `-O0` on modern compilers. It's not just an optimization - it's a language feature.

## Common Misconceptions

1. **"I need std::move on return"** - No! This prevents NRVO
2. **"Returning by const value is const-correct"** - No! It prevents moves
3. **"Copy elision requires optimization flags"** - Mostly false; C++17 guarantees it
4. **"Move constructors aren't needed with elision"** - Still needed as fallback for NRVO

## Summary Checklist

- Return temporaries directly for guaranteed elision (C++17+)
- Return local variables directly for potential NRVO
- Never use `std::move` on local return values
- Don't return by const value
- Ensure copy/move constructors are accessible (except for guaranteed elision)
- Use consistent return paths when possible for better NRVO
- Trust the compiler - modern compilers are excellent at copy elision

The key insight: **write simple, natural code**. The compiler is smarter than manual optimization attempts, especially with copy elision.