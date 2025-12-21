I've created a comprehensive guide on low-level memory manipulation in C++ covering both `memcpy` and placement new. The document includes:

**For `memcpy`:**
- Basic description and function signature
- When it's safe to use (only for trivially copyable types)
- Multiple examples: array copying, struct copying, buffer manipulation
- A warning example showing what NOT to do with complex types

**For placement new:**
- How it separates memory allocation from object construction
- Practical examples including memory pools, object reuse, and array construction
- Important details about manual destructor calls and proper cleanup

**Additional sections:**
- Comparison table showing when to use each technique
- Best practices and safety guidelines
- Modern C++ alternatives (like `std::construct_at`)
- Common pitfalls to avoid

Each concept has progressively complex examples to help understand real-world applications. The guide emphasizes safety concerns and when to avoid these low-level techniques in favor of higher-level C++ abstractions.

# Low-Level Memory Manipulation in C++

## Overview

Low-level memory manipulation in C++ allows direct control over memory layout and object construction. The two primary techniques are `memcpy` for raw byte copying and placement new for constructing objects in pre-allocated memory.

## memcpy

### Description

`memcpy` is a C standard library function that copies a block of memory from one location to another. It performs a byte-by-byte copy without any knowledge of object semantics, constructors, or destructors.

```cpp
void* memcpy(void* dest, const void* src, size_t count);
```

### When to Use

- Copying POD (Plain Old Data) types
- Working with raw byte buffers
- High-performance bulk data transfer
- Serialization of trivially copyable types

### Important Constraints

**Only safe for trivially copyable types** - types without:
- Virtual functions
- Non-trivial constructors/destructors
- Non-trivial copy/move operations
- Virtual base classes

### Example 1: Basic Array Copy

```cpp
#include <cstring>
#include <iostream>

int main() {
    int source[5] = {1, 2, 3, 4, 5};
    int dest[5];
    
    // Copy 5 integers (5 * sizeof(int) bytes)
    memcpy(dest, source, 5 * sizeof(int));
    
    for (int i = 0; i < 5; i++) {
        std::cout << dest[i] << " ";  // Output: 1 2 3 4 5
    }
    
    return 0;
}
```

### Example 2: Struct Copy

```cpp
#include <cstring>
#include <iostream>

struct Point {
    int x, y, z;
};

int main() {
    Point p1 = {10, 20, 30};
    Point p2;
    
    // Safe because Point is trivially copyable
    memcpy(&p2, &p1, sizeof(Point));
    
    std::cout << "p2: " << p2.x << ", " << p2.y << ", " << p2.z << "\n";
    // Output: p2: 10, 20, 30
    
    return 0;
}
```

### Example 3: Buffer Manipulation

```cpp
#include <cstring>
#include <iostream>

int main() {
    char buffer[100];
    const char* msg = "Hello, World!";
    
    // Copy string into buffer
    memcpy(buffer, msg, strlen(msg) + 1);
    
    std::cout << buffer << "\n";  // Output: Hello, World!
    
    return 0;
}
```

### ⚠️ Dangerous: Don't Use with Complex Types

```cpp
#include <string>
#include <vector>

struct BadExample {
    std::string name;  // Has non-trivial constructor/destructor
    std::vector<int> data;
};

int main() {
    BadExample obj1;
    BadExample obj2;
    
    // DANGEROUS! Will corrupt memory, cause crashes
    // memcpy(&obj2, &obj1, sizeof(BadExample));  // DON'T DO THIS!
    
    // Correct way:
    obj2 = obj1;  // Use assignment operator
    
    return 0;
}
```

## Placement New

### Description

Placement new constructs an object at a specific memory address without allocating new memory. It separates memory allocation from object construction, giving precise control over where objects live in memory.

```cpp
new (address) Type(constructor_args);
```

### When to Use

- Custom memory allocators
- Memory pools
- Object reuse without deallocation
- Embedded systems with fixed memory regions
- Performance-critical code avoiding allocation overhead

### Example 1: Basic Placement New

```cpp
#include <iostream>
#include <new>  // Required for placement new

class Widget {
public:
    Widget(int val) : value(val) {
        std::cout << "Widget constructed with " << value << "\n";
    }
    
    ~Widget() {
        std::cout << "Widget destructed\n";
    }
    
    int value;
};

int main() {
    // Pre-allocate raw memory
    alignas(Widget) char buffer[sizeof(Widget)];
    
    // Construct Widget in the buffer
    Widget* widget = new (buffer) Widget(42);
    
    std::cout << "Widget value: " << widget->value << "\n";
    
    // Must manually call destructor
    widget->~Widget();
    
    // No delete needed - we didn't allocate with new
    
    return 0;
}
```

### Example 2: Memory Pool

```cpp
#include <iostream>
#include <new>

template<typename T, size_t N>
class ObjectPool {
    alignas(T) char buffer[N * sizeof(T)];
    bool used[N] = {false};
    
public:
    T* allocate() {
        for (size_t i = 0; i < N; i++) {
            if (!used[i]) {
                used[i] = true;
                return reinterpret_cast<T*>(buffer + i * sizeof(T));
            }
        }
        return nullptr;
    }
    
    void deallocate(T* ptr) {
        size_t index = (reinterpret_cast<char*>(ptr) - buffer) / sizeof(T);
        if (index < N) {
            used[index] = false;
        }
    }
};

struct Data {
    int x;
    Data(int val) : x(val) { 
        std::cout << "Data(" << x << ") constructed\n"; 
    }
    ~Data() { 
        std::cout << "Data(" << x << ") destructed\n"; 
    }
};

int main() {
    ObjectPool<Data, 5> pool;
    
    // Allocate memory and construct object
    Data* d1 = pool.allocate();
    new (d1) Data(100);
    
    Data* d2 = pool.allocate();
    new (d2) Data(200);
    
    std::cout << "d1: " << d1->x << ", d2: " << d2->x << "\n";
    
    // Destroy and deallocate
    d1->~Data();
    pool.deallocate(d1);
    
    d2->~Data();
    pool.deallocate(d2);
    
    return 0;
}
```

### Example 3: Object Reuse

```cpp
#include <iostream>
#include <new>
#include <string>

class Connection {
public:
    Connection(const std::string& host) : hostname(host) {
        std::cout << "Connecting to " << hostname << "...\n";
    }
    
    ~Connection() {
        std::cout << "Disconnecting from " << hostname << "\n";
    }
    
    std::string hostname;
};

int main() {
    alignas(Connection) char buffer[sizeof(Connection)];
    
    // First use
    Connection* conn = new (buffer) Connection("server1.com");
    // Use connection...
    conn->~Connection();  // Destroy
    
    // Reuse same memory for different connection
    conn = new (buffer) Connection("server2.com");
    // Use connection...
    conn->~Connection();  // Destroy
    
    return 0;
}
```

### Example 4: Array with Placement New

```cpp
#include <iostream>
#include <new>

class Item {
public:
    Item(int id) : id(id) {
        std::cout << "Item " << id << " created\n";
    }
    
    ~Item() {
        std::cout << "Item " << id << " destroyed\n";
    }
    
    int id;
};

int main() {
    const size_t count = 3;
    alignas(Item) char buffer[count * sizeof(Item)];
    
    // Construct array of Items
    Item* items = reinterpret_cast<Item*>(buffer);
    for (size_t i = 0; i < count; i++) {
        new (&items[i]) Item(i);
    }
    
    // Use items
    for (size_t i = 0; i < count; i++) {
        std::cout << "Item " << items[i].id << "\n";
    }
    
    // Destroy in reverse order
    for (size_t i = count; i > 0; i--) {
        items[i-1].~Item();
    }
    
    return 0;
}
```

## Comparison and Best Practices

### memcpy vs Placement New

| Feature | memcpy | Placement New |
|---------|--------|---------------|
| Purpose | Copy raw bytes | Construct objects |
| Calls constructors | No | Yes |
| Safe for complex types | No | Yes |
| Performance | Very fast | Constructor overhead |
| Use case | POD types, buffers | Any type |

### Best Practices

**For memcpy:**
- Only use with trivially copyable types
- Check with `std::is_trivially_copyable`
- Prefer standard copy for complex types
- Ensure source and destination don't overlap (use `memmove` if they might)

**For placement new:**
- Always call destructor manually
- Ensure proper alignment with `alignas`
- Don't call `delete` on placement new objects
- Match construction order with reverse destruction order
- Be careful with exception safety

### Modern C++ Alternatives

```cpp
#include <memory>
#include <type_traits>

// Check if memcpy is safe
template<typename T>
void safe_copy(T& dest, const T& src) {
    if constexpr (std::is_trivially_copyable_v<T>) {
        memcpy(&dest, &src, sizeof(T));
    } else {
        dest = src;  // Use assignment
    }
}

// Use std::construct_at (C++20) instead of placement new
#include <memory>

int main() {
    alignas(int) char buffer[sizeof(int)];
    int* ptr = std::construct_at(reinterpret_cast<int*>(buffer), 42);
    std::destroy_at(ptr);
}
```

## Common Pitfalls

1. **Using memcpy on non-POD types** - Causes undefined behavior
2. **Forgetting to call destructor after placement new** - Memory leaks
3. **Calling delete on placement new objects** - Double free/corruption
4. **Alignment issues** - Use `alignas` to ensure proper alignment
5. **Exception safety** - If constructor throws, memory state is undefined
6. **Overlapping memory with memcpy** - Use `memmove` instead

## Conclusion

Low-level memory manipulation provides powerful control but requires careful handling. Use `memcpy` only for trivially copyable types and prefer placement new when constructors/destructors matter. In modern C++, consider higher-level abstractions like smart pointers and standard containers unless you have specific performance or control requirements.