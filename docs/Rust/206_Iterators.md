# Iterators in C++ vs. Rust

## Overview

Iterators are fundamental abstractions in both C++ and Rust that provide a uniform way to traverse collections and sequences. While they serve similar purposes, their design philosophies, safety guarantees, and usage patterns differ significantly.

## C++ Iterators

### Concept and Design

C++ iterators are designed to generalize pointers and provide a consistent interface for accessing elements in containers. They are part of the Standard Template Library (STL) and follow a concept-based approach with five categories: input, output, forward, bidirectional, and random access iterators.

**Key characteristics:**
- Based on pointer-like semantics with operators like `*`, `++`, `--`
- Manual memory management and lifetime tracking
- No built-in safety against invalidation
- Support both mutable and const iterations
- Work with raw pointers and container-specific implementations

### C++ Example - Basic Iteration

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // Using iterator explicitly
    for (std::vector<int>::iterator it = numbers.begin(); 
         it != numbers.end(); ++it) {
        std::cout << *it << " ";
    }
    std::cout << std::endl;
    
    // Using auto (C++11)
    for (auto it = numbers.begin(); it != numbers.end(); ++it) {
        *it *= 2;  // Modify through iterator
    }
    
    // Range-based for loop (C++11) - uses iterators internally
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    
    return 0;
}
```

### C++ Example - Iterator Algorithms

```cpp
#include <iostream>
#include <vector>
#include <algorithm>
#include <iterator>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Using std::find
    auto it = std::find(numbers.begin(), numbers.end(), 5);
    if (it != numbers.end()) {
        std::cout << "Found: " << *it << std::endl;
    }
    
    // Using std::transform
    std::vector<int> squared;
    std::transform(numbers.begin(), numbers.end(), 
                   std::back_inserter(squared),
                   [](int x) { return x * x; });
    
    // Using std::copy with ostream_iterator
    std::copy(squared.begin(), squared.end(),
              std::ostream_iterator<int>(std::cout, " "));
    
    return 0;
}
```

### C++ Iterator Invalidation (Unsafe)

```cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // DANGER: Iterator invalidation
    for (auto it = numbers.begin(); it != numbers.end(); ++it) {
        if (*it == 3) {
            numbers.push_back(10);  // Invalidates 'it'!
            // Continuing to use 'it' here is undefined behavior
        }
    }
    
    return 0;
}
```

## Rust Iterators

### Concept and Design

Rust iterators are built around the `Iterator` trait, which provides a functional programming-oriented approach with strong safety guarantees. They follow a "zero-cost abstraction" principle where iterator chains are optimized to perform as well as hand-written loops.

**Key characteristics:**
- Based on the `Iterator` trait with a required `next()` method
- Ownership and borrowing rules prevent iterator invalidation at compile time
- Lazy evaluation with iterator adapters
- Extensive combinator methods (map, filter, fold, etc.)
- Automatic memory safety through the borrow checker
- Three types: consuming iterators, borrowing iterators, and mutable borrowing iterators

### Rust Example - Basic Iteration

```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5];
    
    // Using iterator explicitly (immutable borrow)
    for num in numbers.iter() {
        println!("{}", num);
    }
    
    // Consuming iterator (takes ownership)
    let numbers = vec![1, 2, 3, 4, 5];
    for num in numbers.into_iter() {
        println!("{}", num);
    }
    // numbers is no longer accessible here
    
    // Mutable iterator
    let mut numbers = vec![1, 2, 3, 4, 5];
    for num in numbers.iter_mut() {
        *num *= 2;
    }
    println!("{:?}", numbers);  // [2, 4, 6, 8, 10]
}
```

### Rust Example - Iterator Combinators

```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    
    // Chaining iterator operations (lazy evaluation)
    let result: Vec<i32> = numbers.iter()
        .filter(|&x| x % 2 == 0)      // Keep even numbers
        .map(|&x| x * x)               // Square them
        .take(3)                       // Take first 3
        .collect();                    // Collect into Vec
    
    println!("{:?}", result);  // [4, 16, 36]
    
    // Using fold (similar to reduce)
    let sum: i32 = numbers.iter().fold(0, |acc, &x| acc + x);
    println!("Sum: {}", sum);
    
    // Using find
    let found = numbers.iter().find(|&&x| x > 5);
    match found {
        Some(&value) => println!("Found: {}", value),
        None => println!("Not found"),
    }
}
```

### Rust Example - Custom Iterator

```rust
struct Counter {
    count: u32,
    max: u32,
}

impl Counter {
    fn new(max: u32) -> Counter {
        Counter { count: 0, max }
    }
}

impl Iterator for Counter {
    type Item = u32;
    
    fn next(&mut self) -> Option<Self::Item> {
        if self.count < self.max {
            self.count += 1;
            Some(self.count)
        } else {
            None
        }
    }
}

fn main() {
    let counter = Counter::new(5);
    
    let sum: u32 = counter
        .filter(|x| x % 2 == 0)
        .map(|x| x * 2)
        .sum();
    
    println!("Sum: {}", sum);  // 12 (2*2 + 4*2)
}
```

### Rust Safety - No Iterator Invalidation

```rust
fn main() {
    let mut numbers = vec![1, 2, 3, 4, 5];
    
    // This won't compile! Borrow checker prevents iterator invalidation
    /*
    for num in numbers.iter() {
        if *num == 3 {
            numbers.push(10);  // ERROR: cannot borrow as mutable
        }
    }
    */
    
    // Correct approach: collect indices first
    let indices: Vec<usize> = numbers.iter()
        .enumerate()
        .filter(|(_, &x)| x == 3)
        .map(|(i, _)| i)
        .collect();
    
    for _ in indices {
        numbers.push(10);  // Safe after iteration
    }
}
```

## Key Differences

### Memory Safety

**C++:** Relies on programmer discipline. Iterator invalidation can lead to undefined behavior, memory corruption, or crashes. The compiler doesn't prevent you from using invalidated iterators.

**Rust:** The borrow checker enforces memory safety at compile time. You cannot modify a collection while iterating over it with an immutable reference, preventing iterator invalidation entirely.

### Ownership Model

**C++:** Iterators don't express ownership. You must manually track whether the underlying container is still valid.

**Rust:** Three distinct iterator types express ownership clearly:
- `iter()` borrows immutably
- `iter_mut()` borrows mutably
- `into_iter()` takes ownership

### Functional Programming

**C++:** Algorithms are separate from containers. You pass iterator pairs to algorithm functions. More imperative style is common.

**Rust:** Iterator adapters are methods on the iterator itself. Encourages functional composition with method chaining. Lazy evaluation means operations aren't performed until you consume the iterator.

### Performance

**C++:** Direct pointer manipulation can be extremely fast. However, manual optimizations are often needed.

**Rust:** Zero-cost abstractions mean iterator chains compile to the same machine code as hand-written loops. The compiler aggressively optimizes iterator chains.

## Comparison Table

| Feature | C++ | Rust |
|---------|-----|------|
| **Safety** | No compile-time protection against invalidation | Borrow checker prevents invalidation at compile time |
| **Syntax** | Pointer-like (`*it`, `++it`, `it->`) | Trait-based (`.next()`, method chaining) |
| **Categories** | 5 iterator categories (input, output, forward, bidirectional, random access) | Single `Iterator` trait with uniform interface |
| **Ownership** | Not expressed in type system | Explicit: `iter()`, `iter_mut()`, `into_iter()` |
| **Mutability** | `iterator` vs `const_iterator` | `iter()` vs `iter_mut()` |
| **Algorithm Style** | Separate functions (e.g., `std::transform`) | Methods on iterator (e.g., `.map()`) |
| **Evaluation** | Eager (operations execute immediately) | Lazy (operations execute on `.collect()` or consumption) |
| **Error Handling** | Undefined behavior on misuse | Compile-time errors prevent misuse |
| **Custom Iterators** | Implement iterator concept with operators | Implement `Iterator` trait with `next()` method |
| **Range Syntax** | `begin()` and `end()` pairs | Single iterator object |
| **Null Termination** | Sentinel value (end iterator) | `Option<T>` (`Some`/`None`) |
| **Composition** | Function composition with nested calls | Method chaining with `.` operator |
| **Performance** | Fast with manual optimization | Zero-cost abstractions, optimized by compiler |
| **Learning Curve** | Must understand iterator categories and invalidation rules | Must understand ownership and borrowing |

## Summary

Both C++ and Rust provide powerful iterator abstractions, but they reflect their languages' different philosophies. C++ iterators offer flexibility and direct control with pointer-like semantics, but require careful manual management to avoid undefined behavior. Rust iterators prioritize safety through the type system and borrow checker, preventing entire classes of bugs at compile time while maintaining excellent performance.

For developers coming from C++, Rust's iterator model may initially feel restrictive, but these restrictions eliminate common sources of bugs. Rust's functional programming style with iterator combinators also encourages more declarative code. Meanwhile, C++ iterators remain familiar to those used to pointer arithmetic and offer fine-grained control when needed, though this comes at the cost of requiring more careful programming practices.