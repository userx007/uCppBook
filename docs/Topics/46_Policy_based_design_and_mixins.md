# Policy-Based Design & Mixins in C++

## Key Highlights:

**Policy-Based Design** - Uses template parameters to inject customizable behavior:
- Threading policies (single vs multi-threaded)
- Smart pointer with ownership and checking policies
- Storage policies (stack vs heap)

**Mixins** - Small reusable classes combined through inheritance:
- Comparison operators (equality, ordering)
- Utility mixins (printable, serializable, counted instances)
- Observer pattern implementation

**Advanced Topics**:
- Combining policies and mixins together
- CRTP (Curiously Recurring Template Pattern) for type-safe static polymorphism
- Practical examples like smart pointers, containers, and data processors

The guide includes runnable code examples that demonstrate real-world applications, from basic concepts to complex compositions. These techniques are foundational to modern C++ library design and offer compile-time flexibility with zero runtime overhead compared to traditional inheritance.


# Policy-Based Design & Mixins in C++

## Overview

**Policy-based design** and **mixins** are advanced C++ template techniques that enable compile-time composition of behavior. They allow you to build flexible, reusable components by combining small, focused pieces of functionality rather than relying on deep inheritance hierarchies.

## Policy-Based Design

Policy-based design uses template parameters to inject customizable behavior into a class. Each "policy" defines a specific aspect of behavior, and policies can be mixed and matched at compile time.

### Basic Concept

```cpp
// Different threading policies
class SingleThreaded {
public:
    void lock() {}
    void unlock() {}
};

class MultiThreaded {
    std::mutex mtx;
public:
    void lock() { mtx.lock(); }
    void unlock() { mtx.unlock(); }
};

// Host class that uses a threading policy
template<typename ThreadingPolicy>
class SharedResource : private ThreadingPolicy {
    int data;
public:
    void write(int value) {
        this->lock();
        data = value;
        this->unlock();
    }
    
    int read() {
        this->lock();
        int result = data;
        this->unlock();
        return result;
    }
};

// Usage
SharedResource<SingleThreaded> singleThreadedResource;
SharedResource<MultiThreaded> multiThreadedResource;
```

### Multiple Policies Example: Smart Pointer

```cpp
// Ownership policies
template<typename T>
struct RefCounted {
    T* ptr;
    int* count;
    
    RefCounted(T* p) : ptr(p), count(new int(1)) {}
    
    void acquire(T* p) {
        ptr = p;
        count = new int(1);
    }
    
    void release() {
        if (--(*count) == 0) {
            delete ptr;
            delete count;
        }
    }
    
    RefCounted* clone() {
        ++(*count);
        return this;
    }
};

template<typename T>
struct DeepCopy {
    T* ptr;
    
    void acquire(T* p) { ptr = p; }
    void release() { delete ptr; }
    DeepCopy* clone() {
        return new DeepCopy{new T(*ptr)};
    }
};

// Checking policies
struct NoChecking {
    static void check(void* ptr) {}
};

struct EnforceNotNull {
    static void check(void* ptr) {
        if (!ptr) throw std::runtime_error("Null pointer!");
    }
};

// Smart pointer with policy-based design
template<typename T, 
         template<typename> class OwnershipPolicy,
         class CheckingPolicy>
class SmartPtr : private OwnershipPolicy<T>, private CheckingPolicy {
    using Ownership = OwnershipPolicy<T>;
public:
    explicit SmartPtr(T* p = nullptr) {
        this->acquire(p);
    }
    
    ~SmartPtr() {
        this->release();
    }
    
    T& operator*() {
        CheckingPolicy::check(this->ptr);
        return *(this->ptr);
    }
    
    T* operator->() {
        CheckingPolicy::check(this->ptr);
        return this->ptr;
    }
};

// Usage examples
SmartPtr<int, RefCounted, NoChecking> ptr1(new int(42));
SmartPtr<std::string, DeepCopy, EnforceNotNull> ptr2(new std::string("hello"));
```

### Storage Policy Example

```cpp
// Storage policies
template<typename T, size_t N>
struct StackStorage {
    T data[N];
    size_t size = 0;
    
    void add(const T& item) {
        if (size < N) data[size++] = item;
    }
    
    T& get(size_t i) { return data[i]; }
    size_t count() const { return size; }
};

template<typename T>
struct HeapStorage {
    std::vector<T> data;
    
    void add(const T& item) {
        data.push_back(item);
    }
    
    T& get(size_t i) { return data[i]; }
    size_t count() const { return data.size(); }
};

// Container using storage policy
template<typename T, template<typename, size_t> class StoragePolicy, size_t N = 10>
class Container : private StoragePolicy<T, N> {
public:
    void push(const T& item) {
        this->add(item);
    }
    
    T& at(size_t i) {
        return this->get(i);
    }
    
    size_t size() const {
        return this->count();
    }
};

// Usage
Container<int, StackStorage, 100> stackContainer;
```

## Mixins

Mixins are small classes that provide specific functionality and are designed to be inherited from. Unlike traditional inheritance, mixins are meant to be combined with multiple other mixins to compose complex behavior.

### Basic Mixin Pattern

```cpp
// Mixin for equality comparison
template<typename Derived>
class EqualityComparable {
public:
    bool operator!=(const Derived& other) const {
        return !static_cast<const Derived*>(this)->operator==(other);
    }
};

// Mixin for ordering
template<typename Derived>
class LessThanComparable {
public:
    bool operator>(const Derived& other) const {
        return other < static_cast<const Derived&>(*this);
    }
    
    bool operator<=(const Derived& other) const {
        return !(static_cast<const Derived&>(*this) > other);
    }
    
    bool operator>=(const Derived& other) const {
        return !(static_cast<const Derived&>(*this) < other);
    }
};

// Using mixins with CRTP (Curiously Recurring Template Pattern)
class Point : public EqualityComparable<Point>,
              public LessThanComparable<Point> {
    int x, y;
public:
    Point(int x, int y) : x(x), y(y) {}
    
    // Only need to define == and <
    bool operator==(const Point& other) const {
        return x == other.x && y == other.y;
    }
    
    bool operator<(const Point& other) const {
        return (x < other.x) || (x == other.x && y < other.y);
    }
    
    // Get !=, >, <=, >= for free!
};
```

### Chaining Mixins

```cpp
// Mixin for printing
template<typename Derived>
class Printable {
public:
    void print(std::ostream& os) const {
        static_cast<const Derived*>(this)->printTo(os);
    }
    
    friend std::ostream& operator<<(std::ostream& os, const Derived& obj) {
        obj.print(os);
        return os;
    }
};

// Mixin for serialization
template<typename Derived>
class Serializable {
public:
    std::string serialize() const {
        std::ostringstream oss;
        static_cast<const Derived*>(this)->writeTo(oss);
        return oss.str();
    }
};

// Mixin for counting instances
template<typename Derived>
class Counted {
    static inline int count = 0;
public:
    Counted() { ++count; }
    Counted(const Counted&) { ++count; }
    ~Counted() { --count; }
    static int getCount() { return count; }
};

// Combine multiple mixins
class Widget : public Printable<Widget>,
               public Serializable<Widget>,
               public Counted<Widget> {
    std::string name;
    int value;
public:
    Widget(const std::string& n, int v) : name(n), value(v) {}
    
    void printTo(std::ostream& os) const {
        os << "Widget(" << name << ", " << value << ")";
    }
    
    void writeTo(std::ostream& os) const {
        os << name << ":" << value;
    }
};

// Usage
Widget w1("test", 42);
Widget w2("demo", 100);
std::cout << w1 << std::endl;  // Uses Printable
std::cout << w1.serialize();    // Uses Serializable
std::cout << "Count: " << Widget::getCount();  // Uses Counted
```

### Advanced Mixin Example: Observer Pattern

```cpp
// Observer mixin
template<typename Event>
class Observable {
    std::vector<std::function<void(const Event&)>> observers;
public:
    void subscribe(std::function<void(const Event&)> callback) {
        observers.push_back(callback);
    }
    
    void notify(const Event& event) {
        for (auto& observer : observers) {
            observer(event);
        }
    }
};

// Subject that can be observed
class Button : public Observable<std::string> {
    std::string label;
public:
    Button(const std::string& l) : label(l) {}
    
    void click() {
        std::cout << "Button clicked: " << label << std::endl;
        notify(label);  // Notify all observers
    }
};

// Usage
Button btn("Submit");
btn.subscribe([](const std::string& label) {
    std::cout << "Handler 1: Button " << label << " was clicked!" << std::endl;
});
btn.subscribe([](const std::string& label) {
    std::cout << "Handler 2: Processing " << label << std::endl;
});
btn.click();
```

## Combining Policies and Mixins

Policies and mixins can work together for maximum flexibility:

```cpp
// Logging policy
class NoLogging {
public:
    template<typename... Args>
    void log(Args&&...) {}
};

class ConsoleLogging {
public:
    template<typename... Args>
    void log(Args&&... args) {
        (std::cout << ... << args) << std::endl;
    }
};

// Validation mixin
template<typename Derived>
class Validatable {
public:
    bool isValid() const {
        return static_cast<const Derived*>(this)->validate();
    }
};

// Cacheable mixin
template<typename Derived, typename Key, typename Value>
class Cacheable {
    mutable std::unordered_map<Key, Value> cache;
protected:
    Value getCached(const Key& key) const {
        auto it = cache.find(key);
        if (it != cache.end()) {
            return it->second;
        }
        Value val = static_cast<const Derived*>(this)->compute(key);
        cache[key] = val;
        return val;
    }
};

// Complex class using both
template<typename LoggingPolicy>
class DataProcessor : public Validatable<DataProcessor<LoggingPolicy>>,
                      public Cacheable<DataProcessor<LoggingPolicy>, int, double>,
                      private LoggingPolicy {
    std::vector<int> data;
public:
    void process() {
        this->log("Processing started");
        if (isValid()) {
            this->log("Validation passed");
            // Process data
        }
        this->log("Processing complete");
    }
    
    bool validate() const {
        return !data.empty();
    }
    
    double compute(int key) const {
        return key * 2.5;  // Example computation
    }
    
    double getValue(int key) const {
        return getCached(key);  // Uses cache
    }
};
```

## Benefits

1. **Compile-time polymorphism**: Zero runtime overhead compared to virtual functions
2. **Flexible composition**: Mix and match behaviors without deep inheritance
3. **Type safety**: Errors caught at compile time
4. **Code reuse**: Small, focused components can be reused across many classes
5. **No diamond problem**: Avoid multiple inheritance issues
6. **Inlining opportunities**: Better optimization by compilers

## Best Practices

1. **Keep policies small and focused**: Each policy should handle one aspect of behavior
2. **Use CRTP for mixins**: Enables static polymorphism and type-safe access to derived class
3. **Document requirements**: Clearly specify what methods policies/mixins expect
4. **Provide defaults**: Offer sensible default policies for common use cases
5. **Consider template aliases**: Simplify usage with type aliases for common combinations

```cpp
// Example of template alias for common configuration
template<typename T>
using ThreadSafePtr = SmartPtr<T, RefCounted, EnforceNotNull>;
```

## When to Use

- **Policy-based design**: When you need to customize specific aspects of behavior at compile time
- **Mixins**: When you want to add optional functionality to classes in a composable way
- **Both**: For building flexible frameworks and libraries where users need extensive customization options

These techniques are extensively used in modern C++ libraries like the STL, Boost, and many others.