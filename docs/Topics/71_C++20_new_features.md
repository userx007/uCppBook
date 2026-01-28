# Comprehensive code artifact demonstrating C++20's major features

## Major Features

[**1. Concepts**](#concepts)
- Type constraints for templates that make requirements explicit
- Replaces complex SFINAE with readable syntax
- Example: `template<Numeric T>` ensures T is a numeric type

[**2. Ranges**](#ranges)
- Composable, lazy-evaluated algorithms
- Pipeline syntax using `|` operator
- Views are lightweight and don't copy data
- Much more expressive than traditional iterators

[**3. Coroutines**](#coroutines)
- Functions that can suspend and resume execution
- Enable generators, async operations, and lazy sequences
- Uses `co_yield`, `co_await`, and `co_return` keywords

[**4. Three-Way Comparison (Spaceship Operator `<=>`)**](#three-way-comparison)
- Single operator generates all six comparison operators
- Returns ordering category (strong, weak, partial)
- `= default` makes it trivial to implement

[**5. Modules**](#modules)
- Modern alternative to header files
- Faster compilation and better encapsulation
- No more header guards needed

[**6. Designated Initializers**](#designated-initializers)
- Initialize struct members by name
- More readable and maintainable
- Borrowed from C99

[**7. `std::format`**](#format)
- Type-safe Python-style string formatting
- Replaces printf and iostreams for many uses
- Supports positional and named arguments

[**8. `std::span`**](#span)
- Non-owning view over contiguous memory
- Works with arrays, vectors, and C-style arrays
- Safer than raw pointers

[**9. Constexpr Enhancements**](#constexpr-enhancements)
- Virtual functions can be constexpr
- Dynamic memory allocation at compile time
- `std::vector` and `std::string` in constexpr contexts

[**10. Lambda Improvements**](#lambda-improvements)
- Template parameters in lambdas
- `[=, this]` capture allowed
- Lambdas in unevaluated contexts

[**11. `std::source_location`**](#source-location)
- Better alternative to `__FILE__`, `__LINE__`, `__func__`
- Automatic propagation through function calls
- Useful for logging and debugging

[**12. Calendar and Time Zones**](#calendar-and-time-zones)
- Comprehensive date/time library
- Time zone support built-in
- Date literals like `2024y/January/15`

[**13. Additional Features:**](#additional-features)
- `[[likely]]` and `[[unlikely]]` attributes for optimization hints
- `constinit` for guaranteed compile-time initialization
- `std::bit_cast` for safe type punning
- Mathematical constants in `std::numbers`
- Init-statements in range-based for loops
- Abbreviated function templates with `auto` parameters

---

## Concepts
[Back to top](#major-features)

### Overview

C++20 Concepts are a major language feature that allows you to specify **constraints on template parameters** in a clear, readable way. They provide compile-time validation of template arguments, replacing cryptic SFINAE (Substitution Failure Is Not An Error) techniques with explicit requirements. Concepts make templates more robust, improve error messages, and serve as documentation for what types a template can accept.

### Why Concepts Matter

Before C++20, template errors were notoriously difficult to understand. If you passed an invalid type to a template, you'd get pages of error messages from deep within the template instantiation. Concepts solve this by checking requirements upfront and providing clear error messages.

### Basic Syntax

#### Defining a Concept

A concept is a named set of requirements:

```cpp
#include <concepts>

// Simple concept: T must be an integral type
template<typename T>
concept Integral = std::is_integral_v<T>;

// Concept with multiple requirements
template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

// Concept using requires expression
template<typename T>
concept Addable = requires(T a, T b) {
    { a + b } -> std::same_as<T>;  // a + b must return type T
};
```

#### Using Concepts

There are several ways to apply concepts to templates:

```cpp
// Method 1: Requires clause
template<typename T>
requires Integral<T>
T add(T a, T b) {
    return a + b;
}

// Method 2: Constrained template parameter (most common)
template<Integral T>
T multiply(T a, T b) {
    return a * b;
}

// Method 3: Abbreviated function template (auto with concept)
auto divide(Integral auto a, Integral auto b) {
    return a / b;
}

// Method 4: Trailing requires clause
template<typename T>
T subtract(T a, T b) requires Integral<T> {
    return a - b;
}
```

### Standard Library Concepts

C++20 provides many built-in concepts in the `<concepts>` header:

```cpp
#include <concepts>
#include <iostream>

// Using standard concepts
template<std::integral T>
void printInt(T value) {
    std::cout << "Integer: " << value << '\n';
}

template<std::floating_point T>
void printFloat(T value) {
    std::cout << "Float: " << value << '\n';
}

// std::same_as - types must be identical
template<typename T, typename U>
requires std::same_as<T, U>
void ensureSameType(T a, U b) {
    std::cout << "Same types!\n";
}

// std::convertible_to - T must be convertible to U
template<typename T, typename U>
requires std::convertible_to<T, U>
U convert(T value) {
    return static_cast<U>(value);
}

int main() {
    printInt(42);           // OK
    printFloat(3.14);       // OK
    // printInt(3.14);      // Error: doesn't satisfy std::integral
    
    ensureSameType(5, 10);  // OK
    // ensureSameType(5, 3.14); // Error: different types
    
    double d = convert<int, double>(42);  // OK
}
```

### Requires Expressions

The `requires` expression allows you to specify detailed requirements:

```cpp
#include <concepts>

// Check if type has specific operations
template<typename T>
concept HasSize = requires(T container) {
    { container.size() } -> std::convertible_to<std::size_t>;
};

// Multiple requirements
template<typename T>
concept Container = requires(T c) {
    typename T::value_type;           // Has nested type
    { c.begin() } -> std::same_as<typename T::iterator>;
    { c.end() } -> std::same_as<typename T::iterator>;
    { c.size() } -> std::convertible_to<std::size_t>;
};

// Compound requirements
template<typename T>
concept Comparable = requires(T a, T b) {
    { a == b } -> std::convertible_to<bool>;
    { a != b } -> std::convertible_to<bool>;
    { a < b } -> std::convertible_to<bool>;
    { a > b } -> std::convertible_to<bool>;
};
```

### Practical Examples

#### Example 1: Safe Numeric Operations

```cpp
#include <concepts>
#include <iostream>
#include <vector>

template<std::floating_point T>
T average(const std::vector<T>& values) {
    if (values.empty()) return T{};
    T sum = T{};
    for (const auto& v : values) {
        sum += v;
    }
    return sum / values.size();
}

int main() {
    std::vector<double> nums = {1.5, 2.5, 3.5, 4.5};
    std::cout << "Average: " << average(nums) << '\n';
    
    // std::vector<std::string> strs = {"hello"};
    // average(strs);  // Error: string doesn't satisfy std::floating_point
}
```

#### Example 2: Generic Printable Type

```cpp
#include <concepts>
#include <iostream>
#include <string>

// Concept for types that can be printed
template<typename T>
concept Printable = requires(std::ostream& os, T value) {
    { os << value } -> std::same_as<std::ostream&>;
};

template<Printable T>
void print(const T& value) {
    std::cout << value << '\n';
}

struct Point {
    int x, y;
    
    friend std::ostream& operator<<(std::ostream& os, const Point& p) {
        return os << "(" << p.x << ", " << p.y << ")";
    }
};

int main() {
    print(42);              // OK
    print(3.14);            // OK
    print("Hello");         // OK
    print(Point{10, 20});   // OK - has operator
}
```

#### Example 3: Concept Composition

```cpp
#include <concepts>
#include <iostream>

// Compose multiple concepts
template<typename T>
concept SignedNumeric = std::signed_integral<T> || std::floating_point<T>;

template<typename T>
concept UnsignedInteger = std::unsigned_integral<T> && !std::same_as<T, bool>;

// Using concept composition
template<SignedNumeric T>
T abs(T value) {
    return value < 0 ? -value : value;
}

template<UnsignedInteger T>
T factorial(T n) {
    T result = 1;
    for (T i = 2; i <= n; ++i) {
        result *= i;
    }
    return result;
}

int main() {
    std::cout << abs(-5) << '\n';        // 5
    std::cout << abs(-3.14) << '\n';     // 3.14
    std::cout << factorial(5u) << '\n';  // 120
}
```

#### Example 4: Iterator Concepts

```cpp
#include <concepts>
#include <iterator>
#include <vector>
#include <list>
#include <iostream>

// Function that works with any forward iterator
template<std::forward_iterator Iter>
void printRange(Iter begin, Iter end) {
    for (auto it = begin; it != end; ++it) {
        std::cout << *it << ' ';
    }
    std::cout << '\n';
}

// Function requiring random access
template<std::random_access_iterator Iter>
auto getMiddle(Iter begin, Iter end) {
    auto distance = std::distance(begin, end);
    return *(begin + distance / 2);
}

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::list<int> lst = {10, 20, 30, 40, 50};
    
    printRange(vec.begin(), vec.end());  // OK: vector has forward iterators
    printRange(lst.begin(), lst.end());  // OK: list has forward iterators
    
    std::cout << "Middle of vector: " << getMiddle(vec.begin(), vec.end()) << '\n';
    // getMiddle(lst.begin(), lst.end()); // Error: list doesn't have random access
}
```

#### Example 5: Custom Concept with Complex Requirements

```cpp
#include <concepts>
#include <iostream>
#include <string>

// Concept for a serializable type
template<typename T>
concept Serializable = requires(T obj, std::string& str) {
    { obj.serialize() } -> std::same_as<std::string>;
    { T::deserialize(str) } -> std::same_as<T>;
};

class User {
    std::string name;
    int age;
    
public:
    User(std::string n, int a) : name(std::move(n)), age(a) {}
    
    std::string serialize() const {
        return name + ":" + std::to_string(age);
    }
    
    static User deserialize(const std::string& data) {
        size_t pos = data.find(':');
        std::string name = data.substr(0, pos);
        int age = std::stoi(data.substr(pos + 1));
        return User(name, age);
    }
    
    void display() const {
        std::cout << name << " (" << age << ")\n";
    }
};

template<Serializable T>
void saveAndLoad(const T& obj) {
    std::string data = obj.serialize();
    std::cout << "Serialized: " << data << '\n';
    
    T loaded = T::deserialize(data);
    std::cout << "Deserialized: ";
    loaded.display();
}

int main() {
    User user("Alice", 30);
    saveAndLoad(user);
}
```

### Advanced Features

#### Subsumption

Concepts can be more or less specific. The compiler uses **subsumption** to choose the most specific overload:

```cpp
#include <concepts>
#include <iostream>

template<typename T>
concept Numeric = std::is_arithmetic_v<T>;

template<typename T>
concept FloatingPoint = Numeric<T> && std::floating_point<T>;

// Less specific
template<Numeric T>
void process(T value) {
    std::cout << "Numeric: " << value << '\n';
}

// More specific (subsumes Numeric)
template<FloatingPoint T>
void process(T value) {
    std::cout << "Float: " << value << '\n';
}

int main() {
    process(42);    // Calls Numeric version
    process(3.14);  // Calls FloatingPoint version (more specific)
}
```

### Benefits of Concepts

1. **Better Error Messages**: Clear, readable compile-time errors
2. **Self-Documenting**: Concepts serve as documentation for template requirements
3. **Overload Resolution**: Enable cleaner function overloading based on type properties
4. **Type Safety**: Catch errors at template definition, not instantiation
5. **Readability**: Much clearer than SFINAE or `enable_if`

### Common Standard Library Concepts

From `<concepts>`:
- `std::same_as` - types are identical
- `std::derived_from` - inheritance relationship
- `std::convertible_to` - implicit conversion possible
- `std::integral` - integral types
- `std::floating_point` - floating-point types
- `std::signed_integral` - signed integer types
- `std::unsigned_integral` - unsigned integer types
- `std::assignable_from` - assignment possible
- `std::swappable` - can be swapped
- `std::copy_constructible` - can be copy constructed
- `std::move_constructible` - can be move constructed

From `<iterator>`:
- `std::input_iterator`
- `std::forward_iterator`
- `std::bidirectional_iterator`
- `std::random_access_iterator`
- `std::contiguous_iterator`


### Key Takeaways

- **Concepts constrain template parameters** with named requirements instead of SFINAE
- **Define concepts** using `concept` keyword with type traits, requires expressions, or other concepts
- **Apply concepts** via requires clauses, constrained template parameters, or abbreviated templates
- **Requires expressions** check for specific operations, nested types, and return types
- **Standard library provides** many ready-to-use concepts in `<concepts>` and `<iterator>`
- **Subsumption rules** allow more specific concepts to override general ones in overload resolution
- **Benefits include** better error messages, self-documenting code, and cleaner generic programming
- **Concepts compose** using logical operators (`&&`, `||`) for complex requirements
- **Replaces SFINAE** with readable, maintainable template constraints
- **Compile-time checking** catches template misuse before instantiation

---

## Ranges
[Back to top](#major-features)

C++20 Ranges is a revolutionary library that fundamentally changes how we work with sequences of data in C++. It provides a more composable, expressive, and efficient way to process collections compared to traditional STL algorithms.

### What Are Ranges?

A **range** is any object that you can iterate over. It's a generalization of the concept of "a pair of iterators." More formally, a range is anything that has a `begin()` and `end()` (or can be used with `std::ranges::begin()` and `std::ranges::end()`).

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // vec is a range - it has begin() and end()
    for (int x : vec) {
        std::cout << x << " ";
    }
}
```

### Key Components of Ranges

#### 1. Range Concepts

C++20 introduces several concepts to classify ranges:

```cpp
#include <ranges>
#include <vector>
#include <list>

// range: anything with begin/end
static_assert(std::ranges::range<std::vector<int>>);

// sized_range: knows its size in O(1)
static_assert(std::ranges::sized_range<std::vector<int>>);

// random_access_range: supports O(1) element access
static_assert(std::ranges::random_access_range<std::vector<int>>);

// Not random access, but still a range
static_assert(!std::ranges::random_access_range<std::list<int>>);
```

#### 2. Range Algorithms

Range algorithms are improved versions of STL algorithms that work directly on ranges rather than iterator pairs:

```cpp
#include <ranges>
#include <algorithm>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {5, 2, 8, 1, 9, 3};
    
    // Old style (C++17 and before)
    std::sort(numbers.begin(), numbers.end());
    
    // New style (C++20)
    std::ranges::sort(numbers);
    
    // Can also use projections
    std::vector<std::string> words = {"apple", "pie", "banana", "cat"};
    
    // Sort by length instead of alphabetically
    std::ranges::sort(words, {}, &std::string::size);
    
    for (const auto& word : words) {
        std::cout << word << " ";  // Output: pie cat apple banana
    }
}
```

#### 3. Views (Range Adapters)

Views are the most powerful feature of ranges. They are **lazy-evaluated** range adaptors that transform ranges without copying data:

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Create a view that filters evens, transforms to square, and takes first 3
    auto result = numbers 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; })
        | std::views::take(3);
    
    // Nothing is computed yet! Views are lazy.
    
    // Now we iterate and compute on-demand
    for (int n : result) {
        std::cout << n << " ";  // Output: 4 16 36
    }
}
```

#### 4. Common View Adaptors

Here's a comprehensive example showing various view adaptors:

```cpp
#include <ranges>
#include <vector>
#include <iostream>
#include <string>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // filter: keep only elements matching a predicate
    auto evens = vec | std::views::filter([](int n) { return n % 2 == 0; });
    
    // transform: apply a function to each element
    auto squares = vec | std::views::transform([](int n) { return n * n; });
    
    // take: take first N elements
    auto first_three = vec | std::views::take(3);
    
    // drop: skip first N elements
    auto skip_three = vec | std::views::drop(3);
    
    // reverse: reverse the order
    auto reversed = vec | std::views::reverse;
    
    // take_while: take elements while predicate is true
    auto while_small = vec | std::views::take_while([](int n) { return n < 5; });
    
    // drop_while: skip elements while predicate is true
    auto after_small = vec | std::views::drop_while([](int n) { return n < 5; });
    
    // split: split a range by a delimiter
    std::string text = "hello,world,foo,bar";
    auto words = text | std::views::split(',');
    
    // join: flatten a range of ranges
    std::vector<std::vector<int>> nested = {{1, 2}, {3, 4}, {5}};
    auto flattened = nested | std::views::join;
    
    // keys/values: for associative containers
    std::map<int, std::string> map = {{1, "one"}, {2, "two"}};
    auto keys = map | std::views::keys;
    auto values = map | std::views::values;
}
```

#### 5. Composition and Piping

The pipe operator `|` allows elegant composition:

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Complex pipeline - reads left to right
    auto pipeline = numbers
        | std::views::filter([](int n) { return n % 2 == 1; })  // odds: 1,3,5,7,9
        | std::views::transform([](int n) { return n * 2; })     // double: 2,6,10,14,18
        | std::views::drop(2)                                     // skip 2: 10,14,18
        | std::views::take(2);                                    // take 2: 10,14
    
    for (int n : pipeline) {
        std::cout << n << " ";  // Output: 10 14
    }
    std::cout << "\n";
}
```

#### 6. Generating Ranges

C++20 provides views for generating sequences:

```cpp
#include <ranges>
#include <iostream>

int main() {
    // iota: generate sequence from start value
    for (int i : std::views::iota(1, 6)) {
        std::cout << i << " ";  // Output: 1 2 3 4 5
    }
    std::cout << "\n";
    
    // Infinite range (combined with take)
    for (int i : std::views::iota(0) | std::views::take(5)) {
        std::cout << i << " ";  // Output: 0 1 2 3 4
    }
    std::cout << "\n";
    
    // repeat: C++23 feature (if available)
    // auto repeated = std::views::repeat(42) | std::views::take(3);
}
```

#### 7. Projections

Projections allow you to specify how to access the value to compare without transforming the data:

```cpp
//Most std::ranges algorithms accept this pattern:
ranges::algorithm(range, comp = {}, proj = {});
std::ranges::sort(people, {}, &Person::age); // default std::ranges::less
auto youngest = std::ranges::min(people, {}, &Person::age);
auto it = std::ranges::find(people, 42, &Person::age);
auto seniors = std::ranges::count_if(
    people,
    [](int age) { return age >= 65; },
    &Person::age
);
```
* `comp` → how to compare (e.g. `<`, `>`)
* `proj` → how to extract the comparison key

```cpp
std::ranges::sort(v, {}, [](const Person& p) { return p.age; });
std::ranges::sort(v, {}, &Person::age);
std::ranges::sort(v, {}, &Person::get_age);
// All of these are valid as long as:
// proj(element) -> comparable value
```

**Full example:**

```cpp
#include <ranges>
#include <algorithm>
#include <vector>
#include <iostream>

struct Person {
    std::string name;
    int age;
};

int main() {
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35}
    };
    
    // Sort by age using projection
    std::ranges::sort(people, {}, &Person::age);
    
    // Find person with age 25
    auto it = std::ranges::find(people, 25, &Person::age);
    if (it != people.end()) {
        std::cout << "Found: " << it->name << "\n";
    }
    
    // Filter people over 28 and get their names
    auto names = people 
        | std::views::filter([](const Person& p) { return p.age > 28; })
        | std::views::transform(&Person::name);
    
    for (const auto& name : names) {
        std::cout << name << " ";  // Output: Alice Charlie
    }
}
```

##### Projections vs transform

###### Transform (bad for sorting)

```cpp
auto ages = people
  | std::views::transform(&Person::age);

std::ranges::sort(ages); // doesn't sort people!
```

This creates a **view of values**, not a rearrangement of the original range.

###### Projection (correct)

```cpp
std::ranges::sort(people, {}, &Person::age);
```

* No copying
* No intermediate range
* Original container reordered

#### 8. Working with Raw Arrays and C-Style Strings

Ranges work seamlessly with C-style arrays:

```cpp
#include <ranges>
#include <iostream>

int main() {
    int arr[] = {1, 2, 3, 4, 5};
    
    // Raw arrays are ranges too
    auto doubled = arr 
        | std::views::transform([](int n) { return n * 2; });
    
    for (int n : doubled) {
        std::cout << n << " ";  // Output: 2 4 6 8 10
    }
}
```

#### 9. Materialization: Converting Views to Containers

Views are lazy, but sometimes you need to store the results:

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    auto view = numbers 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; });
    
    // Convert view to vector using range constructor
    std::vector<int> result(view.begin(), view.end());
    
    // Or using range constructor (C++23)
    // std::vector<int> result(std::from_range, view);
    
    for (int n : result) {
        std::cout << n << " ";  // Output: 4 16
    }
}
```

#### 10. Real-World Example: Data Processing Pipeline

```cpp
#include <ranges>
#include <vector>
#include <string>
#include <iostream>
#include <algorithm>

struct Product {
    std::string name;
    double price;
    int stock;
};

int main() {
    std::vector<Product> inventory = {
        {"Laptop", 999.99, 5},
        {"Mouse", 29.99, 150},
        {"Keyboard", 79.99, 0},
        {"Monitor", 299.99, 8},
        {"USB Cable", 9.99, 200},
        {"Webcam", 59.99, 0}
    };
    
    // Find expensive in-stock items and calculate total value
    auto expensive_instock = inventory
        | std::views::filter([](const Product& p) { return p.stock > 0; })
        | std::views::filter([](const Product& p) { return p.price > 50; })
        | std::views::transform([](const Product& p) { 
            return p.price * p.stock; 
          });
    
    double total_value = 0.0;
    for (double value : expensive_instock) {
        total_value += value;
    }
    
    std::cout << "Total value of expensive in-stock items: $" 
              << total_value << "\n";
    
    // Get names of out-of-stock items
    auto out_of_stock_names = inventory
        | std::views::filter([](const Product& p) { return p.stock == 0; })
        | std::views::transform(&Product::name);
    
    std::cout << "Out of stock: ";
    for (const auto& name : out_of_stock_names) {
        std::cout << name << " ";
    }
    std::cout << "\n";
}
```

### Benefits of Ranges

1. **Composability**: Chain operations naturally with the pipe operator
2. **Lazy Evaluation**: Views don't compute until you iterate, saving memory and CPU
3. **Readability**: Code reads left-to-right like a data pipeline
4. **Safety**: Range algorithms check bounds and work with the full container
5. **Zero-Cost Abstraction**: Optimized to perform as well as hand-written loops
6. **Projections**: Avoid creating temporary transformed containers

### Performance Considerations

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> large_vec(1000000);
    
    // This is efficient - no intermediate containers created
    auto view = large_vec 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; })
        | std::views::take(10);
    
    // Only computes what's needed when we iterate
    for (int n : view) {
        std::cout << n << " ";
    }
    
    // Compare with old style (creates intermediate containers):
    // std::vector<int> temp1, temp2, temp3;
    // copy_if -> temp1, transform -> temp2, copy_n -> temp3
}
```

### Summary

C++20 Ranges revolutionize C++ by providing a modern, functional approach to working with sequences. Views enable lazy evaluation and composition, range algorithms are safer and more convenient than iterator-based algorithms, and the pipe operator creates readable data processing pipelines.

### Key Things to Remember

1. **Ranges are anything with begin/end** - vectors, arrays, lists, even views themselves
2. **Views are lazy** - they don't compute anything until you iterate over them
3. **Use pipe operator `|`** for composing range adaptors (filter, transform, take, etc.)
4. **No intermediate copies** - views operate on the original data without copying
5. **Range algorithms take whole containers** - `std::ranges::sort(vec)` instead of `std::sort(vec.begin(), vec.end())`
6. **Projections avoid transformations** - sort by member without creating temporary containers
7. **Views are lightweight** - cheap to copy and pass around
8. **Materialize when needed** - convert views to containers when you need to store results
9. **Common views**: `filter`, `transform`, `take`, `drop`, `reverse`, `split`, `join`, `iota`
10. **Check compiler support** - full C++20 ranges require GCC 10+, Clang 13+, or MSVC 19.29+

---

## Coroutines
[Back to top](#major-features)

C++20 coroutines represent a fundamental shift in how C++ handles asynchronous programming and generator-style code. A coroutine is a function that can suspend its execution and later resume from where it left off, maintaining its state between suspensions.

### What Makes a Function a Coroutine?

A function becomes a coroutine when it uses at least one of these keywords:

- **`co_await`** - suspends execution until a result is ready
- **`co_yield`** - suspends execution and produces a value
- **`co_return`** - completes the coroutine and optionally returns a value

### Core Components

#### The Promise Type

Every coroutine has an associated promise type that controls its behavior. The promise object is created when the coroutine starts and manages the coroutine's lifecycle.

```cpp
#include <coroutine>
#include <iostream>

// A simple generator coroutine return type
template<typename T>
struct Generator {
    struct promise_type {
        T current_value;
        
        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
    
    std::coroutine_handle<promise_type> handle;
    
    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Generator() { if (handle) handle.destroy(); }
    
    // Make it move-only
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;
    Generator(Generator&& other) : handle(other.handle) { other.handle = nullptr; }
    
    bool move_next() {
        handle.resume();
        return !handle.done();
    }
    
    T current_value() {
        return handle.promise().current_value;
    }
};

// Example: A generator that produces numbers
Generator<int> counter(int start, int end) {
    for (int i = start; i <= end; ++i) {
        co_yield i;  // This makes it a coroutine
    }
}

int main() {
    auto gen = counter(1, 5);
    
    while (gen.move_next()) {
        std::cout << gen.current_value() << " ";
    }
    std::cout << "\n";  // Output: 1 2 3 4 5
}
```

### co_yield - Generators

`co_yield` is perfect for creating sequences of values without computing them all at once:

```cpp
Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        int next = a + b;
        a = b;
        b = next;
    }
}

int main() {
    auto fib = fibonacci();
    
    for (int i = 0; i < 10; ++i) {
        if (fib.move_next()) {
            std::cout << fib.current_value() << " ";
        }
    }
    // Output: 0 1 1 2 3 5 8 13 21 34
}
```

### co_await - Asynchronous Operations

`co_await` enables you to write asynchronous code that looks synchronous:

```cpp
#include <coroutine>
#include <future>
#include <iostream>
#include <thread>

// A simple task type for async operations
struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_never final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };
    
    std::coroutine_handle<promise_type> handle;
    
    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
};

// An awaitable type that simulates async work
struct AsyncWork {
    bool await_ready() const noexcept { return false; }
    
    void await_suspend(std::coroutine_handle<> h) const {
        // Simulate async work in another thread
        std::thread([h]() {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            h.resume();  // Resume the coroutine when work is done
        }).detach();
    }
    
    void await_resume() const noexcept {
        std::cout << "Async work completed!\n";
    }
};

Task example_async() {
    std::cout << "Starting async operation...\n";
    co_await AsyncWork{};  // Suspends here, resumes when work completes
    std::cout << "Continuing after async work\n";
}
```

### co_return - Returning Values

`co_return` completes the coroutine execution:

```cpp
template<typename T>
struct Result {
    struct promise_type {
        T value;
        
        Result get_return_object() {
            return Result{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        void return_value(T v) {
            value = v;
        }
        
        void unhandled_exception() { std::terminate(); }
    };
    
    std::coroutine_handle<promise_type> handle;
    
    Result(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Result() { if (handle) handle.destroy(); }
    
    T get() {
        return handle.promise().value;
    }
};

Result<int> compute() {
    int result = 42;
    co_return result;  // Complete and return a value
}
```

### Practical Example: Range Generator

```cpp
Generator<int> range(int start, int end, int step = 1) {
    for (int i = start; i < end; i += step) {
        co_yield i;
    }
}

int main() {
    // Generate even numbers from 0 to 10
    auto evens = range(0, 10, 2);
    
    while (evens.move_next()) {
        std::cout << evens.current_value() << " ";
    }
    // Output: 0 2 4 6 8
}
```

### Key Concepts

#### Suspend Points

A coroutine can suspend at three points:
1. **Initial suspend** - right after creation (controlled by `initial_suspend()`)
2. **Yield/await points** - when using `co_yield` or `co_await`
3. **Final suspend** - just before destruction (controlled by `final_suspend()`)

#### Coroutine Handle

`std::coroutine_handle<>` is used to control coroutine execution:
- `resume()` - continue execution
- `done()` - check if coroutine has finished
- `destroy()` - clean up the coroutine

#### Awaitables

An awaitable type must provide three methods:
- `await_ready()` - returns true if the result is already available
- `await_suspend()` - called when suspending (can schedule resumption)
- `await_resume()` - called when resuming (returns the result)

### Benefits and Use Cases

**Benefits:**
- Write asynchronous code that reads like synchronous code
- Lazy evaluation and efficient memory usage
- State is automatically maintained between suspensions
- No callback hell or complex state machines

**Common Use Cases:**
- Generators for sequences (avoiding storing entire sequences in memory)
- Asynchronous I/O operations
- Event loops and cooperative multitasking
- Parser implementations
- Async network programming

### Things to Remember

- **A function is a coroutine if it uses `co_await`, `co_yield`, or `co_return`**
- **The promise type controls coroutine behavior and lifecycle**
- **Coroutines don't execute like normal functions - they can suspend and resume**
- **You must define a return type with a nested `promise_type` struct**
- **`co_yield` produces values lazily - perfect for generators**
- **`co_await` enables writing async code that looks synchronous**
- **Coroutine handles manage execution: `resume()`, `done()`, `destroy()`**
- **Always destroy coroutine handles to prevent memory leaks**
- **Coroutines maintain their state (local variables) across suspensions**
- **Awaitables need three methods: `await_ready`, `await_suspend`, `await_resume`**
- **C++20 coroutines are stackless - state is stored in the heap**
- **The compiler transforms coroutines into state machines automatically**

---

## Three Way Comparison
[Back to top](#major-features)

### Overview

The spaceship operator `<=>`, introduced in C++20, is a three-way comparison operator that performs a single comparison and returns a result indicating whether the left operand is less than, equal to, or greater than the right operand. This revolutionary feature simplifies comparison operations and reduces boilerplate code significantly.

### Why the Spaceship Operator?

Before C++20, implementing all comparison operators for a class required writing up to six separate operators: `==`, `!=`, `<`, `<=`, `>`, and `>=`. This was tedious, error-prone, and required careful maintenance to ensure consistency.

**Traditional approach (pre-C++20):**

```cpp
class Point {
    int x, y;
public:
    Point(int x, int y) : x(x), y(y) {}
    
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator!=(const Point& other) const {
        return !(*this == other);
    }
    
    bool operator<(const Point& other) const {
        if (x != other.x) return x < other.x;
        return y < other.y;
    }
    
    bool operator<=(const Point& other) const {
        return !(other < *this);
    }
    
    bool operator>(const Point& other) const {
        return other < *this;
    }
    
    bool operator>=(const Point& other) const {
        return !(*this < other);
    }
};
```

### Return Types of Three-Way Comparison

The spaceship operator returns one of three ordering types from the `<compare>` header:

#### 1. `std::strong_ordering`
For types with total ordering where equivalent values are indistinguishable (like integers).

Possible values:
- `std::strong_ordering::less`
- `std::strong_ordering::equal`
- `std::strong_ordering::greater`

#### 2. `std::weak_ordering`
For types where equivalent values might not be identical (like case-insensitive strings).

Possible values:
- `std::weak_ordering::less`
- `std::weak_ordering::equivalent`
- `std::weak_ordering::greater`

#### 3. `std::partial_ordering`
For types where not all values are comparable (like floating-point with NaN).

Possible values:
- `std::partial_ordering::less`
- `std::partial_ordering::equivalent`
- `std::partial_ordering::greater`
- `std::partial_ordering::unordered`

### Basic Usage

#### Simple Example with Default Spaceship Operator

```cpp
#include <compare>
#include <iostream>

class Point {
    int x, y;
public:
    Point(int x, int y) : x(x), y(y) {}
    
    // Compiler generates all six comparison operators!
    auto operator<=>(const Point&) const = default;
    
    // Still need to explicitly default == for optimal performance
    bool operator==(const Point&) const = default;
};

int main() {
    Point p1(1, 2);
    Point p2(1, 3);
    Point p3(1, 2);
    
    std::cout << (p1 < p2) << '\n';   // true
    std::cout << (p1 == p3) << '\n';  // true
    std::cout << (p2 >= p1) << '\n';  // true
}
```

#### Custom Implementation

```cpp
#include <compare>
#include <string>

class Person {
    std::string name;
    int age;
public:
    Person(std::string name, int age) : name(name), age(age) {}
    
    // Custom three-way comparison: compare by age, then by name
    std::strong_ordering operator<=>(const Person& other) const {
        if (auto cmp = age <=> other.age; cmp != 0)
            return cmp;
        return name <=> other.name;
    }
    
    bool operator==(const Person& other) const {
        return age == other.age && name == other.name;
    }
};
```

### Advanced Examples

#### Mixed-Type Comparisons

```cpp
#include <compare>

class Distance {
    double meters;
public:
    Distance(double m) : meters(m) {}
    
    auto operator<=>(const Distance&) const = default;
    
    // Compare Distance with raw double (meters)
    std::partial_ordering operator<=>(double m) const {
        return meters <=> m;
    }
};

int main() {
    Distance d1(100.0);
    Distance d2(50.0);
    
    bool b1 = d1 > d2;      // true
    bool b2 = d1 > 75.0;    // true (using mixed-type comparison)
}
```

#### Handling Non-Comparable Members

```cpp
#include <compare>
#include <string>
#include <optional>

class Product {
    std::string name;
    double price;
    std::optional<int> stock;
public:
    Product(std::string n, double p, std::optional<int> s = std::nullopt)
        : name(n), price(p), stock(s) {}
    
    std::partial_ordering operator<=>(const Product& other) const {
        // Compare by price first
        if (auto cmp = price <=> other.price; cmp != 0)
            return cmp;
        
        // If both have stock info, compare it
        if (stock.has_value() && other.stock.has_value()) {
            return *stock <=> *other.stock;
        }
        
        // If one has stock and other doesn't, they're unordered
        if (stock.has_value() != other.stock.has_value()) {
            return std::partial_ordering::unordered;
        }
        
        // Both have no stock info, they're equivalent
        return std::partial_ordering::equivalent;
    }
    
    bool operator==(const Product& other) const = default;
};
```

#### Lexicographic Comparison with Tie

```cpp
#include <compare>
#include <tuple>
#include <string>

class Employee {
    std::string department;
    std::string name;
    int id;
public:
    Employee(std::string dept, std::string name, int id)
        : department(dept), name(name), id(id) {}
    
    // Elegant lexicographic comparison using tuple
    auto operator<=>(const Employee& other) const {
        return std::tie(department, name, id) <=> 
               std::tie(other.department, other.name, other.id);
    }
    
    bool operator==(const Employee& other) const = default;
};
```

### Compiler-Generated Comparisons

When you use `= default`, the compiler generates comparisons member-by-member in declaration order:

```cpp
struct Book {
    std::string title;
    std::string author;
    int year;
    
    // Compares: title first, then author, then year
    auto operator<=>(const Book&) const = default;
    bool operator==(const Book&) const = default;
};
```

### Important Behavior Notes

#### Operator `==` and `<=>`

For optimal performance, explicitly default both operators:

```cpp
auto operator<=>(const MyClass&) const = default;
bool operator==(const MyClass&) const = default;  // More efficient for equality
```

If you only provide `<=>`, the compiler can synthesize `==` from it, but it's less efficient as it performs a three-way comparison instead of a simple equality check.

#### Rewritten Expressions

The compiler automatically rewrites comparison expressions:

```cpp
Point p1, p2;

// These use operator<=>
p1 < p2   // becomes: (p1 <=> p2) < 0
p1 <= p2  // becomes: (p1 <=> p2) <= 0
p1 > p2   // becomes: (p1 <=> p2) > 0
p1 >= p2  // becomes: (p1 <=> p2) >= 0

// These use operator==
p1 == p2  // uses operator==
p1 != p2  // becomes: !(p1 == p2)
```

#### Symmetry

The compiler also considers reversed argument order:

```cpp
Distance d(100.0);
bool b = 75.0 < d;  // Compiler tries: d > 75.0
```

### Practical Example: Complete Class

```cpp
#include <compare>
#include <iostream>
#include <string>

class Version {
    int major, minor, patch;
public:
    Version(int maj, int min, int pat) 
        : major(maj), minor(min), patch(pat) {}
    
    auto operator<=>(const Version& other) const {
        if (auto cmp = major <=> other.major; cmp != 0) return cmp;
        if (auto cmp = minor <=> other.minor; cmp != 0) return cmp;
        return patch <=> other.patch;
    }
    
    bool operator==(const Version& other) const = default;
    
    friend std::ostream& operator<<(std::ostream& os, const Version& v) {
        return os << v.major << '.' << v.minor << '.' << v.patch;
    }
};

int main() {
    Version v1(1, 2, 3);
    Version v2(1, 3, 0);
    Version v3(1, 2, 3);
    
    std::cout << "v1 < v2: " << (v1 < v2) << '\n';    // true
    std::cout << "v1 == v3: " << (v1 == v3) << '\n';  // true
    std::cout << "v2 >= v1: " << (v2 >= v1) << '\n';  // true
    
    if (v1 <=> v2 < 0) {
        std::cout << v1 << " is older than " << v2 << '\n';
    }
}
```

### Summary: Key Points to Remember

- **Spaceship operator `<=>` returns ordering**: `strong_ordering`, `weak_ordering`, or `partial_ordering`
- **Use `= default` for automatic generation**: Compiler generates all six comparison operators from `<=>` and `==`
- **Always explicitly default `operator==`**: More efficient than synthesizing from `<=>`
- **Return types matter**: Choose `strong_ordering` for total order, `weak_ordering` for equivalence classes, `partial_ordering` for incomparable values
- **Lexicographic ordering with `std::tie`**: Elegant way to compare multiple members
- **Compiler rewrites expressions**: `a < b` becomes `(a <=> b) < 0`, `a != b` becomes `!(a == b)`
- **Supports mixed-type comparisons**: Can define `<=>` for comparing with different types
- **Declaration order matters with `= default`**: Members are compared in the order they're declared
- **Reduces boilerplate significantly**: Six operators from one or two definitions
- **Include `<compare>` header**: Required for ordering types and spaceship operator

---

## Modules
[Back to top](#major-features)

C++20 modules represent one of the most significant changes to the C++ compilation model since the language's inception. They provide a modern alternative to the traditional header file system, offering faster compilation times, better encapsulation, and cleaner separation of interfaces from implementation.

### What Are Modules?

Modules are a new way to organize and share code in C++. Unlike header files that use textual inclusion with `#include`, modules are compiled once into a binary format that can be efficiently imported by other translation units. This eliminates many problems associated with the preprocessor-based header system.

### Key Advantages Over Headers

**Faster Compilation**: Modules are parsed once and stored in a binary format. When you import a module, the compiler loads this pre-processed representation instead of re-parsing text files repeatedly.

**Better Encapsulation**: Only explicitly exported entities are visible to importers. Implementation details remain truly private without relying on naming conventions or separate files.

**Order Independence**: Unlike headers where order matters due to macro definitions and declarations, module imports are order-independent.

**No Macro Leakage**: Macros defined in a module don't leak into importing code (with some exceptions for the global module fragment).

**Elimination of Include Guards**: No more need for header guards or `#pragma once`.

### Basic Module Syntax

#### Creating a Module

Here's a simple module that exports a function:

```cpp
// math_utils.cpp (module interface unit)
export module math_utils;

export int add(int a, int b) {
    return a + b;
}

export int multiply(int a, int b) {
    return a * b;
}

// This function is NOT exported - it's private to the module
int internal_helper() {
    return 42;
}
```

#### Importing and Using a Module

```cpp
// main.cpp
import math_utils;
import <iostream>; // Can also import standard library headers as modules

int main() {
    std::cout << "5 + 3 = " << add(5, 3) << '\n';
    std::cout << "5 * 3 = " << multiply(5, 3) << '\n';
    
    // internal_helper(); // ERROR: not visible, not exported
    return 0;
}
```

### Module Interface vs Implementation

You can separate the interface (what's exported) from the implementation:

#### Module Interface Unit

```cpp
// calculator.cppm or calculator.ixx
export module calculator;

export class Calculator {
public:
    int add(int a, int b);
    int subtract(int a, int b);
    
private:
    void log_operation(const char* op);
};

export double compute_average(int* values, int count);
```

#### Module Implementation Unit

```cpp
// calculator.cpp
module calculator; // Note: no 'export' keyword

#include <iostream>

// Implement the member functions
int Calculator::add(int a, int b) {
    log_operation("add");
    return a + b;
}

int Calculator::subtract(int a, int b) {
    log_operation("subtract");
    return a - b;
}

void Calculator::log_operation(const char* op) {
    std::cout << "Operation: " << op << '\n';
}

double compute_average(int* values, int count) {
    int sum = 0;
    for (int i = 0; i < count; ++i) {
        sum += values[i];
    }
    return static_cast<double>(sum) / count;
}
```

### Module Partitions

Large modules can be split into partitions for better organization:

```cpp
// graphics:shapes (partition interface)
export module graphics:shapes;

export class Circle {
    double radius;
public:
    Circle(double r) : radius(r) {}
    double area() const;
};

export class Rectangle {
    double width, height;
public:
    Rectangle(double w, double h) : width(w), height(h) {}
    double area() const;
};
```

```cpp
// graphics:colors (another partition)
export module graphics:colors;

export enum class Color { Red, Green, Blue, Yellow };

export const char* color_name(Color c);
```

```cpp
// graphics.cpp (primary module interface)
export module graphics;

export import :shapes;   // Re-export the shapes partition
export import :colors;   // Re-export the colors partition

export void draw(const Circle& c, Color col);
export void draw(const Rectangle& r, Color col);
```

```cpp
// user code
import graphics; // Gets everything: shapes, colors, and drawing functions

int main() {
    Circle c(5.0);
    draw(c, Color::Red);
    return 0;
}
```

### The Global Module Fragment

Sometimes you need to include traditional headers (especially for macros or to interface with legacy code). The global module fragment allows this:

```cpp
// file_utils.cpp
module;  // Begin global module fragment

#include <cstdio>
#include <cerrno>
#define BUFFER_SIZE 4096  // Macro defined here

export module file_utils;

import <string>;
import <vector>;

export class FileReader {
    FILE* file;
    char buffer[BUFFER_SIZE];  // Uses macro from global fragment
    
public:
    FileReader(const std::string& filename);
    std::vector<std::string> read_lines();
    ~FileReader();
};

// Implementation...
FileReader::FileReader(const std::string& filename) {
    file = fopen(filename.c_str(), "r");
}

// ... rest of implementation
```

### Private Module Fragment

For single-file modules, you can include implementation details after the interface:

```cpp
export module utils;

export int public_function(int x) {
    return private_helper(x) * 2;
}

module :private;  // Everything after this is not exported

int private_helper(int x) {
    return x + 10;
}

static void internal_function() {
    // Only visible within this translation unit
}
```

### Importing Standard Library

C++20 allows importing standard library headers as modules:

```cpp
import <iostream>;
import <vector>;
import <string>;
import <algorithm>;

int main() {
    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    std::sort(names.begin(), names.end());
    
    for (const auto& name : names) {
        std::cout << name << '\n';
    }
}
```

### Real-World Example: A Logging Module

```cpp
// logger.cppm
export module logger;

import <string>;
import <iostream>;
import <chrono>;
import <format>;

export enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

export class Logger {
    LogLevel min_level;
    
public:
    Logger(LogLevel level = LogLevel::Info) : min_level(level) {}
    
    void log(LogLevel level, const std::string& message) {
        if (level < min_level) return;
        
        auto now = std::chrono::system_clock::now();
        std::cout << format_timestamp(now) << " ["
                  << level_to_string(level) << "] "
                  << message << '\n';
    }
    
    void debug(const std::string& msg) { log(LogLevel::Debug, msg); }
    void info(const std::string& msg) { log(LogLevel::Info, msg); }
    void warning(const std::string& msg) { log(LogLevel::Warning, msg); }
    void error(const std::string& msg) { log(LogLevel::Error, msg); }

private:
    std::string format_timestamp(std::chrono::system_clock::time_point tp) {
        // Implementation detail - not exported
        auto time = std::chrono::system_clock::to_time_t(tp);
        return std::format("{:%Y-%m-%d %H:%M:%S}", *std::localtime(&time));
    }
    
    const char* level_to_string(LogLevel level) {
        switch (level) {
            case LogLevel::Debug: return "DEBUG";
            case LogLevel::Info: return "INFO";
            case LogLevel::Warning: return "WARN";
            case LogLevel::Error: return "ERROR";
            default: return "UNKNOWN";
        }
    }
};

// Usage
import logger;

int main() {
    Logger log(LogLevel::Debug);
    log.info("Application started");
    log.debug("Processing configuration");
    log.warning("Cache size approaching limit");
    log.error("Failed to connect to database");
}
```

### Comparison: Headers vs Modules

**With Traditional Headers:**
```cpp
// math.h
#ifndef MATH_H
#define MATH_H

#define SQUARE(x) ((x) * (x))  // Macro leaks to all includers

int add(int a, int b);

#endif
```

**With Modules:**
```cpp
// math module
export module math;

// Macro doesn't leak outside
#define SQUARE(x) ((x) * (x))

export int add(int a, int b) {
    return a + b;
}
```

### Compilation Considerations

Module compilation typically requires two phases:

1. **Build Module Interface**: Compile the module interface unit first
2. **Build Consumers**: Compile files that import the module

Example with GCC/Clang:
```bash
# Compile the module
g++ -std=c++20 -fmodules-ts -c math_utils.cpp -o math_utils.o

# Compile the user code
g++ -std=c++20 -fmodules-ts main.cpp math_utils.o -o program
```

Different compilers have different flags and module file extensions (.ixx, .cppm, .mpp).

### Summary: Key Points to Remember

- **Modules replace headers** with a binary compilation model—parse once, import many times
- **Use `export module name;`** to declare a module interface unit
- **Use `import module_name;`** to import a module (not `#include`)
- **Only exported entities are visible**—everything else is truly private
- **Macros don't leak** from modules to importers (better hygiene)
- **No include guards needed**—modules handle this automatically
- **Partitions split large modules**: `export module name:partition;`
- **Global module fragment** (`module;` at top) allows legacy `#include` directives
- **Private module fragment** (`module :private;`) hides implementation in same file
- **Compilation order matters**: build module interfaces before importers
- **Standard library can be imported** with `import <header>;` syntax
- **Faster builds** due to avoiding redundant parsing of headers
- **Better IDE support** with clear module boundaries and dependencies
- **Compiler support varies**—check your toolchain's C++20 module implementation

---

## Designated Initializers
[Back to top](#major-features)

### Overview

Designated initializers are a C++20 feature that allows you to explicitly specify which member variables you're initializing when creating an aggregate type. This makes initialization more readable and less error-prone, especially for structs with many members.

### Basic Syntax

The syntax uses the member name with a dot prefix inside the initialization braces:

```cpp
struct Point {
    int x;
    int y;
    int z;
};

// Traditional aggregate initialization
Point p1 = {1, 2, 3};  // Must remember order

// C++20 designated initializers
Point p2 = {.x = 1, .y = 2, .z = 3};  // Clear and explicit
```

### Key Features and Examples

#### Self-Documenting Code

Designated initializers make your code more readable by explicitly showing which value corresponds to which member:

```cpp
struct Config {
    bool enableLogging;
    int maxConnections;
    double timeout;
    const char* serverName;
};

// Clear what each value represents
Config cfg = {
    .enableLogging = true,
    .maxConnections = 100,
    .timeout = 30.5,
    .serverName = "MainServer"
};
```

#### Partial Initialization

You can initialize only specific members, and the rest will be default-initialized (zero for primitives):

```cpp
struct Rectangle {
    int x;
    int y;
    int width;
    int height;
};

// Only initialize some members
Rectangle rect = {.width = 50, .height = 30};
// x and y are zero-initialized
```

#### Order Matters

Unlike C99 designated initializers, C++20 requires that you specify members in the same order they're declared in the struct:

```cpp
struct Data {
    int a;
    int b;
    int c;
};

// Valid: follows declaration order
Data d1 = {.a = 1, .b = 2, .c = 3};
Data d2 = {.a = 1, .c = 3};  // Also valid (skipping b)

// INVALID: out of order
// Data d3 = {.c = 3, .a = 1, .b = 2};  // Compilation error!
```

#### Mixing with Positional Initialization

You cannot mix designated and positional initializers:

```cpp
struct Point3D {
    int x, y, z;
};

// Valid
Point3D p1 = {1, 2, 3};              // All positional
Point3D p2 = {.x = 1, .y = 2, .z = 3};  // All designated

// INVALID: cannot mix
// Point3D p3 = {1, .y = 2, .z = 3};  // Compilation error!
```

#### Nested Structures

Designated initializers work with nested structures:

```cpp
struct Address {
    const char* street;
    const char* city;
    int zipCode;
};

struct Person {
    const char* name;
    int age;
    Address address;
};

Person p = {
    .name = "Alice",
    .age = 30,
    .address = {
        .street = "123 Main St",
        .city = "Springfield",
        .zipCode = 12345
    }
};
```

#### Arrays of Structs

You can use designated initializers when creating arrays of structs:

```cpp
struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
};

Color palette[] = {
    {.r = 255, .g = 0, .b = 0},      // Red
    {.r = 0, .g = 255, .b = 0},      // Green
    {.r = 0, .g = 0, .b = 255},      // Blue
    {.r = 255, .g = 255, .b = 0}     // Yellow
};
```

#### Real-World Example: Function Parameters

Designated initializers are particularly useful when passing configuration objects to functions:

```cpp
struct HttpRequest {
    const char* method;
    const char* url;
    int timeout;
    bool followRedirects;
    int maxRetries;
};

void sendRequest(const HttpRequest& request) {
    // Implementation
}

int main() {
    // Very clear what each parameter means
    sendRequest({
        .method = "POST",
        .url = "https://api.example.com/data",
        .timeout = 5000,
        .followRedirects = true,
        .maxRetries = 3
    });
}
```

#### With Default Member Initializers

Designated initializers work well with default member initializers:

```cpp
struct Settings {
    int maxUsers = 100;         // Default value
    bool debugMode = false;     // Default value
    double scaleFactor = 1.0;   // Default value
};

// Override only what you need
Settings s1 = {.debugMode = true};  // maxUsers=100, scaleFactor=1.0
Settings s2 = {.maxUsers = 200, .scaleFactor = 1.5};  // debugMode=false
```

### Restrictions and Limitations

#### Must Be Aggregate Types

Designated initializers only work with aggregate types. A type is an aggregate if it:
- Has no user-declared or inherited constructors
- Has no private or protected non-static data members
- Has no virtual functions
- Has no virtual, private, or protected base classes

```cpp
// Valid: aggregate type
struct ValidStruct {
    int x;
    double y;
};

// Invalid: has constructor
struct InvalidStruct {
    int x;
    double y;
    InvalidStruct(int a, double b) : x(a), y(b) {}
};

// Cannot use designated initializers with InvalidStruct
```

#### No Array Element Designation

Unlike C, C++20 doesn't support designating array elements:

```cpp
struct Data {
    int arr[3];
};

// C allows: Data d = {.arr[0] = 1, .arr[2] = 3};
// C++ does NOT allow this - must initialize entire array
Data d = {.arr = {1, 0, 3}};
```

### Benefits

1. **Readability**: Code is self-documenting, making it clear which value corresponds to which member
2. **Maintainability**: Adding new members to the middle of a struct doesn't break existing initialization code that only uses some members
3. **Safety**: Reduces errors from incorrect ordering of initialization values
4. **Flexibility**: Allows partial initialization of structures

### Summary: Key Things to Remember

- **Explicit member naming**: Use `.memberName = value` syntax inside braces
- **Order must match declaration**: Members must be initialized in the order they appear in the struct
- **Aggregates only**: Works only with aggregate types (no user-defined constructors, no private members, no virtual functions)
- **No mixing**: Cannot mix designated and positional initializers in the same initialization
- **Partial initialization allowed**: Unspecified members are default-initialized (zero for primitives)
- **Nested structures supported**: Can designate members of nested structs
- **No array element designation**: Cannot designate individual array elements like C does
- **C++20 feature**: Requires compiler with C++20 support (`-std=c++20`)
- **Self-documenting**: Makes code more readable and less error-prone
- **Works with defaults**: Combines well with default member initializers

---

## Format
[Back to top](#major-features)

### Introduction

`std::format` is a type-safe, extensible text formatting library introduced in C++20 that provides a modern alternative to `printf` and stream-based formatting. It's based on the popular {fmt} library and offers Python-like formatting syntax with compile-time format string checking.

### Basic Syntax

The basic syntax uses curly braces `{}` as replacement fields:

```cpp
#include <format>
#include <string>
#include <iostream>

int main() {
    std::string name = "Alice";
    int age = 30;
    
    // Basic formatting
    std::string result = std::format("Hello, {}! You are {} years old.", name, age);
    std::cout << result << '\n';
    // Output: Hello, Alice! You are 30 years old.
    
    // Direct output (C++23)
    // std::print("Hello, {}! You are {} years old.\n", name, age);
}
```

### Positional and Named Arguments

```cpp
#include <format>
#include <iostream>

int main() {
    // Automatic indexing
    std::cout << std::format("{} + {} = {}", 2, 3, 5) << '\n';
    // Output: 2 + 3 = 5
    
    // Manual indexing (can reorder or reuse arguments)
    std::cout << std::format("{0} + {1} = {2}, and {1} + {0} = {2}", 2, 3, 5) << '\n';
    // Output: 2 + 3 = 5, and 3 + 2 = 5
    
    // Mixing is not allowed (compile error)
    // std::format("{} and {1}", 1, 2); // ERROR!
}
```

### Format Specifications

The general format specification syntax is:
```
{[index]:[fill][align][sign][#][0][width][.precision][type]}
```

#### Width and Alignment

```cpp
#include <format>
#include <iostream>

int main() {
    // Width specification
    std::cout << std::format("|{:10}|", "text") << '\n';
    // Output: |text      | (left-aligned by default)
    
    // Alignment: < (left), > (right), ^ (center)
    std::cout << std::format("|{:<10}|", "left") << '\n';   // |left      |
    std::cout << std::format("|{:>10}|", "right") << '\n';  // |     right|
    std::cout << std::format("|{:^10}|", "center") << '\n'; // |  center  |
    
    // Fill character with alignment
    std::cout << std::format("|{:*<10}|", "fill") << '\n';  // |fill******|
    std::cout << std::format("|{:*>10}|", "fill") << '\n';  // |******fill|
    std::cout << std::format("|{:*^10}|", "fill") << '\n';  // |***fill***|
}
```

#### Numeric Formatting

```cpp
#include <format>
#include <iostream>

int main() {
    int num = 42;
    
    // Different bases
    std::cout << std::format("Decimal: {}", num) << '\n';      // 42
    std::cout << std::format("Hex: {:x}", num) << '\n';        // 2a
    std::cout << std::format("Hex (upper): {:X}", num) << '\n'; // 2A
    std::cout << std::format("Octal: {:o}", num) << '\n';      // 52
    std::cout << std::format("Binary: {:b}", num) << '\n';     // 101010
    
    // With prefix
    std::cout << std::format("Hex with prefix: {:#x}", num) << '\n';  // 0x2a
    std::cout << std::format("Binary with prefix: {:#b}", num) << '\n'; // 0b101010
    
    // Sign handling
    std::cout << std::format("{:+}", 42) << '\n';   // +42
    std::cout << std::format("{:+}", -42) << '\n';  // -42
    std::cout << std::format("{: }", 42) << '\n';   //  42 (space for positive)
    
    // Zero padding
    std::cout << std::format("{:05}", 42) << '\n';  // 00042
    std::cout << std::format("{:+05}", 42) << '\n'; // +0042
}
```

#### Floating-Point Formatting

```cpp
#include <format>
#include <iostream>
#include <numbers>

int main() {
    double pi = std::numbers::pi;
    
    // Default
    std::cout << std::format("{}", pi) << '\n';  // 3.141592653589793
    
    // Fixed-point notation
    std::cout << std::format("{:.2f}", pi) << '\n';  // 3.14
    std::cout << std::format("{:.5f}", pi) << '\n';  // 3.14159
    
    // Scientific notation
    std::cout << std::format("{:.2e}", pi) << '\n';  // 3.14e+00
    std::cout << std::format("{:.2E}", pi) << '\n';  // 3.14E+00
    
    // General format (chooses shortest representation)
    std::cout << std::format("{:.3g}", pi) << '\n';      // 3.14
    std::cout << std::format("{:.3g}", 0.000123) << '\n'; // 0.000123
    std::cout << std::format("{:.3g}", 0.0000123) << '\n'; // 1.23e-05
    
    // Width and precision combined
    std::cout << std::format("{:10.2f}", pi) << '\n';  // "      3.14"
}
```

### Dynamic Width and Precision

```cpp
#include <format>
#include <iostream>

int main() {
    int width = 10;
    int precision = 3;
    double value = 3.14159;
    
    // Dynamic width and precision using nested braces
    std::string result = std::format("{:{}}", "text", width);
    std::cout << result << '\n';  // "text      "
    
    std::string result2 = std::format("{:.{}f}", value, precision);
    std::cout << result2 << '\n';  // "3.142"
    
    // Both dynamic
    std::string result3 = std::format("{:{}.{}f}", value, width, precision);
    std::cout << result3 << '\n';  // "     3.142"
}
```

### Formatting User-Defined Types

You can make your custom types formattable by specializing `std::formatter`:

```cpp
#include <format>
#include <iostream>

struct Point {
    int x, y;
};

// Specialize std::formatter for Point
template <>
struct std::formatter<Point> {
    // Parse format specification (optional)
    constexpr auto parse(std::format_parse_context& ctx) {
        return ctx.begin();  // Simple implementation: no format specs
    }
    
    // Format the Point object
    auto format(const Point& p, std::format_context& ctx) const {
        return std::format_to(ctx.out(), "({}, {})", p.x, p.y);
    }
};

int main() {
    Point p{10, 20};
    std::cout << std::format("Point: {}", p) << '\n';
    // Output: Point: (10, 20)
    
    std::cout << std::format("Origin: {} End: {}", Point{0, 0}, Point{100, 100}) << '\n';
    // Output: Origin: (0, 0) End: (100, 100)
}
```

#### More Advanced Custom Formatter with Format Specs

```cpp
#include <format>
#include <iostream>

struct Vector3D {
    double x, y, z;
};

template <>
struct std::formatter<Vector3D> {
    char presentation = 'f';  // 'f' for fixed, 's' for scientific
    int precision = 2;
    
    constexpr auto parse(std::format_parse_context& ctx) {
        auto it = ctx.begin();
        if (it != ctx.end() && (*it == 'f' || *it == 's')) {
            presentation = *it++;
        }
        return it;
    }
    
    auto format(const Vector3D& v, std::format_context& ctx) const {
        if (presentation == 's') {
            return std::format_to(ctx.out(), "({:.2e}, {:.2e}, {:.2e})", 
                                  v.x, v.y, v.z);
        }
        return std::format_to(ctx.out(), "({:.2f}, {:.2f}, {:.2f})", 
                              v.x, v.y, v.z);
    }
};

int main() {
    Vector3D v{1.23456, 2.34567, 3.45678};
    
    std::cout << std::format("{}", v) << '\n';    // (1.23, 2.35, 3.46) - default
    std::cout << std::format("{:f}", v) << '\n';  // (1.23, 2.35, 3.46) - fixed
    std::cout << std::format("{:s}", v) << '\n';  // (1.23e+00, 2.35e+00, 3.46e+00)
}
```

### Practical Applications

#### Table Formatting

```cpp
#include <format>
#include <iostream>
#include <vector>

struct Product {
    std::string name;
    double price;
    int quantity;
};

int main() {
    std::vector<Product> products = {
        {"Apple", 1.99, 50},
        {"Banana", 0.59, 120},
        {"Orange", 2.49, 75}
    };
    
    std::cout << std::format("{:<15} {:>10} {:>10}\n", "Product", "Price", "Quantity");
    std::cout << std::format("{:-<40}\n", "");
    
    for (const auto& p : products) {
        std::cout << std::format("{:<15} ${:>9.2f} {:>10}\n", 
                                p.name, p.price, p.quantity);
    }
}

/* Output:
Product              Price   Quantity
----------------------------------------
Apple               $     1.99         50
Banana              $     0.59        120
Orange              $     2.49         75
*/
```

#### Error Messages

```cpp
#include <format>
#include <iostream>
#include <stdexcept>

void processFile(const std::string& filename, int lineNumber) {
    throw std::runtime_error(
        std::format("Error in file '{}' at line {}: Invalid format", 
                   filename, lineNumber)
    );
}

int main() {
    try {
        processFile("config.txt", 42);
    } catch (const std::exception& e) {
        std::cerr << e.what() << '\n';
        // Output: Error in file 'config.txt' at line 42: Invalid format
    }
}
```

### Performance Considerations

```cpp
#include <format>
#include <sstream>
#include <iostream>
#include <chrono>

int main() {
    const int iterations = 100000;
    
    // std::format (compile-time checked, type-safe)
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::string s = std::format("Value: {}, Index: {}", 42, i);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto format_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // std::ostringstream (traditional approach)
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; ++i) {
        std::ostringstream oss;
        oss << "Value: " << 42 << ", Index: " << i;
        std::string s = oss.str();
    }
    end = std::chrono::high_resolution_clock::now();
    auto stream_time = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << std::format("std::format time: {}ms\n", format_time.count());
    std::cout << std::format("std::ostringstream time: {}ms\n", stream_time.count());
    
    // std::format is typically faster and more convenient
}
```

### Compile-Time Format String Checking

```cpp
#include <format>

int main() {
    // Correct - compiles fine
    std::string s1 = std::format("{} {}", 1, 2);
    
    // Compile error: too few arguments
    // std::string s2 = std::format("{} {} {}", 1, 2);
    
    // Compile error: invalid format string
    // std::string s3 = std::format("{:x}", "not a number");
    
    // Compile error: mixing automatic and manual indexing
    // std::string s4 = std::format("{} {1}", 1, 2);
}
```

### Related Functions

```cpp
#include <format>
#include <iostream>
#include <vector>

int main() {
    std::vector<char> buffer;
    
    // std::format_to - formats to an output iterator
    std::format_to(std::back_inserter(buffer), "Value: {}", 42);
    std::cout << std::string(buffer.begin(), buffer.end()) << '\n';
    
    // std::format_to_n - formats at most n characters
    buffer.clear();
    auto result = std::format_to_n(std::back_inserter(buffer), 10, 
                                   "This is a long string");
    std::cout << "Wrote " << result.size << " characters\n";
    
    // std::formatted_size - returns the size without formatting
    size_t size = std::formatted_size("Value: {}, {}", 42, "text");
    std::cout << "Would need " << size << " characters\n";
}
```

### Summary: Key Points to Remember

1. **Basic syntax**: `std::format("text {} text", arg)` - uses `{}` as placeholders
2. **Type-safe**: Compile-time checking of format strings and argument types
3. **Header**: `#include <format>`
4. **Indexing**: Automatic `{}` or manual `{0}`, `{1}` (cannot mix both)
5. **Format spec syntax**: `{:[fill][align][sign][#][0][width][.precision][type]}`
6. **Alignment**: `<` (left), `>` (right), `^` (center)
7. **Number formats**: `:d` (decimal), `:x`/`:X` (hex), `:o` (octal), `:b` (binary)
8. **Float formats**: `:f` (fixed), `:e`/`:E` (scientific), `:g`/`:G` (general)
9. **Precision**: `.n` for floating-point precision (e.g., `:.2f`)
10. **Sign**: `+` (always show), `-` (only negative), ` ` (space for positive)
11. **Prefix**: `#` adds base prefix (`0x`, `0b`, `0o`)
12. **Zero padding**: `:0n` pads with zeros to width n
13. **Dynamic width/precision**: Use nested braces `{:{}.{}f}`
14. **Custom types**: Specialize `std::formatter<YourType>`
15. **Related functions**: `std::format_to()`, `std::format_to_n()`, `std::formatted_size()`
16. **Performance**: Generally faster than `std::ostringstream`, safer than `printf`
17. **Escaping braces**: Use `{{` for `{` and `}}` for `}`

---

## Span
[Back to top](#major-features)

### What is std::span?

`std::span` is a lightweight, non-owning view over a contiguous sequence of objects. Introduced in C++20, it provides a safe and efficient way to pass arrays and array-like objects to functions without copying data or losing size information. Think of it as a "reference to an array" that knows its own size.

A span consists of two components: a pointer to the first element and a size (number of elements). It doesn't own the data it points to, so it's cheap to copy and pass around.

### Key Characteristics

**Non-owning**: std::span doesn't manage the lifetime of the underlying data. The original container must outlive the span.

**Contiguous memory**: Works only with contiguous sequences like arrays, std::vector, std::array, or raw pointers with known size.

**Two variants**: Can have static extent (size known at compile-time) or dynamic extent (size known at runtime).

**Safe access**: Provides bounds-checked access in debug mode and range-based operations.

### Basic Usage Examples

#### Creating spans from different sources

```cpp
#include <span>
#include <vector>
#include <array>
#include <iostream>

void print_elements(std::span<const int> data) {
    for (int value : data) {
        std::cout << value << " ";
    }
    std::cout << "\n";
}

int main() {
    // From C-style array
    int c_array[] = {1, 2, 3, 4, 5};
    std::span<int> span1(c_array);
    
    // From std::array
    std::array<int, 5> std_array = {10, 20, 30, 40, 50};
    std::span<int> span2(std_array);
    
    // From std::vector
    std::vector<int> vec = {100, 200, 300};
    std::span<int> span3(vec);
    
    // From pointer and size
    int* ptr = c_array;
    std::span<int> span4(ptr, 3);  // First 3 elements
    
    // All can be passed to the same function
    print_elements(span1);  // 1 2 3 4 5
    print_elements(span2);  // 10 20 30 40 50
    print_elements(span3);  // 100 200 300
    print_elements(span4);  // 1 2 3
}
```

#### Static vs Dynamic Extent

```cpp
#include <span>
#include <array>

int main() {
    std::array<int, 5> arr = {1, 2, 3, 4, 5};
    
    // Static extent - size known at compile time
    std::span<int, 5> static_span(arr);
    // static_span.size() is constexpr
    
    // Dynamic extent - size known at runtime
    std::span<int> dynamic_span(arr);
    // dynamic_span.size() is not constexpr
    
    // Can convert static to dynamic, but not vice versa
    std::span<int> converted = static_span;
}
```

### Common Operations

#### Accessing Elements

```cpp
#include <span>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> vec = {10, 20, 30, 40, 50};
    std::span<int> s(vec);
    
    // Array-style access
    std::cout << s[0] << "\n";        // 10
    std::cout << s.front() << "\n";   // 10
    std::cout << s.back() << "\n";    // 50
    
    // Size information
    std::cout << s.size() << "\n";         // 5
    std::cout << s.size_bytes() << "\n";   // 20 (5 * sizeof(int))
    std::cout << s.empty() << "\n";        // false
    
    // Pointer access
    int* data_ptr = s.data();
}
```

#### Creating Subspans

```cpp
#include <span>
#include <iostream>

void print_span(std::span<const int> s) {
    for (int val : s) std::cout << val << " ";
    std::cout << "\n";
}

int main() {
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8};
    std::span<int> full(arr);
    
    // First N elements
    auto first_three = full.first(3);
    print_span(first_three);  // 1 2 3
    
    // Last N elements
    auto last_three = full.last(3);
    print_span(last_three);   // 6 7 8
    
    // Subspan from offset with count
    auto middle = full.subspan(2, 4);
    print_span(middle);       // 3 4 5 6
    
    // Subspan from offset to end
    auto from_index_3 = full.subspan(3);
    print_span(from_index_3); // 4 5 6 7 8
}
```

### Practical Use Cases

#### Replacing Raw Pointers and Size Parameters

```cpp
// Before C++20 - unsafe and verbose
void process_old(const int* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        // Process data[i]
        // Easy to make mistakes with size
    }
}

// With C++20 - clean and safe
void process_new(std::span<const int> data) {
    for (int value : data) {
        // Process value
        // Can't accidentally go out of bounds
    }
}

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // Old way
    process_old(vec.data(), vec.size());
    
    // New way - much cleaner!
    process_new(vec);
}
```

#### Working with Bytes

```cpp
#include <span>
#include <cstddef>
#include <iostream>

struct Point {
    int x, y;
};

int main() {
    Point p = {10, 20};
    
    // View object as bytes
    std::span<std::byte> byte_view(
        reinterpret_cast<std::byte*>(&p), 
        sizeof(Point)
    );
    
    std::cout << "Size in bytes: " << byte_view.size() << "\n";
    
    // Or use as_bytes and as_writable_bytes
    Point points[] = {{1, 2}, {3, 4}};
    std::span<Point> point_span(points);
    
    auto bytes = std::as_bytes(point_span);
    // bytes is std::span<const std::byte>
    
    auto writable_bytes = std::as_writable_bytes(point_span);
    // writable_bytes is std::span<std::byte>
}
```

#### Matrix Row Views

```cpp
#include <span>
#include <iostream>

class Matrix {
    std::vector<int> data_;
    size_t rows_, cols_;
    
public:
    Matrix(size_t rows, size_t cols) 
        : data_(rows * cols), rows_(rows), cols_(cols) {}
    
    // Return a span representing a single row
    std::span<int> row(size_t r) {
        return std::span<int>(data_.data() + r * cols_, cols_);
    }
    
    std::span<const int> row(size_t r) const {
        return std::span<const int>(data_.data() + r * cols_, cols_);
    }
};

int main() {
    Matrix mat(3, 4);
    
    // Fill first row
    auto row0 = mat.row(0);
    for (size_t i = 0; i < row0.size(); ++i) {
        row0[i] = i * 10;
    }
    
    // Read second row
    auto row1 = mat.row(1);
    for (int val : row1) {
        std::cout << val << " ";
    }
}
```

### Modifying Through Spans

```cpp
#include <span>
#include <vector>
#include <iostream>

void double_values(std::span<int> data) {
    for (int& value : data) {
        value *= 2;
    }
}

void safe_read(std::span<const int> data) {
    for (int value : data) {
        // Can read but not modify
        std::cout << value << " ";
    }
}

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    double_values(vec);  // Modifies the vector
    safe_read(vec);      // 2 4 6 8 10
    
    // const span prevents modification
    const std::span<int> const_span(vec);
    // const_span[0] = 100;  // Error: can't modify through const span
}
```

### Common Pitfalls

#### Dangling Spans

```cpp
#include <span>
#include <vector>

std::span<int> create_dangling_span() {
    std::vector<int> temp = {1, 2, 3};
    return std::span<int>(temp);  // DANGER! temp is destroyed
}

int main() {
    // This span points to deallocated memory
    auto bad_span = create_dangling_span();
    // Using bad_span is undefined behavior!
    
    // Correct approach: return by value or ensure lifetime
    std::vector<int> vec = {1, 2, 3};
    std::span<int> good_span(vec);  // OK: vec outlives good_span
}
```

#### Non-Contiguous Containers

```cpp
#include <span>
#include <list>
#include <deque>

int main() {
    std::list<int> lst = {1, 2, 3};
    // std::span<int> s(lst);  // ERROR! std::list is not contiguous
    
    std::deque<int> dq = {1, 2, 3};
    // std::span<int> s2(dq);  // ERROR! std::deque is not contiguous
    
    // Only works with contiguous containers
}
```

### Performance Benefits

std::span has zero overhead compared to passing raw pointers and sizes separately. The compiler can optimize it just as well as manual pointer arithmetic, but with added safety.

```cpp
#include <span>
#include <vector>

// All these have the same performance:

void process_v1(const int* data, size_t size) {
    for (size_t i = 0; i < size; ++i) {
        // process data[i]
    }
}

void process_v2(std::span<const int> data) {
    for (size_t i = 0; i < data.size(); ++i) {
        // process data[i]
    }
}

void process_v3(std::span<const int> data) {
    for (int value : data) {
        // process value
    }
}
// All three compile to essentially identical machine code
```

### Summary: Key Points to Remember

**What it is**: Non-owning, lightweight view over contiguous sequences with automatic size tracking

**Lifetime safety**: Span doesn't own data; the underlying container must outlive the span to avoid dangling references

**Replaces pointer+size**: Use `std::span<T>` instead of `(T* data, size_t size)` pairs for cleaner, safer APIs

**Two extent types**: Static extent `std::span<T, N>` for compile-time size, dynamic extent `std::span<T>` for runtime size

**Works with**: C-style arrays, std::array, std::vector, raw pointers (with size), but NOT std::list or std::deque

**Const correctness**: Use `std::span<const T>` for read-only views, `std::span<T>` for mutable access

**Subviews**: Create partial views with `first()`, `last()`, and `subspan()` without copying data

**Zero overhead**: Optimizes to the same code as manual pointer arithmetic but with bounds safety

**Common operations**: Access via `[]`, `.front()`, `.back()`, iterate with range-for, get size with `.size()`

**Not for**: Owning data, non-contiguous containers, or situations requiring dynamic memory management

---

### Constexpr Enhancements
[Back to top](#major-features)

C++20 significantly expanded the capabilities of `constexpr`, allowing more code to be evaluated at compile-time. This enhancement makes the language more powerful and can lead to better performance by moving computations from runtime to compile-time.

### What Changed in C++20

Prior to C++20, `constexpr` functions had many restrictions. C++20 relaxed these limitations, allowing:

- **Dynamic memory allocation** (new/delete)
- **Virtual functions**
- **`try-catch` blocks** (though not throwing exceptions)
- **Trivial default initialization**
- **Changing the active member of a union**
- **`std::vector`, `std::string`** and other dynamic containers

### Dynamic Memory Allocation

One of the most significant changes is the ability to use dynamic memory allocation in `constexpr` contexts.

```cpp
#include <memory>
#include <vector>

constexpr int computeSum() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    int sum = 0;
    for (int n : numbers) {
        sum += n;
    }
    return sum;
}

int main() {
    constexpr int result = computeSum();  // Evaluated at compile-time
    static_assert(result == 15);
    return 0;
}
```

The key requirement is that any memory allocated during compile-time evaluation must be deallocated before the evaluation completes.

### Constexpr Virtual Functions

C++20 allows virtual functions to be `constexpr`, enabling polymorphism at compile-time.

```cpp
#include <iostream>

struct Base {
    constexpr virtual int getValue() const {
        return 42;
    }
    constexpr virtual ~Base() = default;
};

struct Derived : Base {
    constexpr int getValue() const override {
        return 100;
    }
};

constexpr int computeValue() {
    Derived d;
    Base* ptr = &d;
    return ptr->getValue();  // Virtual dispatch at compile-time
}

int main() {
    constexpr int val = computeValue();
    static_assert(val == 100);
    return 0;
}
```

### Try-Catch Blocks

You can now use `try-catch` in `constexpr` functions, though you still cannot throw exceptions during constant evaluation.

```cpp
constexpr int safeDivide(int a, int b) {
    try {
        if (b == 0) {
            return -1;  // Can't throw, so return error value
        }
        return a / b;
    } catch (...) {
        return -1;  // This block can exist but won't execute at compile-time
    }
}

int main() {
    constexpr int result1 = safeDivide(10, 2);  // 5
    constexpr int result2 = safeDivide(10, 0);  // -1
    static_assert(result1 == 5);
    static_assert(result2 == -1);
    return 0;
}
```

### Constexpr Destructors

Destructors can now be `constexpr`, which is essential for RAII patterns in compile-time code.

```cpp
#include <string>

struct Resource {
    std::string name;
    
    constexpr Resource(const char* n) : name(n) {}
    constexpr ~Resource() {}  // Constexpr destructor
};

constexpr int useResource() {
    Resource r("test");
    return r.name.length();
}

int main() {
    constexpr int len = useResource();
    static_assert(len == 4);
    return 0;
}
```

### Constexpr std::vector and std::string

Standard library containers like `std::vector` and `std::string` now have `constexpr` support.

```cpp
#include <vector>
#include <string>
#include <algorithm>

constexpr std::vector<int> createVector() {
    std::vector<int> vec;
    for (int i = 0; i < 10; ++i) {
        vec.push_back(i * i);
    }
    return vec;
}

constexpr int computeMax() {
    auto vec = createVector();
    return *std::max_element(vec.begin(), vec.end());
}

constexpr std::string createGreeting(const char* name) {
    std::string greeting = "Hello, ";
    greeting += name;
    greeting += "!";
    return greeting;
}

int main() {
    constexpr int maxVal = computeMax();
    static_assert(maxVal == 81);
    
    constexpr auto msg = createGreeting("World");
    static_assert(msg == "Hello, World!");
    return 0;
}
```

### Practical Example: Compile-Time Parser

Here's a more complex example showing a compile-time string parser:

```cpp
#include <string>
#include <vector>
#include <string_view>

constexpr std::vector<std::string_view> splitString(std::string_view str, char delimiter) {
    std::vector<std::string_view> result;
    size_t start = 0;
    size_t end = str.find(delimiter);
    
    while (end != std::string_view::npos) {
        result.push_back(str.substr(start, end - start));
        start = end + 1;
        end = str.find(delimiter, start);
    }
    
    result.push_back(str.substr(start));
    return result;
}

constexpr size_t countWords(std::string_view text) {
    return splitString(text, ' ').size();
}

int main() {
    constexpr auto wordCount = countWords("The quick brown fox");
    static_assert(wordCount == 4);
    
    constexpr auto parts = splitString("a,b,c,d", ',');
    static_assert(parts.size() == 4);
    static_assert(parts[2] == "c");
    return 0;
}
```

### Limitations and Considerations

While C++20 greatly expanded `constexpr`, some limitations remain:

```cpp
#include <iostream>

// Still NOT allowed in constexpr:
constexpr void cannotDoThis() {
    // Cannot perform I/O operations
    // std::cout << "Hello";  // Error!
    
    // Cannot use goto
    // goto label;  // Error!
    
    // Cannot use inline assembly
    // asm("nop");  // Error!
}

// Cannot throw exceptions during constant evaluation
constexpr int throwExample(bool condition) {
    if (condition) {
        // throw std::runtime_error("Error");  // Would compile but fail at compile-time
        return -1;
    }
    return 42;
}
```

### Consteval: Immediate Functions

C++20 also introduced `consteval`, which guarantees compile-time evaluation:

```cpp
consteval int square(int n) {
    return n * n;
}

int main() {
    constexpr int a = square(5);  // OK: compile-time
    
    int runtime_value = 5;
    // int b = square(runtime_value);  // Error! Must be compile-time
    
    return 0;
}
```

### Things to Remember

- **Dynamic allocation allowed**: `new`, `delete`, `std::vector`, `std::string` work in `constexpr` functions
- **Virtual functions supported**: Compile-time polymorphism is now possible
- **Try-catch permitted**: Exception handling syntax allowed, but cannot throw during constant evaluation
- **Destructors can be constexpr**: Enables RAII patterns at compile-time
- **Transient allocation rule**: Memory allocated during compile-time evaluation must be freed before completion
- **Use `consteval` for guaranteed compile-time**: When you need to enforce compile-time evaluation
- **Limitations remain**: No I/O, no `goto`, no inline assembly, cannot throw exceptions during evaluation
- **Standard library support**: Many STL components now support `constexpr` operations
- **Performance benefits**: Move complex computations to compile-time when possible
- **Debugging consideration**: Compile-time errors can be harder to diagnose than runtime errors

---

## Lambda Improvements
[Back to top](#major-features)

C++20 introduced several significant enhancements to lambda expressions, making them more powerful, flexible, and easier to use. Let me walk you through the key improvements with detailed examples.

### 1. Template Lambdas

C++20 allows lambdas to have explicit template parameters, giving you fine-grained control over generic lambdas.

**Before C++20 (generic lambdas):**
```cpp
auto lambda = [](auto x, auto y) {
    return x + y;
};
```

**C++20 (template lambdas):**
```cpp
auto lambda = []<typename T>(T x, T y) {
    return x + y;
};

// More complex example with multiple template parameters
auto print_pair = []<typename T, typename U>(T first, U second) {
    std::cout << first << ", " << second << '\n';
};

print_pair(42, "hello");  // Works with different types

// Using template parameter constraints
auto constrained = []<std::integral T>(T value) {
    return value * 2;
};
```

This is particularly useful when you need to use the type in the lambda body, such as with `std::vector<T>` or `sizeof(T)`.

### 2. Lambdas in Unevaluated Contexts

C++20 allows lambdas to appear in unevaluated contexts like `decltype`, `sizeof`, and `noexcept`.

```cpp
#include <map>
#include <string>

// Using lambda in decltype
auto lambda = [](int x) { return x * 2; };
using return_type = decltype(lambda(5));  // return_type is int

// Lambda as default comparator for std::map
std::map<std::string, int, 
         decltype([](const std::string& a, const std::string& b) {
             return a.size() < b.size();
         })> size_ordered_map;

size_ordered_map["hello"] = 1;
size_ordered_map["hi"] = 2;
size_ordered_map["goodbye"] = 3;
```

### 3. Lambdas with Capture of Parameter Packs

You can now capture parameter packs by value or reference directly.

```cpp
template<typename... Args>
auto make_lambda(Args... args) {
    // Capture the entire parameter pack
    return [...args = std::move(args)]() mutable {
        // Use the captured pack
        (std::cout << ... << args) << '\n';
    };
}

auto printer = make_lambda(1, 2.5, "hello", 'x');
printer();  // Prints: 12.5hellox

// More practical example: creating a tuple from captured pack
template<typename... Args>
auto capture_as_tuple(Args... args) {
    return [...args = std::move(args)] {
        return std::make_tuple(args...);
    };
}
```

### 4. `[=, this]` Deprecation Warning Fix

C++20 clarifies the capture of `this` in lambdas. Previously, `[=]` would implicitly capture `this` by reference, which was confusing.

```cpp
class MyClass {
    int value = 42;
    
public:
    auto get_lambda_old() {
        // C++17: [=] captures 'this' by reference (confusing!)
        return [=]() { return value; };  // Warning in C++20
    }
    
    auto get_lambda_explicit() {
        // C++20: Be explicit about capturing 'this'
        return [=, this]() { return value; };  // Explicit capture
    }
    
    auto get_lambda_by_value() {
        // C++20: Capture *this by value
        return [*this]() { return value; };  // Copies the object
    }
};
```

### 5. Default Constructible and Assignable Stateless Lambdas

Stateless lambdas (those without captures) are now default constructible and assignable.

```cpp
auto lambda1 = [](int x) { return x * 2; };
auto lambda2 = lambda1;  // Copy assignment works

// Default construction
decltype(lambda1) lambda3;  // Default constructed in C++20
lambda3 = lambda1;          // Assignment works

// Useful for algorithms
std::vector<int> vec = {1, 2, 3, 4, 5};
auto compare = [](int a, int b) { return a > b; };

std::sort(vec.begin(), vec.end(), compare);
std::sort(vec.begin(), vec.end(), decltype(compare){});  // Default construct
```

### 6. `consteval` and `constexpr` Lambdas

Lambdas can now be explicitly marked as `consteval` (must be compile-time) or implicitly `constexpr` when possible.

```cpp
// Implicitly constexpr if possible
auto square = [](int x) { return x * x; };
constexpr int result = square(5);  // Evaluated at compile time

// Explicitly consteval (must be compile-time)
auto compile_time_only = []<int N>() consteval {
    return N * N;
};

constexpr int val = compile_time_only.template operator()<10>();

// Example with template lambda
auto factorial = []<int N>() consteval -> int {
    if constexpr (N <= 1) return 1;
    else return N * factorial.template operator()<N-1>();
};

constexpr int fact5 = factorial.template operator()<5>();  // 120
```

### 7. Pack Expansion in Lambda Init-Capture

You can now initialize captures with pack expansions.

```cpp
template<typename... Args>
auto sum_all(Args... args) {
    return [...args = args * 2]() {
        return (args + ...);  // Fold expression
    };
}

auto doubled_sum = sum_all(1, 2, 3, 4);
std::cout << doubled_sum() << '\n';  // (2 + 4 + 6 + 8) = 20

// Another example with move semantics
template<typename... Args>
auto make_tuple_lambda(Args&&... args) {
    return [...args = std::forward<Args>(args)]() mutable {
        return std::tuple(std::move(args)...);
    };
}
```

### Practical Combined Example

Here's a real-world example combining several C++20 lambda features:

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <string>

template<typename... Filters>
auto create_filter_chain(Filters... filters) {
    return [...filters = std::move(filters)]<typename T>(const std::vector<T>& input) {
        std::vector<T> result = input;
        
        // Apply each filter in sequence
        ([&result, &filters]() {
            result.erase(
                std::remove_if(result.begin(), result.end(), 
                    [&filters](const auto& item) { 
                        return !filters(item); 
                    }),
                result.end()
            );
        }(), ...);
        
        return result;
    };
}

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    auto is_even = [](int x) { return x % 2 == 0; };
    auto greater_than_3 = [](int x) { return x > 3; };
    auto less_than_9 = [](int x) { return x < 9; };
    
    auto filter = create_filter_chain(is_even, greater_than_3, less_than_9);
    auto filtered = filter(numbers);
    
    // Result: {4, 6, 8}
    for (int n : filtered) {
        std::cout << n << ' ';
    }
}
```

### Things to Remember

**Template Lambdas**: Use `[]<typename T>()` for explicit template parameters when you need the type name in the lambda body.

**Unevaluated Contexts**: Lambdas can now be used in `decltype`, `sizeof`, and as template arguments, enabling stateless lambdas as comparators.

**Pack Captures**: Capture parameter packs with `[...args = std::move(args)]` for perfect forwarding and flexible variadic lambdas.

**Explicit `this` Capture**: Use `[=, this]` or `[*this]` to be explicit about capturing the object, avoiding the ambiguous `[=]` behavior.

**Default Constructibility**: Stateless lambdas are now default constructible and assignable, making them more usable with standard algorithms.

**Compile-Time Lambdas**: Lambdas are implicitly `constexpr` when possible, and can be explicitly `consteval` for compile-time-only evaluation.

**Improved Type Deduction**: Template lambdas give better control over generic code, especially with concepts and SFINAE.

---

## Source Location
[Back to top](#major-features)

`std::source_location` is a C++20 feature that provides a way to obtain information about the source code location where it's invoked. It captures details like file name, line number, column number, and function name at compile time, making it incredibly useful for logging, debugging, and error reporting.

### Why It Exists

Before C++20, developers relied on preprocessor macros like `__FILE__`, `__LINE__`, and `__func__` to get source location information. While functional, these macros had limitations—they couldn't be easily passed as default arguments to functions and didn't work well with templates. `std::source_location` solves these problems with a type-safe, modern C++ approach.

### Basic Usage

Here's a simple example showing how to capture and display source location information:

```cpp
#include <iostream>
#include <source_location>

void log_message(const std::string& message,
                 const std::source_location& loc = std::source_location::current()) {
    std::cout << "File: " << loc.file_name() << '\n'
              << "Line: " << loc.line() << '\n'
              << "Column: " << loc.column() << '\n'
              << "Function: " << loc.function_name() << '\n'
              << "Message: " << message << '\n';
}

int main() {
    log_message("Something happened!");
    return 0;
}
```

**Output** (approximate):
```
File: example.cpp
Line: 13
Column: 5
Function: int main()
Message: Something happened!
```

The key insight here is that `std::source_location::current()` is used as a **default argument**. This means it captures the location of the caller, not the location inside the function itself.

### Member Functions

`std::source_location` provides four main member functions:

```cpp
#include <source_location>

void demonstrate_members() {
    auto loc = std::source_location::current();
    
    // Returns the line number (unsigned integer)
    std::uint_least32_t line = loc.line();
    
    // Returns the column number (unsigned integer)
    std::uint_least32_t col = loc.column();
    
    // Returns the file name (const char*)
    const char* file = loc.file_name();
    
    // Returns the function name (const char*)
    const char* func = loc.function_name();
}
```

### Practical Example: Custom Assert

One powerful use case is creating a custom assertion macro replacement:

```cpp
#include <iostream>
#include <source_location>
#include <cstdlib>

void my_assert(bool condition, 
               const char* expression,
               const std::source_location& loc = std::source_location::current()) {
    if (!condition) {
        std::cerr << "Assertion failed: " << expression << '\n'
                  << "  at " << loc.file_name() 
                  << ":" << loc.line() << '\n'
                  << "  in function: " << loc.function_name() << '\n';
        std::abort();
    }
}

#define MY_ASSERT(expr) my_assert((expr), #expr)

int main() {
    int x = 5;
    MY_ASSERT(x > 0);  // passes
    MY_ASSERT(x < 0);  // fails with detailed location info
    return 0;
}
```

### Logger Class Example

Here's a more complete example showing a simple logger class:

```cpp
#include <iostream>
#include <source_location>
#include <string>
#include <chrono>
#include <iomanip>

class Logger {
public:
    enum class Level { INFO, WARNING, ERROR };
    
    static void log(Level level, 
                   const std::string& message,
                   const std::source_location& loc = std::source_location::current()) {
        
        // Print timestamp
        auto now = std::chrono::system_clock::now();
        auto time = std::chrono::system_clock::to_time_t(now);
        std::cout << std::put_time(std::localtime(&time), "%Y-%m-%d %H:%M:%S");
        
        // Print level
        std::cout << " [";
        switch(level) {
            case Level::INFO:    std::cout << "INFO"; break;
            case Level::WARNING: std::cout << "WARN"; break;
            case Level::ERROR:   std::cout << "ERROR"; break;
        }
        std::cout << "] ";
        
        // Print location
        std::cout << loc.file_name() << ":" << loc.line() << " ";
        
        // Print message
        std::cout << message << '\n';
    }
    
    static void info(const std::string& msg,
                    const std::source_location& loc = std::source_location::current()) {
        log(Level::INFO, msg, loc);
    }
    
    static void warning(const std::string& msg,
                       const std::source_location& loc = std::source_location::current()) {
        log(Level::WARNING, msg, loc);
    }
    
    static void error(const std::string& msg,
                     const std::source_location& loc = std::source_location::current()) {
        log(Level::ERROR, msg, loc);
    }
};

int main() {
    Logger::info("Application started");
    Logger::warning("Low memory");
    Logger::error("Connection failed");
    return 0;
}
```

### Template Function Example

`std::source_location` works seamlessly with templates:

```cpp
#include <iostream>
#include <source_location>
#include <type_traits>

template<typename T>
void debug_type(const T& value,
                const std::source_location& loc = std::source_location::current()) {
    std::cout << "Called from: " << loc.function_name() << '\n'
              << "  File: " << loc.file_name() << ":" << loc.line() << '\n'
              << "  Type: " << typeid(T).name() << '\n'
              << "  Value: " << value << '\n';
}

int main() {
    debug_type(42);
    debug_type(3.14);
    debug_type("Hello");
    return 0;
}
```

### Important Considerations

**Default Argument Position**: `std::source_location::current()` should always be the **last** default argument in a function. This ensures it captures the caller's location correctly.

```cpp
// Correct
void func(int x, const std::source_location& loc = std::source_location::current());

// Incorrect - won't capture caller location properly
void func(const std::source_location& loc = std::source_location::current(), int x = 0);
```

**Copy Semantics**: `std::source_location` objects are lightweight and cheap to copy. They contain just four pieces of information and are typically implemented as a structure of pointers or small integers.

**Compile-Time Evaluation**: The location information is captured at compile time, not runtime, so there's minimal performance overhead.

### Comparison with Preprocessor Macros

```cpp
// Old way with macros
#define LOG(msg) \
    std::cout << __FILE__ << ":" << __LINE__ << " " << msg << '\n'

// New way with std::source_location
void log(const std::string& msg,
         const std::source_location& loc = std::source_location::current()) {
    std::cout << loc.file_name() << ":" << loc.line() << " " << msg << '\n';
}

// The std::source_location approach is type-safe, 
// works with templates, and can be passed through multiple functions
```

### Things to Remember

- `std::source_location::current()` captures file name, line number, column number, and function name at compile time
- Use it as a **default argument** (and make it the last one) to capture the caller's location, not the current function's location
- Provides a type-safe alternative to `__FILE__`, `__LINE__`, `__func__`, and `__FUNCTION__` macros
- Perfect for logging, debugging, assertions, and error reporting
- Member functions: `file_name()`, `line()`, `column()`, `function_name()`
- Lightweight and cheap to copy with minimal runtime overhead
- Works seamlessly with templates and modern C++ features
- Requires C++20 and header `<source_location>`

---

## Calendar and Time Zones
[Back to top](#major-features)

C++20 introduced a comprehensive calendar and time zone library as an extension to the `<chrono>` library, bringing date handling and time zone support to the standard library for the first time. This functionality is primarily based on Howard Hinnant's `date` library.

### Core Components

#### 1. Calendar Types

C++20 provides several calendar-specific types in the `std::chrono` namespace:

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    // Individual calendar components
    year y{2024};
    month m{January};
    day d{28};
    
    // Combined date type
    year_month_day ymd{2024y/January/28d};
    
    // Alternative construction methods
    auto ymd2 = 2024y/1/28d;
    auto ymd3 = January/28d/2024y;
    auto ymd4 = 28d/January/2024y;
    
    std::cout << ymd << '\n';  // 2024-01-28
}
```

#### 2. Date Literals and Operators

C++20 introduces user-defined literals for calendar types:

```cpp
using namespace std::chrono;

auto y = 2024y;        // year literal
auto d = 15d;          // day literal
auto jan = January;    // month constant

// The division operator (/) creates calendar types
auto date1 = 2024y/March/15d;
auto date2 = March/15d/2024y;
auto date3 = 15d/March/2024y;

// Month/day without year
auto birthday = March/15d;

// Year/month without day
auto month_ref = 2024y/March;
```

#### 3. Weekday Operations

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    // Get weekday from a date
    year_month_day ymd{2024y/January/28d};
    weekday wd{sys_days{ymd}};
    std::cout << wd << '\n';  // Sun
    
    // Weekday constants
    auto monday = Monday;
    auto friday = Friday;
    
    // Indexed weekdays (e.g., "2nd Tuesday")
    auto second_tuesday = Tuesday[2];
    auto last_friday = Friday[last];
    
    // Create dates using indexed weekdays
    year_month_weekday ymw{2024y/March/Tuesday[2]};
    std::cout << year_month_day{sys_days{ymw}} << '\n';  // 2024-03-12
}
```

#### 4. System Clock and Days

The `sys_days` type represents a count of days since the epoch:

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    // Convert date to time_point
    year_month_day ymd{2024y/January/28d};
    sys_days dp{ymd};
    
    // Add duration to dates
    auto tomorrow = dp + days{1};
    std::cout << year_month_day{tomorrow} << '\n';  // 2024-01-29
    
    // Subtract dates to get duration
    auto ymd2 = 2024y/December/31d;
    auto difference = sys_days{ymd2} - dp;
    std::cout << difference.count() << " days\n";  // 338 days
}
```

#### 5. Date Validation

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    year_month_day ymd1{2024y/February/29d};  // Valid (leap year)
    year_month_day ymd2{2023y/February/29d};  // Invalid
    
    std::cout << std::boolalpha;
    std::cout << ymd1.ok() << '\n';  // true
    std::cout << ymd2.ok() << '\n';  // false
    
    // Last day of month
    year_month_day_last ymdl{2024y/February/last};
    std::cout << year_month_day{ymdl} << '\n';  // 2024-02-29
}
```

### Time Zones

#### 1. Time Zone Database

C++20 provides access to the IANA time zone database:

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    // Get a specific time zone
    auto tz = locate_zone("America/New_York");
    
    // Get current system time zone
    auto local_tz = current_zone();
    
    // Get UTC time zone
    auto utc_tz = locate_zone("UTC");
    
    std::cout << tz->name() << '\n';  // America/New_York
}
```

#### 2. Zoned Time

The `zoned_time` class represents a time point in a specific time zone:

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    // Current time in a specific time zone
    auto ny_time = zoned_time{locate_zone("America/New_York"), 
                               system_clock::now()};
    auto tokyo_time = zoned_time{locate_zone("Asia/Tokyo"), 
                                  system_clock::now()};
    
    std::cout << "New York: " << ny_time << '\n';
    std::cout << "Tokyo: " << tokyo_time << '\n';
    
    // Convert between time zones
    auto ny_tz = locate_zone("America/New_York");
    auto london_tz = locate_zone("Europe/London");
    
    auto ny_now = zoned_time{ny_tz, system_clock::now()};
    auto london_now = zoned_time{london_tz, ny_now};
    
    std::cout << "Same moment in London: " << london_now << '\n';
}
```

#### 3. Local Time

Working with local time (time without time zone information):

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    // Create a local time
    auto local_tp = local_days{2024y/January/28d} + 14h + 30min;
    
    // Associate with a time zone
    auto tz = locate_zone("America/Los_Angeles");
    auto zoned = zoned_time{tz, local_tp};
    
    std::cout << zoned << '\n';
    
    // Get the UTC equivalent
    auto utc_tp = zoned.get_sys_time();
    std::cout << "UTC: " << utc_tp << '\n';
}
```

#### 4. Time Zone Information

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    auto tz = locate_zone("America/New_York");
    auto now = system_clock::now();
    
    // Get time zone info at a specific time
    auto info = tz->get_info(now);
    
    std::cout << "Offset: " << info.offset << '\n';
    std::cout << "Abbreviation: " << info.abbrev << '\n';
    std::cout << "Is DST: " << std::boolalpha << (info.save != 0min) << '\n';
}
```

### Practical Examples

#### Example 1: Date Arithmetic

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    auto start_date = 2024y/January/1d;
    auto end_date = 2024y/December/31d;
    
    // Days between dates
    auto days_diff = sys_days{end_date} - sys_days{start_date};
    std::cout << "Days in 2024: " << days_diff.count() + 1 << '\n';
    
    // Add months
    year_month_day next_month = sys_days{start_date} + months{1};
    std::cout << "One month later: " << next_month << '\n';
    
    // Find next Monday
    auto today = 2024y/January/28d;  // Sunday
    auto today_tp = sys_days{today};
    weekday today_wd{today_tp};
    
    auto days_until_monday = (Monday - today_wd).count();
    auto next_monday = today_tp + days{days_until_monday};
    std::cout << "Next Monday: " << year_month_day{next_monday} << '\n';
}
```

#### Example 2: Meeting Scheduler Across Time Zones

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    // Schedule a meeting for 3 PM New York time
    auto ny_tz = locate_zone("America/New_York");
    auto meeting_local = local_days{2024y/March/15d} + 15h;
    auto meeting_ny = zoned_time{ny_tz, meeting_local};
    
    // What time is this in London?
    auto london_tz = locate_zone("Europe/London");
    auto meeting_london = zoned_time{london_tz, meeting_ny};
    
    // What time is this in Tokyo?
    auto tokyo_tz = locate_zone("Asia/Tokyo");
    auto meeting_tokyo = zoned_time{tokyo_tz, meeting_ny};
    
    std::cout << "Meeting times:\n";
    std::cout << "New York: " << meeting_ny << '\n';
    std::cout << "London: " << meeting_london << '\n';
    std::cout << "Tokyo: " << meeting_tokyo << '\n';
}
```

#### Example 3: Birthday Calculator

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    auto birthday = 1990y/June/15d;
    auto today = 2024y/January/28d;
    
    // Calculate age
    auto age_in_days = sys_days{today} - sys_days{birthday};
    auto age_in_years = floor<years>(age_in_days);
    
    std::cout << "Age: " << age_in_years.count() << " years\n";
    std::cout << "Age in days: " << age_in_days.count() << " days\n";
    
    // Next birthday
    auto next_birthday = year{today.year()}/birthday.month()/birthday.day();
    if (sys_days{next_birthday} < sys_days{today}) {
        next_birthday = year{today.year() + years{1}}/birthday.month()/birthday.day();
    }
    
    auto days_until = sys_days{next_birthday} - sys_days{today};
    std::cout << "Days until next birthday: " << days_until.count() << '\n';
}
```

#### Example 4: Handling Daylight Saving Time

```cpp
#include <chrono>
#include <iostream>

int main() {
    using namespace std::chrono;
    
    auto tz = locate_zone("America/New_York");
    
    // March 10, 2024, 2:30 AM - during DST transition (spring forward)
    auto local_tp = local_days{2024y/March/10d} + 2h + 30min;
    
    try {
        // This time doesn't exist (clocks jump from 2 AM to 3 AM)
        auto zoned = zoned_time{tz, local_tp};
        std::cout << zoned << '\n';
    } catch (const std::exception& e) {
        std::cout << "Time doesn't exist during DST transition\n";
    }
    
    // November 3, 2024, 1:30 AM - ambiguous (happens twice)
    auto local_tp2 = local_days{2024y/November/3d} + 1h + 30min;
    
    // Choose first occurrence
    auto zoned_first = zoned_time{tz, local_tp2, choose::earliest};
    std::cout << "First occurrence: " << zoned_first << '\n';
    
    // Choose second occurrence
    auto zoned_second = zoned_time{tz, local_tp2, choose::latest};
    std::cout << "Second occurrence: " << zoned_second << '\n';
}
```

### Key Features and Benefits

1. **Type Safety**: Calendar types prevent invalid operations at compile time
2. **Expressive Syntax**: Natural date construction with literals and operators
3. **IANA Time Zone Database**: Industry-standard time zone handling
4. **DST Support**: Automatic handling of daylight saving time transitions
5. **Leap Year Handling**: Automatic validation of dates including leap years
6. **Interoperability**: Seamless conversion between different time representations

### Common Pitfalls

1. **Ambiguous Local Times**: During DST transitions, some local times don't exist or occur twice
2. **Time Zone Names**: Use IANA names (e.g., "America/New_York") not abbreviations (e.g., "EST")
3. **Invalid Dates**: Always check `.ok()` when constructing dates programmatically
4. **Month Indexing**: Months use constants (January, February) or 1-based indexing, not 0-based

### Summary: Things to Remember

1. **Calendar Types**: Use `year`, `month`, `day`, and `year_month_day` for date representation
2. **Literals**: `2024y`, `15d` for concise date construction; `/` operator combines components
3. **sys_days**: Convert dates to/from time points for arithmetic operations
4. **Weekday Operations**: Use `weekday` for day-of-week calculations and indexed weekdays (e.g., `Tuesday[2]`)
5. **Validation**: Call `.ok()` to check if dates are valid
6. **Time Zones**: Use `locate_zone()` with IANA names, not abbreviations
7. **zoned_time**: Represents time in a specific time zone with automatic conversions
8. **DST Handling**: Use `choose::earliest` or `choose::latest` for ambiguous local times
9. **Date Arithmetic**: Add `days`, `months`, `years` to `sys_days` for calculations
10. **Current Zone**: Use `current_zone()` for system time zone, `system_clock::now()` for current time
11. **Time Zone Info**: Use `get_info()` to retrieve offset, abbreviation, and DST status
12. **local_time**: Represents time without time zone information; convert to `zoned_time` for zone awareness

---

## Additional Features
[Back to top](#major-features)

### 1. `[[likely]]` and `[[unlikely]]` Attributes

These attributes provide optimization hints to the compiler about which branch of a conditional statement is more probable to execute.

#### Purpose
- Help the compiler optimize branch prediction
- Improve CPU cache utilization
- Reduce branch misprediction penalties

#### Syntax and Examples

```cpp
#include <iostream>
#include <random>

int processValue(int value) {
    if (value > 0) [[likely]] {
        // Compiler optimizes assuming this branch executes most often
        return value * 2;
    } else [[unlikely]] {
        // Less frequently executed path
        return 0;
    }
}

// Switch statement example
void handleError(int errorCode) {
    switch (errorCode) {
        case 0: [[likely]]
            std::cout << "Success\n";
            break;
        case 1: [[unlikely]]
            std::cout << "Minor error\n";
            break;
        case 2: [[unlikely]]
            std::cout << "Critical error\n";
            break;
    }
}

// Performance-critical loop
void processData(const std::vector<int>& data) {
    for (int val : data) {
        if (val != 0) [[likely]] {
            // Normal case: process non-zero values
            std::cout << val << " ";
        } else [[unlikely]] {
            // Rare case: handle zeros specially
            std::cout << "[ZERO] ";
        }
    }
}
```

**Important Notes:**
- These are just hints; compilers may ignore them
- Incorrect hints can degrade performance
- Use profiling data to guide placement
- Don't overuse—reserve for hot paths

### 2. `constinit` Keyword

Ensures that a variable is initialized at compile-time, preventing the "static initialization order fiasco."

#### Purpose
- Guarantee compile-time initialization
- Avoid runtime initialization overhead
- Prevent initialization order bugs in static variables

#### Syntax and Examples

```cpp
#include <iostream>
#include <array>

// Compile-time constant initialization
constinit int globalCounter = 42;

// This would be an error - not compile-time constant
// constinit int invalidInit = std::rand(); // ERROR!

// Useful for thread-safe static initialization
constinit thread_local int threadId = 0;

// With constexpr functions
constexpr int computeValue() {
    return 100 * 2 + 56;
}

constinit int precomputedValue = computeValue();

// Array initialization
constinit std::array<int, 5> lookupTable = {1, 2, 4, 8, 16};

// Can be modified at runtime (unlike constexpr)
void modifyGlobal() {
    globalCounter = 100;  // This is allowed
}

// Difference from constexpr
constexpr int constexprVar = 42;  // Always constant, immutable
constinit int constinitVar = 42;  // Initialized at compile-time, but mutable

int main() {
    std::cout << "Global: " << globalCounter << "\n";
    modifyGlobal();
    std::cout << "Modified: " << globalCounter << "\n";
    
    // constexprVar = 100;  // ERROR: cannot modify
    constinitVar = 100;     // OK: can modify at runtime
}
```

**Key Differences:**
- `constexpr`: Compile-time constant, immutable
- `constinit`: Compile-time initialized, but mutable
- `const`: May initialize at runtime, immutable

### 3. `std::bit_cast` for Safe Type Punning

Provides a safe way to reinterpret the bit pattern of one type as another without undefined behavior.

#### Purpose
- Safe alternative to `reinterpret_cast` and `union` type punning
- Preserve exact bit representation
- Enable low-level optimizations without UB

#### Syntax and Examples

```cpp
#include <bit>
#include <iostream>
#include <cstdint>
#include <cstring>

// Convert float to its integer bit representation
void floatToIntBits() {
    float f = 3.14159f;
    
    // Safe way with std::bit_cast
    uint32_t bits = std::bit_cast<uint32_t>(f);
    std::cout << "Float bits: 0x" << std::hex << bits << std::dec << "\n";
    
    // Convert back
    float f2 = std::bit_cast<float>(bits);
    std::cout << "Reconstructed float: " << f2 << "\n";
}

// Custom type example
struct Vec2 {
    float x, y;
};

struct TwoFloats {
    float data[2];
};

void structConversion() {
    Vec2 vec{1.0f, 2.0f};
    
    // Reinterpret as array - requires same size
    auto arr = std::bit_cast<TwoFloats>(vec);
    std::cout << "Array: " << arr.data[0] << ", " << arr.data[1] << "\n";
}

// Endianness detection
bool isLittleEndian() {
    uint32_t value = 0x01020304;
    auto bytes = std::bit_cast<std::array<uint8_t, 4>>(value);
    return bytes[0] == 0x04;
}

// Double to uint64_t (common in serialization)
uint64_t serializeDouble(double d) {
    return std::bit_cast<uint64_t>(d);
}

double deserializeDouble(uint64_t bits) {
    return std::bit_cast<double>(bits);
}

int main() {
    floatToIntBits();
    structConversion();
    std::cout << "Little endian: " << std::boolalpha << isLittleEndian() << "\n";
    
    double d = 2.718281828;
    uint64_t serialized = serializeDouble(d);
    double restored = deserializeDouble(serialized);
    std::cout << "Serialization round-trip: " << restored << "\n";
}
```

**Requirements:**
- Source and destination types must be the same size
- Both types must be trivially copyable
- Returns by value (not a reference)

### 4. Mathematical Constants in `std::numbers`

Provides precise mathematical constants at compile-time.

#### Purpose
- Avoid magic numbers
- Maximize precision for fundamental constants
- Type-safe constants (float, double, long double)

#### Available Constants and Examples

```cpp
#include <numbers>
#include <iostream>
#include <cmath>

void demonstrateConstants() {
    using namespace std::numbers;
    
    // Fundamental constants
    std::cout << "π = " << pi << "\n";                    // 3.14159...
    std::cout << "e = " << e << "\n";                     // 2.71828...
    std::cout << "√2 = " << sqrt2 << "\n";                // 1.41421...
    std::cout << "√3 = " << sqrt3 << "\n";                // 1.73205...
    std::cout << "φ (golden ratio) = " << phi << "\n";    // 1.61803...
    
    // Logarithm constants
    std::cout << "ln(2) = " << ln2 << "\n";
    std::cout << "ln(10) = " << ln10 << "\n";
    std::cout << "log₂(e) = " << log2e << "\n";
    std::cout << "log₁₀(e) = " << log10e << "\n";
    
    // Inverse constants
    std::cout << "1/π = " << inv_pi << "\n";
    std::cout << "1/√π = " << inv_sqrtpi << "\n";
    std::cout << "2/√π = " << inv_sqrt3 << "\n";
}

// Practical applications
double circleArea(double radius) {
    return std::numbers::pi * radius * radius;
}

double circleCircumference(double radius) {
    return 2.0 * std::numbers::pi * radius;
}

// Convert degrees to radians
double degreesToRadians(double degrees) {
    return degrees * std::numbers::pi / 180.0;
}

// Compound interest calculation
double compoundInterest(double principal, double rate, int years) {
    // Using e^(rt) formula
    return principal * std::pow(std::numbers::e, rate * years);
}

// Type-specific versions
void typeSpecificConstants() {
    float pi_f = std::numbers::pi_v<float>;
    double pi_d = std::numbers::pi_v<double>;
    long double pi_ld = std::numbers::pi_v<long double>;
    
    std::cout << "Float π precision: " << pi_f << "\n";
    std::cout << "Double π precision: " << pi_d << "\n";
    std::cout << "Long double π precision: " << pi_ld << "\n";
}

int main() {
    demonstrateConstants();
    std::cout << "\nCircle with radius 5:\n";
    std::cout << "Area: " << circleArea(5.0) << "\n";
    std::cout << "Circumference: " << circleCircumference(5.0) << "\n";
    
    typeSpecificConstants();
}
```

**Available Constants:**
- `e`, `log2e`, `log10e`
- `pi`, `inv_pi`, `inv_sqrtpi`
- `ln2`, `ln10`
- `sqrt2`, `sqrt3`, `inv_sqrt3`
- `egamma` (Euler-Mascheroni constant)
- `phi` (golden ratio)

### 5. Init-Statements in Range-Based For Loops

Allows you to declare initialization statements before the range-based for loop.

#### Purpose
- Limit variable scope
- Initialize containers or iterators inline
- Cleaner, more expressive code

#### Syntax and Examples

```cpp
#include <iostream>
#include <vector>
#include <map>
#include <mutex>

void basicExample() {
    // Initialize vector in the for statement
    for (auto vec = std::vector{1, 2, 3, 4, 5}; int val : vec) {
        std::cout << val << " ";
    }
    // vec is out of scope here
}

void multipleDeclarations() {
    // Multiple initialization statements
    for (auto data = std::vector{10, 20, 30}; auto& val : data) {
        val *= 2;  // Modify in place
        std::cout << val << " ";
    }
}

// Practical: Lock acquisition
void threadSafeIteration() {
    std::mutex mtx;
    std::vector<int> sharedData = {1, 2, 3, 4, 5};
    
    // Acquire lock for the duration of the loop
    for (std::lock_guard lock(mtx); const auto& val : sharedData) {
        std::cout << val << " ";
    }
    // Lock automatically released when loop ends
}

// Computing a temporary range
void computedRange() {
    auto generateRange = [](int n) {
        std::vector<int> result;
        for (int i = 0; i < n; ++i) {
            result.push_back(i * i);
        }
        return result;
    };
    
    // Generate and iterate in one statement
    for (auto range = generateRange(5); int val : range) {
        std::cout << val << " ";
    }
}

// With structured bindings
void mapIteration() {
    for (auto map = std::map<std::string, int>{{"a", 1}, {"b", 2}}; 
         const auto& [key, value] : map) {
        std::cout << key << ": " << value << "\n";
    }
}

// Conditional iteration
void conditionalProcessing() {
    auto shouldProcess = true;
    
    for (auto data = std::vector{1, 2, 3}; 
         shouldProcess && auto val : data) {
        std::cout << val << " ";
        if (val == 2) shouldProcess = false;
    }
}

int main() {
    basicExample();
    std::cout << "\n";
    multipleDeclarations();
    std::cout << "\n";
    threadSafeIteration();
    std::cout << "\n";
    computedRange();
    std::cout << "\n";
    mapIteration();
    conditionalProcessing();
}
```

**Benefits:**
- Variables scoped to the loop only
- More compact code
- Better resource management (RAII)

### 6. Abbreviated Function Templates with `auto` Parameters

Simplifies generic function syntax by using `auto` parameters, creating implicit templates.

#### Purpose
- Shorter syntax for simple generic functions
- Easier to read and write
- Each `auto` parameter creates a separate template parameter

#### Syntax and Examples

```cpp
#include <iostream>
#include <vector>
#include <string>
#include <concepts>

// Traditional template
template<typename T>
T oldStyleMax(T a, T b) {
    return (a > b) ? a : b;
}

// Abbreviated function template (C++20)
auto newStyleMax(auto a, auto b) {
    return (a > b) ? a : b;
}

// Multiple auto parameters can be different types
auto add(auto a, auto b) {
    return a + b;
}

// With constraints (requires C++20 concepts)
auto multiply(std::integral auto a, std::integral auto b) {
    return a * b;
}

// Generic print function
void print(const auto& value) {
    std::cout << value << "\n";
}

// Works with multiple parameters of same or different types
void printPair(const auto& first, const auto& second) {
    std::cout << first << ", " << second << "\n";
}

// Can still use explicit return types
int sumToInt(auto a, auto b) {
    return static_cast<int>(a + b);
}

// Perfect forwarding still works
void forwardToFunction(auto&& arg) {
    print(std::forward<decltype(arg)>(arg));
}

// Variadic abbreviated templates
void printAll(const auto&... args) {
    ((std::cout << args << " "), ...);
    std::cout << "\n";
}

// Member functions can use auto parameters
struct Calculator {
    auto compute(auto a, auto b, auto operation) {
        return operation(a, b);
    }
};

// Lambda-like flexibility
auto processContainer(auto& container, auto func) {
    for (auto& elem : container) {
        func(elem);
    }
}

int main() {
    // Works with different types
    std::cout << newStyleMax(10, 20) << "\n";           // int
    std::cout << newStyleMax(3.14, 2.71) << "\n";       // double
    std::cout << newStyleMax('a', 'z') << "\n";         // char
    
    // Mixed types
    std::cout << add(5, 3.5) << "\n";                   // int + double
    std::cout << add(std::string("Hello"), std::string(" World")) << "\n";
    
    // Generic printing
    print(42);
    print("C++20");
    print(3.14159);
    
    printPair(1, "one");
    printPair(2.5, 'A');
    
    // Variadic
    printAll(1, 2, 3, "four", 5.5);
    
    // With functors
    Calculator calc;
    auto sum = calc.compute(10, 20, [](auto a, auto b) { return a + b; });
    std::cout << "Sum: " << sum << "\n";
    
    // Container processing
    std::vector<int> vec = {1, 2, 3, 4, 5};
    processContainer(vec, [](auto& x) { x *= 2; });
    
    for (auto val : vec) {
        std::cout << val << " ";
    }
    std::cout << "\n";
}
```

**Equivalent Template Expansion:**

```cpp
// This abbreviated function:
auto add(auto a, auto b) { return a + b; }

// Is equivalent to:
template<typename T, typename U>
auto add(T a, U b) { return a + b; }

// Each 'auto' is a separate template parameter!
```

**Best Practices:**
- Use for simple, obvious generic functions
- Prefer explicit templates for complex APIs
- Combine with concepts for constrained generics
- Great for lambdas and small helper functions

---

### Summary: Key Points to Remember

**`[[likely]]` / `[[unlikely]]`:**
- Optimization hints for branch prediction
- Place on the branch most/least likely to execute
- Use profiling data to guide placement
- Can improve performance in hot paths

**`constinit`:**
- Guarantees compile-time initialization (not runtime)
- Variable is mutable at runtime (unlike `constexpr`)
- Prevents static initialization order problems
- Useful for global/thread-local variables

**`std::bit_cast`:**
- Safe type punning without undefined behavior
- Requires same size and trivially copyable types
- Returns by value, not reference
- Use for serialization, bit manipulation, type reinterpretation

**`std::numbers`:**
- Provides high-precision mathematical constants (π, e, √2, etc.)
- Available in float, double, and long double variants
- Use `std::numbers::pi` instead of hardcoded values
- Access via `std::numbers::constant` or `std::numbers::constant_v<Type>`

**Init-statements in range-for:**
- Declare variables scoped to the loop: `for (init; range-declaration : range)`
- Useful for temporary containers, locks, and computed ranges
- Improves RAII and resource management
- Variables not accessible outside the loop

**Abbreviated function templates:**
- Use `auto` parameters for implicit templates
- Each `auto` is a separate template parameter
- Shorter syntax than traditional templates
- Combine with concepts for constraints
- Best for simple generic functions and lambdas