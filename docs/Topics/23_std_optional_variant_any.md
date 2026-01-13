# Three essential C++17 vocabulary types

## **std::optional<T>**

**Purpose**: Represents a value that may or may not exist—a safer alternative to null pointers or sentinel values.

**Key Features**:
- `has_value()` / `operator bool()` - check if value exists
- `value()` - access value (throws if empty)
- `value_or(default)` - get value or default
- `emplace()` - construct in-place
- `reset()` - clear the optional

**Common Use Cases**:
- Functions that might fail (like `divide()`)
- Optional configuration parameters
- Database fields that can be NULL
- Search operations that might not find anything

## **std::variant<T1, T2, ...>**

**Purpose**: Type-safe union that holds exactly one of several specified types at any time.

**Key Features**:
- `std::get<T>(v)` - extract value (throws if wrong type)
- `std::get_if<T>(&v)` - returns pointer or nullptr
- `std::holds_alternative<T>(v)` - check active type
- `std::visit(visitor, v)` - apply visitor pattern
- `index()` - get zero-based index of active type

**Common Use Cases**:
- Result types (success/error)
- State machines
- Expression trees (AST)
- Polymorphism without virtual functions
- Performance-critical code avoiding heap allocation

## **std::any**

**Purpose**: Type-erased container that can hold any copyable type.

**Key Features**:
- `any_cast<T>(a)` - extract value (throws if wrong type)
- `any_cast<T>(&a)` - pointer version (returns nullptr)
- `type()` - get `std::type_info`
- `has_value()` - check if contains value
- `emplace<T>()` - construct in-place

**Common Use Cases**:
- Plugin architectures
- Scripting language bindings
- Generic property bags
- Dynamic configuration systems
- Bridge between static and dynamic code

## **Decision Matrix**

| Feature | optional | variant | any |
|---------|----------|---------|-----|
| Types known at compile-time | Yes (1 type) | Yes (fixed set) | No |
| Heap allocation | No | No* | Often yes** |
| Type safety | Strong | Strong | Runtime only |
| Performance | Excellent | Excellent | Good |
| Value semantics | May be empty | Always full | May be empty |

*variant uses stack allocation 
any may use small object optimization*


## **Advanced Patterns**

**Monadic Operations** (C++23):
```cpp
std::optional<int> x = 5;
auto result = x.and_then([](int n) { return std::optional{n * 2}; })
               .transform([](int n) { return n + 1; })
               .or_else([]{ return std::optional{0}; });
```

**Overloaded Visitor** for variant:
```cpp
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

std::visit(overloaded{
    [](int i) { /* handle int */ },
    [](const std::string& s) { /* handle string */ }
}, my_variant);
```

These types are fundamental to modern C++ and enable safer, more expressive code!
```cpp
#include <iostream>
#include <optional>
#include <variant>
#include <any>
#include <string>
#include <vector>
#include <map>

/*
============================================================================
                            STD::OPTIONAL
============================================================================
Purpose: Represents a value that may or may not be present
Replaces: Null pointers, sentinel values, and error-prone patterns
Size: sizeof(T) + 1 byte (typically with padding)
*/

// Example 1: Basic std::optional usage
std::optional<int> divide(int a, int b) {
    if (b == 0) {
        return std::nullopt; // No value
    }
    return a / b;
}

// Example 2: Optional in data structures
struct User {
    std::string name;
    std::optional<std::string> middle_name;
    std::optional<int> age;
};

// Example 3: Configuration with optional values
class DatabaseConfig {
    std::string host;
    int port;
    std::optional<std::string> password;
    
public:
    DatabaseConfig(std::string h, int p) : host(h), port(p) {}
    
    void setPassword(const std::string& pwd) {
        password = pwd;
    }
    
    std::string getConnectionString() const {
        std::string conn = host + ":" + std::to_string(port);
        if (password.has_value()) {
            conn += " (authenticated)";
        }
        return conn;
    }
};

void demonstrateOptional() {
    std::cout << "=== STD::OPTIONAL EXAMPLES ===\n\n";
    
    // Basic usage
    auto result1 = divide(10, 2);
    auto result2 = divide(10, 0);
    
    if (result1.has_value()) {
        std::cout << "10/2 = " << result1.value() << "\n";
    }
    
    if (!result2) { // Implicit bool conversion
        std::cout << "10/0 = undefined\n";
    }
    
    // Value_or: provide default if empty
    std::cout << "Result with default: " << result2.value_or(-1) << "\n\n";
    
    // User with optional fields
    User u1{"John", "William", 30};
    User u2{"Jane", std::nullopt, std::nullopt};
    
    std::cout << "User 1 middle name: " 
              << u1.middle_name.value_or("(none)") << "\n";
    std::cout << "User 2 middle name: " 
              << u2.middle_name.value_or("(none)") << "\n\n";
    
    // Monadic operations (C++23)
    std::optional<int> opt_val = 42;
    auto doubled = opt_val.transform([](int x) { return x * 2; });
    std::cout << "Transformed value: " << doubled.value() << "\n\n";
    
    // Emplace: construct in-place
    std::optional<std::vector<int>> opt_vec;
    opt_vec.emplace(10, 5); // Vector of 10 elements, value 5
    std::cout << "Vector size: " << opt_vec->size() << "\n\n";
}

/*
============================================================================
                            STD::VARIANT
============================================================================
Purpose: Type-safe union that can hold one of several types
Replaces: C unions, void*, polymorphism in some cases
Size: sizeof(largest_type) + discriminator
*/

// Example 1: Basic variant usage
using IntOrString = std::variant<int, std::string>;

// Example 2: Result type pattern
template<typename T, typename E>
class Result {
    std::variant<T, E> data;
    
public:
    Result(T value) : data(std::move(value)) {}
    Result(E error) : data(std::move(error)) {}
    
    bool isOk() const { return std::holds_alternative<T>(data); }
    bool isErr() const { return std::holds_alternative<E>(data); }
    
    const T& value() const { return std::get<T>(data); }
    const E& error() const { return std::get<E>(data); }
};

// Example 3: AST/Expression tree
struct Add;
struct Multiply;
struct Number { double value; };

using Expr = std::variant<Number, 
                         std::unique_ptr<Add>, 
                         std::unique_ptr<Multiply>>;

struct Add {
    Expr left, right;
};

struct Multiply {
    Expr left, right;
};

// Visitor pattern with std::visit
struct ExprEvaluator {
    double operator()(const Number& n) const {
        return n.value;
    }
    
    double operator()(const std::unique_ptr<Add>& a) const {
        return std::visit(*this, a->left) + std::visit(*this, a->right);
    }
    
    double operator()(const std::unique_ptr<Multiply>& m) const {
        return std::visit(*this, m->left) * std::visit(*this, m->right);
    }
};

void demonstrateVariant() {
    std::cout << "=== STD::VARIANT EXAMPLES ===\n\n";
    
    // Basic usage
    std::variant<int, double, std::string> v1 = 42;
    std::variant<int, double, std::string> v2 = 3.14;
    std::variant<int, double, std::string> v3 = "hello";
    
    // Check which type is active
    std::cout << "v1 holds int: " << std::holds_alternative<int>(v1) << "\n";
    std::cout << "v1 index: " << v1.index() << "\n\n";
    
    // std::get: throws if wrong type
    try {
        std::cout << "v1 as int: " << std::get<int>(v1) << "\n";
        std::cout << "v1 as string: " << std::get<std::string>(v1) << "\n";
    } catch (const std::bad_variant_access& e) {
        std::cout << "Exception: " << e.what() << "\n\n";
    }
    
    // std::get_if: returns nullptr if wrong type
    if (auto* str = std::get_if<std::string>(&v3)) {
        std::cout << "v3 contains string: " << *str << "\n\n";
    }
    
    // std::visit: apply visitor to variant
    auto printer = [](const auto& val) {
        std::cout << "Value: " << val << "\n";
    };
    
    std::visit(printer, v1);
    std::visit(printer, v2);
    std::visit(printer, v3);
    std::cout << "\n";
    
    // Overloaded visitor pattern
    struct {
        void operator()(int i) const { std::cout << "Int: " << i << "\n"; }
        void operator()(double d) const { std::cout << "Double: " << d << "\n"; }
        void operator()(const std::string& s) const { 
            std::cout << "String: " << s << "\n"; 
        }
    } overloaded_printer;
    
    std::visit(overloaded_printer, v1);
    
    // Result type pattern
    Result<int, std::string> success(42);
    Result<int, std::string> failure("Error: division by zero");
    
    std::cout << "\nResult is ok: " << success.isOk() << "\n";
    std::cout << "Failure message: " << failure.error() << "\n\n";
}

/*
============================================================================
                             STD::ANY
============================================================================
Purpose: Type-safe container for single values of any type
Replaces: void*, boost::any
Size: Small object optimization or heap allocation
Performance: Slower than variant (uses type erasure)
*/

// Example 1: Heterogeneous container
void demonstrateAny() {
    std::cout << "=== STD::ANY EXAMPLES ===\n\n";
    
    // Basic usage
    std::any a1 = 42;
    std::any a2 = 3.14;
    std::any a3 = std::string("hello");
    
    // Check if any contains a value
    std::cout << "a1 has value: " << a1.has_value() << "\n";
    
    // Get type information
    std::cout << "a1 type: " << a1.type().name() << "\n\n";
    
    // any_cast: extract value
    try {
        int val = std::any_cast<int>(a1);
        std::cout << "a1 value: " << val << "\n";
        
        // Wrong type throws
        double d = std::any_cast<double>(a1);
    } catch (const std::bad_any_cast& e) {
        std::cout << "Exception: " << e.what() << "\n\n";
    }
    
    // Pointer version doesn't throw
    if (auto* ptr = std::any_cast<std::string>(&a3)) {
        std::cout << "a3 contains: " << *ptr << "\n\n";
    }
    
    // Example: Property bag / Generic container
    std::map<std::string, std::any> properties;
    properties["name"] = std::string("Widget");
    properties["count"] = 42;
    properties["price"] = 19.99;
    properties["active"] = true;
    
    std::cout << "Properties:\n";
    for (const auto& [key, value] : properties) {
        std::cout << "  " << key << ": ";
        
        if (value.type() == typeid(std::string)) {
            std::cout << std::any_cast<std::string>(value);
        } else if (value.type() == typeid(int)) {
            std::cout << std::any_cast<int>(value);
        } else if (value.type() == typeid(double)) {
            std::cout << std::any_cast<double>(value);
        } else if (value.type() == typeid(bool)) {
            std::cout << std::any_cast<bool>(value);
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // Emplace
    std::any a4;
    a4.emplace<std::vector<int>>(5, 100); // Vector of 5 elements, value 100
    auto& vec = std::any_cast<std::vector<int>&>(a4);
    std::cout << "Vector size: " << vec.size() << "\n";
    std::cout << "First element: " << vec[0] << "\n\n";
    
    // Reset
    a1.reset();
    std::cout << "After reset, a1 has value: " << a1.has_value() << "\n\n";
}

/*
============================================================================
                         COMPARISON & GUIDELINES
============================================================================

WHEN TO USE STD::OPTIONAL:
- Function may or may not return a value
- Data member that's legitimately optional
- Replacing null pointers or sentinel values
- Configuration or settings with defaults

WHEN TO USE STD::VARIANT:
- Value must be one of a fixed set of types (known at compile time)
- Type-safe discriminated unions
- State machines
- Expression trees / AST
- Return multiple error types
- Performance-critical code (no heap allocation)

WHEN TO USE STD::ANY:
- Type is not known at compile time
- Plugin systems
- Scripting language bindings
- Generic property bags
- Bridge between type-safe and dynamic code
- Type information needs to be preserved across API boundaries

PERFORMANCE COMPARISON:
- std::optional: Zero overhead when empty, size overhead when full
- std::variant: No heap allocation, size of largest alternative
- std::any: Often uses heap allocation (except small object optimization)

SAFETY:
- std::optional: Exception if accessing empty optional with .value()
- std::variant: Exception if std::get<T> used with wrong type
- std::any: Exception if any_cast<T> used with wrong type
============================================================================
*/

int main() {
    demonstrateOptional();
    std::cout << "\n";
    demonstrateVariant();
    std::cout << "\n";
    demonstrateAny();
    
    std::cout << "\n=== COMPARISON ===\n\n";
    
    std::cout << "Size comparisons:\n";
    std::cout << "sizeof(int): " << sizeof(int) << "\n";
    std::cout << "sizeof(std::optional<int>): " << sizeof(std::optional<int>) << "\n";
    std::cout << "sizeof(std::variant<int, double, std::string>): " 
              << sizeof(std::variant<int, double, std::string>) << "\n";
    std::cout << "sizeof(std::any): " << sizeof(std::any) << "\n";
    
    return 0;
}
```
---

# C++ std::optional<T>

`std::optional<T>` is a wrapper type that may or may not contain a value of type `T`. It's useful for representing optional return values, optional function parameters, or any situation where a value might be absent without using pointers or special sentinel values.

## Header
```cpp
#include <optional>
```

## Basic Concepts

An `std::optional<T>` can be in one of two states:
- **Engaged** - contains a value of type `T`
- **Disengaged** - does not contain a value (represented by `std::nullopt`)

## Construction

```cpp
#include <optional>
#include <string>

// Default construction - disengaged
std::optional<int> opt1;

// Construct with nullopt - disengaged
std::optional<int> opt2 = std::nullopt;

// Construct with a value - engaged
std::optional<int> opt3 = 5;
std::optional<int> opt4(10);
std::optional<std::string> opt5 = "hello";

// In-place construction
std::optional<std::string> opt6(std::in_place, 5, 'a'); // "aaaaa"

// Copy and move construction
std::optional<int> opt7 = opt3;
std::optional<int> opt8 = std::move(opt4);
```

## Checking for a Value

```cpp
std::optional<int> opt = 42;
std::optional<int> empty;

// has_value() - returns bool
if (opt.has_value()) {
    // opt contains a value
}

// operator bool - implicit conversion
if (opt) {
    // opt contains a value
}

if (!empty) {
    // empty does not contain a value
}
```

## Accessing the Value

```cpp
std::optional<int> opt = 42;

// value() - throws std::bad_optional_access if disengaged
int x = opt.value();

// operator* - undefined behavior if disengaged
int y = *opt;

// operator-> - undefined behavior if disengaged
std::optional<std::string> str = "hello";
size_t len = str->length();

// value_or() - returns the value or a default if disengaged
std::optional<int> empty;
int z = empty.value_or(100); // z = 100
int w = opt.value_or(100);   // w = 42
```

## Modifying the Value

```cpp
std::optional<int> opt;

// Assignment
opt = 42;              // Now contains 42
opt = std::nullopt;    // Now disengaged

// emplace() - constructs value in-place, destroys old value if present
std::optional<std::string> str;
str.emplace("hello");           // Direct construction
str.emplace(5, 'x');           // "xxxxx"

// reset() - destroys contained value and makes optional disengaged
opt = 42;
opt.reset();  // Now disengaged (same as opt = std::nullopt)

// swap()
std::optional<int> a = 1;
std::optional<int> b = 2;
a.swap(b);  // a = 2, b = 1
```

## Comparison Operators

```cpp
std::optional<int> a = 5;
std::optional<int> b = 10;
std::optional<int> empty;

// Compare two optionals
bool eq = (a == b);        // false
bool ne = (a != b);        // true
bool lt = (a < b);         // true
bool le = (a <= b);        // true
bool gt = (a > b);         // false
bool ge = (a >= b);        // false

// Compare with nullopt
bool is_null = (empty == std::nullopt);  // true
bool not_null = (a != std::nullopt);     // true

// Compare with value directly
bool eq_val = (a == 5);    // true
bool ne_val = (a != 10);   // true

// Empty optional is less than any engaged optional
bool cmp = (empty < a);    // true
```

## Complete Example

```cpp
#include <iostream>
#include <optional>
#include <string>

std::optional<int> divide(int a, int b) {
    if (b == 0) {
        return std::nullopt;  // Return empty optional
    }
    return a / b;  // Return value
}

int main() {
    auto result1 = divide(10, 2);
    auto result2 = divide(10, 0);
    
    // Check and access
    if (result1.has_value()) {
        std::cout << "Result 1: " << result1.value() << '\n';
    }
    
    if (result2) {
        std::cout << "Result 2: " << *result2 << '\n';
    } else {
        std::cout << "Result 2: division by zero\n";
    }
    
    // Using value_or
    std::cout << "Result 1 or default: " << result1.value_or(-1) << '\n';
    std::cout << "Result 2 or default: " << result2.value_or(-1) << '\n';
    
    // Modifying
    std::optional<std::string> name;
    name = "Alice";
    std::cout << "Name length: " << name->length() << '\n';
    
    name.reset();
    std::cout << "Name is " << (name ? "set" : "not set") << '\n';
    
    return 0;
}
```

## Key Methods Summary

| Method | Description |
|--------|-------------|
| `has_value()` | Returns `true` if contains a value |
| `operator bool` | Implicit conversion to `bool` |
| `value()` | Returns the value, throws if empty |
| `operator*` | Returns reference to value (UB if empty) |
| `operator->` | Access member of contained value (UB if empty) |
| `value_or(default)` | Returns value or default if empty |
| `emplace(args...)` | Constructs value in-place |
| `reset()` | Destroys value, makes optional empty |
| `swap(other)` | Swaps with another optional |

## Best Practices

- Use `has_value()` or implicit `bool` conversion before accessing the value with `operator*` or `operator->`
- Use `value()` when you want exception safety
- Use `value_or()` for simple default value scenarios
- Prefer `std::optional` over pointers or sentinel values for representing optional data

---

# C++ std::variant<T...>

`std::variant<T...>` is a type-safe union that can hold a value of one of several specified types. At any given time, it contains exactly one value of one of its alternative types. It's useful for representing values that can be one of several different types without resorting to inheritance or `void*`.

## Header
```cpp
#include <variant>
```

## Basic Concepts

A `std::variant<T1, T2, T3>` can hold a value of type `T1`, `T2`, or `T3` at any given moment. Unlike a union, it knows which type it currently holds and provides type-safe access.

## Construction

```cpp
#include <variant>
#include <string>

// Default construction - holds first type with default value
std::variant<int, double, std::string> v1;  // holds int(0)

// Construct with a specific value
std::variant<int, double, std::string> v2 = 42;           // holds int
std::variant<int, double, std::string> v3 = 3.14;        // holds double
std::variant<int, double, std::string> v4 = "hello";     // holds string

// Explicit type construction using in_place_type
std::variant<int, double, std::string> v5(std::in_place_type<std::string>, "world");
std::variant<int, double, std::string> v6(std::in_place_type<std::string>, 5, 'x');  // "xxxxx"

// Explicit index construction using in_place_index
std::variant<int, double, std::string> v7(std::in_place_index<2>, "test");  // index 2 is string

// Copy and move construction
std::variant<int, double> v8 = v2;
std::variant<int, double> v9 = std::move(v3);
```

## Querying the Active Type

```cpp
std::variant<int, double, std::string> v = 42;

// index() - returns the zero-based index of the active type
size_t idx = v.index();  // 0 (int is first type)

v = 3.14;
idx = v.index();  // 1 (double is second type)

v = "hello";
idx = v.index();  // 2 (string is third type)

// holds_alternative<T>() - checks if variant holds a specific type
bool is_int = std::holds_alternative<int>(v);        // false
bool is_double = std::holds_alternative<double>(v);  // false
bool is_string = std::holds_alternative<std::string>(v);  // true
```

## Accessing the Value

```cpp
std::variant<int, double, std::string> v = 42;

// get<T>() - throws std::bad_variant_access if wrong type
try {
    int x = std::get<int>(v);     // OK, returns 42
    double y = std::get<double>(v);  // throws!
} catch (const std::bad_variant_access& e) {
    // Handle error
}

// get<Index>() - access by index
int a = std::get<0>(v);  // OK, index 0 is int
// double b = std::get<1>(v);  // throws!

// get_if<T>() - returns pointer or nullptr if wrong type
if (int* ptr = std::get_if<int>(&v)) {
    // v holds int, use *ptr
    std::cout << *ptr << '\n';
}

if (double* ptr = std::get_if<double>(&v)) {
    // This won't execute
} else {
    // v doesn't hold double
}

// get_if<Index>() - access by index
if (int* ptr = std::get_if<0>(&v)) {
    std::cout << *ptr << '\n';
}
```

## Modifying the Value

```cpp
std::variant<int, double, std::string> v = 42;

// Assignment - changes the active type if needed
v = 3.14;      // Now holds double
v = "hello";   // Now holds string
v = 100;       // Now holds int again

// emplace<T>() - constructs value in-place
v.emplace<std::string>("world");
v.emplace<std::string>(10, 'a');  // "aaaaaaaaaa"
v.emplace<int>(42);

// emplace<Index>() - constructs by index
v.emplace<2>("test");  // index 2 is string
v.emplace<0>(99);      // index 0 is int

// swap()
std::variant<int, double> a = 5;
std::variant<int, double> b = 3.14;
a.swap(b);  // a holds 3.14, b holds 5
```

## Visiting with std::visit

`std::visit` allows you to apply a visitor (callable) to the variant, with the visitor being called with the currently active value.

```cpp
#include <iostream>
#include <variant>
#include <string>

std::variant<int, double, std::string> v = 42;

// Using a lambda as visitor
std::visit([](auto&& arg) {
    std::cout << "Value: " << arg << '\n';
}, v);

// Using overloaded lambdas (C++17 helper pattern)
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

v = "hello";
std::visit(overloaded{
    [](int x) { std::cout << "int: " << x << '\n'; },
    [](double x) { std::cout << "double: " << x << '\n'; },
    [](const std::string& x) { std::cout << "string: " << x << '\n'; }
}, v);

// Visitor returning a value
v = 3.14;
double result = std::visit([](auto&& arg) -> double {
    using T = std::decay_t<decltype(arg)>;
    if constexpr (std::is_same_v<T, int>) {
        return static_cast<double>(arg);
    } else if constexpr (std::is_same_v<T, double>) {
        return arg;
    } else {
        return 0.0;
    }
}, v);
```

## Comparison Operators

```cpp
std::variant<int, double> a = 5;
std::variant<int, double> b = 10;
std::variant<int, double> c = 3.14;

// Variants compare by index first, then by value
bool eq = (a == b);   // false (same index, different values)
bool ne = (a != b);   // true
bool lt = (a < b);    // true (5 < 10)
bool le = (a <= b);   // true
bool gt = (a > b);    // false
bool ge = (a >= b);   // false

// Different active types
bool lt2 = (a < c);   // true (index 0 < index 1)
```

## Special State: valueless_by_exception

A variant can become valueless if an exception is thrown during type-changing operations.

```cpp
struct Thrower {
    Thrower() = default;
    Thrower(const Thrower&) { throw std::runtime_error("error"); }
};

std::variant<int, Thrower> v = 42;

try {
    v = Thrower{};  // May throw during assignment
} catch (...) {
    // v might be valueless now
}

// Check if valueless
if (v.valueless_by_exception()) {
    std::cout << "Variant is in invalid state\n";
    std::cout << "Index: " << v.index() << '\n';  // variant_npos
}
```

## Complete Example

```cpp
#include <iostream>
#include <variant>
#include <string>
#include <vector>

using Data = std::variant<int, double, std::string>;

void printData(const Data& d) {
    // Using index()
    std::cout << "Active index: " << d.index() << ", ";
    
    // Using visit
    std::visit([](auto&& arg) {
        std::cout << "Value: " << arg << '\n';
    }, d);
}

Data processData(int type) {
    if (type == 0) return 42;
    if (type == 1) return 3.14;
    return std::string("hello");
}

int main() {
    std::vector<Data> dataList;
    dataList.push_back(42);
    dataList.push_back(3.14);
    dataList.push_back("world");
    
    for (const auto& data : dataList) {
        // Type-safe access with holds_alternative
        if (std::holds_alternative<int>(data)) {
            std::cout << "Integer: " << std::get<int>(data) << '\n';
        } else if (std::holds_alternative<double>(data)) {
            std::cout << "Double: " << std::get<double>(data) << '\n';
        } else if (std::holds_alternative<std::string>(data)) {
            std::cout << "String: " << std::get<std::string>(data) << '\n';
        }
    }
    
    // Safe access with get_if
    Data d = 100;
    if (auto ptr = std::get_if<int>(&d)) {
        std::cout << "Safely accessed int: " << *ptr << '\n';
    }
    
    // Using emplace
    d.emplace<std::string>("modified");
    printData(d);
    
    // Process and assign
    d = processData(1);
    printData(d);
    
    return 0;
}
```

## Pattern Matching Example

```cpp
#include <variant>
#include <string>
#include <iostream>

// Helper for overloaded visitors
template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;

enum class Operation { Add, Subtract, Multiply };

using Value = std::variant<int, double, std::string, Operation>;

std::string describe(const Value& v) {
    return std::visit(overloaded{
        [](int x) { return std::string("Integer: ") + std::to_string(x); },
        [](double x) { return std::string("Double: ") + std::to_string(x); },
        [](const std::string& s) { return std::string("String: ") + s; },
        [](Operation op) {
            switch (op) {
                case Operation::Add: return std::string("Add operation");
                case Operation::Subtract: return std::string("Subtract operation");
                case Operation::Multiply: return std::string("Multiply operation");
            }
            return std::string("Unknown operation");
        }
    }, v);
}

int main() {
    std::cout << describe(Value{42}) << '\n';
    std::cout << describe(Value{3.14}) << '\n';
    std::cout << describe(Value{"hello"}) << '\n';
    std::cout << describe(Value{Operation::Add}) << '\n';
}
```

## Key Methods and Functions Summary

| Method/Function | Description |
|-----------------|-------------|
| `index()` | Returns the zero-based index of the active type |
| `holds_alternative<T>(v)` | Returns `true` if variant holds type `T` |
| `get<T>(v)` | Returns the value if holds `T`, throws otherwise |
| `get<Index>(v)` | Returns the value at index, throws otherwise |
| `get_if<T>(&v)` | Returns pointer to value or `nullptr` |
| `get_if<Index>(&v)` | Returns pointer to value at index or `nullptr` |
| `emplace<T>(args...)` | Constructs value of type `T` in-place |
| `emplace<Index>(args...)` | Constructs value at index in-place |
| `swap(other)` | Swaps with another variant |
| `visit(visitor, v)` | Applies visitor to the active value |
| `valueless_by_exception()` | Returns `true` if in invalid state |

## Constants

- `std::variant_npos` - Value returned by `index()` when variant is valueless
- `std::variant_size_v<V>` - Number of alternatives in variant
- `std::variant_alternative_t<I, V>` - Type of the alternative at index `I`

## Best Practices

- Use `holds_alternative<T>()` or `get_if<T>()` for safe type checking before access
- Use `std::visit` for exhaustive handling of all possible types
- Prefer `get_if` over `get` when you're not certain about the active type
- Use the overloaded visitor pattern for clean, type-specific handling
- The first alternative should be default-constructible (or use `std::monostate` as first type if none are)

---

# C++ std::any

`std::any` is a type-safe container for single values of any copy-constructible type. Unlike `std::variant`, it doesn't require you to specify the possible types in advance. It can hold any type at runtime and provides type-safe access through casting.

## Header
```cpp
#include <any>
```

## Basic Concepts

`std::any` can store a value of any copy-constructible type. It manages the lifetime of the contained object and allows type-safe retrieval. If the container is empty, it's said to be in an "empty" state.

## Construction

```cpp
#include <any>
#include <string>
#include <vector>

// Default construction - empty any
std::any a1;

// Construct with a value
std::any a2 = 42;                           // holds int
std::any a3 = 3.14;                         // holds double
std::any a4 = std::string("hello");         // holds string
std::any a5 = std::vector<int>{1, 2, 3};   // holds vector

// In-place construction
std::any a6(std::in_place_type<std::string>, "world");
std::any a7(std::in_place_type<std::string>, 5, 'x');  // "xxxxx"
std::any a8(std::in_place_type<std::vector<int>>, {1, 2, 3});

// Copy and move construction
std::any a9 = a2;
std::any a10 = std::move(a3);
```

## Checking for a Value

```cpp
std::any a = 42;
std::any empty;

// has_value() - returns true if contains a value
if (a.has_value()) {
    std::cout << "a contains a value\n";
}

if (!empty.has_value()) {
    std::cout << "empty is empty\n";
}
```

## Querying the Type

```cpp
#include <iostream>
#include <typeinfo>

std::any a = 42;

// type() - returns const std::type_info& of contained type
const std::type_info& ti = a.type();
std::cout << "Type: " << ti.name() << '\n';

// Compare type with typeid
if (a.type() == typeid(int)) {
    std::cout << "a contains an int\n";
}

if (a.type() == typeid(double)) {
    std::cout << "This won't print\n";
}

// Empty any returns typeid(void)
std::any empty;
if (empty.type() == typeid(void)) {
    std::cout << "empty has no value\n";
}
```

## Accessing the Value

```cpp
#include <any>
#include <string>

std::any a = 42;

// any_cast<T>() - throws std::bad_any_cast if wrong type
try {
    int x = std::any_cast<int>(a);           // OK, returns 42
    double y = std::any_cast<double>(a);     // throws!
} catch (const std::bad_any_cast& e) {
    std::cout << "Bad cast: " << e.what() << '\n';
}

// any_cast with pointer - returns nullptr if wrong type
if (int* ptr = std::any_cast<int>(&a)) {
    std::cout << "Value: " << *ptr << '\n';
} else {
    std::cout << "Not an int\n";
}

if (double* ptr = std::any_cast<double>(&a)) {
    std::cout << "This won't execute\n";
}

// any_cast with const
const std::any ca = std::string("hello");
if (const std::string* ptr = std::any_cast<std::string>(&ca)) {
    std::cout << "String: " << *ptr << '\n';
}

// Casting reference - throws if wrong type
a = std::string("world");
std::string& ref = std::any_cast<std::string&>(a);
ref = "modified";  // Modifies the contained value
```

## Modifying the Value

```cpp
std::any a;

// Assignment - changes the contained value
a = 42;              // Now holds int
a = 3.14;           // Now holds double
a = "hello";        // Now holds const char*
a = std::string("world");  // Now holds string

// emplace<T>() - constructs value in-place, returns reference
std::string& str = a.emplace<std::string>("test");
str += " modified";

a.emplace<std::string>(10, 'a');  // "aaaaaaaaaa"
a.emplace<std::vector<int>>(std::initializer_list<int>{1, 2, 3});

// reset() - destroys contained value, makes any empty
a.reset();
std::cout << std::boolalpha << a.has_value() << '\n';  // false

// swap()
std::any x = 100;
std::any y = std::string("hello");
x.swap(y);  // x holds string, y holds int
```

## Type Casting Variations

```cpp
std::any a = 42;

// Copy out value
int x = std::any_cast<int>(a);

// Get reference (can modify)
int& ref = std::any_cast<int&>(a);
ref = 100;

// Get const reference
const int& cref = std::any_cast<const int&>(a);

// Pointer access (safe - returns nullptr on failure)
int* ptr = std::any_cast<int>(&a);
if (ptr) {
    *ptr = 200;
}

// const pointer access
const std::any ca = 42;
const int* cptr = std::any_cast<int>(&ca);
```

## Complete Example

```cpp
#include <iostream>
#include <any>
#include <string>
#include <vector>

void printAny(const std::any& a) {
    if (!a.has_value()) {
        std::cout << "Empty\n";
        return;
    }
    
    // Check type and print accordingly
    if (a.type() == typeid(int)) {
        std::cout << "int: " << std::any_cast<int>(a) << '\n';
    } else if (a.type() == typeid(double)) {
        std::cout << "double: " << std::any_cast<double>(a) << '\n';
    } else if (a.type() == typeid(std::string)) {
        std::cout << "string: " << std::any_cast<std::string>(a) << '\n';
    } else {
        std::cout << "Unknown type: " << a.type().name() << '\n';
    }
}

int main() {
    std::vector<std::any> container;
    
    // Store different types
    container.push_back(42);
    container.push_back(3.14);
    container.push_back(std::string("hello"));
    container.push_back(std::vector<int>{1, 2, 3});
    
    // Print all
    for (const auto& item : container) {
        printAny(item);
    }
    
    // Safe access with pointer cast
    std::any a = 100;
    if (int* ptr = std::any_cast<int>(&a)) {
        std::cout << "Successfully accessed: " << *ptr << '\n';
        *ptr = 200;
        std::cout << "Modified to: " << *ptr << '\n';
    }
    
    // Using emplace
    a.emplace<std::string>("constructed in place");
    printAny(a);
    
    // Reset
    a.reset();
    std::cout << "After reset, has_value: " << std::boolalpha 
              << a.has_value() << '\n';
    
    return 0;
}
```

## Working with Custom Types

```cpp
#include <iostream>
#include <any>
#include <string>

struct Person {
    std::string name;
    int age;
    
    Person(std::string n, int a) : name(std::move(n)), age(a) {}
};

int main() {
    // Store custom type
    std::any a = Person("Alice", 30);
    
    // Check type
    if (a.type() == typeid(Person)) {
        std::cout << "Contains a Person\n";
    }
    
    // Access with pointer
    if (Person* p = std::any_cast<Person>(&a)) {
        std::cout << "Name: " << p->name << ", Age: " << p->age << '\n';
        p->age = 31;  // Modify
    }
    
    // In-place construction
    a.emplace<Person>("Bob", 25);
    
    // Access with reference
    try {
        Person& person = std::any_cast<Person&>(a);
        std::cout << "Name: " << person.name << '\n';
    } catch (const std::bad_any_cast& e) {
        std::cout << "Cast failed\n";
    }
    
    return 0;
}
```

## Error Handling Example

```cpp
#include <iostream>
#include <any>
#include <string>

int main() {
    std::any a = 42;
    
    // Method 1: Try-catch with any_cast
    try {
        std::string s = std::any_cast<std::string>(a);
        std::cout << s << '\n';
    } catch (const std::bad_any_cast& e) {
        std::cout << "Exception: " << e.what() << '\n';
    }
    
    // Method 2: Pointer cast (no exception)
    if (std::string* ptr = std::any_cast<std::string>(&a)) {
        std::cout << *ptr << '\n';
    } else {
        std::cout << "Not a string\n";
    }
    
    // Method 3: Check type first
    if (a.type() == typeid(int)) {
        int value = std::any_cast<int>(a);
        std::cout << "Value: " << value << '\n';
    }
    
    return 0;
}
```

## Practical Use Case: Configuration Storage

```cpp
#include <iostream>
#include <any>
#include <string>
#include <map>

class Config {
    std::map<std::string, std::any> settings;
    
public:
    template<typename T>
    void set(const std::string& key, T value) {
        settings[key] = std::move(value);
    }
    
    template<typename T>
    T get(const std::string& key, T defaultValue = T{}) const {
        auto it = settings.find(key);
        if (it == settings.end()) {
            return defaultValue;
        }
        
        if (const T* ptr = std::any_cast<T>(&it->second)) {
            return *ptr;
        }
        return defaultValue;
    }
    
    bool has(const std::string& key) const {
        return settings.find(key) != settings.end();
    }
};

int main() {
    Config config;
    
    // Store various types
    config.set("port", 8080);
    config.set("host", std::string("localhost"));
    config.set("timeout", 30.5);
    config.set("debug", true);
    
    // Retrieve with type safety
    int port = config.get<int>("port");
    std::string host = config.get<std::string>("host");
    double timeout = config.get<double>("timeout");
    bool debug = config.get<bool>("debug");
    
    std::cout << "Port: " << port << '\n';
    std::cout << "Host: " << host << '\n';
    std::cout << "Timeout: " << timeout << '\n';
    std::cout << "Debug: " << std::boolalpha << debug << '\n';
    
    // Get with default
    int maxConnections = config.get<int>("max_connections", 100);
    std::cout << "Max connections: " << maxConnections << '\n';
    
    return 0;
}
```

## Key Methods Summary

| Method | Description |
|--------|-------------|
| `has_value()` | Returns `true` if contains a value |
| `type()` | Returns `const std::type_info&` of contained type |
| `any_cast<T>(a)` | Returns value of type `T`, throws if wrong type |
| `any_cast<T>(&a)` | Returns pointer to value or `nullptr` if wrong type |
| `any_cast<T&>(a)` | Returns reference to value, throws if wrong type |
| `emplace<T>(args...)` | Constructs value of type `T` in-place |
| `reset()` | Destroys contained value, makes any empty |
| `swap(other)` | Swaps with another any |

## Key Differences from std::variant and std::optional

| Feature | std::any | std::variant | std::optional |
|---------|----------|--------------|---------------|
| Types known at compile time | No | Yes | Yes |
| Can be empty | Yes | No (always has a value) | Yes |
| Runtime type checking | Required | Not required | Not applicable |
| Type erasure | Yes | No | No |
| Performance overhead | Higher (heap allocation possible) | Lower (stack-based) | Lower (stack-based) |

## Requirements and Limitations

- The contained type must be **copy-constructible**
- `std::any` may use small object optimization for small types
- Larger types may require heap allocation
- Cannot store references (store pointers or `std::reference_wrapper` instead)
- Cannot store arrays directly (use `std::array` or `std::vector`)

## Best Practices

- Use `any_cast` with pointers for safe, exception-free access
- Check `has_value()` before accessing when unsure if empty
- Use `type()` to check the contained type before casting
- Prefer `std::variant` when the set of possible types is known at compile time
- Use `std::any` for truly dynamic scenarios like plugin systems or configuration storage
- Always catch `std::bad_any_cast` when using value or reference casts