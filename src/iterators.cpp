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