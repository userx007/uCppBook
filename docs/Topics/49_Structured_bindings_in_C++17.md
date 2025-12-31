# Structured Bindings (C++17) — Decomposition Declarations

Structured bindings are a C++17 feature that allows you to decompose objects into their constituent elements and bind them to named variables in a single declaration. This provides a convenient syntax for unpacking data structures.

## Basic Syntax

```cpp
auto [identifier1, identifier2, ...] = expression;
auto& [identifier1, identifier2, ...] = expression;  // reference binding
const auto& [identifier1, identifier2, ...] = expression;  // const reference
```

## What Can Be Decomposed?

### 1. **Arrays**
You can decompose C-style arrays and `std::array`:

```cpp
int arr[3] = {1, 2, 3};
auto [a, b, c] = arr;  // a=1, b=2, c=3

std::array<int, 2> point = {10, 20};
auto [x, y] = point;
```

### 2. **Tuple-like Objects**
Works with `std::tuple`, `std::pair`, and any type supporting `std::tuple_size` and `std::get`:

```cpp
std::tuple<int, double, std::string> data = {42, 3.14, "hello"};
auto [num, pi, str] = data;

std::pair<std::string, int> person = {"Alice", 30};
auto [name, age] = person;

// Common with std::map
std::map<std::string, int> scores;
for (const auto& [key, value] : scores) {
    // key and value are directly accessible
}
```

### 3. **Structs and Classes**
For structs/classes with public non-static data members:

```cpp
struct Point {
    double x;
    double y;
};

Point p{3.0, 4.0};
auto [x, y] = p;  // x=3.0, y=4.0
```

## Key Features

### Reference Bindings
You can create references to avoid copies:

```cpp
std::tuple<int, std::string> data = {42, "hello"};
auto& [num, str] = data;  // References to tuple elements
num = 100;  // Modifies the original tuple
```

### Const Bindings
```cpp
const auto& [x, y] = getPoint();  // const references
```

### Structured Bindings with Range-Based For Loops
This is particularly useful with containers:

```cpp
std::map<std::string, int> map = {{"a", 1}, {"b", 2}};
for (const auto& [key, value] : map) {
    std::cout << key << ": " << value << '\n';
}
```

## Benefits

1. **Improved Readability**: Code intent is clearer compared to accessing `.first`, `.second`, or using `std::get<N>`
2. **Reduced Boilerplate**: No need for temporary variables or explicit unpacking
3. **Type Safety**: Compiler ensures correct number of identifiers
4. **Better Semantics**: Named variables convey meaning better than numeric indices

## Important Rules and Limitations

1. **Number of identifiers must match exactly**: You must declare the exact number of elements being decomposed
   ```cpp
   std::pair<int, int> p{1, 2};
   auto [a, b] = p;     // OK
   auto [a] = p;        // Error: wrong count
   auto [a, b, c] = p;  // Error: wrong count
   ```

2. **All identifiers declared together**: You cannot selectively bind only some elements
   ```cpp
   auto [_, y] = point;  // Must declare all; use _ for ignored values
   ```

3. **Not actual variables**: The identifiers are bindings to the decomposed object's elements, not independent variables

4. **Cannot specify types individually**: All bindings share the same cv-qualifiers and reference type
   ```cpp
   auto [a, b] = tuple;        // OK
   int [a, b] = tuple;         // Error: can't specify type
   auto [int a, double b] = t; // Error: can't specify individual types
   ```

5. **Works with public members only**: For structs/classes, all non-static data members must be public

## Practical Examples

### Returning Multiple Values
```cpp
std::tuple<bool, std::string, int> parseData(const std::string& input) {
    // parsing logic
    return {true, "result", 42};
}

auto [success, message, value] = parseData(input);
if (success) {
    std::cout << message << ": " << value << '\n';
}
```

### Iterating Over Maps
```cpp
std::unordered_map<std::string, std::vector<int>> data;
for (auto& [key, vec] : data) {
    vec.push_back(0);  // Modify in place
}
```

### Custom Types
```cpp
struct RGB {
    uint8_t r, g, b;
};

RGB color{255, 128, 0};
auto [red, green, blue] = color;
```

Structured bindings significantly improve code clarity and conciseness, making C++ code more expressive and easier to maintain, especially when working with tuple-like types and containers that return pairs.