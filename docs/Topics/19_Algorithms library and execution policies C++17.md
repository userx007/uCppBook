# Algorithms Library and Execution Policies (C++17)

## Overview

The **Algorithms Library** in C++ is part of the Standard Template Library (STL) and provides a collection of functions for performing operations on sequences of elements. C++17 introduced **execution policies** that enable parallel and vectorized execution of many standard algorithms.

## Algorithms Library

The algorithms library (`<algorithm>` and `<numeric>` headers) includes functions for:

- **Sorting**: `std::sort`, `std::partial_sort`, `std::stable_sort`
- **Searching**: `std::find`, `std::binary_search`, `std::find_if`
- **Modifying sequences**: `std::copy`, `std::transform`, `std::replace`
- **Numeric operations**: `std::accumulate`, `std::reduce`, `std::transform_reduce`
- **And many more**: partitioning, permutations, heap operations, etc.

## Execution Policies (C++17)

C++17 introduced **execution policies** to allow parallel and vectorized execution of algorithms. These are defined in the `<execution>` header.

### Types of Execution Policies

There are three standard execution policies in the `std::execution` namespace:

1. **`sequenced_policy` (seq)**
   - Sequential execution (no parallelization)
   - Default behavior when no policy is specified
   - Operations execute in the calling thread

2. **`parallel_policy` (par)**
   - Parallel execution across multiple threads
   - May execute in multiple threads
   - Operations may be interleaved but not run concurrently on the same element

3. **`parallel_unsequenced_policy` (par_unseq)**
   - Parallel and vectorized execution
   - Most aggressive optimization
   - Operations may be interleaved and vectorized (SIMD)
   - Strictest requirements on callable objects

### Basic Usage

```cpp
#include <algorithm>
#include <execution>
#include <vector>

std::vector<int> data = {5, 2, 9, 1, 5, 6};

// Sequential (traditional)
std::sort(data.begin(), data.end());

// Parallel execution
std::sort(std::execution::par, data.begin(), data.end());

// Parallel and vectorized
std::sort(std::execution::par_unseq, data.begin(), data.end());
```

## Supported Algorithms

C++17 added execution policy overloads to 69 algorithms, including:

- `std::for_each`, `std::for_each_n`
- `std::sort`, `std::stable_sort`
- `std::transform`
- `std::find`, `std::find_if`
- `std::count`, `std::count_if`
- `std::copy`, `std::move`
- `std::reduce`, `std::transform_reduce` (new in C++17)

## Important Considerations

### Thread Safety Requirements

When using parallel policies:
- **Element access functions** must be thread-safe
- **No data races**: Don't modify shared state without synchronization
- **Exception safety**: If an exception is thrown during parallel execution, `std::terminate()` is called

```cpp
// BAD - data race!
int counter = 0;
std::for_each(std::execution::par, v.begin(), v.end(),
    [&](int x) { counter++; }); // UNSAFE

// GOOD - use atomic or reduce
std::atomic<int> counter{0};
std::for_each(std::execution::par, v.begin(), v.end(),
    [&](int x) { counter++; }); // Safe
```

### Vectorization Requirements (par_unseq)

For `parallel_unsequenced_policy`:
- Functions must be **safe to vectorize**
- No locks, allocations, or I/O operations
- No exceptions
- No access to non-thread-local shared state

### Performance Benefits

Execution policies provide performance benefits when:
- Working with **large datasets** (overhead for small data)
- Operations are **computationally intensive**
- Hardware has **multiple cores** available
- The algorithm implementation supports parallelization

### Example: Practical Use Case

```cpp
#include <algorithm>
#include <execution>
#include <vector>
#include <cmath>

std::vector<double> data(10'000'000);
// Initialize data...

// Transform with expensive operation
std::transform(std::execution::par_unseq,
    data.begin(), data.end(), data.begin(),
    [](double x) { return std::sqrt(x * x + 1.0); });
```

## Compiler Support

Execution policies require:
- **Compiler support**: Most modern compilers (GCC 9+, Clang 9+, MSVC 2017+)
- **Threading library**: Often requires Intel TBB or similar
- **Linking**: May need to link against additional libraries

```bash
# Example with GCC and TBB
g++ -std=c++17 program.cpp -ltbb
```

## Summary

C++17 execution policies provide a **simple, declarative way** to request parallel execution of standard algorithms. By adding a single policy parameter, you can potentially achieve significant performance improvements on multi-core systems, while the standard library handles the complexity of parallelization.

## Quick Interview Highlights:

**Most Commonly Asked:**
- `std::sort`, `std::find`, `std::transform`
- `std::accumulate` / `std::reduce`
- `std::remove` / `std::remove_if` (remember: it doesn't actually erase!)
- `std::unique` (removes consecutive duplicates only)

**Common Gotchas:**
1. **`std::remove` doesn't erase** - returns new end iterator, must call `erase()`
2. **Binary search requires sorted data** - `binary_search`, `lower_bound`, `upper_bound`
3. **Set operations require sorted ranges** - `set_union`, `set_intersection`, etc.
4. **`std::unique` only removes consecutive duplicates** - sort first if needed

**Complexity Reminders:**
- `std::sort`: O(n log n)
- `std::find`: O(n)
- `std::binary_search`: O(log n)
- `std::accumulate`/`std::reduce`: O(n)

**Execution Policies (C++17):**
- `seq` - sequential (default)
- `par` - parallel threads
- `par_unseq` - parallel + vectorized (strictest requirements)

**Remember:** When using parallel policies, your lambda must be thread-safe (no data races, no shared mutable state without synchronization)!

```cpp
#include <algorithm>
#include <numeric>
#include <execution>
#include <vector>
#include <iostream>
#include <string>

// ============================================================================
// SORTING ALGORITHMS
// ============================================================================

void sorting_examples() {
    std::vector<int> v = {5, 2, 9, 1, 5, 6};

    // std::sort - O(n log n), not stable
    std::sort(v.begin(), v.end());
    // Result: {1, 2, 5, 5, 6, 9}

    // With custom comparator (descending)
    std::sort(v.begin(), v.end(), std::greater<int>());
    // Result: {9, 6, 5, 5, 2, 1}

    // std::stable_sort - O(n log n), maintains relative order of equal elements
    std::vector<std::pair<int, char>> pairs = {{1, 'a'}, {2, 'b'}, {1, 'c'}};
    std::stable_sort(pairs.begin(), pairs.end(),
        [](const auto& a, const auto& b) { return a.first < b.first; });
    // Result: {1,'a'}, {1,'c'}, {2,'b'} - 'a' stays before 'c'

    // std::partial_sort - sorts first N elements
    v = {5, 2, 9, 1, 5, 6};
    std::partial_sort(v.begin(), v.begin() + 3, v.end());
    // Result: first 3 are sorted: {1, 2, 5, ...}

    // std::nth_element - partition around nth element
    v = {5, 2, 9, 1, 5, 6};
    std::nth_element(v.begin(), v.begin() + 3, v.end());
    // v[3] is in its sorted position, smaller on left, larger on right

    // Parallel execution (C++17)
    std::sort(std::execution::par, v.begin(), v.end());
}

// ============================================================================
// SEARCHING ALGORITHMS
// ============================================================================

void searching_examples() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9};

    // std::find - linear search, returns iterator
    auto it = std::find(v.begin(), v.end(), 5);
    if (it != v.end()) {
        std::cout << "Found: " << *it << '\n';
    }

    // std::find_if - search with predicate
    auto even = std::find_if(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });
    // Finds first even number: 2

    // std::find_if_not - search for element NOT matching predicate
    auto odd = std::find_if_not(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });

    // std::binary_search - O(log n), requires sorted range
    bool found = std::binary_search(v.begin(), v.end(), 5);
    // Returns true/false

    // std::lower_bound - first element >= value
    auto lb = std::lower_bound(v.begin(), v.end(), 5);
    // Points to 5

    // std::upper_bound - first element > value
    auto ub = std::upper_bound(v.begin(), v.end(), 5);
    // Points to 6

    // std::equal_range - returns pair of lower_bound and upper_bound
    auto range = std::equal_range(v.begin(), v.end(), 5);
    // range.first = lower_bound, range.second = upper_bound

    // std::search - find subsequence
    std::vector<int> pattern = {4, 5, 6};
    auto pos = std::search(v.begin(), v.end(), pattern.begin(), pattern.end());

    // std::adjacent_find - find consecutive equal elements
    std::vector<int> v2 = {1, 2, 3, 3, 4};
    auto adj = std::adjacent_find(v2.begin(), v2.end());
    // Points to first 3
}

// ============================================================================
// COUNTING & COMPARISON
// ============================================================================

void counting_examples() {
    std::vector<int> v = {1, 2, 3, 2, 5, 2, 7};

    // std::count - count occurrences
    int cnt = std::count(v.begin(), v.end(), 2);
    // Returns 3

    // std::count_if - count with predicate
    int even_cnt = std::count_if(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });
    // Returns 3 (three even numbers)

    // std::equal - check if ranges are equal
    std::vector<int> v2 = {1, 2, 3, 2, 5, 2, 7};
    bool same = std::equal(v.begin(), v.end(), v2.begin());

    // std::mismatch - find first difference
    std::vector<int> v3 = {1, 2, 4, 2, 5};
    auto mm = std::mismatch(v.begin(), v.end(), v3.begin());
    // mm.first points to 3 in v, mm.second points to 4 in v3

    // std::lexicographical_compare - dictionary order comparison
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {1, 2, 4};
    bool less = std::lexicographical_compare(a.begin(), a.end(),
                                              b.begin(), b.end());
    // Returns true (3 < 4)
}

// ============================================================================
// MODIFYING SEQUENCE OPERATIONS
// ============================================================================

void modifying_examples() {
    std::vector<int> v = {1, 2, 3, 4, 5};
    std::vector<int> dest(5);

    // std::copy - copy elements
    std::copy(v.begin(), v.end(), dest.begin());

    // std::copy_if - conditional copy
    std::vector<int> evens;
    std::copy_if(v.begin(), v.end(), std::back_inserter(evens),
        [](int x) { return x % 2 == 0; });
    // evens = {2, 4}

    // std::copy_n - copy n elements
    std::copy_n(v.begin(), 3, dest.begin());

    // std::move - move elements (for move-only types)
    std::vector<std::string> src = {"a", "b", "c"};
    std::vector<std::string> dst(3);
    std::move(src.begin(), src.end(), dst.begin());
    // src elements now in valid but unspecified state

    // std::transform - apply function and store result
    std::vector<int> squared(5);
    std::transform(v.begin(), v.end(), squared.begin(),
        [](int x) { return x * x; });
    // squared = {1, 4, 9, 16, 25}

    // std::transform (binary) - combine two ranges
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};
    std::vector<int> sum(3);
    std::transform(a.begin(), a.end(), b.begin(), sum.begin(),
        [](int x, int y) { return x + y; });
    // sum = {5, 7, 9}

    // std::fill - fill with value
    std::fill(v.begin(), v.end(), 0);
    // v = {0, 0, 0, 0, 0}

    // std::fill_n - fill n elements
    std::fill_n(v.begin(), 3, 42);

    // std::generate - fill with generated values
    int n = 0;
    std::generate(v.begin(), v.end(), [&n]() { return n++; });
    // v = {0, 1, 2, 3, 4}

    // std::replace - replace value
    v = {1, 2, 3, 2, 5};
    std::replace(v.begin(), v.end(), 2, 99);
    // v = {1, 99, 3, 99, 5}

    // std::replace_if - conditional replace
    std::replace_if(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; }, 0);

    // std::remove - "remove" elements (moves to end)
    v = {1, 2, 3, 2, 5};
    auto new_end = std::remove(v.begin(), v.end(), 2);
    v.erase(new_end, v.end()); // Actually erase
    // v = {1, 3, 5}

    // std::remove_if - conditional remove
    v = {1, 2, 3, 4, 5};
    new_end = std::remove_if(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });
    v.erase(new_end, v.end());
    // v = {1, 3, 5}

    // std::unique - remove consecutive duplicates
    v = {1, 1, 2, 2, 3, 1};
    new_end = std::unique(v.begin(), v.end());
    v.erase(new_end, v.end());
    // v = {1, 2, 3, 1}

    // std::reverse - reverse elements
    std::reverse(v.begin(), v.end());

    // std::rotate - rotate elements
    v = {1, 2, 3, 4, 5};
    std::rotate(v.begin(), v.begin() + 2, v.end());
    // v = {3, 4, 5, 1, 2}

    // std::shuffle - random shuffle
    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(v.begin(), v.end(), g);
}

// ============================================================================
// PARTITIONING
// ============================================================================

void partition_examples() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6};

    // std::partition - partition by predicate (may reorder)
    auto pivot = std::partition(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });
    // Even numbers before pivot, odd after

    // std::stable_partition - maintains relative order
    v = {1, 2, 3, 4, 5, 6};
    pivot = std::stable_partition(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });
    // v = {2, 4, 6, 1, 3, 5}

    // std::is_partitioned - check if partitioned
    bool partitioned = std::is_partitioned(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });

    // std::partition_point - find partition point
    auto pp = std::partition_point(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });
}

// ============================================================================
// NUMERIC ALGORITHMS
// ============================================================================

void numeric_examples() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // std::accumulate - sum/fold (sequential)
    int sum = std::accumulate(v.begin(), v.end(), 0);
    // Returns 15

    // With custom operation
    int product = std::accumulate(v.begin(), v.end(), 1,
        [](int a, int b) { return a * b; });
    // Returns 120 (factorial of 5)

    // std::reduce - like accumulate but allows parallel execution
    int sum2 = std::reduce(std::execution::par, v.begin(), v.end(), 0);

    // std::transform_reduce - transform then reduce
    int sum_of_squares = std::transform_reduce(
        std::execution::par,
        v.begin(), v.end(),
        0,
        std::plus<>(),
        [](int x) { return x * x; }
    );
    // Returns 55 (1+4+9+16+25)

    // Binary version - dot product
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {4, 5, 6};
    int dot_product = std::transform_reduce(
        a.begin(), a.end(), b.begin(), 0
    );
    // Returns 32 (1*4 + 2*5 + 3*6)

    // std::partial_sum - cumulative sum
    std::vector<int> cumsum(5);
    std::partial_sum(v.begin(), v.end(), cumsum.begin());
    // cumsum = {1, 3, 6, 10, 15}

    // std::inclusive_scan - like partial_sum, parallelizable
    std::vector<int> scan(5);
    std::inclusive_scan(std::execution::par,
        v.begin(), v.end(), scan.begin());

    // std::exclusive_scan - excludes current element
    std::exclusive_scan(std::execution::par,
        v.begin(), v.end(), scan.begin(), 0);
    // scan = {0, 1, 3, 6, 10}

    // std::adjacent_difference - differences between adjacent elements
    std::vector<int> diffs(5);
    std::adjacent_difference(v.begin(), v.end(), diffs.begin());
    // diffs = {1, 1, 1, 1, 1}

    // std::iota - fill with incrementing values
    std::iota(v.begin(), v.end(), 10);
    // v = {10, 11, 12, 13, 14}

    // std::inner_product - dot product
    a = {1, 2, 3};
    b = {4, 5, 6};
    int ip = std::inner_product(a.begin(), a.end(), b.begin(), 0);
    // Returns 32
}

// ============================================================================
// MIN/MAX ALGORITHMS
// ============================================================================

void minmax_examples() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9, 2, 6};

    // std::min / std::max
    int a = 5, b = 3;
    int minimum = std::min(a, b);  // 3
    int maximum = std::max(a, b);  // 5

    // std::min_element / std::max_element
    auto min_it = std::min_element(v.begin(), v.end());
    // Points to 1

    auto max_it = std::max_element(v.begin(), v.end());
    // Points to 9

    // std::minmax_element - both in one pass
    auto [min_it2, max_it2] = std::minmax_element(v.begin(), v.end());

    // std::clamp (C++17) - constrain value to range
    int val = std::clamp(15, 0, 10);  // Returns 10
    val = std::clamp(-5, 0, 10);      // Returns 0
    val = std::clamp(5, 0, 10);       // Returns 5
}

// ============================================================================
// SET OPERATIONS (on sorted ranges)
// ============================================================================

void set_operations_examples() {
    std::vector<int> a = {1, 2, 3, 4, 5};
    std::vector<int> b = {3, 4, 5, 6, 7};
    std::vector<int> result;

    // std::set_union - union of two sets
    std::set_union(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(result));
    // result = {1, 2, 3, 4, 5, 6, 7}

    result.clear();

    // std::set_intersection - intersection
    std::set_intersection(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(result));
    // result = {3, 4, 5}

    result.clear();

    // std::set_difference - elements in a but not in b
    std::set_difference(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(result));
    // result = {1, 2}

    result.clear();

    // std::set_symmetric_difference - elements in either but not both
    std::set_symmetric_difference(a.begin(), a.end(), b.begin(), b.end(),
        std::back_inserter(result));
    // result = {1, 2, 6, 7}

    // std::includes - check if all elements of b are in a
    bool contains = std::includes(a.begin(), a.end(), b.begin(), b.end());
    // Returns false
}

// ============================================================================
// HEAP OPERATIONS
// ============================================================================

void heap_examples() {
    std::vector<int> v = {3, 1, 4, 1, 5, 9};

    // std::make_heap - create max heap
    std::make_heap(v.begin(), v.end());
    // v[0] is largest element

    // std::push_heap - add element to heap
    v.push_back(10);
    std::push_heap(v.begin(), v.end());

    // std::pop_heap - move largest to end
    std::pop_heap(v.begin(), v.end());
    int largest = v.back();
    v.pop_back();

    // std::sort_heap - sort heap (destroys heap property)
    std::sort_heap(v.begin(), v.end());

    // std::is_heap - check if range is heap
    bool is_heap = std::is_heap(v.begin(), v.end());
}

// ============================================================================
// PERMUTATION OPERATIONS
// ============================================================================

void permutation_examples() {
    std::vector<int> v = {1, 2, 3};

    // std::next_permutation - generate next permutation
    do {
        // Process permutation
        for (int x : v) std::cout << x << ' ';
        std::cout << '\n';
    } while (std::next_permutation(v.begin(), v.end()));
    // Generates: 123, 132, 213, 231, 312, 321

    // std::prev_permutation - generate previous permutation
    v = {3, 2, 1};
    std::prev_permutation(v.begin(), v.end());
    // v = {3, 1, 2}

    // std::is_permutation - check if one is permutation of other
    std::vector<int> a = {1, 2, 3};
    std::vector<int> b = {3, 1, 2};
    bool is_perm = std::is_permutation(a.begin(), a.end(), b.begin());
    // Returns true
}

// ============================================================================
// OTHER USEFUL ALGORITHMS
// ============================================================================

void other_algorithms() {
    std::vector<int> v = {1, 2, 3, 4, 5};

    // std::for_each - apply function to each element
    std::for_each(v.begin(), v.end(), [](int& x) { x *= 2; });
    // v = {2, 4, 6, 8, 10}

    // std::for_each_n - apply to first n elements
    std::for_each_n(v.begin(), 3, [](int& x) { x += 1; });
    // v = {3, 5, 7, 8, 10}

    // std::all_of - check if all satisfy predicate
    bool all_positive = std::all_of(v.begin(), v.end(),
        [](int x) { return x > 0; });

    // std::any_of - check if any satisfy predicate
    bool has_even = std::any_of(v.begin(), v.end(),
        [](int x) { return x % 2 == 0; });

    // std::none_of - check if none satisfy predicate
    bool no_negative = std::none_of(v.begin(), v.end(),
        [](int x) { return x < 0; });

    // std::swap - swap two values
    int a = 5, b = 10;
    std::swap(a, b);

    // std::swap_ranges - swap elements between ranges
    std::vector<int> v1 = {1, 2, 3};
    std::vector<int> v2 = {4, 5, 6};
    std::swap_ranges(v1.begin(), v1.end(), v2.begin());
    // v1 = {4, 5, 6}, v2 = {1, 2, 3}
}

// ============================================================================
// EXECUTION POLICY EXAMPLES
// ============================================================================

void execution_policy_examples() {
    std::vector<int> v(1'000'000);
    std::iota(v.begin(), v.end(), 0);

    // Sequential (default)
    std::sort(v.begin(), v.end());

    // Parallel
    std::sort(std::execution::par, v.begin(), v.end());

    // Parallel + Vectorized
    std::sort(std::execution::par_unseq, v.begin(), v.end());

    // Works with many algorithms
    auto sum = std::reduce(std::execution::par, v.begin(), v.end(), 0);

    std::vector<int> result(v.size());
    std::transform(std::execution::par_unseq,
        v.begin(), v.end(), result.begin(),
        [](int x) { return x * x; });
}

int main() {
    std::cout << "C++ Algorithms Library - Interview Reference\n";
    std::cout << "============================================\n\n";

    // Uncomment to run examples
    // sorting_examples();
    // searching_examples();
    // counting_examples();
    // modifying_examples();
    // partition_examples();
    // numeric_examples();
    // minmax_examples();
    // set_operations_examples();
    // heap_examples();
    // permutation_examples();
    // other_algorithms();
    // execution_policy_examples();

    return 0;
}
```
