 // c++ -std=c++20 17b_iterator_custom.cpp 

#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <iterator>
#include <concepts>
#include <type_traits>
#include <sstream>

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


int main(int argc, char const *argv[])
{
    custom_iterator_example();
    return 0;
}