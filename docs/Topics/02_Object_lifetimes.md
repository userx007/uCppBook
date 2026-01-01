# C++ Object Lifetime & Storage Duration

## Core Concepts

**Storage duration** determines *when* storage is allocated/deallocated for an object, while **lifetime** determines *when* the object's constructor and destructor run.

## Storage Durations

### 1. **Automatic Storage Duration**
Objects declared in block scope (without `static`, `extern`, or `thread_local`):

```cpp
void function() {
    int x = 5;           // automatic storage
    std::string s("hi"); // automatic storage
    // destroyed when leaving scope (LIFO order)
}
```

- Storage allocated on function entry
- Destroyed in reverse order of construction when scope exits
- Includes function parameters and temporaries

### 2. **Static Storage Duration**
Objects that exist for the program's entire duration:

```cpp
int global = 42;              // static storage
static int file_scope = 10;   // static storage

void function() {
    static int counter = 0;   // static storage, initialized once
    counter++;
}
```

- **Global variables**: Initialized before `main()` starts
- **Static local variables**: Initialized on first execution (thread-safe in C++11+)
- Destroyed in reverse order of initialization when program exits
- Zero-initialized if no initializer provided

### 3. **Thread Storage Duration** (C++11)
One instance per thread:

```cpp
thread_local int tls_var = 0; // separate copy per thread

void thread_function() {
    thread_local std::string buffer; // separate per thread
}
```

- Initialized when thread starts (or on first use for block scope)
- Destroyed when thread exits

### 4. **Dynamic Storage Duration**
Manually managed memory:

```cpp
int* p = new int(42);      // dynamic allocation
delete p;                   // manual deallocation

Widget* w = new Widget();
delete w;                   // destructor called, then memory freed
```

- Allocated with `new`, `new[]`, `malloc()`, etc.
- **Must** be explicitly deallocated
- Lifetime controlled by programmer (or smart pointers)

## Object Lifetime

An object's lifetime begins when:
1. Storage is obtained
2. Initialization is complete

Lifetime ends when:
- Destructor is called (for class types)
- Storage is released

### Important Distinctions

```cpp
{
    std::string s;           // lifetime: constructor → end of scope

    int* p = new int(5);     // p's lifetime: here → end of scope
                             // *p's lifetime: here → delete
    delete p;                // *p's lifetime ends
}                            // p's lifetime ends (pointer itself)
```

## Temporal Dead Zones

Using an object outside its lifetime is undefined behavior:

```cpp
int* dangling() {
    int x = 42;
    return &x;  // BAD: x's lifetime ends at '}'
}

int* ptr = dangling();
*ptr = 10;      // UB: accessing dead object
```

## Temporaries

Temporary objects have special lifetime rules:

```cpp
std::string getName() { return "Alice"; }

const std::string& ref = getName();  // lifetime extended!
// Temporary lives until ref goes out of scope

std::string&& rref = getName();      // also extends lifetime (C++11)

getName().size();  // temporary destroyed at end of full expression
```

**Key rule**: Binding a temporary to a `const&` or `&&` extends its lifetime to match the reference's scope.

## Common Pitfalls

```cpp
// 1. Dangling pointers/references
std::string* bad() {
    std::string local = "temp";
    return &local;  // UB: returns address of destroyed object
}

// 2. Use after free
int* p = new int(5);
delete p;
*p = 10;  // UB: accessing freed memory

// 3. Double delete
delete p;
delete p;  // UB: deleting already freed memory

// 4. Mixing new/delete with malloc/free
int* x = new int(5);
free(x);  // UB: mismatched allocation/deallocation
```

## Best Practices

1. **Prefer automatic storage** (stack allocation) when possible
2. **Use RAII** (Resource Acquisition Is Initialization) for resource management
3. **Use smart pointers** (`unique_ptr`, `shared_ptr`) instead of raw `new`/`delete`
4. **Avoid returning references/pointers to local objects**
5. **Initialize variables at declaration** to avoid using uninitialized objects

```cpp
// Good: RAII with smart pointers
std::unique_ptr<Widget> w = std::make_unique<Widget>();
// automatically deleted when w goes out of scope

// Good: value semantics
std::vector<int> vec = {1, 2, 3};
// automatically cleaned up
```

Understanding these concepts is crucial for writing safe, leak-free C++ code and avoiding undefined behavior.

Great question! Let's clarify what's happening here:

## What Actually Happens

```cpp
std::string getName() { return "Alice"; }

const std::string& ref = getName();  // NO copy, NO move
std::string&& rref = getName();      // NO copy, NO move
getName().size();                     // NO copy, NO move
```

**All three cases bind directly to the temporary** - no copy or move occurs!

## The Details

### Return Value Optimization (RVO)

```cpp
std::string getName() { return "Alice"; }
```

Modern compilers use **RVO (Return Value Optimization)** or **copy elision**:
- The string `"Alice"` is constructed **directly in the caller's space**
- No temporary is created in `getName()`
- No copy or move happens

This is **mandatory** in C++17+ for this case (it's a prvalue).

### Binding to References

```cpp
const std::string& ref = getName();
```

- The temporary returned by `getName()` is created
- `ref` **binds directly** to that temporary (no copy/move)
- The temporary's lifetime is **extended** to match `ref`'s scope

```cpp
std::string&& rref = getName();
```

- Same story: `rref` binds directly to the temporary
- No move operation occurs (moving requires calling the move constructor)
- Lifetime is extended

### The Key Insight

```cpp
// This involves NO operations:
const std::string& ref = getName();

// THIS would involve a move:
std::string value = getName();  // move constructor called
```

## Demonstrating with a Custom Class

```cpp
#include <iostream>
#include <string>

struct Tracker {
    std::string name;

    Tracker(const char* n) : name(n) {
        std::cout << "Constructor: " << name << "\n";
    }

    Tracker(const Tracker& other) : name(other.name) {
        std::cout << "Copy constructor: " << name << "\n";
    }

    Tracker(Tracker&& other) : name(std::move(other.name)) {
        std::cout << "Move constructor: " << name << "\n";
    }

    ~Tracker() {
        std::cout << "Destructor: " << name << "\n";
    }
};

Tracker getTracker() {
    return Tracker("temp");
}

int main() {
    std::cout << "=== Binding to const ref ===\n";
    const Tracker& ref = getTracker();
    std::cout << "Using ref\n";

    std::cout << "\n=== Binding to rvalue ref ===\n";
    Tracker&& rref = getTracker();
    std::cout << "Using rref\n";

    std::cout << "\n=== Taking by value ===\n";
    Tracker value = getTracker();
    std::cout << "Using value\n";
}
```

**Output:**
```
=== Binding to const ref ===
Constructor: temp
Using ref

=== Binding to rvalue ref ===
Constructor: temp
Using rref

=== Taking by value ===
Constructor: temp
Using value
```

Notice: only **one constructor** call in each case! The references bind directly to the temporary without any copy or move.

## Summary

| Expression | Copy? | Move? | Notes |
|------------|-------|-------|-------|
| `const T& r = getT();` | ❌ | ❌ | Direct binding, lifetime extended |
| `T&& r = getT();` | ❌ | ❌ | Direct binding, lifetime extended |
| `T val = getT();` | ❌ | ✅* | Move constructor called (or elided) |
| `getT().method();` | ❌ | ❌ | Temporary destroyed after expression |

*In C++17+, even the move might be elided depending on the context.*

The key takeaway: **binding a reference to a temporary is just aliasing** - it creates another name for the same object, with no copying or moving involved.

This expression can be named in several ways depending on what aspect you want to emphasize:

## Common Names

### 1. **Lifetime Extension** (most common)
```cpp
const T& r = getT();  // lifetime extension of temporary
```
This emphasizes that the temporary's lifetime is being extended to match the reference's scope.

### 2. **Binding a Reference to a Temporary**
```cpp
const T& r = getT();  // binding reference to temporary
```
This is the most literal/technical description.

### 3. **Const Reference Binding** or **Reference Binding**
```cpp
const T& r = getT();  // const reference binding
```
Emphasizes the mechanism being used.

### 4. **Temporary Materialization** (C++17 terminology)
```cpp
const T& r = getT();  // temporary materialization and binding
```
In C++17+, this involves creating a temporary object from a prvalue (the technical term is "temporary materialization conversion").

## In Different Contexts

**When discussing optimization:**
- "Taking by const reference" (to avoid copies)
```cpp
void process(const T& obj);  // taking by const reference
```

**When discussing safety:**
- "Dangling reference" (when lifetime extension doesn't apply)
```cpp
const T& ref = getT();  // safe: lifetime extended
const T& ref2 = some_container.front();  // potentially dangling!
```

**When discussing forwarding:**
- "Forwarding reference" or "universal reference" (when using `T&&` in template context)
```cpp
template<typename T>
void func(T&& arg);  // forwarding/universal reference
```

## The Most Precise Term

In formal C++ terminology, this is:

**"Binding a const lvalue reference to a temporary, causing lifetime extension"**

But in everyday usage, programmers typically just say:
- **"Lifetime extension"** (when emphasizing the lifetime aspect)
- **"Binding to a temporary"** (when explaining what's happening)
- **"Taking by const ref"** (when discussing function parameters)

So if you're writing documentation or discussing with other C++ programmers, **"lifetime extension"** is probably the most recognized and concise term for this pattern.

This expression can be named:

## Common Names

### 1. **Rvalue Reference Binding** (most common)
```cpp
T&& r = getT();  // rvalue reference binding
```
This is the most straightforward technical term.

### 2. **Lifetime Extension via Rvalue Reference**
```cpp
T&& r = getT();  // lifetime extension via rvalue reference
```
Emphasizes that the temporary's lifetime is extended (same as with const lvalue references).

### 3. **Binding an Rvalue Reference to a Temporary**
```cpp
T&& r = getT();  // binding rvalue reference to temporary
```
Most literal/descriptive.

### 4. **Move Semantics Setup** (contextual)
```cpp
T&& r = getT();  // preparing for move
auto value = std::move(r);  // actual move happens here
```
When used as a precursor to moving.

## Important Distinction

**This is NOT the same as a "Forwarding Reference" or "Universal Reference":**

```cpp
// Regular rvalue reference (non-template)
std::string&& r = getName();  // rvalue reference binding

// Forwarding/Universal reference (template context)
template<typename T>
void func(T&& arg);  // forwarding reference / universal reference
```

The key difference:
- `T&&` where `T` is a **concrete type** → **rvalue reference**
- `T&&` where `T` is a **template parameter** → **forwarding reference** (can bind to lvalue or rvalue)

## In Different Contexts

**When discussing move semantics:**
```cpp
T&& r = getT();  // "capturing an rvalue reference"
```

**When discussing reference categories:**
```cpp
T&& r = getT();  // "rvalue reference" (as opposed to lvalue reference)
```

**When explaining lifetime:**
```cpp
T&& r = getT();  // "extending temporary's lifetime with rvalue reference"
```

## The Most Precise Term

In formal C++ terminology:

**"Binding an rvalue reference to a temporary, causing lifetime extension"**

In everyday usage:
- **"Rvalue reference binding"** (most common)
- **"Binding to an rvalue"** (slightly less formal)
- **"Taking by rvalue reference"** (for function parameters)

## Comparison

```cpp
const T& r1 = getT();  // const lvalue reference binding (lifetime extension)
T&& r2 = getT();       // rvalue reference binding (lifetime extension)
```

Both extend lifetime, but `T&&` also allows you to move from the object:

```cpp
T&& r = getT();
auto moved = std::move(r);  // can move from r
```

So **"rvalue reference binding"** is the standard term you'll see in C++ documentation and discussions.

