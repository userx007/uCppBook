# Small Object Optimization and Empty Base Optimization in C++

These are two important optimization techniques in C++ that reduce memory overhead and improve performance. Let me explain each with detailed examples.

## Small Object Optimization (SOO)

**Small Object Optimization** is a technique where containers avoid heap allocation for small objects by storing them directly within the container's internal buffer.

### Why It Matters
- **Avoids heap allocation overhead** - malloc/new are expensive
- **Better cache locality** - data is stored inline
- **No pointer indirection** - faster access
- **Eliminates allocation/deallocation costs**

### Common Example: std::string

Most `std::string` implementations use SOO. Short strings (typically 15-23 characters) are stored directly in the string object rather than on the heap.## Empty Base Optimization (EBO)

**Empty Base Optimization** is a compiler optimization that allows empty base classes to occupy zero bytes when used as a base class (though they still cannot have zero size when standalone).

### Why It Matters
- **Reduces memory footprint** of class hierarchies
- **Critical for policy-based design** and template metaprogramming
- **Standard library uses it extensively** (allocators, comparators, etc.)
- **No runtime cost** - purely compile-time optimization

### The Problem Without EBO

In C++, no object can have size 0 (to ensure distinct addresses). Empty classes have size ≥ 1:## Key Differences and Trade-offs

### Small Object Optimization
**Advantages:**
- Eliminates dynamic allocation for common small cases
- Better cache performance
- Faster construction/destruction
- No fragmentation

**Disadvantages:**
- Larger object size (must hold the buffer)
- Copy operations copy more data
- More complex implementation
- Fixed buffer size trade-off

**When to Use:**
- When most instances will be small (strings, vectors, functions)
- When allocation overhead is significant
- When cache locality matters

### Empty Base Optimization

**Advantages:**
- Zero overhead for stateless components
- Essential for policy-based design
- Enables zero-cost abstractions
- Compiler handles it automatically

**Disadvantages:**
- Only works with empty classes
- Requires inheritance or `[[no_unique_address]]`
- Can complicate interface design
- Doesn't work with `final` classes

**When to Use:**
- Allocators (almost always empty in standard containers)
- Comparators and hash functions
- Policy classes in template metaprogramming
- Stateless function objects

## Real-World Applications

**Standard Library Examples:**
- `std::string` - Uses SOO (typically 15-23 byte buffer)
- `std::function` - Uses SOO for small callable objects
- `std::vector` - Uses EBO for allocators
- `std::shared_ptr` - Uses both (SOO for control block, EBO for deleter)
- `std::unique_ptr` - Uses EBO for custom deleters

**Modern C++20 Improvement:**
The `[[no_unique_address]]` attribute provides EBO benefits without inheritance:
```cpp
template<typename T, typename Allocator>
class Container {
    [[no_unique_address]] Allocator alloc_;  // Zero size if empty!
    T* data_;
};
```

Both optimizations are crucial for creating efficient, zero-overhead abstractions in modern C++!

```cpp
#include <iostream>
#include <memory>
#include <type_traits>

// Empty classes still have size >= 1
struct EmptyClass {};

struct EmptyStateless {
    void do_something() { std::cout << "Doing something\n"; }
};

// Without EBO: composition wastes space
template<typename T, typename Allocator>
class VectorWithoutEBO {
private:
    T* data_;
    size_t size_;
    Allocator alloc_;  // Wastes space if Allocator is empty!
};

// With EBO: inheritance saves space
template<typename T, typename Allocator>
class VectorWithEBO : private Allocator {
private:
    T* data_;
    size_t size_;
    // Allocator takes 0 bytes as base class!
    
public:
    Allocator& get_allocator() { return *this; }
};

// Modern approach: [[no_unique_address]] attribute (C++20)
template<typename T, typename Allocator>
class VectorModern {
private:
    T* data_;
    size_t size_;
    [[no_unique_address]] Allocator alloc_;
};

// Practical example: Custom allocator
template<typename T>
struct CustomAllocator {
    using value_type = T;
    
    T* allocate(size_t n) {
        std::cout << "CustomAllocator::allocate(" << n << ")\n";
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    
    void deallocate(T* p, size_t) {
        std::cout << "CustomAllocator::deallocate\n";
        ::operator delete(p);
    }
};

// Stateful allocator (non-empty)
template<typename T>
struct StatefulAllocator {
    using value_type = T;
    int id_;
    
    explicit StatefulAllocator(int id = 0) : id_(id) {}
    
    T* allocate(size_t n) {
        std::cout << "StatefulAllocator[" << id_ << "]::allocate(" << n << ")\n";
        return static_cast<T*>(::operator new(n * sizeof(T)));
    }
    
    void deallocate(T* p, size_t) {
        ::operator delete(p);
    }
};

// Example with policies (common use case for EBO)
struct DefaultPolicy {
    void on_access() const {}
};

struct LoggingPolicy {
    void on_access() const {
        std::cout << "Accessing element\n";
    }
};

// Container with policy
template<typename T, typename Policy = DefaultPolicy>
class PolicyContainer : private Policy {
private:
    T data_[10];
    size_t size_;
    
public:
    PolicyContainer() : size_(0) {}
    
    void push_back(const T& value) {
        if (size_ < 10) {
            data_[size_++] = value;
        }
    }
    
    T& operator[](size_t i) {
        this->on_access();  // Call policy method
        return data_[i];
    }
    
    size_t size() const { return size_; }
};

// Compressed pair implementation (like std::pair but with EBO)
template<typename T1, typename T2,
         bool = std::is_empty_v<T1> && !std::is_final_v<T1>,
         bool = std::is_empty_v<T2> && !std::is_final_v<T2>>
class CompressedPair;

// Both empty
template<typename T1, typename T2>
class CompressedPair<T1, T2, true, true> : private T1, private T2 {
public:
    CompressedPair() = default;
    CompressedPair(const T1& t1, const T2& t2) : T1(t1), T2(t2) {}
    
    T1& first() { return *this; }
    T2& second() { return *this; }
    const T1& first() const { return *this; }
    const T2& second() const { return *this; }
};

// First empty
template<typename T1, typename T2>
class CompressedPair<T1, T2, true, false> : private T1 {
    T2 second_;
public:
    CompressedPair() = default;
    CompressedPair(const T1& t1, const T2& t2) : T1(t1), second_(t2) {}
    
    T1& first() { return *this; }
    T2& second() { return second_; }
    const T1& first() const { return *this; }
    const T2& second() const { return second_; }
};

// Second empty
template<typename T1, typename T2>
class CompressedPair<T1, T2, false, true> : private T2 {
    T1 first_;
public:
    CompressedPair() = default;
    CompressedPair(const T1& t1, const T2& t2) : first_(t1), T2(t2) {}
    
    T1& first() { return first_; }
    T2& second() { return *this; }
    const T1& first() const { return first_; }
    const T2& second() const { return *this; }
};

// Neither empty
template<typename T1, typename T2>
class CompressedPair<T1, T2, false, false> {
    T1 first_;
    T2 second_;
public:
    CompressedPair() = default;
    CompressedPair(const T1& t1, const T2& t2) : first_(t1), second_(t2) {}
    
    T1& first() { return first_; }
    T2& second() { return second_; }
    const T1& first() const { return first_; }
    const T2& second() const { return second_; }
};

int main() {
    std::cout << "=== Empty Base Optimization Demo ===\n\n";
    
    // Size comparison
    std::cout << "Size of empty class: " << sizeof(EmptyClass) << " bytes\n";
    std::cout << "Size of int: " << sizeof(int) << " bytes\n\n";
    
    // Without EBO
    std::cout << "VectorWithoutEBO<int, CustomAllocator<int>> size: "
              << sizeof(VectorWithoutEBO<int, CustomAllocator<int>>) << " bytes\n";
    
    // With EBO
    std::cout << "VectorWithEBO<int, CustomAllocator<int>> size: "
              << sizeof(VectorWithEBO<int, CustomAllocator<int>>) << " bytes\n";
    
    // Modern C++20
    std::cout << "VectorModern<int, CustomAllocator<int>> size: "
              << sizeof(VectorModern<int, CustomAllocator<int>>) << " bytes\n\n";
    
    // With stateful allocator (can't optimize)
    std::cout << "With stateful allocator (has data member):\n";
    std::cout << "VectorWithEBO<int, StatefulAllocator<int>> size: "
              << sizeof(VectorWithEBO<int, StatefulAllocator<int>>) << " bytes\n\n";
    
    // Policy example
    std::cout << "=== Policy-Based Design with EBO ===\n";
    PolicyContainer<int, DefaultPolicy> quiet;
    quiet.push_back(42);
    quiet.push_back(100);
    std::cout << "Silent container size: " << sizeof(quiet) << " bytes\n";
    
    PolicyContainer<int, LoggingPolicy> loud;
    loud.push_back(42);
    loud.push_back(100);
    std::cout << "Logging container size: " << sizeof(loud) << " bytes\n";
    std::cout << "Accessing element: ";
    int val = loud[0];
    std::cout << "Got value: " << val << "\n\n";
    
    // Compressed pair example
    std::cout << "=== Compressed Pair ===\n";
    std::cout << "std::pair<int, EmptyClass> size: " 
              << sizeof(std::pair<int, EmptyClass>) << " bytes\n";
    std::cout << "CompressedPair<int, EmptyClass> size: "
              << sizeof(CompressedPair<int, EmptyClass>) << " bytes\n";
    
    CompressedPair<int, EmptyClass> cp(42, EmptyClass{});
    std::cout << "First: " << cp.first() << "\n";
    
    return 0;
}
```