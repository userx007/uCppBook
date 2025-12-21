# Stack vs Heap vs Static Storage in C++

## Overview

C++ provides three primary storage locations for variables, each with distinct characteristics, performance implications, and use cases.

## Stack Storage

### Characteristics
- **Automatic storage duration** - variables are automatically created and destroyed
- **LIFO (Last In, First Out)** structure
- **Fast allocation/deallocation** - simply moving a stack pointer
- **Limited size** - typically 1-8 MB depending on system
- **Memory managed automatically** by compiler
- **Local scope** - variables exist only within their block

### Syntax
```cpp
void function() {
    int x = 10;              // Stack allocation
    double arr[100];         // Stack array
    std::string str = "hi";  // Object on stack
}  // x, arr, and str automatically destroyed here
```

### When to Use
- Local variables with predictable lifetime
- Small to medium-sized data structures
- Function parameters and return values
- When you need automatic cleanup

### Advantages
- Very fast allocation and deallocation
- No memory leaks (automatic cleanup)
- Good cache locality
- No fragmentation

### Disadvantages
- Limited size
- Cannot return addresses of local variables (dangling pointers)
- Fixed size at compile time (for arrays)
- Stack overflow risk with deep recursion or large objects

---

## Heap Storage (Dynamic Memory)

### Characteristics
- **Dynamic storage duration** - exists until explicitly deleted
- **Manual memory management** required (or use smart pointers)
- **Slower allocation/deallocation** - involves memory allocator
- **Large available space** - limited by system RAM
- **Flexible size** - can allocate memory at runtime
- **Global accessibility** - can be accessed anywhere via pointers

### Syntax
```cpp
// Raw pointers (C++03 style - avoid in modern C++)
int* ptr = new int(42);           // Allocate single int
delete ptr;                       // Must manually delete

int* arr = new int[100];          // Allocate array
delete[] arr;                     // Must use delete[] for arrays

// Modern C++ (C++11+) - Preferred
#include <memory>
std::unique_ptr<int> ptr = std::make_unique<int>(42);
std::shared_ptr<int> shared = std::make_shared<int>(100);
// Automatic cleanup when smart pointer goes out of scope

std::vector<int> vec(1000);      // Internally uses heap
```

### When to Use
- Large data structures
- Size unknown at compile time
- Data that must outlive the function that creates it
- When you need to transfer ownership
- Polymorphic objects (base class pointers to derived objects)

### Advantages
- Much larger available memory
- Flexible runtime allocation
- Can easily resize (with reallocation)
- Objects can outlive their creating scope

### Disadvantages
- Slower allocation/deallocation
- Risk of memory leaks if not properly managed
- Risk of fragmentation
- Worse cache locality
- Manual management complexity (without smart pointers)

---

## Static Storage

### Characteristics
- **Static storage duration** - exists for program's entire lifetime
- **Allocated before main() executes**
- **Destroyed after main() completes**
- **Single instance** throughout program execution
- **Zero-initialized by default** (for built-in types)
- **Thread-local option available** (thread_local keyword)

### Types of Static Storage

#### 1. Global Variables
```cpp
int globalVar = 100;  // Static storage, external linkage

void function() {
    globalVar = 200;  // Accessible anywhere
}
```

#### 2. Static Local Variables
```cpp
void counter() {
    static int count = 0;  // Initialized once, persists between calls
    count++;
    std::cout << count << std::endl;
}

// First call: prints 1
// Second call: prints 2
// Third call: prints 3
```

#### 3. Static Class Members
```cpp
class MyClass {
public:
    static int instanceCount;  // Shared by all instances
    
    MyClass() { instanceCount++; }
};

int MyClass::instanceCount = 0;  // Definition outside class
```

#### 4. Static Variables in Functions
```cpp
void initOnce() {
    static bool initialized = false;
    if (!initialized) {
        // Initialization code runs only once
        initialized = true;
    }
}
```

### When to Use
- Global configuration or state
- Singleton patterns
- Counters that persist across function calls
- Data shared between all instances of a class
- Constants with internal linkage (`static const`)

### Advantages
- Persists for entire program lifetime
- Only one copy in memory (efficient for shared data)
- Initialized before program starts (for globals)
- Thread-safe initialization (C++11+)

### Disadvantages
- Increases program startup time
- Can make code harder to test (global state)
- Potential initialization order issues (static initialization fiasco)
- Memory occupied for entire program lifetime
- Can make code less modular

---

## Comparison Table

| Feature | Stack | Heap | Static |
|---------|-------|------|--------|
| **Speed** | Fastest | Slower | Fast (after init) |
| **Size** | Limited (MB) | Large (GB) | As needed |
| **Lifetime** | Automatic | Manual | Program lifetime |
| **Allocation** | Compile-time | Runtime | Before main() |
| **Scope** | Local | Global via pointer | Global or local |
| **Management** | Automatic | Manual/Smart ptrs | Automatic |
| **Fragmentation** | None | Possible | None |

---

## Memory Layout in C++ Programs

```
High Memory
┌─────────────────┐
│   Stack         │  Local variables, function calls
│   ↓ grows down  │
├─────────────────┤
│                 │
│   Free Memory   │
│                 │
├─────────────────┤
│   ↑ grows up    │
│   Heap          │  Dynamic allocations (new/malloc)
├─────────────────┤
│   BSS Segment   │  Uninitialized static/global variables
├─────────────────┤
│   Data Segment  │  Initialized static/global variables
├─────────────────┤
│   Text Segment  │  Program code (read-only)
└─────────────────┘
Low Memory
```

---

## Best Practices

### Stack
- Prefer stack allocation for small, local objects
- Be cautious with large arrays or deep recursion
- Use stack for RAII (Resource Acquisition Is Initialization) pattern

### Heap
- Use smart pointers (`unique_ptr`, `shared_ptr`) instead of raw pointers
- Prefer `std::vector` over raw arrays
- Use `make_unique` and `make_shared` for exception safety
- Profile for performance-critical code

### Static
- Avoid global variables when possible (prefer dependency injection)
- Use `static` for constants with internal linkage
- Be aware of initialization order issues
- Consider thread safety for static variables

---

## Common Pitfalls

### Dangling Pointers (Stack)
```cpp
int* dangling() {
    int x = 42;
    return &x;  // ERROR: x destroyed when function returns
}
```

### Memory Leaks (Heap)
```cpp
void leak() {
    int* ptr = new int(42);
    // Forgot to delete ptr - memory leak!
}
```

### Static Initialization Fiasco
```cpp
// file1.cpp
static MyClass obj1;

// file2.cpp
static MyClass obj2(obj1);  // Undefined: which initializes first?
```

---

## Example: All Three Together

```cpp
#include <iostream>
#include <memory>

// Static storage - global variable
int globalCounter = 0;

class Example {
    static int instanceCount;  // Static storage - class member
    
public:
    Example() {
        instanceCount++;
        globalCounter++;
    }
    
    static int getCount() { return instanceCount; }
};

int Example::instanceCount = 0;

void demonstrate() {
    // Stack storage
    int stackVar = 10;
    Example stackObj;
    
    // Heap storage
    auto heapObj = std::make_unique<Example>();
    
    // Static storage - local static
    static int callCount = 0;
    callCount++;
    
    std::cout << "Stack variable: " << stackVar << std::endl;
    std::cout << "Call count: " << callCount << std::endl;
    std::cout << "Instance count: " << Example::getCount() << std::endl;
    std::cout << "Global counter: " << globalCounter << std::endl;
}

int main() {
    demonstrate();  // callCount: 1, instances: 2, global: 2
    demonstrate();  // callCount: 2, instances: 4, global: 4
    return 0;
}
```

---

## Choosing the Right Storage

**Use Stack when:**
- Object lifetime matches scope
- Size is small and known at compile time
- You want automatic cleanup
- Performance is critical

**Use Heap when:**
- Size is large or unknown until runtime
- Object must outlive creating scope
- You need dynamic resizing
- Working with polymorphic types

**Use Static when:**
- Need single instance across entire program
- Sharing data between function calls
- Class-wide shared data
- Program configuration/constants