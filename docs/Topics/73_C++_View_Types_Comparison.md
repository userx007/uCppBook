# C++ View Types Comparison

C++ offers several "view" types that provide non-owning references to data. Here's a comprehensive comparison:

## `std::string_view` (C++17)

`std::string_view` is a lightweight, non-owning reference to a sequence of characters. It provides a read-only view into string data without copying it.

**Key characteristics:**
- Specifically designed for character sequences (strings)
- Contains a pointer and a length
- Provides string-like operations (substr, find, compare, etc.)
- Works with `const char*`, `std::string`, and string literals
- Read-only - cannot modify the underlying data

**Common use cases:** Function parameters that accept string data without caring about the source type, avoiding unnecessary string copies.

```cpp
void process(std::string_view sv) {
    // Works with std::string, const char*, or string literals
}
```

## `std::span` (C++20)

`std::span` is a non-owning view over a contiguous sequence of objects. It's the generalized version of `string_view` for any type.

**Key characteristics:**
- Works with any contiguous data type (arrays, vectors, C-arrays)
- Contains a pointer and a size
- Can be mutable (`std::span<T>`) or const (`std::span<const T>`)
- Supports subscripting and iteration
- Fixed-size (`std::span<int, 5>`) or dynamic (`std::span<int>`)

**Common use cases:** Function parameters accepting array-like data, working with subranges of containers.

```cpp
void process(std::span<int> data) {
    for (auto& val : data) {
        val *= 2; // Can modify if not const
    }
}
```

## Ranges Views (C++20)

The Ranges library introduces a powerful system of composable views through `std::ranges::view` concept.

**Key types include:**

**`std::ranges::ref_view`** - Creates a view from a lvalue range
**`std::ranges::filter_view`** - Filters elements based on a predicate
**`std::ranges::transform_view`** - Transforms elements using a function
**`std::ranges::take_view`** - Takes first N elements
**`std::ranges::drop_view`** - Skips first N elements
**`std::ranges::reverse_view`** - Reverses iteration order

These views are lazy, composable, and don't own their data:

```cpp
std::vector<int> vec = {1, 2, 3, 4, 5, 6};
auto result = vec | std::views::filter([](int n) { return n % 2 == 0; })
                  | std::views::transform([](int n) { return n * n; });
// Result: 4, 16, 36 (computed lazily)
```

## Key Comparisons

**Ownership:** None of these types own their data. They're all non-owning views that must not outlive the data they reference.

**Mutability:** `std::string_view` is always read-only. `std::span` can be mutable or const. Ranges views depend on the underlying range.

**Specialization vs Generalization:** `std::string_view` is specialized for strings with string-specific operations. `std::span` is generalized for any contiguous sequence. Ranges views are the most flexible, working with any range (not just contiguous).

**When to use what:**
- Use `std::string_view` for string parameters and string manipulation
- Use `std::span` for array-like data of any type, especially when you need contiguous memory guarantees
- Use ranges views when you need composable transformations, lazy evaluation, or working with non-contiguous ranges

**Danger zone:** All views are susceptible to dangling references. Never return a view to local data:

```cpp
// DANGEROUS!
std::string_view bad() {
    std::string local = "temporary";
    return local; // Returns view to destroyed string
}
```

The common thread is that all these types provide efficient, zero-copy access to data without taking ownership, but they require careful lifetime management to avoid dangling references.

---

# C++ Views: Comprehensive Usage Examples

## **std::string_view Examples**

1. **Function parameters** - Accepting any string type without copying
2. **Efficient parsing** - Splitting strings without allocations (the tokens are views into the original string)
3. **Compile-time operations** - Using `constexpr` with string_view
4. **Substring operations** - Creating views into portions of strings

## **std::span Examples**

1. **Generic array processing** - Works with vectors, arrays, and C-arrays
2. **Read-only spans** - Using `std::span<const T>` for immutable access
3. **Fixed-size spans** - Compile-time size checking with `std::span<T, N>`
4. **Subspan operations** - Creating views into portions of arrays (sliding windows)

## **Ranges Views Examples**

1. **Filter & Transform** - Chain operations lazily
2. **Take/Drop/Reverse** - Manipulate range boundaries and direction
3. **Complex composition** - Combine multiple view operations
4. **Enumerate pattern** - Get index and value together using `zip` and `iota`
5. **Join (flatten)** - Flatten nested ranges
6. **Split** - Split ranges by delimiter
7. **Chunk** - Group elements into fixed-size chunks
8. **Practical pipeline** - Real-world data filtering and processing

## Key Takeaways from Examples

**Lazy evaluation**: Ranges views don't compute until you iterate. The pipeline `filter | transform | take` only processes elements as needed.

**Composability**: You can chain views with the pipe operator (`|`) for readable data transformations.

**Zero-copy**: All views reference existing data rather than copying it, making them very efficient.

**Type safety**: `std::span<T, N>` provides compile-time size checking.

The code is ready to compile with C++20 (use `-std=c++20` flag). Try modifying the examples to experiment with different view combinations!

```cpp
#include <iostream>
#include <string>
#include <string_view>
#include <span>
#include <vector>
#include <array>
#include <ranges>
#include <algorithm>
#include <numeric>

// ============================================================================
// std::string_view Examples
// ============================================================================

// Example 1: Function parameters - avoid unnecessary copies
void printPrefix(std::string_view text, size_t len) {
    std::cout << "First " << len << " chars: " 
              << text.substr(0, len) << "\n";
}

// Example 2: Parsing without allocations
std::vector<std::string_view> split(std::string_view text, char delim) {
    std::vector<std::string_view> result;
    size_t start = 0;
    
    while (start < text.size()) {
        size_t end = text.find(delim, start);
        if (end == std::string_view::npos) {
            result.push_back(text.substr(start));
            break;
        }
        result.push_back(text.substr(start, end - start));
        start = end + 1;
    }
    return result;
}

// Example 3: String literal optimization
constexpr bool startsWith(std::string_view text, std::string_view prefix) {
    return text.size() >= prefix.size() && 
           text.substr(0, prefix.size()) == prefix;
}

void stringViewExamples() {
    std::cout << "=== std::string_view Examples ===\n\n";
    
    // Works with different string types
    std::string str = "Hello, World!";
    const char* cstr = "C-style string";
    
    printPrefix(str, 5);           // std::string
    printPrefix(cstr, 7);          // const char*
    printPrefix("literal", 3);     // string literal
    
    // Efficient parsing
    std::string csv = "apple,banana,cherry,date";
    auto tokens = split(csv, ',');
    std::cout << "\nTokens: ";
    for (const auto& token : tokens) {
        std::cout << "[" << token << "] ";
    }
    std::cout << "\n";
    
    // Compile-time operations
    constexpr std::string_view msg = "https://example.com";
    static_assert(startsWith(msg, "https://"));
    
    // Substring without allocation
    std::string_view sv = str;
    std::string_view sub = sv.substr(7, 5);  // "World"
    std::cout << "Substring: " << sub << "\n\n";
}

// ============================================================================
// std::span Examples
// ============================================================================

// Example 1: Function accepting array-like data
void processData(std::span<int> data) {
    std::cout << "Processing " << data.size() << " elements: ";
    for (auto& val : data) {
        val *= 2;  // Can modify elements
        std::cout << val << " ";
    }
    std::cout << "\n";
}

// Example 2: Read-only span
double average(std::span<const double> values) {
    if (values.empty()) return 0.0;
    double sum = std::accumulate(values.begin(), values.end(), 0.0);
    return sum / values.size();
}

// Example 3: Fixed-size span for known dimensions
void process3DPoint(std::span<float, 3> point) {
    std::cout << "Point: (" << point[0] << ", " 
              << point[1] << ", " << point[2] << ")\n";
}

// Example 4: Subspan operations
void analyzeWindow(std::span<const int> data, size_t windowSize) {
    for (size_t i = 0; i + windowSize <= data.size(); ++i) {
        auto window = data.subspan(i, windowSize);
        int sum = std::accumulate(window.begin(), window.end(), 0);
        std::cout << "Window " << i << " sum: " << sum << "\n";
    }
}

void spanExamples() {
    std::cout << "=== std::span Examples ===\n\n";
    
    // Works with various container types
    std::vector<int> vec = {1, 2, 3, 4, 5};
    std::array<int, 4> arr = {10, 20, 30, 40};
    int carray[] = {100, 200, 300};
    
    processData(vec);
    processData(arr);
    processData(carray);
    
    // Read-only span
    std::vector<double> scores = {85.5, 92.0, 78.5, 88.0};
    std::cout << "Average: " << average(scores) << "\n";
    
    // Fixed-size span
    float coords[] = {1.0f, 2.0f, 3.0f};
    process3DPoint(coords);
    
    // Subspan operations
    std::vector<int> series = {1, 2, 3, 4, 5, 6, 7, 8};
    std::cout << "Moving window analysis:\n";
    analyzeWindow(series, 3);
    
    std::cout << "\n";
}

// ============================================================================
// Ranges Views Examples
// ============================================================================

void rangesFilterTransform() {
    std::cout << "=== Filter & Transform ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Filter even numbers, square them
    auto result = numbers 
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; });
    
    std::cout << "Even squares: ";
    for (int n : result) {
        std::cout << n << " ";
    }
    std::cout << "\n\n";
}

void rangesTakeDropReverse() {
    std::cout << "=== Take, Drop, Reverse ===\n";
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Take first 5 elements
    auto first5 = data | std::views::take(5);
    std::cout << "First 5: ";
    for (int n : first5) std::cout << n << " ";
    std::cout << "\n";
    
    // Drop first 3 elements
    auto skip3 = data | std::views::drop(3);
    std::cout << "After dropping 3: ";
    for (int n : skip3) std::cout << n << " ";
    std::cout << "\n";
    
    // Reverse view
    auto reversed = data | std::views::reverse;
    std::cout << "Reversed: ";
    for (int n : reversed) std::cout << n << " ";
    std::cout << "\n\n";
}

void rangesComposition() {
    std::cout << "=== Complex Composition ===\n";
    
    std::vector<std::string> words = {
        "hello", "world", "cpp", "programming", "ranges", "views"
    };
    
    // Get first 4 words with length > 3, convert to uppercase
    auto result = words
        | std::views::filter([](const auto& s) { return s.length() > 3; })
        | std::views::take(4)
        | std::views::transform([](const auto& s) {
            std::string upper = s;
            std::ranges::transform(upper, upper.begin(), ::toupper);
            return upper;
        });
    
    std::cout << "Filtered & transformed: ";
    for (const auto& word : result) {
        std::cout << word << " ";
    }
    std::cout << "\n\n";
}

void rangesEnumerate() {
    std::cout << "=== Enumerate Pattern ===\n";
    
    std::vector<std::string> items = {"apple", "banana", "cherry"};
    
    // Enumerate using zip with iota
    auto indexed = std::views::zip(
        std::views::iota(0),
        items
    );
    
    for (const auto& [idx, item] : indexed) {
        std::cout << idx << ": " << item << "\n";
    }
    std::cout << "\n";
}

void rangesJoin() {
    std::cout << "=== Join (Flatten) ===\n";
    
    std::vector<std::vector<int>> nested = {
        {1, 2, 3},
        {4, 5},
        {6, 7, 8, 9}
    };
    
    // Flatten nested vectors
    auto flattened = nested | std::views::join;
    std::cout << "Flattened: ";
    for (int n : flattened) {
        std::cout << n << " ";
    }
    std::cout << "\n\n";
}

void rangesSplit() {
    std::cout << "=== Split View ===\n";
    
    std::string text = "one,two,three,four";
    
    // Split by comma
    auto parts = text | std::views::split(',');
    
    std::cout << "Parts: ";
    for (const auto& part : parts) {
        std::cout << "[";
        for (char c : part) {
            std::cout << c;
        }
        std::cout << "] ";
    }
    std::cout << "\n\n";
}

void rangesChunking() {
    std::cout << "=== Chunk View ===\n";
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9};
    
    // Process in chunks of 3
    auto chunks = data | std::views::chunk(3);
    
    int chunkNum = 1;
    for (const auto& chunk : chunks) {
        std::cout << "Chunk " << chunkNum++ << ": ";
        for (int n : chunk) {
            std::cout << n << " ";
        }
        std::cout << "\n";
    }
    std::cout << "\n";
}

void rangesPracticalExample() {
    std::cout << "=== Practical Example: Data Pipeline ===\n";
    
    // Simulate sensor readings
    std::vector<int> readings = {
        15, 102, 23, 105, 18, 200, 22, 98, 19, 150, 25, 101
    };
    
    // Find average of valid readings (20-100 range), excluding outliers
    auto validReadings = readings
        | std::views::filter([](int r) { return r >= 20 && r <= 100; });
    
    int count = std::ranges::distance(validReadings);
    int sum = std::accumulate(validReadings.begin(), validReadings.end(), 0);
    
    std::cout << "Valid readings: ";
    for (int r : validReadings) {
        std::cout << r << " ";
    }
    std::cout << "\n";
    std::cout << "Count: " << count << ", Average: " 
              << (count > 0 ? sum / count : 0) << "\n\n";
}

// ============================================================================
// Main
// ============================================================================

int main() {
    stringViewExamples();
    spanExamples();
    
    std::cout << "=== Ranges Views Examples ===\n\n";
    rangesFilterTransform();
    rangesTakeDropReverse();
    rangesComposition();
    rangesEnumerate();
    rangesJoin();
    rangesSplit();
    rangesChunking();
    rangesPracticalExample();
    
    return 0;
}
```

---

# C++ Range Views: Detailed Examples

Here are comprehensive examples demonstrating each of these range views:

## 1. `std::ranges::ref_view` - Creates a view from an lvalue range

```cpp
#include <ranges>
#include <vector>
#include <iostream>

void process_range(std::ranges::view auto v) {
    for (int n : v) {
        std::cout << n << " ";
    }
    std::cout << "\n";
}

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // ref_view allows treating an lvalue container as a view
    auto ref = std::ranges::ref_view(vec);
    
    // Can pass to functions expecting views
    process_range(ref);
    
    // Modifications through ref_view affect the original container
    for (int& n : ref) {
        n *= 2;
    }
    
    std::cout << "Original vector after modification: ";
    for (int n : vec) {
        std::cout << n << " ";  // Output: 2 4 6 8 10
    }
    std::cout << "\n";
    
    // More commonly used via std::views::all
    auto all_view = std::views::all(vec);
    std::cout << "Using views::all: ";
    process_range(all_view);
}
```

**Key points:**
- `ref_view` doesn't copy the container, it just references it
- Useful for treating existing containers as views
- The lifetime of the original container must exceed the view's usage

## 2. `std::ranges::filter_view` - Filters elements based on a predicate

```cpp
#include <ranges>
#include <vector>
#include <iostream>
#include <string>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Filter even numbers using lambda
    auto evens = numbers | std::views::filter([](int n) { return n % 2 == 0; });
    
    std::cout << "Even numbers: ";
    for (int n : evens) {
        std::cout << n << " ";  // Output: 2 4 6 8 10
    }
    std::cout << "\n";
    
    // Filter with more complex predicate
    auto greater_than_5 = numbers | std::views::filter([](int n) { return n > 5; });
    
    std::cout << "Numbers > 5: ";
    for (int n : greater_than_5) {
        std::cout << n << " ";  // Output: 6 7 8 9 10
    }
    std::cout << "\n";
    
    // Filtering strings
    std::vector<std::string> words = {"apple", "apricot", "banana", "avocado", "cherry"};
    auto a_words = words | std::views::filter([](const std::string& s) {
        return s[0] == 'a';
    });
    
    std::cout << "Words starting with 'a': ";
    for (const auto& word : a_words) {
        std::cout << word << " ";  // Output: apple apricot avocado
    }
    std::cout << "\n";
    
    // Chaining filters
    auto filtered = numbers 
        | std::views::filter([](int n) { return n > 3; })
        | std::views::filter([](int n) { return n % 2 == 0; });
    
    std::cout << "Even numbers > 3: ";
    for (int n : filtered) {
        std::cout << n << " ";  // Output: 4 6 8 10
    }
    std::cout << "\n";
}
```

**Key points:**
- Lazy evaluation: predicate is called only when iterating
- Views are composable - can chain multiple filters
- Original container is unchanged

## 3. `std::ranges::transform_view` - Transforms elements using a function

```cpp
#include <ranges>
#include <vector>
#include <iostream>
#include <string>
#include <cctype>

struct Person {
    std::string name;
    int age;
};

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // Simple transformation: square each number
    auto squared = numbers | std::views::transform([](int n) { return n * n; });
    
    std::cout << "Squared: ";
    for (int n : squared) {
        std::cout << n << " ";  // Output: 1 4 9 16 25
    }
    std::cout << "\n";
    
    // Transform to different type
    auto as_strings = numbers | std::views::transform([](int n) {
        return std::to_string(n);
    });
    
    std::cout << "As strings: ";
    for (const auto& s : as_strings) {
        std::cout << "\"" << s << "\" ";  // Output: "1" "2" "3" "4" "5"
    }
    std::cout << "\n";
    
    // Transform with member access
    std::vector<Person> people = {
        {"Alice", 30},
        {"Bob", 25},
        {"Charlie", 35}
    };
    
    auto names = people | std::views::transform(&Person::name);
    
    std::cout << "Names: ";
    for (const auto& name : names) {
        std::cout << name << " ";  // Output: Alice Bob Charlie
    }
    std::cout << "\n";
    
    // Complex transformation: uppercase strings
    std::vector<std::string> words = {"hello", "world", "cpp"};
    auto uppercase = words | std::views::transform([](const std::string& s) {
        std::string result = s;
        for (char& c : result) c = std::toupper(c);
        return result;
    });
    
    std::cout << "Uppercase: ";
    for (const auto& word : uppercase) {
        std::cout << word << " ";  // Output: HELLO WORLD CPP
    }
    std::cout << "\n";
    
    // Chaining transform and filter
    auto processed = numbers
        | std::views::transform([](int n) { return n * 2; })
        | std::views::filter([](int n) { return n > 5; });
    
    std::cout << "Doubled and > 5: ";
    for (int n : processed) {
        std::cout << n << " ";  // Output: 6 8 10
    }
    std::cout << "\n";
}
```

**Key points:**
- Can transform to different types
- Supports member pointers for direct access
- Lazy: transformation happens during iteration
- Composable with other views

## 4. `std::ranges::take_view` - Takes first N elements

```cpp
#include <ranges>
#include <vector>
#include <iostream>
#include <string>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Take first 5 elements
    auto first_five = numbers | std::views::take(5);
    
    std::cout << "First 5: ";
    for (int n : first_five) {
        std::cout << n << " ";  // Output: 1 2 3 4 5
    }
    std::cout << "\n";
    
    // Take more than available
    auto take_many = numbers | std::views::take(20);
    
    std::cout << "Take 20 (only 10 available): ";
    for (int n : take_many) {
        std::cout << n << " ";  // Output: 1 2 3 4 5 6 7 8 9 10
    }
    std::cout << "\n";
    
    // Take zero elements
    auto take_none = numbers | std::views::take(0);
    std::cout << "Take 0: ";
    for (int n : take_none) {
        std::cout << n << " ";  // Output: (nothing)
    }
    std::cout << "(empty)\n";
    
    // Combining with other views
    auto processed = numbers
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::take(3);
    
    std::cout << "First 3 even numbers: ";
    for (int n : processed) {
        std::cout << n << " ";  // Output: 2 4 6
    }
    std::cout << "\n";
    
    // Take from infinite range (iota)
    auto first_ten_squares = std::views::iota(1)
        | std::views::transform([](int n) { return n * n; })
        | std::views::take(10);
    
    std::cout << "First 10 squares: ";
    for (int n : first_ten_squares) {
        std::cout << n << " ";  // Output: 1 4 9 16 25 36 49 64 81 100
    }
    std::cout << "\n";
    
    // take_while (C++20): take elements while predicate is true
    auto take_small = numbers | std::views::take_while([](int n) { return n < 6; });
    
    std::cout << "Take while < 6: ";
    for (int n : take_small) {
        std::cout << n << " ";  // Output: 1 2 3 4 5
    }
    std::cout << "\n";
}
```

**Key points:**
- Safe: handles cases where N exceeds container size
- Essential for working with infinite ranges
- `take_while` available for predicate-based taking

## 5. `std::ranges::drop_view` - Skips first N elements

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Drop first 5 elements
    auto skip_five = numbers | std::views::drop(5);
    
    std::cout << "Drop first 5: ";
    for (int n : skip_five) {
        std::cout << n << " ";  // Output: 6 7 8 9 10
    }
    std::cout << "\n";
    
    // Drop more than available
    auto drop_many = numbers | std::views::drop(20);
    
    std::cout << "Drop 20 (only 10 available): ";
    for (int n : drop_many) {
        std::cout << n << " ";  // Output: (nothing)
    }
    std::cout << "(empty)\n";
    
    // Drop zero elements
    auto drop_none = numbers | std::views::drop(0);
    std::cout << "Drop 0: ";
    for (int n : drop_none) {
        std::cout << n << " ";  // Output: 1 2 3 4 5 6 7 8 9 10
    }
    std::cout << "\n";
    
    // Pagination: drop and take combined
    int page_size = 3;
    int page = 2;  // zero-indexed
    
    auto page_items = numbers 
        | std::views::drop(page * page_size)
        | std::views::take(page_size);
    
    std::cout << "Page 2 (size 3): ";
    for (int n : page_items) {
        std::cout << n << " ";  // Output: 7 8 9
    }
    std::cout << "\n";
    
    // drop_while: skip elements while predicate is true
    auto drop_small = numbers | std::views::drop_while([](int n) { return n <= 5; });
    
    std::cout << "Drop while <= 5: ";
    for (int n : drop_small) {
        std::cout << n << " ";  // Output: 6 7 8 9 10
    }
    std::cout << "\n";
    
    // Complex example: get middle elements
    auto middle = numbers
        | std::views::drop(3)   // skip first 3
        | std::views::take(4);  // take next 4
    
    std::cout << "Middle elements (indices 3-6): ";
    for (int n : middle) {
        std::cout << n << " ";  // Output: 4 5 6 7
    }
    std::cout << "\n";
}
```

**Key points:**
- Returns empty range if N exceeds size
- Useful for pagination with `take`
- `drop_while` available for predicate-based dropping

## 6. `std::ranges::reverse_view` - Reverses iteration order

```cpp
#include <ranges>
#include <vector>
#include <iostream>
#include <string>
#include <list>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // Basic reverse
    auto reversed = numbers | std::views::reverse;
    
    std::cout << "Reversed: ";
    for (int n : reversed) {
        std::cout << n << " ";  // Output: 5 4 3 2 1
    }
    std::cout << "\n";
    
    // Reverse strings
    std::vector<std::string> words = {"first", "second", "third"};
    auto rev_words = words | std::views::reverse;
    
    std::cout << "Reversed words: ";
    for (const auto& word : rev_words) {
        std::cout << word << " ";  // Output: third second first
    }
    std::cout << "\n";
    
    // Double reverse (back to original)
    auto double_reversed = numbers 
        | std::views::reverse 
        | std::views::reverse;
    
    std::cout << "Double reversed: ";
    for (int n : double_reversed) {
        std::cout << n << " ";  // Output: 1 2 3 4 5
    }
    std::cout << "\n";
    
    // Reverse with other views
    auto last_three_reversed = numbers
        | std::views::reverse
        | std::views::take(3);
    
    std::cout << "Last 3 in reverse: ";
    for (int n : last_three_reversed) {
        std::cout << n << " ";  // Output: 5 4 3
    }
    std::cout << "\n";
    
    // Get last N elements in original order
    auto last_three = numbers
        | std::views::reverse
        | std::views::take(3)
        | std::views::reverse;
    
    std::cout << "Last 3 in original order: ";
    for (int n : last_three) {
        std::cout << n << " ";  // Output: 3 4 5
    }
    std::cout << "\n";
    
    // Reverse filtered elements
    auto even_reversed = numbers
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::reverse;
    
    std::cout << "Even numbers reversed: ";
    for (int n : even_reversed) {
        std::cout << n << " ";  // Output: 4 2
    }
    std::cout << "\n";
    
    // Works with bidirectional ranges (e.g., list)
    std::list<int> lst = {10, 20, 30, 40};
    auto rev_list = lst | std::views::reverse;
    
    std::cout << "Reversed list: ";
    for (int n : rev_list) {
        std::cout << n << " ";  // Output: 40 30 20 10
    }
    std::cout << "\n";
    
    // Modifying through reverse view
    std::vector<int> mutable_vec = {1, 2, 3, 4, 5};
    auto mutable_rev = mutable_vec | std::views::reverse;
    
    for (int& n : mutable_rev) {
        n *= 10;
    }
    
    std::cout << "Original after modification: ";
    for (int n : mutable_vec) {
        std::cout << n << " ";  // Output: 10 20 30 40 50
    }
    std::cout << "\n";
}
```

**Key points:**
- Requires bidirectional iterators (works with `vector`, `list`, `deque`, not `forward_list`)
- Can be reversed again to get original order
- Modifications through the view affect the original container
- Composable with all other views

## Complete Example: Combining All Views

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> data = {15, 8, 23, 4, 42, 16, 11, 30, 7, 19};
    
    // Complex pipeline: 
    // 1. Filter even numbers
    // 2. Transform by squaring
    // 3. Reverse the order
    // 4. Drop first 2
    // 5. Take next 2
    auto result = data
        | std::views::filter([](int n) { return n % 2 == 0; })
        | std::views::transform([](int n) { return n * n; })
        | std::views::reverse
        | std::views::drop(2)
        | std::views::take(2);
    
    std::cout << "Complex pipeline result: ";
    for (int n : result) {
        std::cout << n << " ";
    }
    std::cout << "\n";
    
    // Breaking it down:
    // Original: {15, 8, 23, 4, 42, 16, 11, 30, 7, 19}
    // After filter (even): {8, 4, 42, 16, 30}
    // After transform (square): {64, 16, 1764, 256, 900}
    // After reverse: {900, 256, 1764, 16, 64}
    // After drop(2): {1764, 16, 64}
    // After take(2): {1764, 16}
}
```

These views demonstrate the power of C++20 ranges: lazy evaluation, composability, and zero-cost abstractions for elegant data processing pipelines!

***Note:*** These views:

**`std::ranges::ref_view`** - Creates a view from a lvalue range
**`std::ranges::filter_view`** - Filters elements based on a predicate
**`std::ranges::transform_view`** - Transforms elements using a function
**`std::ranges::take_view`** - Takes first N elements
**`std::ranges::drop_view`** - Skips first N elements
**`std::ranges::reverse_view`** - Reverses iteration order

exist, but they're typically accessed through the **`std::views`** namespace (as range adaptors), not as `*_view` class templates directly.

## How to Actually Use These Views

```cpp
#include <ranges>
#include <vector>
#include <iostream>

int main() {
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    // ❌ This is uncommon/verbose:
    // std::ranges::filter_view filtered(vec, [](int n) { return n % 2 == 0; });
    
    // ✅ This is the normal way - using std::views adaptors:
    auto filtered = vec | std::views::filter([](int n) { return n % 2 == 0; });
    auto transformed = vec | std::views::transform([](int n) { return n * 2; });
    auto taken = vec | std::views::take(3);
    auto dropped = vec | std::views::drop(2);
    auto reversed = vec | std::views::reverse;
    auto ref = vec | std::views::all;  // equivalent to ref_view
    
    // The pipe syntax (|) is the idiomatic way to use ranges
}
```

## The Relationship

- **`std::ranges::filter_view`** - The underlying class template (exists but rarely used directly)
- **`std::views::filter`** - The range adaptor you actually use (returns a `filter_view`)

The same pattern applies to all of them:

| Class Template (rarely used directly) | Range Adaptor (what you actually use) |
|---------------------------------------|--------------------------------------|
| `std::ranges::ref_view` | `std::views::all` |
| `std::ranges::filter_view` | `std::views::filter` |
| `std::ranges::transform_view` | `std::views::transform` |
| `std::ranges::take_view` | `std::views::take` |
| `std::ranges::drop_view` | `std::views::drop` |
| `std::ranges::reverse_view` | `std::views::reverse` |

