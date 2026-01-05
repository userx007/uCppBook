## Template Specialization

**Template specialization** lets you provide a custom implementation for specific types, instead of using the generic template version.

### Full Specialization Example

```cpp
// Generic template
template <typename T>
class Storage {
public:
    void print() {
        std::cout << "Generic storage\n";
    }
};

// Full specialization for bool
template <>
class Storage<bool> {
public:
    void print() {
        std::cout << "Specialized storage for bool\n";
    }
};

// Usage
Storage<int> s1;     // Uses generic template
Storage<bool> s2;    // Uses specialized version
s1.print();          // "Generic storage"
s2.print();          // "Specialized storage for bool"
```

**Why?** Sometimes a type needs special handling. For example, `std::vector<bool>` is specialized to save space by packing bits.

## Partial Specialization

**Partial specialization** lets you specialize for a *subset* of template parameters, or for specific patterns.

### Example 1: Specializing for Pointers

```cpp
// Generic template
template <typename T>
class Container {
public:
    void info() {
        std::cout << "Regular type\n";
    }
};

// Partial specialization for pointer types
template <typename T>
class Container<T*> {
public:
    void info() {
        std::cout << "Pointer type\n";
    }
};

// Usage
Container<int> c1;      // Uses generic version
Container<int*> c2;     // Uses pointer specialization
c1.info();              // "Regular type"
c2.info();              // "Pointer type"
```

### Example 2: Multiple Template Parameters

```cpp
// Generic template with 2 parameters
template <typename T, typename U>
class Pair {
public:
    void describe() {
        std::cout << "Two different types\n";
    }
};

// Partial specialization when both types are the same
template <typename T>
class Pair<T, T> {
public:
    void describe() {
        std::cout << "Both types are the same\n";
    }
};

// Usage
Pair<int, double> p1;   // Uses generic
Pair<int, int> p2;      // Uses partial specialization
p1.describe();          // "Two different types"
p2.describe();          // "Both types are the same"
```

## Key Differences

| **Full Specialization** | **Partial Specialization** |
|------------------------|----------------------------|
| All template parameters specified | Some parameters remain generic |
| `template <>` | `template <typename T>` (or similar) |
| Works with functions and classes | Only works with classes |

## Function Template Specialization

Functions can only be **fully specialized**, not partially:

```cpp
// Generic function template
template <typename T>
void process(T value) {
    std::cout << "Generic: " << value << "\n";
}

// Full specialization for const char*
template <>
void process<const char*>(const char* value) {
    std::cout << "String: " << value << "\n";
}

process(42);        // "Generic: 42"
process("hello");   // "String: hello"
```

**Note:** For partial specialization behavior with functions, use function overloading instead.

## Practical Use Cases

1. **Optimizations**: Special handling for specific types (e.g., `std::vector<bool>`)
2. **Type traits**: Detecting properties of types at compile-time
3. **Different algorithms**: Using different implementations based on type characteristics
4. **Smart pointers**: Special behavior for pointer types
