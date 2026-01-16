# Collections & Containers: C++ vs. Rust

## Overview

Both C++ and Rust provide rich standard libraries with collection types, but they differ significantly in their design philosophy, safety guarantees, and API patterns. C++ offers the Standard Template Library (STL) with mature, performance-oriented containers, while Rust's collections prioritize memory safety and ownership semantics alongside performance.

## C++ STL Containers

### Vector (Dynamic Array)

C++'s `std::vector` is a dynamic array that manages its own memory. It provides random access and efficient insertion/removal at the end.

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    // Creation and initialization
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // Adding elements
    numbers.push_back(6);
    numbers.emplace_back(7); // Constructs in-place
    
    // Accessing elements
    std::cout << "First: " << numbers[0] << std::endl;        // No bounds checking
    std::cout << "Second: " << numbers.at(1) << std::endl;    // Bounds checked
    
    // Iteration
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // Modifying elements
    numbers[2] = 99;
    
    // Capacity operations
    numbers.reserve(100);  // Pre-allocate space
    std::cout << "Capacity: " << numbers.capacity() << std::endl;
    std::cout << "Size: " << numbers.size() << std::endl;
    
    // Algorithms
    std::sort(numbers.begin(), numbers.end());
    auto it = std::find(numbers.begin(), numbers.end(), 99);
    if (it != numbers.end()) {
        std::cout << "Found 99 at position: " << (it - numbers.begin()) << std::endl;
    }
    
    return 0;
}
```

### Map (Ordered Key-Value Store)

`std::map` is a sorted associative container using a red-black tree implementation.

```cpp
#include <iostream>
#include <map>
#include <string>

int main() {
    // Creation
    std::map<std::string, int> ages;
    
    // Insertion
    ages["Alice"] = 30;
    ages.insert({"Bob", 25});
    ages.emplace("Charlie", 35);
    
    // Access
    std::cout << "Alice's age: " << ages["Alice"] << std::endl;
    
    // Safe access with find
    auto it = ages.find("David");
    if (it != ages.end()) {
        std::cout << "David's age: " << it->second << std::endl;
    } else {
        std::cout << "David not found" << std::endl;
    }
    
    // Iteration (sorted by key)
    for (const auto& [name, age] : ages) {
        std::cout << name << ": " << age << std::endl;
    }
    
    // Removal
    ages.erase("Bob");
    
    return 0;
}
```

### Set (Unique Sorted Elements)

```cpp
#include <iostream>
#include <set>

int main() {
    std::set<int> unique_numbers = {5, 2, 8, 2, 1, 8};
    
    // Duplicates automatically removed, elements sorted
    for (int num : unique_numbers) {
        std::cout << num << " ";  // Output: 1 2 5 8
    }
    std::cout << std::endl;
    
    // Insert
    auto [iter, inserted] = unique_numbers.insert(3);
    std::cout << "Inserted: " << inserted << std::endl;
    
    // Check membership
    if (unique_numbers.count(5) > 0) {
        std::cout << "5 is in the set" << std::endl;
    }
    
    return 0;
}
```

### Unordered Containers (Hash-Based)

```cpp
#include <iostream>
#include <unordered_map>
#include <unordered_set>
#include <string>

int main() {
    // Hash map - O(1) average lookup
    std::unordered_map<std::string, int> word_count;
    word_count["hello"] = 5;
    word_count["world"] = 3;
    
    // Hash set
    std::unordered_set<int> seen = {1, 2, 3, 4};
    seen.insert(5);
    
    return 0;
}
```

## Rust Standard Collections

### Vec (Dynamic Array)

Rust's `Vec<T>` is similar to `std::vector` but with ownership semantics and built-in safety.

```rust
fn main() {
    // Creation and initialization
    let mut numbers = vec![1, 2, 3, 4, 5];
    
    // Adding elements
    numbers.push(6);
    
    // Accessing elements
    println!("First: {}", numbers[0]);           // Panics on out of bounds
    println!("Second: {}", numbers.get(1).unwrap()); // Returns Option<&T>
    
    // Safe access with pattern matching
    match numbers.get(10) {
        Some(value) => println!("Value: {}", value),
        None => println!("Index out of bounds"),
    }
    
    // Iteration
    for num in &numbers {
        print!("{} ", num);
    }
    println!();
    
    // Mutable iteration
    for num in &mut numbers {
        *num *= 2;
    }
    
    // Consuming iteration (takes ownership)
    for num in numbers.clone() {
        println!("{}", num);
    }
    
    // Capacity operations
    numbers.reserve(100);
    println!("Capacity: {}", numbers.capacity());
    println!("Length: {}", numbers.len());
    
    // Functional operations
    let doubled: Vec<i32> = numbers.iter().map(|x| x * 2).collect();
    let sum: i32 = numbers.iter().sum();
    
    println!("Sum: {}", sum);
}
```

### HashMap (Hash-Based Key-Value Store)

```rust
use std::collections::HashMap;

fn main() {
    // Creation
    let mut ages: HashMap<String, i32> = HashMap::new();
    
    // Insertion
    ages.insert("Alice".to_string(), 30);
    ages.insert("Bob".to_string(), 25);
    
    // Access returns Option<&V>
    match ages.get("Alice") {
        Some(age) => println!("Alice's age: {}", age),
        None => println!("Alice not found"),
    }
    
    // Entry API for efficient updates
    ages.entry("Charlie".to_string()).or_insert(35);
    
    // Update existing or insert new
    *ages.entry("Alice".to_string()).or_insert(0) += 1;
    
    // Iteration
    for (name, age) in &ages {
        println!("{}: {}", name, age);
    }
    
    // Removal returns Option<V>
    if let Some(removed_age) = ages.remove("Bob") {
        println!("Removed Bob, age was: {}", removed_age);
    }
    
    // Ownership demonstration
    let key = "Diana".to_string();
    ages.insert(key, 28);
    // key is moved, can't use it anymore
    // println!("{}", key); // Compile error!
}
```

### HashSet (Unique Elements)

```rust
use std::collections::HashSet;

fn main() {
    let mut unique_numbers: HashSet<i32> = HashSet::new();
    
    // Insertion
    unique_numbers.insert(5);
    unique_numbers.insert(2);
    unique_numbers.insert(8);
    unique_numbers.insert(2); // Duplicate, won't be added
    
    // Check membership
    if unique_numbers.contains(&5) {
        println!("5 is in the set");
    }
    
    // Set operations
    let set_a: HashSet<i32> = [1, 2, 3, 4].iter().cloned().collect();
    let set_b: HashSet<i32> = [3, 4, 5, 6].iter().cloned().collect();
    
    // Union
    let union: HashSet<_> = set_a.union(&set_b).cloned().collect();
    println!("Union: {:?}", union);
    
    // Intersection
    let intersection: HashSet<_> = set_a.intersection(&set_b).cloned().collect();
    println!("Intersection: {:?}", intersection);
    
    // Difference
    let difference: HashSet<_> = set_a.difference(&set_b).cloned().collect();
    println!("Difference: {:?}", difference);
}
```

### BTreeMap and BTreeSet (Ordered Containers)

```rust
use std::collections::{BTreeMap, BTreeSet};

fn main() {
    // BTreeMap - sorted by key
    let mut scores = BTreeMap::new();
    scores.insert("Alice", 95);
    scores.insert("Charlie", 88);
    scores.insert("Bob", 92);
    
    // Iteration in sorted order
    for (name, score) in &scores {
        println!("{}: {}", name, score);
    }
    // Output: Alice: 95, Bob: 92, Charlie: 88
    
    // BTreeSet - sorted unique elements
    let sorted_nums: BTreeSet<i32> = [5, 2, 8, 1, 9, 2].iter().cloned().collect();
    println!("Sorted: {:?}", sorted_nums); // {1, 2, 5, 8, 9}
}
```

## Key Safety Differences

### Iterator Invalidation

**C++:** Modifying a container while iterating can invalidate iterators, leading to undefined behavior.

```cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // DANGEROUS: Iterator invalidation
    for (auto it = numbers.begin(); it != numbers.end(); ++it) {
        if (*it == 3) {
            numbers.push_back(10); // May invalidate 'it' - UNDEFINED BEHAVIOR
        }
    }
    
    return 0;
}
```

**Rust:** The borrow checker prevents iterator invalidation at compile time.

```rust
fn main() {
    let mut numbers = vec![1, 2, 3, 4, 5];
    
    // COMPILE ERROR: Cannot borrow as mutable while iterating
    for num in &numbers {
        if *num == 3 {
            // numbers.push(10); // Compile error!
        }
    }
    
    // Correct approach: collect indices first
    let indices_to_process: Vec<usize> = numbers.iter()
        .enumerate()
        .filter(|(_, &n)| n == 3)
        .map(|(i, _)| i)
        .collect();
    
    for _ in indices_to_process {
        numbers.push(10); // Safe, not iterating anymore
    }
}
```

### Bounds Checking

**C++:** Mix of checked and unchecked access.

```cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int> v = {1, 2, 3};
    
    // Unchecked - undefined behavior if out of bounds
    int x = v[10]; // No error, undefined behavior!
    
    // Checked - throws exception
    try {
        int y = v.at(10);
    } catch (const std::out_of_range& e) {
        std::cout << "Out of range!" << std::endl;
    }
    
    return 0;
}
```

**Rust:** Default indexing panics on bounds violation, with `get()` returning `Option`.

```rust
fn main() {
    let v = vec![1, 2, 3];
    
    // Panics at runtime if out of bounds (safe, program terminates)
    // let x = v[10]; // Panic: index out of bounds
    
    // Safe access returns Option<&T>
    match v.get(10) {
        Some(value) => println!("Value: {}", value),
        None => println!("Index out of bounds"), // Handled gracefully
    }
}
```

### Memory Safety in Containers

**C++:** Raw pointers in containers require manual lifetime management.

```cpp
#include <vector>
#include <iostream>

int main() {
    std::vector<int*> pointers;
    
    {
        int x = 42;
        pointers.push_back(&x);
    } // x goes out of scope
    
    // UNDEFINED BEHAVIOR: Dangling pointer
    // std::cout << *pointers[0] << std::endl;
    
    return 0;
}
```

**Rust:** Ownership system prevents dangling references.

```rust
fn main() {
    let mut pointers: Vec<&i32> = Vec::new();
    
    {
        let x = 42;
        // pointers.push(&x); // Compile error: x doesn't live long enough
    }
    
    // Compile error prevents the dangling reference from existing
}
```

## API Design Differences

### Entry API Pattern

Rust's `Entry` API provides an elegant solution for conditional insertion/update.

**Rust:**
```rust
use std::collections::HashMap;

fn main() {
    let mut word_count: HashMap<String, u32> = HashMap::new();
    let text = "hello world hello rust";
    
    for word in text.split_whitespace() {
        *word_count.entry(word.to_string()).or_insert(0) += 1;
    }
    
    println!("{:?}", word_count);
}
```

**C++:** Requires more verbose code for the same pattern.

```cpp
#include <map>
#include <string>
#include <iostream>
#include <sstream>

int main() {
    std::map<std::string, int> word_count;
    std::string text = "hello world hello cpp";
    std::istringstream iss(text);
    std::string word;
    
    while (iss >> word) {
        // Check if exists, then increment or initialize
        if (word_count.find(word) != word_count.end()) {
            word_count[word]++;
        } else {
            word_count[word] = 1;
        }
        // Or use: word_count[word]++; (creates default value 0 if not exists)
    }
    
    for (const auto& [w, count] : word_count) {
        std::cout << w << ": " << count << std::endl;
    }
    
    return 0;
}
```

### Functional Programming Style

**Rust:** Strong iterator support with zero-cost abstractions.

```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    
    let result: Vec<i32> = numbers.iter()
        .filter(|&&x| x % 2 == 0)
        .map(|&x| x * x)
        .collect();
    
    println!("{:?}", result); // [4, 16, 36, 64, 100]
}
```

**C++:** Requires algorithms library or range-v3/C++20 ranges.

```cpp
#include <vector>
#include <algorithm>
#include <iostream>

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    std::vector<int> result;
    
    // C++20 ranges (more similar to Rust)
    // auto even_squares = numbers 
    //     | std::views::filter([](int x) { return x % 2 == 0; })
    //     | std::views::transform([](int x) { return x * x; });
    
    // Traditional approach
    std::copy_if(numbers.begin(), numbers.end(), 
                 std::back_inserter(result),
                 [](int x) { return x % 2 == 0; });
    
    std::transform(result.begin(), result.end(), result.begin(),
                   [](int x) { return x * x; });
    
    for (int n : result) {
        std::cout << n << " ";
    }
    std::cout << std::endl;
    
    return 0;
}
```

## Summary Table

| Feature | C++ STL | Rust Collections |
|---------|---------|------------------|
| **Primary Containers** | `vector`, `deque`, `list`, `map`, `set`, `unordered_map`, `unordered_set` | `Vec`, `VecDeque`, `LinkedList`, `BTreeMap`, `BTreeSet`, `HashMap`, `HashSet` |
| **Default Bounds Checking** | No (operator[]), Yes (at()) | Yes (panics on []), Optional (get() returns Option) |
| **Iterator Safety** | No compile-time guarantees, invalidation possible | Compile-time enforcement via borrow checker |
| **Memory Safety** | Manual management, potential for dangling pointers | Guaranteed by ownership system |
| **Null Handling** | Can store nullptrs, manual checking | No null, use `Option<T>` |
| **Thread Safety** | Requires manual synchronization | Enforced by Send/Sync traits at compile time |
| **Error Handling** | Exceptions (at()), undefined behavior ([]) | Panics ([]), Result/Option (get()) |
| **Performance** | Highly optimized, zero-overhead | Zero-cost abstractions, comparable performance |
| **API Consistency** | Iterator-based, algorithm library | Iterator traits, functional methods on containers |
| **Ordered Containers** | `std::map`, `std::set` (Red-Black tree) | `BTreeMap`, `BTreeSet` (B-tree) |
| **Hash Containers** | `unordered_map`, `unordered_set` | `HashMap`, `HashSet` |
| **Entry API** | No direct equivalent | Elegant `entry()` API for conditional operations |
| **Functional Style** | C++20 ranges, transform/accumulate | Built-in iterator methods (map, filter, fold) |
| **Move Semantics** | Explicit std::move required | Automatic move by default, explicit copy |
| **Generic Constraints** | Template specialization, SFINAE/concepts | Trait bounds (more ergonomic) |
| **Initialization** | Initializer lists, constructors | vec! macro, from_iter, collect |
| **Slice Support** | No built-in slices (use iterators/ranges) | First-class slice types [T] and &[T] |

## Conclusion

Both C++ and Rust provide powerful collection libraries, but their design philosophies differ fundamentally. C++ prioritizes flexibility and performance with optional safety features, requiring developers to be disciplined about proper usage. Rust enforces safety at compile time through its ownership system and borrow checker, preventing entire classes of bugs (iterator invalidation, data races, dangling pointers) while maintaining comparable performance. The choice between them often depends on whether you prioritize flexibility and mature ecosystems (C++) or compile-time safety guarantees and modern ergonomics (Rust).