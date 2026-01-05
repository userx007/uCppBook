# Guide on C++ iterators and iterator categories 

## Key Sections:

**1. Iterator Categories (6 types in hierarchy):**
- **Input Iterator** - Read-only, single-pass (e.g., `istream_iterator`)
- **Output Iterator** - Write-only, single-pass (e.g., `ostream_iterator`, `back_inserter`)
- **Forward Iterator** - Multi-pass, read/write (e.g., `forward_list`)
- **Bidirectional Iterator** - Can move backward (e.g., `list`, `set`, `map`)
- **Random Access Iterator** - Jump to any position (e.g., `vector`, `deque`)
- **Contiguous Iterator** (C++20) - Guaranteed memory contiguity (e.g., `vector`, `array`)

**2. Practical Examples:**
- Each category has working code demonstrating its capabilities
- Shows what operations are supported (increment, decrement, arithmetic, subscript)
- Demonstrates real-world usage with STL containers

**3. Advanced Topics:**
- Iterator traits for compile-time type information
- Standard iterator operations (`advance`, `distance`, `next`, `prev`)
- Custom iterator implementation example
- Iterator invalidation rules for different containers

**4. Best Practices:**
- When to use `const_iterator`
- Caching `end()` iterators
- Avoiding iterator invalidation
- Preferring algorithms over manual iteration

The code is fully compilable and demonstrates the progression of capabilities from weaker to stronger iterator categories. Each iterator type builds on the previous one's functionality!

```cpp
/*
================================================================================
                C++ ITERATORS & ITERATOR CATEGORIES
                     Exhaustive Guide with Examples
================================================================================

TABLE OF CONTENTS:
1. Introduction to Iterators
2. Iterator Categories (6 types)
3. Iterator Traits
4. Operations & Algorithms
5. Custom Iterators
6. Best Practices

================================================================================
1. INTRODUCTION TO ITERATORS
================================================================================

Iterators are objects that point to elements in a container and allow
traversal through the container. They act as a generalization of pointers
and provide a uniform interface for accessing container elements.

Key Concepts:
- Iterators decouple algorithms from containers
- They provide abstraction over different data structures
- Different iterator categories support different operations
*/

#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <set>
#include <iterator>
#include <algorithm>

// Basic iterator usage
void basic_iterator_example() {
    std::vector<int> vec = {1, 2, 3, 4, 5};

    // Using iterators to traverse
    std::cout << "Forward traversal: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // Reverse traversal
    std::cout << "Reverse traversal: ";
    for (auto it = vec.rbegin(); it != vec.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}

/*
================================================================================
2. ITERATOR CATEGORIES
================================================================================

C++ defines 6 iterator categories in a hierarchy (C++20):

    Input Iterator
         ↓
    Forward Iterator
         ↓
    Bidirectional Iterator
         ↓
    Random Access Iterator
         ↓
    Contiguous Iterator (C++20)

Plus: Output Iterator (separate branch)

Each category supports specific operations and has different guarantees.
*/

// ============================================================================
// 2.1 INPUT ITERATOR
// ============================================================================
/*
Input Iterators:
- Read-only, single-pass
- Can be incremented (++)
- Can be dereferenced for reading (*it)
- Can be compared for equality (==, !=)
- Examples: std::istream_iterator

Operations supported:
- *it (read)
- ++it, it++ (increment)
- it1 == it2, it1 != it2 (comparison)
*/

void input_iterator_example() {
    std::cout << "\n=== INPUT ITERATOR ===\n";

    // Reading from input stream (conceptual example)
    // std::istream_iterator<int> input(std::cin);
    // std::istream_iterator<int> eof;
    //
    // while (input != eof) {
    //     std::cout << *input << " ";
    //     ++input;  // Single pass only!
    // }

    // Practical example with find_if
    std::vector<int> data = {1, 2, 3, 4, 5};
    auto it = std::find_if(data.begin(), data.end(),
                          [](int x) { return x > 3; });
    if (it != data.end()) {
        std::cout << "Found: " << *it << "\n";
    }
}

// ============================================================================
// 2.2 OUTPUT ITERATOR
// ============================================================================
/*
Output Iterators:
- Write-only, single-pass
- Can be incremented (++)
- Can be dereferenced for writing (*it = value)
- Examples: std::ostream_iterator, std::back_inserter

Operations supported:
- *it = value (write)
- ++it, it++ (increment)
*/

void output_iterator_example() {
    std::cout << "\n=== OUTPUT ITERATOR ===\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};

    // Using ostream_iterator
    std::cout << "Using ostream_iterator: ";
    std::copy(vec.begin(), vec.end(),
              std::ostream_iterator<int>(std::cout, " "));
    std::cout << "\n";

    // Using back_inserter
    std::vector<int> dest;
    std::copy(vec.begin(), vec.end(), std::back_inserter(dest));
    std::cout << "Copied with back_inserter: ";
    for (int x : dest) std::cout << x << " ";
    std::cout << "\n";
}

// ============================================================================
// 2.3 FORWARD ITERATOR
// ============================================================================
/*
Forward Iterators:
- Multi-pass (can traverse multiple times)
- Can read and write (if not const)
- Can be incremented (++)
- Can be compared for equality
- Examples: std::forward_list iterators, std::unordered_set iterators

Operations supported:
- All Input Iterator operations
- Multi-pass guarantee
- Default constructible
*/

void forward_iterator_example() {
    std::cout << "\n=== FORWARD ITERATOR ===\n";

    std::forward_list<int> flist = {1, 2, 3, 4, 5};

    // Multi-pass traversal
    auto it1 = flist.begin();
    auto it2 = flist.begin();

    ++it1;
    std::cout << "it1: " << *it1 << ", it2: " << *it2 << "\n";

    // Can traverse again
    std::cout << "Forward list: ";
    for (auto it = flist.begin(); it != flist.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}

// ============================================================================
// 2.4 BIDIRECTIONAL ITERATOR
// ============================================================================
/*
Bidirectional Iterators:
- All Forward Iterator operations
- Can move backward (--)
- Examples: std::list, std::set, std::map iterators

Operations supported:
- All Forward Iterator operations
- --it, it-- (decrement)
*/

void bidirectional_iterator_example() {
    std::cout << "\n=== BIDIRECTIONAL ITERATOR ===\n";

    std::list<int> lst = {1, 2, 3, 4, 5};

    // Forward traversal
    std::cout << "Forward: ";
    for (auto it = lst.begin(); it != lst.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // Backward traversal
    std::cout << "Backward: ";
    for (auto it = lst.end(); it != lst.begin(); ) {
        --it;  // Decrement before use (end() points past last element)
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // Using reverse_iterator
    std::cout << "Using rbegin/rend: ";
    for (auto it = lst.rbegin(); it != lst.rend(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}

// ============================================================================
// 2.5 RANDOM ACCESS ITERATOR
// ============================================================================
/*
Random Access Iterators:
- All Bidirectional Iterator operations
- Can jump to arbitrary positions (it + n, it - n)
- Can calculate distance (it2 - it1)
- Can compare with <, >, <=, >=
- Supports subscript operator (it[n])
- Examples: std::vector, std::deque, std::array, raw pointers

Operations supported:
- All Bidirectional Iterator operations
- it + n, n + it, it - n (arithmetic)
- it += n, it -= n (compound assignment)
- it1 - it2 (distance)
- it[n] (subscript)
- it1 < it2, it1 > it2, it1 <= it2, it1 >= it2 (relational)
*/

void random_access_iterator_example() {
    std::cout << "\n=== RANDOM ACCESS ITERATOR ===\n";

    std::vector<int> vec = {10, 20, 30, 40, 50, 60, 70, 80, 90, 100};

    auto it = vec.begin();

    // Jump forward
    it += 5;
    std::cout << "After += 5: " << *it << "\n";

    // Jump backward
    it -= 2;
    std::cout << "After -= 2: " << *it << "\n";

    // Arithmetic operations
    auto it2 = vec.begin() + 7;
    std::cout << "begin() + 7: " << *it2 << "\n";

    // Calculate distance
    std::cout << "Distance: " << (it2 - it) << "\n";

    // Subscript operator
    std::cout << "it[2]: " << it[2] << "\n";

    // Relational operators
    std::cout << "it < it2: " << (it < it2) << "\n";

    // Efficient binary search (requires random access)
    auto found = std::binary_search(vec.begin(), vec.end(), 50);
    std::cout << "Binary search for 50: " << (found ? "Found" : "Not found") << "\n";
}

// ============================================================================
// 2.6 CONTIGUOUS ITERATOR (C++20)
// ============================================================================
/*
Contiguous Iterators:
- All Random Access Iterator operations
- Guarantee that elements are stored contiguously in memory
- Pointer arithmetic is valid
- Examples: std::vector, std::array, std::string, raw pointers

This is the strongest iterator category.
*/

void contiguous_iterator_example() {
    std::cout << "\n=== CONTIGUOUS ITERATOR (C++20) ===\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};

    // Elements are guaranteed to be contiguous
    auto it = vec.begin();
    int* ptr = &(*it);

    std::cout << "Elements via pointer arithmetic: ";
    for (size_t i = 0; i < vec.size(); ++i) {
        std::cout << ptr[i] << " ";
    }
    std::cout << "\n";

    // This guarantee allows optimizations
    // Can pass &(*vec.begin()) to C APIs
    std::cout << "Address continuity: ";
    std::cout << "Address of vec[0]: " << &vec[0] << ", ";
    std::cout << "Address of vec[1]: " << &vec[1] << "\n";
}

/*
================================================================================
3. ITERATOR TRAITS
================================================================================

Iterator traits provide information about iterator properties.
*/

#include <type_traits>

template<typename Iterator>
void print_iterator_category() {
    using category = typename std::iterator_traits<Iterator>::iterator_category;

    std::cout << "Iterator category: ";
    if (std::is_same_v<category, std::input_iterator_tag>) {
        std::cout << "Input Iterator\n";
    } else if (std::is_same_v<category, std::output_iterator_tag>) {
        std::cout << "Output Iterator\n";
    } else if (std::is_same_v<category, std::forward_iterator_tag>) {
        std::cout << "Forward Iterator\n";
    } else if (std::is_same_v<category, std::bidirectional_iterator_tag>) {
        std::cout << "Bidirectional Iterator\n";
    } else if (std::is_same_v<category, std::random_access_iterator_tag>) {
        std::cout << "Random Access Iterator\n";
    }
}

void iterator_traits_example() {
    std::cout << "\n=== ITERATOR TRAITS ===\n";

    std::vector<int> vec;
    std::list<int> lst;
    std::forward_list<int> flst;

    std::cout << "std::vector: ";
    print_iterator_category<decltype(vec.begin())>();

    std::cout << "std::list: ";
    print_iterator_category<decltype(lst.begin())>();

    std::cout << "std::forward_list: ";
    print_iterator_category<decltype(flst.begin())>();
}

/*
================================================================================
4. ITERATOR OPERATIONS & ALGORITHMS
================================================================================
*/

void iterator_operations_example() {
    std::cout << "\n=== ITERATOR OPERATIONS ===\n";

    std::vector<int> vec = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

    // std::advance - moves iterator by n positions
    auto it = vec.begin();
    std::advance(it, 5);
    std::cout << "After advance(5): " << *it << "\n";

    // std::distance - calculates distance between iterators
    auto dist = std::distance(vec.begin(), it);
    std::cout << "Distance from begin: " << dist << "\n";

    // std::next - returns iterator n positions ahead
    auto next_it = std::next(vec.begin(), 3);
    std::cout << "std::next(begin, 3): " << *next_it << "\n";

    // std::prev - returns iterator n positions before
    auto prev_it = std::prev(vec.end(), 2);
    std::cout << "std::prev(end, 2): " << *prev_it << "\n";
}

/*
================================================================================
5. CUSTOM ITERATORS
================================================================================
*/

// Example: Simple range iterator
template<typename T>
class RangeIterator {
private:
    T current;
    T step;

public:
    // Iterator traits - required for STL compatibility
    using iterator_category = std::input_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;

    RangeIterator(T start, T step = 1) : current(start), step(step) {}

    T operator*() const { return current; }

    RangeIterator& operator++() {
        current += step;
        return *this;
    }

    RangeIterator operator++(int) {
        RangeIterator tmp = *this;
        ++(*this);
        return tmp;
    }

    bool operator==(const RangeIterator& other) const {
        return current == other.current;
    }

    bool operator!=(const RangeIterator& other) const {
        return !(*this == other);
    }
};

// Helper class for range-based for
template<typename T>
class Range {
private:
    T start_val, end_val, step_val;

public:
    Range(T start, T end, T step = 1)
        : start_val(start), end_val(end), step_val(step) {}

    RangeIterator<T> begin() const { return RangeIterator<T>(start_val, step_val); }
    RangeIterator<T> end() const { return RangeIterator<T>(end_val, step_val); }
};

void custom_iterator_example() {
    std::cout << "\n=== CUSTOM ITERATOR ===\n";

    std::cout << "Range(0, 10, 2): ";
    for (int i : Range<int>(0, 10, 2)) {
        std::cout << i << " ";
    }
    std::cout << "\n";
}

/*
================================================================================
6. BEST PRACTICES & COMMON PATTERNS
================================================================================
*/

void best_practices_example() {
    std::cout << "\n=== BEST PRACTICES ===\n";

    std::vector<int> vec = {1, 2, 3, 4, 5};

    // 1. Prefer const_iterator when not modifying
    for (auto it = vec.cbegin(); it != vec.cend(); ++it) {
        // *it = 10;  // Error: can't modify through const_iterator
    }

    // 2. Use range-based for when possible
    std::cout << "Range-based for: ";
    for (const auto& val : vec) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    // 3. Cache end() in loops for non-constant containers
    auto end_it = vec.end();
    for (auto it = vec.begin(); it != end_it; ++it) {
        // Process
    }

    // 4. Use algorithms instead of manual iteration
    auto pos = std::find(vec.begin(), vec.end(), 3);
    if (pos != vec.end()) {
        std::cout << "Found 3 at position: "
                  << std::distance(vec.begin(), pos) << "\n";
    }

    // 5. Be careful with iterator invalidation
    std::vector<int> v = {1, 2, 3, 4, 5};
    // auto it = v.begin();
    // v.push_back(6);  // May invalidate iterators!
    // std::cout << *it;  // Undefined behavior
}

/*
================================================================================
ITERATOR INVALIDATION RULES
================================================================================

VECTOR:
- Insert/erase: all iterators at or after modification point
- push_back: all if reallocation occurs
- reserve: all if reallocation occurs

DEQUE:
- Insert/erase in middle: all iterators
- Insert/erase at ends: only end iterators

LIST/FORWARD_LIST:
- Insert: no invalidation
- Erase: only to erased elements

SET/MAP/MULTISET/MULTIMAP:
- Insert: no invalidation
- Erase: only to erased elements

UNORDERED CONTAINERS:
- Insert: all if rehashing occurs
- Erase: only to erased elements
*/

/*
================================================================================
SUMMARY TABLE
================================================================================

Category            Read  Write Multi Forward Backward Random Examples
                          pass
-----------------------------------------------------------------------------
Input               Yes   No    No    Yes     No       No     istream_iterator
Output              No    Yes   No    Yes     No       No     ostream_iterator
Forward             Yes   Yes   Yes   Yes     No       No     forward_list
Bidirectional       Yes   Yes   Yes   Yes     Yes      No     list, set, map
Random Access       Yes   Yes   Yes   Yes     Yes      Yes    vector, deque
Contiguous          Yes   Yes   Yes   Yes     Yes      Yes    vector, array
                                                              (+ memory guarantee)
*/

int main() {
    basic_iterator_example();
    input_iterator_example();
    output_iterator_example();
    forward_iterator_example();
    bidirectional_iterator_example();
    random_access_iterator_example();
    contiguous_iterator_example();
    iterator_traits_example();
    iterator_operations_example();
    custom_iterator_example();
    best_practices_example();

    return 0;
}

/*
================================================================================
ADDITIONAL NOTES
================================================================================

C++20 Concepts:
- std::input_iterator
- std::output_iterator
- std::forward_iterator
- std::bidirectional_iterator
- std::random_access_iterator
- std::contiguous_iterator

These provide compile-time checks for iterator requirements.

Iterator Adaptors:
- std::reverse_iterator - reverses iteration direction
- std::move_iterator - moves instead of copies
- std::insert_iterator - inserts at position
- std::back_insert_iterator - pushes to back
- std::front_insert_iterator - pushes to front

Common Pitfalls:
1. Iterator invalidation after container modification
2. Comparing iterators from different containers
3. Using == instead of != in loops (portability)
4. Dereferencing end() iterator
5. Off-by-one errors with end()
*/
```

---

### 1. `auto it` just **deduces the type**

When you write:

```cpp
std::vector<int> vec = {1, 2, 3, 4};
auto it = vec.begin();
```

The compiler **deduces `it` as the exact iterator type returned by `vec.begin()`**, which in this case is `std::vector<int>::iterator`. The compiler does **not** look at how you use `it` later to “decide” what kind of iterator to make. The iterator type is **fixed by the container**:

| Container          | Iterator type         |
| ------------------ | --------------------- |
| `vector`           | RandomAccessIterator  |
| `list`             | BidirectionalIterator |
| `forward_list`     | ForwardIterator       |
| `istream_iterator` | InputIterator         |
| `ostream_iterator` | OutputIterator        |

---

### 2. What `auto` actually does

`auto` is just **type deduction**. It’s equivalent to:

```cpp
std::vector<int>::iterator it = vec.begin();
```

The iterator type is **already determined by the container**, not by how you write `++it` or `--it`.

So, the compiler **doesn’t detect your operations** to upgrade the iterator. If you try:

```cpp
std::forward_list<int> fl = {1,2,3};
auto it = fl.begin();
--it; // ERROR!
```

You get a **compile-time error**, because `std::forward_list` iterators are **forward iterators** and do **not support `--`**. The compiler doesn’t “magically convert” it to a bidirectional iterator.

---

### 3. How C++ algorithms use iterators

C++ algorithms (like `std::sort`, `std::copy`) often require certain iterator **capabilities**:

* `std::sort` → requires **RandomAccessIterator**.
* `std::reverse` → requires **BidirectionalIterator**.
* `std::copy` → requires **InputIterator** for source, **OutputIterator** for destination.

If you pass an iterator that doesn’t support what the algorithm needs, you’ll get a **compile-time error**.

Example:

```cpp
std::forward_list<int> fl = {3,1,2};
std::reverse(fl.begin(), fl.end()); // ERROR: forward iterator cannot go backwards
```

---

### ✅ Key points

1. `auto` **deduces the iterator type from the container**, not from how you use it.
2. The container determines the **iterator category**.
3. Using operations not supported by that iterator type is a **compile-time error**.
4. Algorithms specify **requirements** on the iterator type; if the iterator doesn’t meet the requirement, compilation fails.

---

Perfect! Let’s make a **clear diagram of C++ iterator types**, the containers that provide them, and what operations are allowed. I’ll make it structured like a table for clarity.

---

### **C++ Iterator Types Overview**

| Iterator Type                   | Containers / Examples                       | Supported Operations                                        | Notes                                                                               |
| ------------------------------- | ------------------------------------------- | ----------------------------------------------------------- | ----------------------------------------------------------------------------------- |
| **Input Iterator**              | `istream_iterator<T>`                       | `++it`, `*it` (read)                                        | Single-pass only; read-only. Cannot go backward or jump.                            |
| **Output Iterator**             | `ostream_iterator<T>`, `back_inserter(vec)` | `++it`, `*it = value` (write)                               | Single-pass only; write-only. Cannot read or go backward.                           |
| **Forward Iterator**            | `forward_list<T>`, `unordered_map/set`      | `++it`, `*it` (read/write)                                  | Multi-pass; can traverse multiple times forward. Cannot go backward.                |
| **Bidirectional Iterator**      | `list<T>`, `set<T>`, `map<K,V>`             | `++it`, `--it`, `*it`                                       | Can move forward/backward; supports multi-pass traversal.                           |
| **Random Access Iterator**      | `vector<T>`, `deque<T>`, `array<T,N>`       | `++it`, `--it`, `it + n`, `it - n`, `it[n]`, `<, >, <=, >=` | Jump to any position; supports all arithmetic operations.                           |
| **Contiguous Iterator** (C++20) | `vector<T>`, `array<T,N>`                   | All Random Access operations + guaranteed contiguous memory | Allows pointer-like operations (`std::to_address`, interoperability with C arrays). |

---

### **Visual Hierarchy of Iterator Capabilities**

```
Input/Output Iterator
       ↑
   Forward Iterator
       ↑
Bidirectional Iterator
       ↑
Random Access Iterator
       ↑
Contiguous Iterator (C++20)
```

* Each type **inherits capabilities** of the types below it.
* Algorithms specify **minimum required iterator type**, e.g., `std::sort` requires Random Access; `std::reverse` requires Bidirectional.

---

### **Key Takeaways**

* `auto it = container.begin();` → type is **fixed by the container**.
* The compiler **does not choose an iterator type based on your code usage**.
* Trying unsupported operations (like `--` on a forward iterator) → **compile-time error**.

---

Great! Here's a **cheat sheet mapping common STL containers to their iterator types**. This is extremely handy when you want to know which operations are allowed.

---

### **STL Containers → Iterator Types**

| Container                                          | Iterator Type              | Notes / Allowed Operations               |
| -------------------------------------------------- | -------------------------- | ---------------------------------------- |
| `vector<T>`                                        | Random Access / Contiguous | `++it, --it, it + n, it - n, it[n]`      |
| `deque<T>`                                         | Random Access              | `++it, --it, it + n, it - n, it[n]`      |
| `array<T, N>`                                      | Random Access / Contiguous | Same as vector; fixed size               |
| `list<T>`                                          | Bidirectional              | `++it, --it` only; cannot jump randomly  |
| `forward_list<T>`                                  | Forward                    | `++it` only; single direction            |
| `set<T>`, `map<K,V>`                               | Bidirectional              | `++it, --it`; sorted, unique keys        |
| `multiset<T>`, `multimap<K,V>`                     | Bidirectional              | Same as set/map, allows duplicates       |
| `unordered_set<T>`, `unordered_map<K,V>`           | Forward                    | `++it` only; unordered hash-based        |
| `unordered_multiset<T>`, `unordered_multimap<K,V>` | Forward                    | Same as above                            |
| `istream_iterator<T>`                              | Input                      | Read-only, single-pass from stream       |
| `ostream_iterator<T>`                              | Output                     | Write-only, single-pass to stream        |
| `back_insert_iterator<Container>`                  | Output                     | Write-only, pushes to container          |
| `front_insert_iterator<Container>`                 | Output                     | Write-only, pushes to front              |
| `insert_iterator<Container>`                       | Output                     | Write-only, inserts at specific position |

---

### **Quick Rules**

1. **Forward-only containers** → no `--` or random access.
2. **Bidirectional containers** → can go forward/backward, but no random jump.
3. **Random-access containers** → can jump anywhere, index with `[]`.
4. **Contiguous iterators** → memory layout like a C array (vector, array).

---

### 1. **Iterator category is a type trait**

Every iterator in C++ has a **category tag**:

```cpp
std::vector<int>::iterator it;
using Cat = std::iterator_traits<decltype(it)>::iterator_category;
```

For `std::vector<T>`:

* `iterator_category` is `std::random_access_iterator_tag`.
* And in C++20, it’s also considered **contiguous**.

For `std::list<T>`:

* `iterator_category` is `std::bidirectional_iterator_tag`.

---

### 2. **Difference between Bidirectional and Random/Contiguous**

| Feature                     | Bidirectional | Random Access / Contiguous |
| --------------------------- | ------------- | -------------------------- |
| `++it`                      | ✅             | ✅                          |
| `--it`                      | ✅             | ✅                          |
| `it + n` / `it - n`         | ❌             | ✅                          |
| `it[n]`                     | ❌             | ✅                          |
| Memory contiguity           | ❌             | ✅ (vector/array)           |
| Pointer interop (`&vec[0]`) | ❌             | ✅                          |

So the **difference isn’t “detected at runtime”**, it’s entirely **encoded in the iterator type**:

* `vector<T>::iterator` supports `operator+`, `operator-`, `operator[]`.
* `list<T>::iterator` only supports `++` and `--`.

---

### 3. **Why `vector` can be contiguous**

Since `vector` stores elements in contiguous memory:

```cpp
int* p = &vec[0];  // guaranteed valid
```

C++20 formalized this with the **contiguous iterator tag**, which is just a subclass of `random_access_iterator_tag`. That allows:

* Interop with raw pointers
* Algorithms expecting contiguous memory (like `std::memcpy` optimizations)

---

### 4. **No runtime detection**

* The compiler **knows the exact type** from `vec.begin()`.
* It **does not check what operations you actually call** to “upgrade” the iterator.
* All the operations that are allowed **depend on the iterator type**. If you try something illegal (like `vec.begin()--` for `forward_list`) → compile-time error.

---

✅ **Summary:**

* Bidirectional vs contiguous (or random access) is **encoded in the iterator type**.
* `vector` iterators are random access and contiguous because the container supports arithmetic, indexing, and contiguous memory layout.
* `list` iterators are bidirectional because you can go forward/backward but can’t jump or index.

---

### 1. **Iterator tags in C++20**

C++20 introduced **`std::contiguous_iterator_tag`**

```cpp
#include <iterator>

std::vector<int>::iterator it;
using Cat = std::iterator_traits<decltype(it)>::iterator_category;
```

For `std::vector<T>::iterator`:

* `Cat` is `std::contiguous_iterator_tag`.
* `std::contiguous_iterator_tag` **inherits from** `std::random_access_iterator_tag`.
* That means all algorithms expecting RandomAccessIterator will still work.

---

### 2. **Key points**

| Feature                  | Bidirectional | Random Access | Contiguous |
| ------------------------ | ------------- | ------------- | ---------- |
| `++it`                   | ✅             | ✅             | ✅          |
| `--it`                   | ✅             | ✅             | ✅          |
| `it + n` / `it - n`      | ❌             | ✅             | ✅          |
| `it[n]`                  | ❌             | ✅             | ✅          |
| Memory contiguous?       | ❌             | ❌             | ✅          |
| Pointer interop (`&*it`) | ❌             | ❌             | ✅          |

So **`std::contiguous_iterator_tag` is the C++20 way to mark iterators that are guaranteed to be contiguous** (like `vector` and `array`).

---

### 3. **Why it matters**

* Algorithms that rely on pointer-like operations (like `std::to_address`) can detect this tag and optimize accordingly.
* You can still use `std::vector` iterators wherever a RandomAccessIterator is required.

---

### 1. **Iterator category typedef vs `contiguous_iterator_tag`**

Your code:

```cpp
using category = typename std::iterator_traits<Iterator>::iterator_category;
```

`std::vector<T>::iterator` **actually has `std::random_access_iterator_tag` as its `iterator_category`**, **even in C++20**.

* The **C++20 contiguous iterator support** is implemented via a **separate concept** called `std::contiguous_iterator`, **not by changing `iterator_category`**.
* `iterator_category` is still `random_access_iterator_tag` for backward compatibility.

---

### 2. **How contiguous iterators are identified in C++20**

C++20 introduces:

```cpp
template<class I>
concept contiguous_iterator =
    std::random_access_iterator<I> &&
    std::is_pointer_v<std::to_address(I)>;
```

So the standard way to check if an iterator is contiguous is **using concepts**, not `iterator_category`. For example:

```cpp
#include <vector>
#include <iterator>
#include <concepts>
#include <iostream>

int main() {
    std::vector<int> vec;

    auto it = vec.begin();

    if constexpr (std::contiguous_iterator<decltype(it)>) {
        std::cout << "Contiguous iterator\n";
    } else {
        std::cout << "Not contiguous\n";
    }
}
```

Output:

```
Contiguous iterator
```

---

### 3. **Why your function prints Random Access**

* `std::iterator_traits<Iterator>::iterator_category` is **never updated to `contiguous_iterator_tag`** in C++20.
* All contiguous iterators (vector, array) **still report RandomAccessIterator**.
* To detect contiguity, **use C++20 concepts** (`std::contiguous_iterator`) instead of `iterator_category`.

---

✅ **Takeaway**

* `iterator_category` → **legacy trait**, unchanged for contiguous iterators.
* `std::contiguous_iterator` → **modern C++20 concept**, use this for detecting contiguous memory iterators.

---

Here's a **C++20 version of your function** that correctly detects **contiguous iterators** using the `std::contiguous_iterator` concept while keeping the older categories:

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <iterator>
#include <concepts>
#include <type_traits>

template<typename Iterator>
void print_iterator_category() {
    if constexpr (std::contiguous_iterator<Iterator>) {
        std::cout << "Contiguous Iterator\n";
    } else {
        using category = typename std::iterator_traits<Iterator>::iterator_category;

        if constexpr (std::is_same_v<category, std::input_iterator_tag>) {
            std::cout << "Input Iterator\n";
        } else if constexpr (std::is_same_v<category, std::output_iterator_tag>) {
            std::cout << "Output Iterator\n";
        } else if constexpr (std::is_same_v<category, std::forward_iterator_tag>) {
            std::cout << "Forward Iterator\n";
        } else if constexpr (std::is_same_v<category, std::bidirectional_iterator_tag>) {
            std::cout << "Bidirectional Iterator\n";
        } else if constexpr (std::is_same_v<category, std::random_access_iterator_tag>) {
            std::cout << "Random Access Iterator\n";
        } else {
            std::cout << "Unknown Iterator Type\n";
        }
    }
}

int main() {
    std::vector<int> vec;
    std::list<int> lst;
    std::forward_list<int> fl;

    std::cout << "vector: ";
    print_iterator_category<decltype(vec.begin())>();

    std::cout << "list: ";
    print_iterator_category<decltype(lst.begin())>();

    std::cout << "forward_list: ";
    print_iterator_category<decltype(fl.begin())>();
}
```

### ✅ Output (C++20)

```
vector: Contiguous Iterator
list: Bidirectional Iterator
forward_list: Forward Iterator
```

---

### Explanation

1. **`std::contiguous_iterator<Iterator>`** checks both:

   * Random access capability
   * Memory is contiguous (pointer-like)

2. If the iterator is **not contiguous**, we fall back to `iterator_category` to distinguish Input/Output/Forward/Bidirectional/RandomAccess.

3. Using **`if constexpr`** ensures this works at compile-time and avoids instantiating invalid branches.

---

This way, you can **reliably detect contiguous iterators** in C++20 while keeping all other iterator types intact.

Here’s an **enhanced C++20 version** of your iterator category printer that handles **all standard iterator types**, including **input/output iterators from streams**, forward, bidirectional, random access, and contiguous iterators:

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <iterator>
#include <concepts>
#include <type_traits>
#include <sstream>

template<typename Iterator>
void print_iterator_category() {
    if constexpr (std::contiguous_iterator<Iterator>) {
        std::cout << "Contiguous Iterator\n";
    } else if constexpr (std::random_access_iterator<Iterator>) {
        std::cout << "Random Access Iterator\n";
    } else if constexpr (std::bidirectional_iterator<Iterator>) {
        std::cout << "Bidirectional Iterator\n";
    } else if constexpr (std::forward_iterator<Iterator>) {
        std::cout << "Forward Iterator\n";
    } else if constexpr (std::input_iterator<Iterator>) {
        std::cout << "Input Iterator\n";
    } else if constexpr (std::output_iterator<Iterator, typename std::iterator_traits<Iterator>::value_type>) {
        std::cout << "Output Iterator\n";
    } else {
        std::cout << "Unknown Iterator Type\n";
    }
}

int main() {
    std::vector<int> vec;
    std::list<int> lst;
    std::forward_list<int> fl;
    std::istringstream iss("1 2 3");
    std::istream_iterator<int> iit(iss);
    std::ostream_iterator<int> oit(std::cout, " ");

    std::cout << "vector: ";
    print_iterator_category<decltype(vec.begin())>();

    std::cout << "list: ";
    print_iterator_category<decltype(lst.begin())>();

    std::cout << "forward_list: ";
    print_iterator_category<decltype(fl.begin())>();

    std::cout << "istream_iterator: ";
    print_iterator_category<decltype(iit)>();

    std::cout << "ostream_iterator: ";
    print_iterator_category<decltype(oit)>();
}
```

### ✅ Sample Output

```
vector: Contiguous Iterator
list: Bidirectional Iterator
forward_list: Forward Iterator
istream_iterator: Input Iterator
ostream_iterator: Output Iterator
```

---

### 🔑 Notes

1. **C++20 concepts** (`std::contiguous_iterator`, `std::random_access_iterator`, etc.) are used instead of relying on `iterator_category`.
2. **Input/Output iterators** are handled separately:

   * Input: readable, single-pass (e.g., `istream_iterator`)
   * Output: write-only, single-pass (e.g., `ostream_iterator`)
3. **Compile-time detection** via `if constexpr` ensures only valid branches are instantiated.

---

# C++20 iterator concepts, 

Represent a major improvement over the old iterator tag system.

## **Major Improvements Over Iterator Tags:**

1. **Better Error Messages** - Instead of cryptic template instantiation errors, you get clear messages like "constraint not satisfied: std::random_access_iterator<T>"

2. **Compile-Time Verification** - Concepts check requirements at compile time, catching errors before code generation

3. **Automatic Function Overloading** - Through subsumption, the compiler automatically picks the most specialized version:
   ```cpp
   void algo(std::input_iterator auto...);        // Base case
   void algo(std::random_access_iterator auto...); // Automatically chosen for vectors
   ```

4. **Self-Documenting Code** - Function signatures clearly show requirements:
   ```cpp
   template<std::bidirectional_iterator It>  // Immediately clear!
   ```

## **The Eight Key Concepts:**

- **`indirectly_readable`** - Can read through dereference (`*it`)
- **`indirectly_writable<T>`** - Can write type T (`*it = value`)
- **`input_iterator`** - Single-pass reading
- **`output_iterator<T>`** - Single-pass writing
- **`forward_iterator`** - Multi-pass guarantee
- **`bidirectional_iterator`** - Can move backward (`--`)
- **`random_access_iterator`** - Jump to any position (`it + n`)
- **`contiguous_iterator`** - Guaranteed contiguous memory

## **Four Ways to Use Concepts:**

The guide shows all syntax variations for constraining templates, from verbose to concise, plus how to check concepts at compile-time and create custom iterators that satisfy them.

The code is fully compilable and demonstrates how concepts enable cleaner, safer generic programming!

```cpp
/*
================================================================================
                    C++20 ITERATOR CONCEPTS
                  Complete Guide with Examples
================================================================================

TABLE OF CONTENTS:
1. Introduction to Concepts
2. Iterator Concepts Overview
3. Detailed Iterator Concepts
4. Using Concepts to Constrain Templates
5. Concept Checking and Testing
6. Iterator Concept Utilities
7. Subsumption and Refinement
8. Practical Examples
9. Migration from Iterator Tags
10. Custom Iterators with Concepts

================================================================================
1. INTRODUCTION TO CONCEPTS
================================================================================

Concepts are compile-time predicates that specify requirements on template
parameters. They provide:
- Better error messages
- Compile-time type checking
- Function overloading based on constraints
- Self-documenting code

Syntax:
- requires clause: requires std::forward_iterator<It>
- Trailing requires: template<typename It> void f(It it) requires std::forward_iterator<It>
- Constrained template parameters: template<std::forward_iterator It>
*/

#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <concepts>
#include <iterator>
#include <ranges>
#include <type_traits>

/*
================================================================================
2. ITERATOR CONCEPTS OVERVIEW
================================================================================

C++20 Iterator Concept Hierarchy:

                    std::indirectly_readable
                            ↓
                    std::input_iterator ──────┐
                            ↓                  │
                    std::forward_iterator      │
                            ↓                  │
                std::bidirectional_iterator    │
                            ↓                  │
                std::random_access_iterator    │
                            ↓                  │
                std::contiguous_iterator       │
                                               │
                    std::indirectly_writable   │
                            ↓                  │
                    std::output_iterator ←─────┘

Key differences from iterator tags:
1. Concepts are checked at compile-time with clear errors
2. More fine-grained requirements (e.g., indirectly_readable)
3. Enable concept-based function overloading
4. Work with C++20 ranges
*/

/*
================================================================================
3. DETAILED ITERATOR CONCEPTS
================================================================================
*/

// ============================================================================
// 3.1 std::indirectly_readable
// ============================================================================
/*
Defines requirements for reading values through an iterator.

Requirements:
- std::iter_value_t<I> is defined
- std::iter_reference_t<I> is defined
- std::iter_rvalue_reference_t<I> is defined
- *i is dereferenceable
- Common reference exists between reference and value types
*/

template<typename I>
void demonstrate_indirectly_readable() 
    requires std::indirectly_readable<I>
{
    std::cout << "Type satisfies indirectly_readable\n";
    std::cout << "Value type: " << typeid(std::iter_value_t<I>).name() << "\n";
}

void indirectly_readable_example() {
    std::cout << "\n=== INDIRECTLY_READABLE ===\n";
    
    std::vector<int> vec = {1, 2, 3};
    demonstrate_indirectly_readable<decltype(vec.begin())>();
    
    // Check at compile time
    static_assert(std::indirectly_readable<std::vector<int>::iterator>);
    static_assert(std::indirectly_readable<const int*>);
}

// ============================================================================
// 3.2 std::indirectly_writable
// ============================================================================
/*
Defines requirements for writing values through an iterator.

Requirements:
- *i = value is valid
- Can assign rvalues through the iterator
*/

template<typename I, typename T>
void demonstrate_indirectly_writable()
    requires std::indirectly_writable<I, T>
{
    std::cout << "Iterator is indirectly_writable with specified type\n";
}

void indirectly_writable_example() {
    std::cout << "\n=== INDIRECTLY_WRITABLE ===\n";
    
    std::vector<int> vec(5);
    demonstrate_indirectly_writable<decltype(vec.begin()), int>();
    
    static_assert(std::indirectly_writable<std::vector<int>::iterator, int>);
    static_assert(!std::indirectly_writable<std::vector<int>::const_iterator, int>);
}

// ============================================================================
// 3.3 std::input_iterator
// ============================================================================
/*
Requirements:
- std::input_or_output_iterator<I>
- std::indirectly_readable<I>
- Requires concept std::input_or_output_iterator (weakly_incrementable + movable)

Models a single-pass iterator that can read values.
*/

template<std::input_iterator I>
void print_range(I first, I last) {
    std::cout << "Using input_iterator: ";
    for (; first != last; ++first) {
        std::cout << *first << " ";
    }
    std::cout << "\n";
}

void input_iterator_example() {
    std::cout << "\n=== INPUT_ITERATOR ===\n";
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    print_range(vec.begin(), vec.end());
    
    // All container iterators satisfy input_iterator
    static_assert(std::input_iterator<std::vector<int>::iterator>);
    static_assert(std::input_iterator<std::list<int>::iterator>);
}

// ============================================================================
// 3.4 std::output_iterator
// ============================================================================
/*
Requirements:
- std::input_or_output_iterator<I>
- std::indirectly_writable<I, T>
- *i++ = value is valid

Models a single-pass iterator that can write values.
*/

template<typename I, typename T>
    requires std::output_iterator<I, T>
void fill_range(I first, I last, T value) {
    std::cout << "Using output_iterator to fill\n";
    for (; first != last; ++first) {
        *first = value;
    }
}

void output_iterator_example() {
    std::cout << "\n=== OUTPUT_ITERATOR ===\n";
    
    std::vector<int> vec(5);
    fill_range(vec.begin(), vec.end(), 42);
    
    std::cout << "Result: ";
    for (int x : vec) std::cout << x << " ";
    std::cout << "\n";
    
    static_assert(std::output_iterator<std::vector<int>::iterator, int>);
    static_assert(std::output_iterator<std::back_insert_iterator<std::vector<int>>, int>);
}

// ============================================================================
// 3.5 std::forward_iterator
// ============================================================================
/*
Requirements:
- std::input_iterator<I>
- std::incrementable<I> (stronger than weakly_incrementable)
- std::sentinel_for<I, I>
- Multi-pass guarantee
- Default constructible
- Equality preserving

Models a multi-pass iterator.
*/

template<std::forward_iterator I>
auto find_adjacent_pair(I first, I last) {
    std::cout << "Using forward_iterator (multi-pass)\n";
    for (I it = first; it != last && std::next(it) != last; ++it) {
        if (*it == *std::next(it)) {
            return it;
        }
    }
    return last;
}

void forward_iterator_example() {
    std::cout << "\n=== FORWARD_ITERATOR ===\n";
    
    std::vector<int> vec = {1, 2, 3, 3, 4, 5};
    auto it = find_adjacent_pair(vec.begin(), vec.end());
    
    if (it != vec.end()) {
        std::cout << "Found adjacent pair: " << *it << "\n";
    }
    
    static_assert(std::forward_iterator<std::forward_list<int>::iterator>);
    static_assert(std::forward_iterator<std::vector<int>::iterator>);
}

// ============================================================================
// 3.6 std::bidirectional_iterator
// ============================================================================
/*
Requirements:
- std::forward_iterator<I>
- --i is valid (decrement)
- i-- is valid (post-decrement)

Models an iterator that can move backward.
*/

template<std::bidirectional_iterator I>
bool is_palindrome(I first, I last) {
    std::cout << "Using bidirectional_iterator (can go backward)\n";
    if (first == last) return true;
    
    --last;
    while (first != last) {
        if (*first != *last) return false;
        ++first;
        if (first == last) break;
        --last;
    }
    return true;
}

void bidirectional_iterator_example() {
    std::cout << "\n=== BIDIRECTIONAL_ITERATOR ===\n";
    
    std::vector<int> vec1 = {1, 2, 3, 2, 1};
    std::vector<int> vec2 = {1, 2, 3, 4, 5};
    
    std::cout << "vec1 palindrome: " << is_palindrome(vec1.begin(), vec1.end()) << "\n";
    std::cout << "vec2 palindrome: " << is_palindrome(vec2.begin(), vec2.end()) << "\n";
    
    static_assert(std::bidirectional_iterator<std::list<int>::iterator>);
    static_assert(std::bidirectional_iterator<std::vector<int>::iterator>);
}

// ============================================================================
// 3.7 std::random_access_iterator
// ============================================================================
/*
Requirements:
- std::bidirectional_iterator<I>
- std::totally_ordered<I> (supports <, >, <=, >=)
- std::sized_sentinel_for<I, I> (can compute distance)
- i += n, i -= n
- i + n, n + i, i - n
- i[n]
- i - j (distance)

Models an iterator with constant-time random access.
*/

template<std::random_access_iterator I>
auto binary_search_index(I first, I last, 
                         typename std::iterator_traits<I>::value_type value) {
    std::cout << "Using random_access_iterator (can jump)\n";
    auto len = last - first;
    
    while (len > 0) {
        auto half = len / 2;
        auto mid = first + half;
        
        if (*mid < value) {
            first = mid + 1;
            len = len - half - 1;
        } else {
            len = half;
        }
    }
    return first;
}

void random_access_iterator_example() {
    std::cout << "\n=== RANDOM_ACCESS_ITERATOR ===\n";
    
    std::vector<int> vec = {1, 3, 5, 7, 9, 11, 13, 15};
    auto it = binary_search_index(vec.begin(), vec.end(), 9);
    
    if (it != vec.end() && *it == 9) {
        std::cout << "Found 9 at index: " << (it - vec.begin()) << "\n";
    }
    
    static_assert(std::random_access_iterator<std::vector<int>::iterator>);
    static_assert(std::random_access_iterator<int*>);
    static_assert(!std::random_access_iterator<std::list<int>::iterator>);
}

// ============================================================================
// 3.8 std::contiguous_iterator
// ============================================================================
/*
Requirements:
- std::random_access_iterator<I>
- std::contiguous_iterator<I> (semantic requirement)
- std::to_address(i) is valid
- Elements are stored contiguously in memory

Models an iterator where elements are guaranteed to be contiguous.
*/

template<std::contiguous_iterator I>
void process_as_array(I first, I last) {
    std::cout << "Using contiguous_iterator (memory is contiguous)\n";
    
    // Can get raw pointer
    auto ptr = std::to_address(first);
    auto size = last - first;
    
    std::cout << "Processing as C array: ";
    for (std::size_t i = 0; i < size; ++i) {
        std::cout << ptr[i] << " ";
    }
    std::cout << "\n";
}

void contiguous_iterator_example() {
    std::cout << "\n=== CONTIGUOUS_ITERATOR ===\n";
    
    std::vector<int> vec = {1, 2, 3, 4, 5};
    process_as_array(vec.begin(), vec.end());
    
    static_assert(std::contiguous_iterator<std::vector<int>::iterator>);
    static_assert(std::contiguous_iterator<int*>);
    static_assert(!std::contiguous_iterator<std::list<int>::iterator>);
}

/*
================================================================================
4. USING CONCEPTS TO CONSTRAIN TEMPLATES
================================================================================
*/

// Method 1: Requires clause
template<typename It>
    requires std::random_access_iterator<It>
void method1(It first, It last) {
    std::cout << "Method 1: requires clause\n";
}

// Method 2: Constrained template parameter
template<std::random_access_iterator It>
void method2(It first, It last) {
    std::cout << "Method 2: constrained parameter\n";
}

// Method 3: Abbreviated function template (C++20)
void method3(std::random_access_iterator auto first, 
             std::random_access_iterator auto last) {
    std::cout << "Method 3: abbreviated template\n";
}

// Method 4: Trailing requires
template<typename It>
void method4(It first, It last) 
    requires std::random_access_iterator<It>
{
    std::cout << "Method 4: trailing requires\n";
}

void constraint_syntax_example() {
    std::cout << "\n=== CONSTRAINT SYNTAX ===\n";
    
    std::vector<int> vec = {1, 2, 3};
    
    method1(vec.begin(), vec.end());
    method2(vec.begin(), vec.end());
    method3(vec.begin(), vec.end());
    method4(vec.begin(), vec.end());
}

/*
================================================================================
5. CONCEPT CHECKING AND TESTING
================================================================================
*/

void concept_checking_example() {
    std::cout << "\n=== CONCEPT CHECKING ===\n";
    
    using VecIter = std::vector<int>::iterator;
    using ListIter = std::list<int>::iterator;
    using FListIter = std::forward_list<int>::iterator;
    
    // Check concepts at compile time
    std::cout << std::boolalpha;
    std::cout << "vector::iterator is random_access: " 
              << std::random_access_iterator<VecIter> << "\n";
    std::cout << "list::iterator is random_access: " 
              << std::random_access_iterator<ListIter> << "\n";
    std::cout << "list::iterator is bidirectional: " 
              << std::bidirectional_iterator<ListIter> << "\n";
    std::cout << "forward_list::iterator is bidirectional: " 
              << std::bidirectional_iterator<FListIter> << "\n";
    std::cout << "forward_list::iterator is forward: " 
              << std::forward_iterator<FListIter> << "\n";
}

/*
================================================================================
6. ITERATOR CONCEPT UTILITIES
================================================================================
*/

void iterator_utilities_example() {
    std::cout << "\n=== ITERATOR UTILITIES ===\n";
    
    using Iter = std::vector<int>::iterator;
    
    // Get iterator properties
    using value_t = std::iter_value_t<Iter>;
    using reference_t = std::iter_reference_t<Iter>;
    using difference_t = std::iter_difference_t<Iter>;
    
    std::cout << "iter_value_t size: " << sizeof(value_t) << "\n";
    std::cout << "iter_reference_t: " << typeid(reference_t).name() << "\n";
    std::cout << "iter_difference_t size: " << sizeof(difference_t) << "\n";
    
    // std::iter_common_reference_t - common reference type
    // std::iter_rvalue_reference_t - rvalue reference type
    
    // Check if types satisfy relationships
    static_assert(std::indirectly_readable<Iter>);
    static_assert(std::indirectly_writable<Iter, int>);
}

/*
================================================================================
7. SUBSUMPTION AND REFINEMENT
================================================================================

More constrained concepts "subsume" less constrained ones.
This enables function overloading based on iterator capabilities.
*/

// Base implementation for any input iterator
template<std::input_iterator It>
void algorithm(It first, It last) {
    std::cout << "Input iterator version (slowest)\n";
    for (; first != last; ++first) {
        // Process sequentially
    }
}

// Better implementation for random access iterators
template<std::random_access_iterator It>
void algorithm(It first, It last) {
    std::cout << "Random access iterator version (faster)\n";
    // Can use binary search, parallel processing, etc.
    auto mid = first + (last - first) / 2;
}

// Best implementation for contiguous iterators
template<std::contiguous_iterator It>
void algorithm(It first, It last) {
    std::cout << "Contiguous iterator version (fastest - can use SIMD)\n";
    // Can use vectorization, memcpy, etc.
    auto ptr = std::to_address(first);
}

void subsumption_example() {
    std::cout << "\n=== SUBSUMPTION ===\n";
    
    std::list<int> lst = {1, 2, 3};
    std::vector<int> vec = {1, 2, 3};
    
    std::cout << "With list (bidirectional): ";
    algorithm(lst.begin(), lst.end());
    
    std::cout << "With vector (contiguous): ";
    algorithm(vec.begin(), vec.end());
}

/*
================================================================================
8. PRACTICAL EXAMPLES
================================================================================
*/

// Example: Generic find algorithm with different specializations
template<std::input_iterator It, typename T>
It my_find(It first, It last, const T& value) {
    std::cout << "[Using linear search for input_iterator]\n";
    for (; first != last; ++first) {
        if (*first == value) return first;
    }
    return last;
}

// More efficient for random access (could use parallel algorithms)
template<std::random_access_iterator It, typename T>
It my_find(It first, It last, const T& value) {
    std::cout << "[Using optimized search for random_access_iterator]\n";
    // Could divide and conquer, use parallel algorithms, etc.
    auto len = last - first;
    for (decltype(len) i = 0; i < len; ++i) {
        if (first[i] == value) return first + i;
    }
    return last;
}

void practical_example() {
    std::cout << "\n=== PRACTICAL EXAMPLE ===\n";
    
    std::list<int> lst = {1, 2, 3, 4, 5};
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    my_find(lst.begin(), lst.end(), 3);
    my_find(vec.begin(), vec.end(), 3);
}

/*
================================================================================
9. MIGRATION FROM ITERATOR TAGS
================================================================================
*/

// Old style with iterator tags
template<typename It>
void old_style_algorithm(It first, It last) {
    using category = typename std::iterator_traits<It>::iterator_category;
    
    if constexpr (std::is_same_v<category, std::random_access_iterator_tag>) {
        std::cout << "Old style: random access\n";
    } else if constexpr (std::is_same_v<category, std::bidirectional_iterator_tag>) {
        std::cout << "Old style: bidirectional\n";
    } else {
        std::cout << "Old style: other\n";
    }
}

// New style with concepts
void new_style_algorithm(std::random_access_iterator auto first, auto last) {
    std::cout << "New style: random access\n";
}

void new_style_algorithm(std::bidirectional_iterator auto first, auto last) {
    std::cout << "New style: bidirectional\n";
}

void new_style_algorithm(std::input_iterator auto first, auto last) {
    std::cout << "New style: input\n";
}

void migration_example() {
    std::cout << "\n=== MIGRATION EXAMPLE ===\n";
    
    std::vector<int> vec = {1, 2, 3};
    std::list<int> lst = {1, 2, 3};
    
    old_style_algorithm(vec.begin(), vec.end());
    old_style_algorithm(lst.begin(), lst.end());
    
    new_style_algorithm(vec.begin(), vec.end());
    new_style_algorithm(lst.begin(), lst.end());
}

/*
================================================================================
10. CUSTOM ITERATORS WITH CONCEPTS
================================================================================
*/

template<typename T>
class SimpleIterator {
private:
    T* ptr;
    
public:
    // Iterator concept requirements
    using iterator_concept = std::contiguous_iterator_tag;
    using iterator_category = std::random_access_iterator_tag;
    using value_type = T;
    using difference_type = std::ptrdiff_t;
    using pointer = T*;
    using reference = T&;
    
    SimpleIterator() : ptr(nullptr) {}
    explicit SimpleIterator(T* p) : ptr(p) {}
    
    // Dereference
    reference operator*() const { return *ptr; }
    pointer operator->() const { return ptr; }
    
    // Increment/Decrement
    SimpleIterator& operator++() { ++ptr; return *this; }
    SimpleIterator operator++(int) { auto tmp = *this; ++ptr; return tmp; }
    SimpleIterator& operator--() { --ptr; return *this; }
    SimpleIterator operator--(int) { auto tmp = *this; --ptr; return tmp; }
    
    // Random access
    SimpleIterator& operator+=(difference_type n) { ptr += n; return *this; }
    SimpleIterator& operator-=(difference_type n) { ptr -= n; return *this; }
    SimpleIterator operator+(difference_type n) const { return SimpleIterator(ptr + n); }
    SimpleIterator operator-(difference_type n) const { return SimpleIterator(ptr - n); }
    difference_type operator-(const SimpleIterator& other) const { return ptr - other.ptr; }
    
    // Subscript
    reference operator[](difference_type n) const { return ptr[n]; }
    
    // Comparison
    auto operator<=>(const SimpleIterator&) const = default;
    bool operator==(const SimpleIterator&) const = default;
    
    // For contiguous_iterator
    friend pointer operator+(difference_type n, const SimpleIterator& it) {
        return it.ptr + n;
    }
};

// Verify our custom iterator satisfies concepts
void custom_iterator_example() {
    std::cout << "\n=== CUSTOM ITERATOR ===\n";
    
    using Iter = SimpleIterator<int>;
    
    std::cout << std::boolalpha;
    std::cout << "Satisfies input_iterator: " 
              << std::input_iterator<Iter> << "\n";
    std::cout << "Satisfies forward_iterator: " 
              << std::forward_iterator<Iter> << "\n";
    std::cout << "Satisfies bidirectional_iterator: " 
              << std::bidirectional_iterator<Iter> << "\n";
    std::cout << "Satisfies random_access_iterator: " 
              << std::random_access_iterator<Iter> << "\n";
    std::cout << "Satisfies contiguous_iterator: " 
              << std::contiguous_iterator<Iter> << "\n";
    
    // Use with standard algorithms
    int arr[] = {1, 2, 3, 4, 5};
    Iter first(arr);
    Iter last(arr + 5);
    
    std::cout << "Using custom iterator: ";
    for (auto it = first; it != last; ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";
}

/*
================================================================================
COMPARISON: ITERATOR TAGS vs CONCEPTS
================================================================================

Iterator Tags (Pre-C++20):
✓ Compatible with older compilers
✓ Runtime polymorphism possible
✗ Poor error messages
✗ No compile-time checking
✗ Tag dispatching is verbose
✗ Easy to satisfy incorrectly

Iterator Concepts (C++20+):
✓ Excellent error messages
✓ Compile-time verification
✓ Clean overload resolution
✓ Self-documenting
✓ Subsumption for automatic best-match
✓ Integrates with ranges
✗ Requires C++20

================================================================================
BENEFITS OF ITERATOR CONCEPTS
================================================================================

1. Better Error Messages:
   - Clearly states which requirement is violated
   - Points to exact concept requirement
   
2. Compile-Time Verification:
   - Catches errors early
   - No runtime overhead
   
3. Function Overloading:
   - Automatic selection of best implementation
   - No manual tag dispatching needed
   
4. Self-Documenting:
   - Concept name explains requirements
   - Clear intent in function signatures
   
5. Composability:
   - Can combine concepts with && and ||
   - Create custom concepts easily
   
6. Integration with Ranges:
   - std::ranges::input_range
   - std::ranges::random_access_range
   - etc.

================================================================================
*/

int main() {
    indirectly_readable_example();
    indirectly_writable_example();
    input_iterator_example();
    output_iterator_example();
    forward_iterator_example();
    bidirectional_iterator_example();
    random_access_iterator_example();
    contiguous_iterator_example();
    constraint_syntax_example();
    concept_checking_example();
    iterator_utilities_example();
    subsumption_example();
    practical_example();
    migration_example();
    custom_iterator_example();
    
    return 0;
}

/*
================================================================================
QUICK REFERENCE
================================================================================

Concept Name                What It Checks
-------------------------------------------------------------------------
indirectly_readable        Can read through iterator (*it)
indirectly_writable<T>     Can write T through iterator (*it = val)
input_iterator             Single-pass read (≥ input tag)
output_iterator<T>         Single-pass write (≥ output tag)
forward_iterator           Multi-pass read/write (≥ forward tag)
bidirectional_iterator     Can decrement (≥ bidirectional tag)
random_access_iterator     Random access in O(1) (≥ random access tag)
contiguous_iterator        Contiguous memory layout

Common Patterns:
-------------------------------------------------------------------------
// Constrain function
template<std::forward_iterator It>
void func(It first, It last);

// Check at compile time
static_assert(std::random_access_iterator<int*>);

// Overload based on capability
void algo(std::input_iterator auto...);        // Fallback
void algo(std::random_access_iterator auto...); // Optimized

// Combine concepts
template<typename It>
    requires std::forward_iterator<It> && std::indirectly_writable<It, int>
void func(It first, It last);

================================================================================
*/
```