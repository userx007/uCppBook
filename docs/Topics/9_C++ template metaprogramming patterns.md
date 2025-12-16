## SFINAE (Substitution Failure Is Not An Error)

SFINAE is a core C++ principle: when the compiler substitutes template parameters and encounters an invalid type, it doesn't produce an error—it just removes that template from consideration.

**Basic Example:**

```cpp
#include <iostream>
#include <type_traits>

// This overload is only considered if T has a size() method
template<typename T>
auto getSize(const T& container) -> decltype(container.size()) {
    return container.size();
}

// This overload handles other types (like arrays)
template<typename T, size_t N>
size_t getSize(const T (&array)[N]) {
    return N;
}

int main() {
    std::vector<int> vec = {1, 2, 3};
    int arr[] = {1, 2, 3, 4};

    std::cout << getSize(vec) << "\n";  // Calls first overload: 3
    std::cout << getSize(arr) << "\n";  // Calls second overload: 4
}
```

When you pass an array to the first `getSize`, `container.size()` is invalid, so that overload is silently removed (SFINAE), and the array overload is selected.

## `std::enable_if` Pattern

`std::enable_if` leverages SFINAE to conditionally enable/disable templates based on compile-time conditions.

**Example: Different implementations for integral vs floating-point types:**

```cpp
#include <iostream>
#include <type_traits>

// Enabled only for integral types
template<typename T>
typename std::enable_if<std::is_integral<T>::value, T>::type
process(T value) {
    std::cout << "Processing integer: ";
    return value * 2;
}

// Enabled only for floating-point types
template<typename T>
typename std::enable_if<std::is_floating_point<T>::value, T>::type
process(T value) {
    std::cout << "Processing float: ";
    return value * 1.5;
}

int main() {
    std::cout << process(10) << "\n";      // Processing integer: 20
    std::cout << process(3.14) << "\n";    // Processing float: 4.71
}
```

**C++14+ cleaner syntax:**

```cpp
// Using enable_if_t (C++14)
template<typename T>
std::enable_if_t<std::is_integral_v<T>, T>  // C++17: is_integral_v
process(T value) {
    return value * 2;
}
```

**Common pattern - as template parameter:**

```cpp
template<typename T,
         typename = std::enable_if_t<std::is_arithmetic<T>::value>>
T add(T a, T b) {
    return a + b;
}
```

## Tag Dispatch

Tag dispatch uses empty "tag" types to select function overloads at compile-time, providing a cleaner alternative to `enable_if` in many cases.

**Example: Iterator category dispatch:**

```cpp
#include <iostream>
#include <iterator>
#include <vector>
#include <list>

// Tag types
struct random_access_tag {};
struct bidirectional_tag {};

// Implementation for random access iterators (fast)
template<typename Iter>
void advanceImpl(Iter& it, int n, random_access_tag) {
    std::cout << "Fast advance (random access)\n";
    it += n;  // O(1)
}

// Implementation for bidirectional iterators (slow)
template<typename Iter>
void advanceImpl(Iter& it, int n, bidirectional_tag) {
    std::cout << "Slow advance (bidirectional)\n";
    for (int i = 0; i < n; ++i) {
        ++it;  // O(n)
    }
}

// Dispatch based on iterator category
template<typename Iter>
void advance(Iter& it, int n) {
    using category = typename std::iterator_traits<Iter>::iterator_category;

    // Select tag based on iterator type
    using tag = std::conditional_t
        std::is_same_v<category, std::random_access_iterator_tag>,
        random_access_tag,
        bidirectional_tag
    >;

    advanceImpl(it, n, tag{});
}

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    auto vecIt = vec.begin();
    advance(vecIt, 3);  // Fast advance

    std::list<int> lst = {1, 2, 3, 4, 5};
    auto lstIt = lst.begin();
    advance(lstIt, 3);  // Slow advance
}
```

**Simpler example - type-based dispatch:**

```cpp
#include <iostream>
#include <string>

// Tags
struct int_tag {};
struct string_tag {};

// Overloads
void printImpl(int value, int_tag) {
    std::cout << "Integer: " << value << "\n";
}

void printImpl(const std::string& value, string_tag) {
    std::cout << "String: " << value << "\n";
}

// Dispatcher
template<typename T>
void print(const T& value) {
    using tag = std::conditional_t
        std::is_integral_v<T>,
        int_tag,
        string_tag
    >;
    printImpl(value, tag{});
}

int main() {
    print(42);           // Integer: 42
    print(std::string("Hello"));  // String: Hello
}
```

## Practical Comparison

**When to use each:**

1. **SFINAE**: Automatic overload resolution based on type properties
2. **`enable_if`**: Explicit conditional template instantiation, good for complex conditions
3. **Tag dispatch**: Clean, explicit dispatch logic; easier to debug and extend

**Real-world example combining techniques:**

```cpp
#include <iostream>
#include <type_traits>
#include <vector>

// Serialize arithmetic types
template<typename T>
std::enable_if_t<std::is_arithmetic_v<T>, std::string>
serialize(T value) {
    return std::to_string(value);
}

// Serialize containers (has begin/end)
template<typename T>
auto serialize(const T& container)
    -> decltype(container.begin(), container.end(), std::string()) {
    std::string result = "[";
    for (const auto& item : container) {
        result += serialize(item) + ",";
    }
    result.back() = ']';
    return result;
}

int main() {
    std::cout << serialize(42) << "\n";           // "42"
    std::cout << serialize(3.14) << "\n";         // "3.140000"

    std::vector<int> vec = {1, 2, 3};
    std::cout << serialize(vec) << "\n";          // "[1,2,3]"
}
```

## C++20 Concepts (Modern Alternative)

C++20 introduces concepts, which largely replace these patterns with clearer syntax:

```cpp
#include <concepts>

template<std::integral T>
T process(T value) {
    return value * 2;
}

template<std::floating_point T>
T process(T value) {
    return value * 1.5;
}
```

These techniques are fundamental for understanding template-heavy libraries (like the STL) and writing flexible, type-safe generic code!