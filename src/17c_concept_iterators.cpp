 // c++ -std=c++20 17c_concept_iterators.cpp 

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
                    std::input_iterator  ──────┐
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