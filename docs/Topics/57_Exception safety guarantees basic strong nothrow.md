Comprehensive guide to C++ exception safety guarantees with detailed examples.
The document includes:

- **Basic Guarantee**: Ensures no resource leaks and valid object state, but the state may change in unpredictable ways when exceptions occur

- **Strong Guarantee**: Provides "commit-or-rollback" semantics - either the operation succeeds completely or the state remains exactly as it was before. I've included examples of the copy-and-swap idiom, which is a key technique for achieving this guarantee

- **Nothrow Guarantee**: The strictest level - operations marked `noexcept` that never throw exceptions under any circumstances, crucial for destructors, swap operations, and move semantics

Each section includes practical code examples showing how to implement these guarantees, common pitfalls to avoid, and guidance on when to use each level. The examples progress from simple to complex, demonstrating real-world scenarios like vector implementation, copy-and-swap idiom, and move semantics.

# C++ Exception Safety Guarantees: Basic, Strong, and Nothrow

## Overview

Exception safety refers to the guarantees that a function or operation provides about the state of a program when an exception is thrown. These guarantees are crucial for writing robust, maintainable C++ code, especially when dealing with resource management and complex data structures.

The three standard levels of exception safety are:
1. **Basic Guarantee** (weak guarantee)
2. **Strong Guarantee** (commit-or-rollback)
3. **Nothrow Guarantee** (failure transparency)

---

## 1. Basic Exception Safety Guarantee

### Definition
The basic guarantee ensures that if an exception is thrown:
- No resources are leaked (memory, file handles, locks, etc.)
- All objects remain in a valid state (invariants are maintained)
- The program can continue to function correctly

However, the exact state of objects after an exception may have changed in unpredictable ways.

### Example: Basic Guarantee

```cpp
class BasicSafetyVector {
private:
    int* data;
    size_t size;
    size_t capacity;
    
public:
    BasicSafetyVector(size_t cap) : data(new int[cap]), size(0), capacity(cap) {}
    
    ~BasicSafetyVector() {
        delete[] data;
    }
    
    // Basic exception safety
    void push_back(int value) {
        if (size >= capacity) {
            // Need to reallocate
            size_t new_capacity = capacity * 2;
            int* new_data = new int[new_capacity]; // May throw
            
            // Copy existing elements (may throw if int copy throws)
            for (size_t i = 0; i < size; ++i) {
                new_data[i] = data[i];
            }
            
            // Only now do we modify the object
            delete[] data;
            data = new_data;
            capacity = new_capacity;
        }
        
        data[size++] = value;
    }
};
```

**Analysis**: If `new int[new_capacity]` throws, the object is unchanged. If copying throws partway through, the old data is still intact. No leaks occur because we delete `data` only after successful allocation and copying.

### Basic Guarantee Characteristics
- ✓ No resource leaks
- ✓ Object remains valid and usable
- ✗ Object state may have changed unpredictably
- ✗ Cannot guarantee unchanged state on failure

---

## 2. Strong Exception Safety Guarantee

### Definition
The strong guarantee provides "commit-or-rollback" semantics:
- If the operation succeeds, it completes fully
- If an exception is thrown, the program state is **exactly as it was before the operation**
- This is also called the "transactional guarantee"

### Example: Strong Guarantee

```cpp
#include <memory>
#include <algorithm>

class StrongSafetyVector {
private:
    std::unique_ptr<int[]> data;
    size_t size;
    size_t capacity;
    
public:
    StrongSafetyVector(size_t cap) 
        : data(new int[cap]), size(0), capacity(cap) {}
    
    // Strong exception safety using copy-and-swap idiom
    void push_back(int value) {
        if (size >= capacity) {
            // Create new buffer
            size_t new_capacity = capacity * 2;
            std::unique_ptr<int[]> new_data(new int[new_capacity]);
            
            // Copy all existing data to new buffer
            std::copy(data.get(), data.get() + size, new_data.get());
            
            // Add new element
            new_data[size] = value;
            
            // Commit changes (nothrow operations)
            data = std::move(new_data);
            capacity = new_capacity;
            ++size;
        } else {
            // Simple case - still provides strong guarantee
            data[size++] = value;
        }
    }
    
    // Strong guarantee using copy-and-swap for assignment
    StrongSafetyVector& operator=(const StrongSafetyVector& other) {
        if (this != &other) {
            // Create temporary copy (may throw)
            StrongSafetyVector temp(other.capacity);
            std::copy(other.data.get(), other.data.get() + other.size, 
                     temp.data.get());
            temp.size = other.size;
            
            // Swap with temporary (nothrow)
            swap(temp);
        }
        return *this;
    }
    
private:
    void swap(StrongSafetyVector& other) noexcept {
        std::swap(data, other.data);
        std::swap(size, other.size);
        std::swap(capacity, other.capacity);
    }
};
```

**Analysis**: If any operation throws during `push_back`, none of the member variables have been modified yet. The original state is preserved because we only commit changes after all potentially-throwing operations complete.

### Copy-and-Swap Idiom

The copy-and-swap idiom is a common technique for achieving strong exception safety:

```cpp
class Resource {
private:
    int* data;
    size_t size;
    
public:
    Resource(size_t s) : data(new int[s]), size(s) {}
    
    Resource(const Resource& other) 
        : data(new int[other.size]), size(other.size) {
        std::copy(other.data, other.data + size, data);
    }
    
    ~Resource() {
        delete[] data;
    }
    
    // Strong guarantee assignment using copy-and-swap
    Resource& operator=(Resource other) { // Pass by value (copies)
        swap(other); // Swap with copy (nothrow)
        return *this; // Old data destroyed when 'other' goes out of scope
    }
    
    void swap(Resource& other) noexcept {
        std::swap(data, other.data);
        std::swap(size, other.size);
    }
};
```

### Strong Guarantee Characteristics
- ✓ No resource leaks
- ✓ Object remains valid
- ✓ **Complete rollback on failure** - state is unchanged
- ✗ May require additional resources (copying, temporary buffers)
- ✗ Can be more complex to implement

---

## 3. Nothrow Exception Safety Guarantee

### Definition
The nothrow guarantee means:
- The operation **will not throw any exceptions** under any circumstances
- The operation always succeeds
- This is the strongest guarantee possible

In C++11 and later, functions with this guarantee should be marked `noexcept`.

### Example: Nothrow Guarantee

```cpp
class NoThrowSafetyExample {
private:
    int value;
    int* ptr;
    
public:
    // Destructor must be nothrow (implicitly noexcept)
    ~NoThrowSafetyExample() noexcept {
        delete ptr; // delete never throws
    }
    
    // Swap operations should be nothrow
    void swap(NoThrowSafetyExample& other) noexcept {
        std::swap(value, other.value);
        std::swap(ptr, other.ptr);
    }
    
    // Move constructor - typically nothrow
    NoThrowSafetyExample(NoThrowSafetyExample&& other) noexcept
        : value(other.value), ptr(other.ptr) {
        other.ptr = nullptr; // Leave other in valid state
        other.value = 0;
    }
    
    // Move assignment - typically nothrow
    NoThrowSafetyExample& operator=(NoThrowSafetyExample&& other) noexcept {
        if (this != &other) {
            delete ptr;
            value = other.value;
            ptr = other.ptr;
            other.ptr = nullptr;
            other.value = 0;
        }
        return *this;
    }
    
    // Simple getter - nothrow
    int get_value() const noexcept {
        return value;
    }
    
    // Simple setter - nothrow
    void set_value(int v) noexcept {
        value = v;
    }
};
```

### Critical Nothrow Operations

Certain operations must be nothrow for proper C++ semantics:

```cpp
class CriticalNoThrow {
public:
    // Destructors are implicitly noexcept
    ~CriticalNoThrow() {
        // MUST NOT THROW
        // If a destructor throws during stack unwinding from
        // another exception, std::terminate() is called
    }
    
    // Swap should be noexcept for use in standard algorithms
    void swap(CriticalNoThrow& other) noexcept {
        // Swap operations on built-in types never throw
    }
    
    // Move operations should ideally be noexcept
    // STL containers optimize for noexcept moves
    CriticalNoThrow(CriticalNoThrow&& other) noexcept {
        // Move without allocation or throwing operations
    }
};
```

### Nothrow Guarantee Characteristics
- ✓ Absolutely no exceptions thrown
- ✓ Always succeeds
- ✓ Can be used during stack unwinding
- ✓ Enables compiler optimizations
- ✗ Most restrictive - limits what you can do

---

## Practical Comparison

```cpp
class ComparisonExample {
private:
    std::vector<int> data;
    
public:
    // No guarantee - can leak or corrupt
    void bad_append(int value) {
        data.reserve(data.size() + 1); // May throw
        data.push_back(value);         // May throw
        // If second throw occurs, capacity increased but no element added
    }
    
    // Basic guarantee - no leaks, valid state
    void basic_append(int value) {
        try {
            data.push_back(value); // May throw, but vector stays valid
        } catch (...) {
            // data is valid but may have changed (capacity grew)
            throw;
        }
    }
    
    // Strong guarantee - all or nothing
    void strong_append(int value) {
        auto temp = data;           // Copy current state
        temp.push_back(value);      // Modify copy (may throw)
        data = std::move(temp);     // Commit (noexcept move)
    }
    
    // Nothrow guarantee - cannot fail
    size_t size() const noexcept {
        return data.size();
    }
    
    void clear() noexcept {
        data.clear();
    }
};
```

---

## Choosing the Right Guarantee

### Use **Nothrow** for:
- Destructors (required)
- Swap operations
- Move constructors and move assignment (preferred)
- Simple getters/setters
- Cleanup operations

### Use **Strong** for:
- Assignment operators
- Insert/update operations where rollback is critical
- Operations on transactional systems
- High-level operations where consistency is paramount

### Use **Basic** for:
- Complex operations where strong guarantee is too expensive
- Operations that modify multiple independent objects
- When performance is critical and rollback capability isn't needed
- Low-level operations where maintaining invariants is sufficient

---

## Key Takeaways

1. **Basic Guarantee**: No leaks, valid state, but state may change
2. **Strong Guarantee**: All-or-nothing, complete rollback on failure
3. **Nothrow Guarantee**: Never throws, always succeeds

Understanding and properly implementing these guarantees is essential for writing exception-safe C++ code, especially in libraries and reusable components. The standard library itself provides these guarantees consistently across its containers and algorithms.

Always document which guarantee your functions provide, and prefer stronger guarantees when feasible without sacrificing performance or clarity.