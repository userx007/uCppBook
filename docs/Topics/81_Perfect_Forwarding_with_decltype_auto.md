# Perfect Forwarding with `decltype(auto)`

```cpp
template<typename Container>
decltype(auto) getElement(Container&& c, size_t index) {
    return std::forward<Container>(c)[index];
}
```

This code demonstrates **perfect forwarding** combined with **`decltype(auto)`** to preserve the exact type and value category of a returned element.

## Breaking Down Each Part

### 1. Universal Reference (Forwarding Reference)

```cpp
template<typename Container>
void func(Container&& c)  // NOT an rvalue reference!
//                   ^^   // This is a UNIVERSAL/FORWARDING reference
```

**Key Rule:** `T&&` in a template is a **universal reference** that can bind to:
- Lvalues (binds as lvalue reference)
- Rvalues (binds as rvalue reference)

```cpp
std::vector<int> vec = {1, 2, 3};

getElement(vec, 0);              // Container = std::vector<int>&
getElement(std::vector<int>{}, 0); // Container = std::vector<int>
```

### 2. `std::forward<Container>(c)`

**Purpose:** Preserves the value category (lvalue vs rvalue) when passing `c` along.

```cpp
// Without forward - always treats as lvalue
return c[index];  // Always lvalue, loses rvalue-ness

// With forward - preserves original category
return std::forward<Container>(c)[index];
```

**How it works:**

```cpp
std::vector<int> vec = {1, 2, 3};

// Case 1: Lvalue passed
getElement(vec, 0);
// Container deduced as: std::vector<int>&
// forward returns: std::vector<int>& (lvalue reference)

// Case 2: Rvalue passed  
getElement(std::vector<int>{1, 2, 3}, 0);
// Container deduced as: std::vector<int>
// forward returns: std::vector<int>&& (rvalue reference)
```

### 3. `decltype(auto)` Return Type

**Purpose:** Preserves the **exact type** including reference and const qualifiers.

```cpp
// auto - strips references and cv-qualifiers
auto func1() {
    int x = 42;
    int& ref = x;
    return ref;  // Returns int (not int&!)
}

// decltype(auto) - preserves everything
decltype(auto) func2() {
    int x = 42;
    int& ref = x;
    return ref;  // Returns int& (preserves reference!)
}
```

## Complete Comparison: Why Each Piece Matters

```cpp
#include <vector>
#include <iostream>

// Version 1: Plain auto (LOSES information)
template<typename Container>
auto getElement_v1(Container&& c, size_t index) {
    return std::forward<Container>(c)[index];
}

// Version 2: decltype(auto) (PRESERVES everything)
template<typename Container>
decltype(auto) getElement_v2(Container&& c, size_t index) {
    return std::forward<Container>(c)[index];
}

int main() {
    std::vector<int> vec = {1, 2, 3};
    
    // Version 1: Returns int (copy!)
    auto elem1 = getElement_v1(vec, 0);
    elem1 = 999;  // Modifies copy, NOT vec[0]
    std::cout << vec[0] << "\n";  // Still 1
    
    // Version 2: Returns int& (reference!)
    decltype(auto) elem2 = getElement_v2(vec, 0);
    elem2 = 999;  // Modifies vec[0] directly!
    std::cout << vec[0] << "\n";  // Now 999
}
```

## Real-World Scenarios

### Scenario 1: Modifying through reference

```cpp
template<typename Container>
decltype(auto) getElement(Container&& c, size_t index) {
    return std::forward<Container>(c)[index];
}

std::vector<int> vec = {10, 20, 30};

// Get mutable reference
decltype(auto) elem = getElement(vec, 1);
elem = 999;  // ✓ Modifies vec[1]

std::cout << vec[1];  // 999
```

### Scenario 2: Const correctness preserved

```cpp
const std::vector<int> cvec = {10, 20, 30};

// Returns const int& (preserves const!)
decltype(auto) elem = getElement(cvec, 1);
// elem = 999;  // ✗ Compiler error: elem is const
```

### Scenario 3: Working with rvalues

```cpp
// Rvalue vector
auto temp = getElement(std::vector<int>{10, 20, 30}, 1);
// Returns: int&& (rvalue reference)
// Can move from it if needed
```

## Step-by-Step Type Deduction

```cpp
template<typename Container>
decltype(auto) getElement(Container&& c, size_t index) {
    return std::forward<Container>(c)[index];
}

// Example 1: Lvalue
std::vector<int> vec = {1, 2, 3};
getElement(vec, 0);

// Step 1: Template deduction
// Container = std::vector<int>&  (reference collapsing)

// Step 2: Parameter type
// Container&& = std::vector<int>& && = std::vector<int>&

// Step 3: std::forward
// std::forward<std::vector<int>&>(c) returns std::vector<int>&

// Step 4: operator[]
// std::vector<int>& :: operator[](size_t) returns int&

// Step 5: decltype(auto)
// decltype(c[index]) = int&
// Return type: int&
```

```cpp
// Example 2: Rvalue
getElement(std::vector<int>{1, 2, 3}, 0);

// Step 1: Template deduction
// Container = std::vector<int>  (no reference)

// Step 2: Parameter type
// Container&& = std::vector<int>&&

// Step 3: std::forward
// std::forward<std::vector<int>>(c) returns std::vector<int>&&

// Step 4: operator[]
// std::vector<int>&& :: operator[](size_t) returns int&&

// Step 5: decltype(auto)
// decltype(std::move(vec)[index]) = int&&
// Return type: int&&
```

## Common Mistakes and Fixes

### Mistake 1: Using `auto` instead of `decltype(auto)`

```cpp
// WRONG: Strips reference
template<typename Container>
auto getElement(Container&& c, size_t index) {
    return std::forward<Container>(c)[index];
    // Returns int (copy), not int&
}

std::vector<int> vec = {1, 2, 3};
getElement(vec, 0) = 999;  // Modifies temporary, not vec!
```

### Mistake 2: Forgetting `std::forward`

```cpp
// WRONG: Always treats c as lvalue
template<typename Container>
decltype(auto) getElement(Container&& c, size_t index) {
    return c[index];  // Loses rvalue-ness!
}
```

### Mistake 3: Using plain reference

```cpp
// WRONG: Can't bind rvalues
template<typename Container>
decltype(auto) getElement(Container& c, size_t index) {
    return c[index];
}

// This won't compile:
getElement(std::vector<int>{1, 2, 3}, 0);  // ✗ Error!
```

## Practical Complete Example

```cpp
#include <vector>
#include <string>
#include <iostream>

template<typename Container>
decltype(auto) getElement(Container&& c, size_t index) {
    return std::forward<Container>(c)[index];
}

int main() {
    // Example 1: Mutable vector
    std::vector<int> vec = {10, 20, 30};
    getElement(vec, 1) = 999;  // Modifies vec[1]
    std::cout << vec[1] << "\n";  // 999
    
    // Example 2: Const vector
    const std::vector<std::string> cvec = {"a", "b", "c"};
    std::cout << getElement(cvec, 1) << "\n";  // "b"
    // getElement(cvec, 1) = "x";  // ✗ Error: const
    
    // Example 3: Temporary vector (rvalue)
    std::cout << getElement(std::vector<int>{100, 200, 300}, 2) << "\n";  // 300
    
    // Example 4: Array
    int arr[] = {5, 6, 7};
    getElement(arr, 0) = 555;
    std::cout << arr[0] << "\n";  // 555
    
    return 0;
}
```

## When to Use This Pattern

✅ **Use when:**
- Building generic library code
- Need to preserve exact types (reference, const, rvalue)
- Wrapping/forwarding container access
- Performance-critical code (avoids copies)

❌ **Don't use when:**
- Simple application code (overkill)
- You explicitly want copies
- Return type is obvious and fixed

## Key Takeaways

1. **`Container&&`** in templates = universal reference (accepts lvalues and rvalues)
2. **`std::forward<Container>(c)`** = preserves value category when forwarding
3. **`decltype(auto)`** = preserves exact return type including references and const
4. **Together:** Perfectly forwards containers and returns the exact type of `c[index]`

This pattern is the gold standard for generic container element access in modern C++!