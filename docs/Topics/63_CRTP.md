The **CRTP (Curiously Recurring Template Pattern)** is a C++ idiom where a class derives from a template instantiated with the derived class itself:

```cpp
template <typename Derived>
class Base { };

class Derived : public Base<Derived> { };
```

It looks strange at first, but it enables **compile-time polymorphism** and several powerful patterns that are hard or impossible with classic inheritance.

Below are the **main use cases**, **why CRTP makes sense**, and **its advantages**.

---

## 1. Static (Compile-Time) Polymorphism

### Replace virtual functions when runtime polymorphism isn’t needed

### Problem with virtual functions

```cpp
struct Shape {
    virtual double area() const = 0;
};
```

* Requires vtable
* Runtime dispatch
* Prevents inlining
* Adds indirection cost

### CRTP solution

```cpp
template <typename Derived>
struct Shape {
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
};

struct Circle : Shape<Circle> {
    double area_impl() const { return 3.14 * r * r; }
    double r;
};
```

### Why CRTP works here

* Dispatch happens at **compile time**
* No virtual functions
* Calls can be **fully inlined**

### When this makes sense

✔ Performance-critical code
✔ No need to store objects via base pointers
✔ Known types at compile time

---

## 2. Mixins / Code Reuse Without Virtual Inheritance

### Share behavior across unrelated types

CRTP is often used to create **reusable behavior blocks**.

```cpp
template <typename Derived>
class Printable {
public:
    void print() const {
        static_cast<const Derived*>(this)->print_impl();
    }
};
```

```cpp
class User : public Printable<User> {
public:
    void print_impl() const {
        std::cout << "User\n";
    }
};
```

### Benefits

* No virtual base class
* No diamond inheritance issues
* Zero runtime overhead

### When this makes sense

✔ Reusable functionality
✔ Behavior depends on derived class
✔ You want “traits with behavior”

---

## 3. Interface Enforcement at Compile Time

### Enforce APIs without virtual functions

CRTP can enforce that a derived class implements specific functions.

```cpp
template <typename Derived>
class Serializer {
public:
    void serialize() {
        static_cast<Derived*>(this)->serialize_impl();
    }
};
```

If `serialize_impl()` is missing → **compile-time error**.

### Advantages over pure virtual interfaces

| Virtual Interface          | CRTP                   |
| -------------------------- | ---------------------- |
| Runtime check              | Compile-time check     |
| Requires vtable            | No vtable              |
| Can be violated at runtime | Guaranteed correctness |

### When this makes sense

✔ Library APIs
✔ Strong compile-time guarantees
✔ Header-only libraries

---

## 4. Static Polymorphic Containers (Policy-Based Design)

Used heavily in **STL-like designs** and **policy classes**.

```cpp
template <typename Derived, typename LoggerPolicy>
class Service : private LoggerPolicy {
public:
    void execute() {
        this->log("executing");
        static_cast<Derived*>(this)->execute_impl();
    }
};
```

### Benefits

* Behavior selected at compile time
* No runtime overhead
* Easy customization

### When this makes sense

✔ Highly configurable systems
✔ Zero-cost abstractions
✔ Embedded / performance-sensitive code

---

## 5. Enable/Disable Features at Compile Time

### Static feature composition

```cpp
template <typename Derived>
class Comparable {
public:
    bool operator!=(const Derived& other) const {
        return !static_cast<const Derived*>(this)->equals(other);
    }
};
```

Derived class only implements `equals()`.

### Why CRTP is good here

* Avoids code duplication
* Adds operators automatically
* No runtime cost

---

## 6. Implementing Object Counters or Shared Static State

### Per-type static data

```cpp
template <typename Derived>
class InstanceCounter {
public:
    InstanceCounter() { ++count; }
    ~InstanceCounter() { --count; }

    static inline int count = 0;
};
```

```cpp
class Foo : public InstanceCounter<Foo> {};
class Bar : public InstanceCounter<Bar> {};
```

Each type gets its **own static counter**.

### Use case

✔ Per-type metadata
✔ No global maps or RTTI

---

## Advantages of CRTP (Summary)

### ✔ Zero Runtime Overhead

* No vtables
* No virtual dispatch

### ✔ Better Optimization

* Inlining
* Dead code elimination
* Constant propagation

### ✔ Strong Compile-Time Guarantees

* Missing methods cause compile errors
* No runtime surprises

### ✔ Flexible Composition

* Mixins
* Policies
* Feature injection

---

## Downsides / When NOT to Use CRTP

### ❌ Increased Complexity

* Harder to read
* Confusing error messages

### ❌ Code Bloat

* Each instantiation generates code

### ❌ No Runtime Polymorphism

* Cannot store heterogeneous objects in a single container via base pointer

### ❌ Tight Coupling

* Base class depends on derived class interface

---

## CRTP vs Virtual Functions (Quick Comparison)

| Feature               | CRTP         | Virtual |
| --------------------- | ------------ | ------- |
| Dispatch              | Compile-time | Runtime |
| Performance           | ⭐⭐⭐⭐         | ⭐⭐      |
| Flexibility           | Lower        | Higher  |
| Object slicing safety | Compile-time | Runtime |
| Use in containers     | ❌            | ✔       |

---

## Rule of Thumb

Use **CRTP** when:

* You know all types at compile time
* Performance matters
* You want zero-cost abstractions

Use **virtual functions** when:

* Runtime polymorphism is required
* Types are not known at compile time
* Simplicity is more important than speed

---

1. **Real STL / standard-library CRTP examples**
2. **CRTP vs Concepts (C++20)**
3. **Common CRTP pitfalls (with fixes)**

---

# 1️⃣ Real STL Examples of CRTP

The STL uses CRTP **everywhere**, even if it’s not obvious at first glance.

---

## 1.1 `std::iterator` (pre-C++17 classic example)

> *Now deprecated, but historically important.*

```cpp
template<
    class Category,
    class T,
    class Distance = std::ptrdiff_t,
    class Pointer = T*,
    class Reference = T&
>
struct iterator { };
```

User iterators inherited from it:

```cpp
class MyIter : public std::iterator<
    std::forward_iterator_tag, int
> { };
```

➡ This is **CRTP-like behavior**, but incomplete.
Modern STL replaced this with **traits + concepts** because CRTP caused issues.

---

## 1.2 `std::enable_shared_from_this` (REAL CRTP)

```cpp
class Foo : public std::enable_shared_from_this<Foo> {
public:
    std::shared_ptr<Foo> get() {
        return shared_from_this();
    }
};
```

### Why CRTP is needed here

* `enable_shared_from_this<T>` must know the **exact derived type**
* Each type needs its **own weak_ptr**

```cpp
template<class T>
class enable_shared_from_this {
protected:
    mutable std::weak_ptr<T> weak_this;
};
```

✔ Zero runtime cost
✔ Per-type storage
✔ Compile-time correctness

---

## 1.3 `std::ranges::view_interface` (Modern C++20 CRTP)

```cpp
template<class Derived>
class view_interface {
public:
    bool empty() const {
        return static_cast<const Derived&>(*this).begin()
             == static_cast<const Derived&>(*this).end();
    }
};
```

Used like:

```cpp
class MyView : public std::ranges::view_interface<MyView> {
public:
    int* begin();
    int* end();
};
```

### Why CRTP is perfect here

* Adds default implementations
* Uses derived’s `begin()/end()`
* No virtual dispatch
* Fully inlineable

➡ **This is the canonical modern CRTP use case**

---

## 1.4 `std::ranges::range_adaptor_closure` (Advanced CRTP)

Used for pipe syntax:

```cpp
range | views::transform(f) | views::filter(p)
```

Internally uses CRTP to:

* Compose adaptors
* Preserve type information
* Avoid runtime polymorphism

---

# 2️⃣ CRTP vs Concepts (C++20)

This is *very important*: **Concepts do NOT replace CRTP** — they solve *different problems*.

---

## 2.1 What CRTP Does

CRTP provides:

* **Behavior**
* **Implementation reuse**
* **Static polymorphism**

```cpp
template<typename Derived>
struct Printable {
    void print() const {
        static_cast<const Derived*>(this)->print_impl();
    }
};
```

---

## 2.2 What Concepts Do

Concepts provide:

* **Constraints**
* **Readable diagnostics**
* **API validation**

```cpp
template<typename T>
concept Printable = requires(const T& t) {
    t.print();
};
```

---

## 2.3 CRTP vs Concepts Side-by-Side

### Same problem, different tools

#### Using CRTP

```cpp
template<typename Derived>
struct Shape {
    double area() const {
        return static_cast<const Derived*>(this)->area_impl();
    }
};
```

#### Using Concepts

```cpp
template<typename T>
concept Shape = requires(const T& s) {
    { s.area() } -> std::same_as<double>;
};
```

| Aspect                  | CRTP   | Concepts |
| ----------------------- | ------ | -------- |
| Provides implementation | ✔      | ❌        |
| Provides constraints    | ❌      | ✔        |
| Runtime overhead        | 0      | 0        |
| Error messages          | ❌ ugly | ✔ great  |
| Reuse code              | ✔      | ❌        |

---

## 2.4 Best Practice: **CRTP + Concepts Together**

This is how modern STL does it.

```cpp
template<typename Derived>
class view_interface {
public:
    void check() const requires std::ranges::range<Derived> {
        // safe to use begin/end
    }
};
```

### Rule

> **Concepts constrain. CRTP implements.**

---

# 3️⃣ Common CRTP Pitfalls (and How to Fix Them)

These are the mistakes even experienced C++ devs make.

---

## ❌ Pitfall 1: Calling Derived Methods in Base Constructor

```cpp
template<typename D>
struct Base {
    Base() {
        static_cast<D*>(this)->foo(); // ❌ UB
    }
};
```

### Why it’s wrong

* `Derived` is not constructed yet
* Same issue as virtual calls in constructors

### ✅ Fix

Use **post-construction hooks**:

```cpp
template<typename D>
struct Base {
    void init() {
        static_cast<D*>(this)->foo();
    }
};
```

Call explicitly after construction.

---

## ❌ Pitfall 2: Forgetting the Interface Contract

```cpp
class Bad : public Shape<Bad> {
    // forgot area_impl()
};
```

### Error = unreadable template explosion

### ✅ Fix with Concepts

```cpp
template<typename D>
concept ShapeImpl = requires(const D& d) {
    { d.area_impl() } -> std::same_as<double>;
};

template<ShapeImpl D>
struct Shape { };
```

---

## ❌ Pitfall 3: Accidental Slicing

```cpp
void f(Shape<Circle> s); // ❌ slicing
```

CRTP bases are **not polymorphic bases**.

### ✅ Fix

* Pass by reference
* Or use templates

```cpp
template<typename T>
void f(const Shape<T>& s);
```

---

## ❌ Pitfall 4: Overusing CRTP Instead of Virtuals

CRTP is NOT a universal replacement.

Bad:

```cpp
std::vector<Shape<...>> shapes; // impossible
```

### When CRTP is wrong

* Plugin systems
* Runtime-loaded types
* ABI stability required

---

## ❌ Pitfall 5: Code Bloat

Each instantiation generates new code:

```cpp
Shape<A>, Shape<B>, Shape<C> // duplicates
```

### Mitigation

* Extract non-dependent code into non-template helpers
* Use CRTP only for hot paths

---

## ❌ Pitfall 6: Multiple CRTP Bases (Diamond Confusion)

```cpp
class X :
    public A<X>,
    public B<X> { };
```

### Problems

* Name collisions
* Ambiguous `static_cast`

### Fix

* Use `using` declarations
* Or composition instead

---

# 🧠 Final Mental Model

### Use CRTP when:

✔ You need **behavior reuse**
✔ Performance matters
✔ Types known at compile time
✔ Header-only or library internals

### Use Concepts when:

✔ You need **clear constraints**
✔ Better error messages
✔ Public APIs

### Use BOTH when:

✔ Designing modern, high-quality C++ libraries (STL style)

---

If you want next:

* A **step-by-step CRTP design checklist**
* CRTP vs `std::variant`
* Real-world **range/view implementation walkthrough**

Just say the word.

