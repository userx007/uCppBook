# Smart Pointers in C++ - Detailed Description

## What Are Smart Pointers?

Smart pointers are template classes in C++ that wrap raw pointers and provide automatic memory management through RAII (Resource Acquisition Is Initialization). They automatically deallocate memory when the smart pointer goes out of scope, preventing memory leaks and dangling pointers.

Introduced in C++11 (with some in Boost before), they're defined in the `<memory>` header and are the modern C++ way to manage dynamic memory.

## The Three Main Types

### 1. `unique_ptr<T>` - Exclusive Ownership

**Concept:** Represents unique ownership of a resource. Cannot be copied, only moved.

**Key Characteristics:**
- Zero overhead compared to raw pointers (same size, no reference counting)
- Non-copyable but movable
- Automatically deletes the owned object when destroyed
- Can use custom deleters
- Can manage arrays with `unique_ptr<T[]>`

**Internal Implementation:**
- Stores a single pointer
- Destructor calls `delete` (or custom deleter) on the pointer
- Copy constructor/assignment are deleted
- Move operations transfer ownership and nullify the source

**Common Usage:**
```cpp
std::unique_ptr<MyClass> ptr = std::make_unique<MyClass>(args);
// Preferred over: new MyClass(args)

// Transfer ownership
std::unique_ptr<MyClass> ptr2 = std::move(ptr); // ptr is now nullptr
```

**When to Use:**
- Default choice for single ownership
- Factory functions returning newly created objects
- Managing resources in class members (RAII)
- Replacing raw `new`/`delete`

### 2. `shared_ptr<T>` - Shared Ownership

**Concept:** Allows multiple pointers to share ownership of an object. Uses reference counting to track how many `shared_ptr` instances point to the resource. Deletes the object when the last `shared_ptr` is destroyed.

**Key Characteristics:**
- Reference counted (atomic for thread safety)
- Copyable and movable
- Larger overhead (2 pointers: object pointer + control block pointer)
- Control block stores: reference count, weak count, deleter, allocator
- Copying increments ref count, destruction decrements it

**Internal Implementation:**
```
shared_ptr contains:
├── Object pointer (T*)
└── Control block pointer
    ├── Strong ref count (shared_ptr count)
    ├── Weak ref count (weak_ptr count)
    ├── Deleter
    └── Allocator
```

**Common Usage:**
```cpp
std::shared_ptr<MyClass> ptr1 = std::make_shared<MyClass>(args);
// Preferred over: std::shared_ptr<MyClass>(new MyClass(args))
// make_shared is more efficient (single allocation)

std::shared_ptr<MyClass> ptr2 = ptr1; // Both own the object
// Object deleted when both ptr1 and ptr2 are destroyed
```

**When to Use:**
- Multiple owners need access to the same object
- Object lifetime managed by multiple components
- Returning objects from functions where ownership is shared
- Thread-safe shared resources (ref counting is atomic)

**Performance Considerations:**
- Reference counting has overhead (atomic operations)
- Control block requires extra allocation (unless using `make_shared`)
- Cache performance may suffer (indirect access through control block)

### 3. `weak_ptr<T>` - Non-Owning Observer

**Concept:** A weak reference to an object owned by `shared_ptr`. Doesn't affect reference count. Prevents circular reference issues.

**Key Characteristics:**
- Doesn't participate in ownership (no ref count increment)
- Can detect if the object still exists
- Must be converted to `shared_ptr` to access the object (`lock()`)
- Increments weak count in the control block

**Common Usage:**
```cpp
std::shared_ptr<MyClass> shared = std::make_shared<MyClass>();
std::weak_ptr<MyClass> weak = shared;

// Later, safely access:
if (auto locked = weak.lock()) {
    // locked is a shared_ptr, object is safe to use
    locked->doSomething();
} else {
    // Object has been deleted
}
```

**When to Use:**
- Breaking circular references (parent-child relationships)
- Caches (cache entries that can expire)
- Observer patterns (observers don't own the subject)
- Callbacks that shouldn't keep objects alive

## Common Patterns and Best Practices

### Factory Functions
```cpp
std::unique_ptr<Widget> createWidget() {
    return std::make_unique<Widget>();
}
```

### Breaking Circular References
```cpp
class Parent {
    std::shared_ptr<Child> child;
};

class Child {
    std::weak_ptr<Parent> parent; // Weak to break cycle
};
```

### Custom Deleters
```cpp
std::unique_ptr<FILE, decltype(&fclose)> file(fopen("file.txt", "r"), &fclose);
std::shared_ptr<SDL_Window> window(SDL_CreateWindow(...), SDL_DestroyWindow);
```

### Prefer `make_unique` and `make_shared`
- More exception-safe
- `make_shared` is more efficient (single allocation)
- Cleaner syntax

## Common Pitfalls

1. **Mixing smart and raw pointers:** Don't create multiple `shared_ptr` from the same raw pointer
2. **Circular references:** Use `weak_ptr` to break cycles
3. **`shared_ptr` to `this`:** Use `enable_shared_from_this` base class
4. **Thread safety:** `shared_ptr` control block is thread-safe, but the pointed-to object is not
5. **Performance:** Don't use `shared_ptr` when `unique_ptr` suffices

---

# Interview Summary - Smart Pointers

## Key Talking Points

**"Smart pointers provide automatic memory management through RAII. The three main types each serve different ownership models: `unique_ptr` for exclusive ownership with zero overhead, `shared_ptr` for shared ownership with reference counting, and `weak_ptr` for non-owning observation."**

## Common Interview Questions

### 1. **Why use smart pointers over raw pointers?**
- Automatic memory management (no manual delete)
- Exception safety (RAII guarantees cleanup)
- Clear ownership semantics
- Prevents memory leaks and dangling pointers
- Modern C++ best practice

### 2. **What's the difference between `unique_ptr` and `shared_ptr`?**
- **Ownership:** `unique_ptr` = exclusive, `shared_ptr` = shared
- **Copyability:** `unique_ptr` is move-only, `shared_ptr` is copyable
- **Performance:** `unique_ptr` has zero overhead, `shared_ptr` has ref counting overhead
- **Size:** `unique_ptr` = 1 pointer, `shared_ptr` = 2 pointers
- **Use case:** `unique_ptr` is default choice, `shared_ptr` when multiple owners needed

### 3. **How does `shared_ptr` reference counting work?**
- Control block stores strong count (shared_ptr) and weak count (weak_ptr)
- Copying increments count, destruction decrements
- When strong count reaches 0, object is deleted
- When weak count reaches 0, control block is deleted
- Reference counting is atomic (thread-safe)

### 4. **What problem does `weak_ptr` solve?**
- Breaks circular references (e.g., parent-child relationships)
- Provides non-owning observation
- Allows checking if object still exists without keeping it alive
- Must use `.lock()` to get `shared_ptr` for safe access

### 5. **What's the circular reference problem?**
```cpp
// Problem: Memory leak
class Node {
    std::shared_ptr<Node> next;
    std::shared_ptr<Node> prev; // Circular reference!
};

// Solution: Use weak_ptr
class Node {
    std::shared_ptr<Node> next;
    std::weak_ptr<Node> prev; // Breaks cycle
};
```

### 6. **Why prefer `make_unique`/`make_shared` over `new`?**
- **Exception safety:** Prevents leaks in function arguments
- **Efficiency:** `make_shared` does single allocation (object + control block)
- **Cleaner code:** No explicit `new`
- **Type safety:** Type inferred from arguments

### 7. **How do you use custom deleters?**
```cpp
// unique_ptr (deleter is part of type)
auto deleter = [](FILE* f) { if(f) fclose(f); };
std::unique_ptr<FILE, decltype(deleter)> file(fopen("f.txt", "r"), deleter);

// shared_ptr (deleter not part of type)
std::shared_ptr<FILE> file(fopen("f.txt", "r"), fclose);
```

### 8. **Can you convert between smart pointer types?**
- `unique_ptr` → `shared_ptr`: Yes (move semantics)
- `shared_ptr` → `unique_ptr`: No (shared ownership can't become exclusive)
- `shared_ptr` ↔ `weak_ptr`: Yes (via constructor and `.lock()`)

### 9. **What's `enable_shared_from_this`?**
Allows an object to safely create `shared_ptr` to itself:
```cpp
class MyClass : public std::enable_shared_from_this<MyClass> {
    void registerCallback() {
        callbacks.push_back(shared_from_this()); // Safe
    }
};
```

### 10. **Are smart pointers thread-safe?**
- `shared_ptr` control block (ref counting) is thread-safe
- The pointed-to object is **not** automatically thread-safe
- Accessing the same object from multiple threads needs synchronization
- Copying/destroying `shared_ptr` from multiple threads is safe

## Quick Comparison Table

| Feature | `unique_ptr` | `shared_ptr` | `weak_ptr` |
|---------|-------------|-------------|-----------|
| Ownership | Exclusive | Shared | None (observer) |
| Copyable | No | Yes | Yes |
| Overhead | Zero | Ref counting | Weak ref count |
| Size | 8 bytes | 16 bytes | 16 bytes |
| Use case | Default | Multiple owners | Break cycles |

## Red Flags to Avoid in Interviews

❌ "Smart pointers are slower, so I avoid them" (wrong mindset)
❌ "I always use `shared_ptr`" (use `unique_ptr` by default)
❌ Mixing raw and smart pointers carelessly
❌ Not mentioning RAII when explaining smart pointers
❌ Forgetting that `weak_ptr` requires `.lock()` before access

## Key Takeaway

**"Use `unique_ptr` by default for single ownership. Use `shared_ptr` only when you genuinely need shared ownership. Use `weak_ptr` to observe without owning, especially to break cycles. Smart pointers embody RAII and are fundamental to modern C++ resource management."**


## Code Sections:

1. **`unique_ptr` Basics** - Creation, moving, arrays, reset/release
2. **Custom Deleters** - File handles and lambda deleters
3. **Factory Pattern** - Returning `unique_ptr` from functions
4. **`shared_ptr` Basics** - Reference counting, copying, conversion
5. **`weak_ptr` Basics** - Observing without owning, `.lock()` usage
6. **Circular Reference Problem** - Shows memory leak scenario
7. **Circular Reference Solution** - Using `weak_ptr` to fix it
8. **Parent-Child Relationship** - Real-world hierarchy example
9. **`enable_shared_from_this`** - Safe `shared_ptr` to `this`
10. **Cache Pattern** - Using `weak_ptr` for expirable cache
11. **Common Pitfalls** - What to avoid
12. **Size Comparison** - Memory overhead of each type

## Key Features:

✅ **Fully compilable** - Copy and run with any C++14+ compiler
✅ **Detailed output** - Shows constructors/destructors to track object lifetimes
✅ **Real patterns** - Factory functions, observers, caches, parent-child
✅ **Best practices** - Uses `make_unique`/`make_shared`, demonstrates RAII
✅ **Common mistakes** - Shows what NOT to do with explanations

Compile with:
```bash
g++ -std=c++14 -o smart_pointers smart_pointers.cpp
./smart_pointers
```

The code demonstrates the differences between the three smart pointer types and when to use each one in practice!

```cpp
#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

// ============================================================================
// 1. BASIC UNIQUE_PTR USAGE
// ============================================================================

class Resource {
    std::string name;
public:
    Resource(const std::string& n) : name(n) {
        std::cout << "Resource '" << name << "' created\n";
    }
    ~Resource() {
        std::cout << "Resource '" << name << "' destroyed\n";
    }
    void use() { std::cout << "Using " << name << "\n"; }
};

void uniquePtrBasics() {
    std::cout << "\n=== UNIQUE_PTR BASICS ===\n";

    // Preferred way: make_unique (C++14)
    auto ptr1 = std::make_unique<Resource>("ptr1");
    ptr1->use();

    // Old way (avoid)
    std::unique_ptr<Resource> ptr2(new Resource("ptr2"));

    // Moving ownership (unique_ptr is move-only)
    auto ptr3 = std::move(ptr1); // ptr1 is now nullptr
    if (!ptr1) {
        std::cout << "ptr1 is now null after move\n";
    }

    // Array support
    auto arr = std::make_unique<int[]>(5);
    arr[0] = 10;
    std::cout << "Array element: " << arr[0] << "\n";

    // reset() to change ownership or delete
    ptr3.reset(new Resource("ptr3-new"));

    // release() to give up ownership (rarely needed)
    Resource* raw = ptr3.release(); // ptr3 is now nullptr, we own raw
    delete raw; // Manual cleanup required

} // ptr2 automatically destroyed here

// ============================================================================
// 2. UNIQUE_PTR WITH CUSTOM DELETERS
// ============================================================================

void customDeleterExample() {
    std::cout << "\n=== CUSTOM DELETERS ===\n";

    // File handle with custom deleter
    auto fileDeleter = [](FILE* f) {
        if (f) {
            std::cout << "Closing file\n";
            fclose(f);
        }
    };

    std::unique_ptr<FILE, decltype(fileDeleter)> file(
        fopen("test.txt", "w"),
        fileDeleter
    );

    if (file) {
        fprintf(file.get(), "Hello from unique_ptr!\n");
    }

    // C++ resource with lambda deleter
    auto deleter = [](Resource* r) {
        std::cout << "Custom delete: ";
        delete r;
    };

    std::unique_ptr<Resource, decltype(deleter)> customPtr(
        new Resource("custom"),
        deleter
    );

} // File and resource automatically cleaned up

// ============================================================================
// 3. FACTORY PATTERN WITH UNIQUE_PTR
// ============================================================================

std::unique_ptr<Resource> createResource(const std::string& name) {
    return std::make_unique<Resource>(name);
}

void factoryPattern() {
    std::cout << "\n=== FACTORY PATTERN ===\n";
    auto res = createResource("factory-created");
    res->use();
}

// ============================================================================
// 4. BASIC SHARED_PTR USAGE
// ============================================================================

void sharedPtrBasics() {
    std::cout << "\n=== SHARED_PTR BASICS ===\n";

    // Preferred: make_shared (more efficient, single allocation)
    auto ptr1 = std::make_shared<Resource>("shared1");
    std::cout << "ptr1 use_count: " << ptr1.use_count() << "\n";

    {
        // Copying increases reference count
        auto ptr2 = ptr1;
        std::cout << "After copy, use_count: " << ptr1.use_count() << "\n";

        auto ptr3 = ptr1;
        std::cout << "After another copy: " << ptr1.use_count() << "\n";

    } // ptr2 and ptr3 destroyed, count decremented

    std::cout << "After scope exit: " << ptr1.use_count() << "\n";

    // Shared pointer with custom deleter
    auto customShared = std::shared_ptr<Resource>(
        new Resource("custom-shared"),
        [](Resource* r) {
            std::cout << "Custom shared deleter: ";
            delete r;
        }
    );

    // Convert unique_ptr to shared_ptr
    auto unique = std::make_unique<Resource>("converted");
    std::shared_ptr<Resource> converted = std::move(unique);
    std::cout << "Converted use_count: " << converted.use_count() << "\n";

} // All resources cleaned up

// ============================================================================
// 5. WEAK_PTR USAGE
// ============================================================================

void weakPtrBasics() {
    std::cout << "\n=== WEAK_PTR BASICS ===\n";

    std::weak_ptr<Resource> weak;

    {
        auto shared = std::make_shared<Resource>("observed");
        weak = shared; // weak doesn't increase ref count

        std::cout << "shared use_count: " << shared.use_count() << "\n";
        std::cout << "weak expired: " << weak.expired() << "\n";

        // Access through weak_ptr using lock()
        if (auto locked = weak.lock()) {
            locked->use();
            std::cout << "Locked use_count: " << locked.use_count() << "\n";
        }

    } // shared destroyed here

    std::cout << "After shared destroyed, weak expired: " << weak.expired() << "\n";

    if (auto locked = weak.lock()) {
        std::cout << "This won't print\n";
    } else {
        std::cout << "Object no longer exists\n";
    }
}

// ============================================================================
// 6. CIRCULAR REFERENCE PROBLEM AND SOLUTION
// ============================================================================

// PROBLEM: Circular reference causes memory leak
class NodeBad {
public:
    std::shared_ptr<NodeBad> next;
    std::shared_ptr<NodeBad> prev; // Creates cycle!
    std::string data;

    NodeBad(const std::string& d) : data(d) {
        std::cout << "NodeBad '" << data << "' created\n";
    }
    ~NodeBad() {
        std::cout << "NodeBad '" << data << "' destroyed\n";
    }
};

void circularReferenceProblem() {
    std::cout << "\n=== CIRCULAR REFERENCE PROBLEM ===\n";

    auto node1 = std::make_shared<NodeBad>("node1");
    auto node2 = std::make_shared<NodeBad>("node2");

    node1->next = node2;
    node2->prev = node1; // Creates circular reference!

    std::cout << "node1 use_count: " << node1.use_count() << "\n";
    std::cout << "node2 use_count: " << node2.use_count() << "\n";

    // Memory leak: nodes won't be destroyed!
}

// SOLUTION: Use weak_ptr to break the cycle
class NodeGood {
public:
    std::shared_ptr<NodeGood> next;
    std::weak_ptr<NodeGood> prev; // Weak breaks the cycle
    std::string data;

    NodeGood(const std::string& d) : data(d) {
        std::cout << "NodeGood '" << data << "' created\n";
    }
    ~NodeGood() {
        std::cout << "NodeGood '" << data << "' destroyed\n";
    }
};

void circularReferenceSolution() {
    std::cout << "\n=== CIRCULAR REFERENCE SOLUTION ===\n";

    auto node1 = std::make_shared<NodeGood>("node1");
    auto node2 = std::make_shared<NodeGood>("node2");

    node1->next = node2;
    node2->prev = node1; // weak_ptr doesn't increase ref count

    std::cout << "node1 use_count: " << node1.use_count() << "\n";
    std::cout << "node2 use_count: " << node2.use_count() << "\n";

    // Properly cleaned up!
}

// ============================================================================
// 7. PARENT-CHILD RELATIONSHIP WITH WEAK_PTR
// ============================================================================

class Child;

class Parent {
public:
    std::string name;
    std::vector<std::shared_ptr<Child>> children;

    Parent(const std::string& n) : name(n) {
        std::cout << "Parent '" << name << "' created\n";
    }
    ~Parent() {
        std::cout << "Parent '" << name << "' destroyed\n";
    }
};

class Child {
public:
    std::string name;
    std::weak_ptr<Parent> parent; // Weak to prevent cycle

    Child(const std::string& n) : name(n) {
        std::cout << "Child '" << name << "' created\n";
    }
    ~Child() {
        std::cout << "Child '" << name << "' destroyed\n";
    }

    void printParent() {
        if (auto p = parent.lock()) {
            std::cout << "My parent is: " << p->name << "\n";
        } else {
            std::cout << "Parent no longer exists\n";
        }
    }
};

void parentChildExample() {
    std::cout << "\n=== PARENT-CHILD RELATIONSHIP ===\n";

    auto parent = std::make_shared<Parent>("Dad");
    auto child1 = std::make_shared<Child>("Alice");
    auto child2 = std::make_shared<Child>("Bob");

    child1->parent = parent;
    child2->parent = parent;

    parent->children.push_back(child1);
    parent->children.push_back(child2);

    child1->printParent();

    // All properly cleaned up due to weak_ptr
}

// ============================================================================
// 8. ENABLE_SHARED_FROM_THIS
// ============================================================================

class Observable : public std::enable_shared_from_this<Observable> {
    std::vector<std::weak_ptr<Observable>> observers;
    std::string name;
public:
    Observable(const std::string& n) : name(n) {}

    void registerAsObserver(std::shared_ptr<Observable> other) {
        // Need to create shared_ptr to 'this'
        // WRONG: std::shared_ptr<Observable>(this) - creates new control block!
        // RIGHT: shared_from_this()
        other->observers.push_back(shared_from_this());
    }

    void notify() {
        std::cout << name << " notifying observers...\n";
        for (auto& weak : observers) {
            if (auto obs = weak.lock()) {
                std::cout << "  Observer still alive\n";
            }
        }
    }
};

void enableSharedFromThisExample() {
    std::cout << "\n=== ENABLE_SHARED_FROM_THIS ===\n";

    auto obj1 = std::make_shared<Observable>("obj1");
    auto obj2 = std::make_shared<Observable>("obj2");

    obj1->registerAsObserver(obj2);
    obj2->notify();
}

// ============================================================================
// 9. CACHE PATTERN WITH WEAK_PTR
// ============================================================================

class ExpensiveObject {
    int id;
public:
    ExpensiveObject(int i) : id(i) {
        std::cout << "ExpensiveObject " << id << " created\n";
    }
    ~ExpensiveObject() {
        std::cout << "ExpensiveObject " << id << " destroyed\n";
    }
    int getId() const { return id; }
};

class Cache {
    std::vector<std::weak_ptr<ExpensiveObject>> cache;
public:
    std::shared_ptr<ExpensiveObject> get(int id) {
        // Clean expired entries
        cache.erase(
            std::remove_if(cache.begin(), cache.end(),
                [](const auto& weak) { return weak.expired(); }),
            cache.end()
        );

        // Check if object exists in cache
        for (auto& weak : cache) {
            if (auto obj = weak.lock()) {
                if (obj->getId() == id) {
                    std::cout << "Cache hit for " << id << "\n";
                    return obj;
                }
            }
        }

        // Cache miss - create new
        std::cout << "Cache miss for " << id << "\n";
        auto obj = std::make_shared<ExpensiveObject>(id);
        cache.push_back(obj);
        return obj;
    }
};

void cachePatternExample() {
    std::cout << "\n=== CACHE PATTERN ===\n";

    Cache cache;

    {
        auto obj1 = cache.get(1);
        auto obj1_again = cache.get(1); // Cache hit

        std::cout << "obj1 use_count: " << obj1.use_count() << "\n";

    } // obj1 destroyed, but weak_ptr in cache remains

    std::cout << "After scope, trying to get object 1:\n";
    auto obj1_later = cache.get(1); // Cache miss (expired)
}

// ============================================================================
// 10. COMMON PITFALLS
// ============================================================================

void commonPitfalls() {
    std::cout << "\n=== COMMON PITFALLS ===\n";

    // PITFALL 1: Creating multiple shared_ptr from same raw pointer
    std::cout << "Pitfall 1: Multiple shared_ptr from raw pointer\n";
    // Resource* raw = new Resource("dangerous");
    // std::shared_ptr<Resource> ptr1(raw);
    // std::shared_ptr<Resource> ptr2(raw); // WRONG! Double delete!
    // Solution: Use make_shared or enable_shared_from_this

    // PITFALL 2: Forgetting to break cycles
    std::cout << "Pitfall 2: See circular reference examples above\n";

    // PITFALL 3: Thread safety misconception
    std::cout << "Pitfall 3: shared_ptr control block is thread-safe,\n";
    std::cout << "           but the object itself is NOT!\n";

    auto shared = std::make_shared<std::vector<int>>();
    // Multiple threads copying shared is safe
    // Multiple threads accessing shared->push_back() needs mutex!

    // PITFALL 4: Performance - don't use shared_ptr everywhere
    std::cout << "Pitfall 4: Use unique_ptr by default, shared_ptr only when needed\n";

    // PITFALL 5: Returning shared_ptr by value is fine (move semantics)
    std::cout << "Pitfall 5: Returning smart pointers by value is efficient (RVO/move)\n";
}

// ============================================================================
// 11. SMART POINTER SIZE COMPARISON
// ============================================================================

void sizeComparison() {
    std::cout << "\n=== SIZE COMPARISON ===\n";
    std::cout << "Raw pointer:   " << sizeof(int*) << " bytes\n";
    std::cout << "unique_ptr:    " << sizeof(std::unique_ptr<int>) << " bytes\n";
    std::cout << "shared_ptr:    " << sizeof(std::shared_ptr<int>) << " bytes\n";
    std::cout << "weak_ptr:      " << sizeof(std::weak_ptr<int>) << " bytes\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    uniquePtrBasics();
    customDeleterExample();
    factoryPattern();
    sharedPtrBasics();
    weakPtrBasics();
    circularReferenceProblem();
    circularReferenceSolution();
    parentChildExample();
    enableSharedFromThisExample();
    cachePatternExample();
    commonPitfalls();
    sizeComparison();

    std::cout << "\n=== PROGRAM END ===\n";
    return 0;
}
```
