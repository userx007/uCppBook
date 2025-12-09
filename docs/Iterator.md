**Iterator Features:**
- **Iterator traits**: Defines the iterator as a random access iterator, which is the most powerful type
- **Dereference operators** (`*`, `->`): Access the element
- **Increment/Decrement** (`++`, `--`): Move forward/backward
- **Arithmetic operators** (`+`, `-`, `+=`, `-=`): Jump multiple positions
- **Comparison operators**: Compare iterator positions
- **Subscript operator** (`[]`): Random access to elements

**Container Functions:**
- `begin()` and `end()`: Return iterators to the first element and one past the last
- Standard operations like `push_back()`, `operator[]`, and `size()`

**Benefits:**
- Works with range-based for loops
- Compatible with STL algorithms
- Supports all iterator operations (random access)

```cpp
#include <iostream>
#include <iterator>
#include <algorithm>

// A simple dynamic array container with custom iterators
template<typename T>
class MyVector {
private:
    T* data;
    size_t capacity;
    size_t length;

    void resize() {
        capacity = capacity == 0 ? 1 : capacity * 2;
        T* newData = new T[capacity];
        for (size_t i = 0; i < length; ++i) {
            newData[i] = data[i];
        }
        delete[] data;
        data = newData;
    }

public:
    // Iterator class
    class Iterator {
    private:
        T* ptr;

    public:
        // Iterator traits
        using iterator_category = std::random_access_iterator_tag;
        using value_type = T;
        using difference_type = std::ptrdiff_t;
        using pointer = T*;
        using reference = T&;

        Iterator(T* p) : ptr(p) {}

        // Dereference operators
        reference operator*() const { return *ptr; }
        pointer operator->() const { return ptr; }

        // Increment/Decrement operators
        Iterator& operator++() { ++ptr; return *this; }
        Iterator operator++(int) { Iterator tmp = *this; ++ptr; return tmp; }
        Iterator& operator--() { --ptr; return *this; }
        Iterator operator--(int) { Iterator tmp = *this; --ptr; return tmp; }

        // Arithmetic operators
        Iterator operator+(difference_type n) const { return Iterator(ptr + n); }
        Iterator operator-(difference_type n) const { return Iterator(ptr - n); }
        difference_type operator-(const Iterator& other) const { return ptr - other.ptr; }

        Iterator& operator+=(difference_type n) { ptr += n; return *this; }
        Iterator& operator-=(difference_type n) { ptr -= n; return *this; }

        // Subscript operator
        reference operator[](difference_type n) const { return ptr[n]; }

        // Comparison operators
        bool operator==(const Iterator& other) const { return ptr == other.ptr; }
        bool operator!=(const Iterator& other) const { return ptr != other.ptr; }
        bool operator<(const Iterator& other) const { return ptr < other.ptr; }
        bool operator>(const Iterator& other) const { return ptr > other.ptr; }
        bool operator<=(const Iterator& other) const { return ptr <= other.ptr; }
        bool operator>=(const Iterator& other) const { return ptr >= other.ptr; }
    };

    // Constructor and destructor
    MyVector() : data(nullptr), capacity(0), length(0) {}

    ~MyVector() { delete[] data; }

    // Add element
    void push_back(const T& value) {
        if (length >= capacity) resize();
        data[length++] = value;
    }

    // Access elements
    T& operator[](size_t index) { return data[index]; }
    const T& operator[](size_t index) const { return data[index]; }

    size_t size() const { return length; }

    // Iterator functions
    Iterator begin() { return Iterator(data); }
    Iterator end() { return Iterator(data + length); }
};

int main() {
    MyVector<int> vec;

    // Add elements
    for (int i = 1; i <= 10; ++i) {
        vec.push_back(i * 10);
    }

    // Iterate using custom iterator
    std::cout << "Elements: ";
    for (auto it = vec.begin(); it != vec.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << "\n";

    // Range-based for loop (works because we have begin/end)
    std::cout << "Range-based for: ";
    for (const auto& val : vec) {
        std::cout << val << " ";
    }
    std::cout << "\n";

    // Using STL algorithms
    std::cout << "Reversed: ";
    std::for_each(vec.end() - 1, vec.begin() - 1, [](int n) {
        std::cout << n << " ";
    });
    std::cout << "\n";

    // Iterator arithmetic
    auto it = vec.begin();
    std::cout << "Third element: " << *(it + 2) << "\n";
    std::cout << "Using subscript: " << it[4] << "\n";

    return 0;
}
```