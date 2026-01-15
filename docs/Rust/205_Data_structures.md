# Data Structures: C++ vs Rust Comprehensive Comparison

## Overview

Both C++ and Rust provide rich standard libraries with essential data structures. C++ uses the Standard Template Library (STL), while Rust uses its standard collections library. While they share similar concepts, their implementations differ in memory safety guarantees, ownership semantics, and API design.

## 1. Dynamic Arrays

### C++: `std::vector<T>`

A dynamically resizable array that stores elements contiguously in memory.

```cpp
#include <vector>
#include <iostream>

int main() {
    // Creation and initialization
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // Adding elements
    numbers.push_back(6);
    
    // Accessing elements
    std::cout << "First: " << numbers[0] << std::endl;
    std::cout << "At index 2: " << numbers.at(2) << std::endl;
    
    // Iteration
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    
    // Size and capacity
    std::cout << "Size: " << numbers.size() << std::endl;
    std::cout << "Capacity: " << numbers.capacity() << std::endl;
    
    // Removing elements
    numbers.pop_back();
    
    return 0;
}
```

### Rust: `Vec<T>`

Similar to C++'s vector but with ownership semantics and guaranteed memory safety.

```rust
fn main() {
    // Creation and initialization
    let mut numbers = vec![1, 2, 3, 4, 5];
    
    // Adding elements
    numbers.push(6);
    
    // Accessing elements
    println!("First: {}", numbers[0]);
    println!("At index 2: {}", numbers.get(2).unwrap());
    
    // Iteration
    for num in &numbers {
        print!("{} ", num);
    }
    
    // Size and capacity
    println!("Size: {}", numbers.len());
    println!("Capacity: {}", numbers.capacity());
    
    // Removing elements
    numbers.pop();
}
```

**Key Differences:**
- Rust's `get()` returns `Option<&T>` for safe bounds checking
- Rust requires explicit mutability with `mut`
- Rust's ownership prevents iterator invalidation at compile time

## 2. Hash Maps (Key-Value Store)

### C++: `std::unordered_map<K, V>`

Hash table implementation providing average O(1) lookup time.

```cpp
#include <unordered_map>
#include <string>
#include <iostream>

int main() {
    // Creation and initialization
    std::unordered_map<std::string, int> ages;
    
    // Insertion
    ages["Alice"] = 30;
    ages["Bob"] = 25;
    ages.insert({"Charlie", 35});
    
    // Access
    std::cout << "Alice's age: " << ages["Alice"] << std::endl;
    
    // Check if key exists
    if (ages.find("Bob") != ages.end()) {
        std::cout << "Bob found!" << std::endl;
    }
    
    // Iteration
    for (const auto& [name, age] : ages) {
        std::cout << name << ": " << age << std::endl;
    }
    
    // Remove
    ages.erase("Charlie");
    
    return 0;
}
```

### Rust: `HashMap<K, V>`

Similar hash map with ownership semantics and no default initialization.

```rust
use std::collections::HashMap;

fn main() {
    // Creation and initialization
    let mut ages = HashMap::new();
    
    // Insertion
    ages.insert("Alice".to_string(), 30);
    ages.insert("Bob".to_string(), 25);
    ages.insert("Charlie".to_string(), 35);
    
    // Access
    if let Some(&age) = ages.get("Alice") {
        println!("Alice's age: {}", age);
    }
    
    // Check if key exists
    if ages.contains_key("Bob") {
        println!("Bob found!");
    }
    
    // Iteration
    for (name, age) in &ages {
        println!("{}: {}", name, age);
    }
    
    // Remove
    ages.remove("Charlie");
}
```

**Key Differences:**
- C++ `operator[]` creates default value if key doesn't exist; Rust's `get()` returns `Option`
- Rust requires explicit `to_string()` or ownership transfer
- Rust's `entry()` API provides elegant upsert patterns

## 3. Ordered Maps (Sorted Key-Value Store)

### C++: `std::map<K, V>`

Binary search tree (typically red-black tree) with O(log n) operations.

```cpp
#include <map>
#include <string>
#include <iostream>

int main() {
    // Creation (automatically sorted by key)
    std::map<std::string, int> scores;
    
    scores["Alice"] = 95;
    scores["Charlie"] = 88;
    scores["Bob"] = 92;
    
    // Iteration (sorted order)
    for (const auto& [name, score] : scores) {
        std::cout << name << ": " << score << std::endl;
    }
    // Output: Alice: 95, Bob: 92, Charlie: 88
    
    // Range queries
    auto it = scores.lower_bound("Bob");
    std::cout << "First >= Bob: " << it->first << std::endl;
    
    return 0;
}
```

### Rust: `BTreeMap<K, V>`

B-tree implementation with similar O(log n) guarantees.

```rust
use std::collections::BTreeMap;

fn main() {
    // Creation (automatically sorted by key)
    let mut scores = BTreeMap::new();
    
    scores.insert("Alice".to_string(), 95);
    scores.insert("Charlie".to_string(), 88);
    scores.insert("Bob".to_string(), 92);
    
    // Iteration (sorted order)
    for (name, score) in &scores {
        println!("{}: {}", name, score);
    }
    // Output: Alice: 95, Bob: 92, Charlie: 88
    
    // Range queries
    for (name, score) in scores.range("Bob".to_string()..) {
        println!("Range: {}: {}", name, score);
    }
}
```

**Key Differences:**
- C++ uses red-black tree; Rust uses B-tree (better cache locality)
- Both maintain sorted order by key
- Rust's range API uses Rust's range syntax

## 4. Hash Sets (Unique Elements)

### C++: `std::unordered_set<T>`

Hash table storing unique elements with O(1) average lookup.

```cpp
#include <unordered_set>
#include <string>
#include <iostream>

int main() {
    // Creation
    std::unordered_set<std::string> fruits = {"apple", "banana", "orange"};
    
    // Insertion
    fruits.insert("grape");
    fruits.insert("apple"); // Duplicate, won't be added
    
    // Check membership
    if (fruits.count("banana") > 0) {
        std::cout << "Banana exists" << std::endl;
    }
    
    // Iteration
    for (const auto& fruit : fruits) {
        std::cout << fruit << std::endl;
    }
    
    // Remove
    fruits.erase("orange");
    
    return 0;
}
```

### Rust: `HashSet<T>`

Similar hash-based set with ownership semantics.

```rust
use std::collections::HashSet;

fn main() {
    // Creation
    let mut fruits = HashSet::new();
    fruits.insert("apple".to_string());
    fruits.insert("banana".to_string());
    fruits.insert("orange".to_string());
    
    // Insertion
    fruits.insert("grape".to_string());
    fruits.insert("apple".to_string()); // Duplicate, won't be added
    
    // Check membership
    if fruits.contains("banana") {
        println!("Banana exists");
    }
    
    // Iteration
    for fruit in &fruits {
        println!("{}", fruit);
    }
    
    // Set operations
    let vegetables: HashSet<_> = ["carrot", "lettuce"].iter().cloned().collect();
    let union: HashSet<_> = fruits.union(&vegetables).collect();
    
    // Remove
    fruits.remove("orange");
}
```

**Key Differences:**
- Rust provides rich set operations (union, intersection, difference)
- Similar performance characteristics
- Rust's `contains()` vs C++'s `count()`

## 5. Ordered Sets (Sorted Unique Elements)

### C++: `std::set<T>`

Binary search tree storing unique sorted elements.

```cpp
#include <set>
#include <iostream>

int main() {
    // Creation (automatically sorted)
    std::set<int> numbers = {5, 2, 8, 1, 9};
    
    // Insertion
    numbers.insert(3);
    numbers.insert(5); // Duplicate, won't be added
    
    // Iteration (sorted order)
    for (int num : numbers) {
        std::cout << num << " ";
    }
    // Output: 1 2 3 5 8 9
    
    // Range queries
    auto it = numbers.lower_bound(5);
    std::cout << "\nFirst >= 5: " << *it << std::endl;
    
    return 0;
}
```

### Rust: `BTreeSet<T>`

B-tree based sorted set.

```rust
use std::collections::BTreeSet;

fn main() {
    // Creation (automatically sorted)
    let mut numbers = BTreeSet::new();
    numbers.insert(5);
    numbers.insert(2);
    numbers.insert(8);
    numbers.insert(1);
    numbers.insert(9);
    
    // Insertion
    numbers.insert(3);
    numbers.insert(5); // Duplicate, won't be added
    
    // Iteration (sorted order)
    for num in &numbers {
        print!("{} ", num);
    }
    // Output: 1 2 3 5 8 9
    
    // Range queries
    for num in numbers.range(5..) {
        println!("\nRange >= 5: {}", num);
    }
}
```

## 6. Double-Ended Queue

### C++: `std::deque<T>`

Double-ended queue allowing efficient insertion/removal at both ends.

```cpp
#include <deque>
#include <iostream>

int main() {
    std::deque<int> dq = {3, 4, 5};
    
    // Add to front and back
    dq.push_front(2);
    dq.push_back(6);
    
    // Access
    std::cout << "Front: " << dq.front() << std::endl;
    std::cout << "Back: " << dq.back() << std::endl;
    
    // Random access
    std::cout << "At index 2: " << dq[2] << std::endl;
    
    // Remove from both ends
    dq.pop_front();
    dq.pop_back();
    
    return 0;
}
```

### Rust: `VecDeque<T>`

Similar double-ended queue implementation.

```rust
use std::collections::VecDeque;

fn main() {
    let mut dq = VecDeque::from(vec![3, 4, 5]);
    
    // Add to front and back
    dq.push_front(2);
    dq.push_back(6);
    
    // Access
    println!("Front: {:?}", dq.front());
    println!("Back: {:?}", dq.back());
    
    // Random access
    println!("At index 2: {}", dq[2]);
    
    // Remove from both ends
    dq.pop_front();
    dq.pop_back();
}
```

## 7. Linked Lists

### C++: `std::list<T>` and `std::forward_list<T>`

Doubly-linked list and singly-linked list respectively.

```cpp
#include <list>
#include <forward_list>
#include <iostream>

int main() {
    // Doubly-linked list
    std::list<int> dll = {1, 2, 3, 4};
    dll.push_front(0);
    dll.push_back(5);
    
    // Singly-linked list
    std::forward_list<int> sll = {1, 2, 3};
    sll.push_front(0);
    
    return 0;
}
```

### Rust: `LinkedList<T>`

Doubly-linked list (singly-linked list not in std).

```rust
use std::collections::LinkedList;

fn main() {
    let mut list = LinkedList::new();
    list.push_back(1);
    list.push_back(2);
    list.push_front(0);
    
    // Iteration
    for item in &list {
        println!("{}", item);
    }
}
```

**Note:** Linked lists are rarely needed in both languages due to cache-unfriendly performance. Use `Vec`/`vector` instead unless you specifically need O(1) insertion/removal in the middle.

## 8. Binary Heap (Priority Queue)

### C++: `std::priority_queue<T>`

Max-heap by default (can be customized to min-heap).

```cpp
#include <queue>
#include <iostream>

int main() {
    // Max heap (default)
    std::priority_queue<int> max_heap;
    max_heap.push(3);
    max_heap.push(1);
    max_heap.push(5);
    max_heap.push(2);
    
    std::cout << "Max: " << max_heap.top() << std::endl; // 5
    max_heap.pop();
    
    // Min heap
    std::priority_queue<int, std::vector<int>, std::greater<int>> min_heap;
    min_heap.push(3);
    min_heap.push(1);
    min_heap.push(5);
    
    std::cout << "Min: " << min_heap.top() << std::endl; // 1
    
    return 0;
}
```

### Rust: `BinaryHeap<T>`

Max-heap by default (wrap in `Reverse` for min-heap).

```rust
use std::collections::BinaryHeap;
use std::cmp::Reverse;

fn main() {
    // Max heap (default)
    let mut max_heap = BinaryHeap::new();
    max_heap.push(3);
    max_heap.push(1);
    max_heap.push(5);
    max_heap.push(2);
    
    println!("Max: {:?}", max_heap.peek()); // Some(5)
    max_heap.pop();
    
    // Min heap
    let mut min_heap = BinaryHeap::new();
    min_heap.push(Reverse(3));
    min_heap.push(Reverse(1));
    min_heap.push(Reverse(5));
    
    println!("Min: {:?}", min_heap.peek()); // Some(Reverse(1))
}
```

## Summary Comparison Table

| Feature | C++ | Rust | Key Differences |
|---------|-----|------|-----------------|
| **Dynamic Array** | `std::vector<T>` | `Vec<T>` | Similar API; Rust has ownership checks |
| **Hash Map** | `std::unordered_map<K,V>` | `HashMap<K,V>` | C++ `[]` creates entries; Rust returns `Option` |
| **Ordered Map** | `std::map<K,V>` (Red-Black Tree) | `BTreeMap<K,V>` (B-Tree) | Different underlying structures |
| **Hash Set** | `std::unordered_set<T>` | `HashSet<T>` | Rust has richer set operations |
| **Ordered Set** | `std::set<T>` | `BTreeSet<T>` | Similar functionality |
| **Deque** | `std::deque<T>` | `VecDeque<T>` | Similar performance |
| **Linked List** | `std::list<T>`, `std::forward_list<T>` | `LinkedList<T>` | Rarely used in practice |
| **Priority Queue** | `std::priority_queue<T>` | `BinaryHeap<T>` | Max-heap default in both |
| **Stack** | `std::stack<T>` (adapter) | Use `Vec::push/pop` | Rust uses Vec methods |
| **Queue** | `std::queue<T>` (adapter) | Use `VecDeque` | No dedicated queue in Rust std |
| **Memory Safety** | Manual management, UB possible | Compile-time guarantees | Rust prevents data races, use-after-free |
| **Default Initialization** | Default constructible types allowed | No default, explicit initialization | Rust avoids uninitialized memory |
| **Iteration Safety** | Iterator invalidation possible | Borrow checker prevents | Rust compiler enforces |
| **Error Handling** | Exceptions or undefined behavior | `Option`/`Result` types | Rust forces explicit handling |
| **Hashing** | Custom via specialization | Custom via trait implementation | Different mechanisms |
| **Move Semantics** | C++11+ move semantics | Move by default | Rust moves by default, explicit copy |

## Key Philosophical Differences

**C++ Philosophy:**
- Zero-cost abstractions with programmer responsibility
- Flexibility with potential for undefined behavior
- Rich template metaprogramming capabilities
- Exception-based error handling

**Rust Philosophy:**
- Zero-cost abstractions with compile-time safety guarantees
- Memory safety without garbage collection
- Ownership system prevents entire classes of bugs
- Explicit error handling with `Result` and `Option`
- No data races guaranteed at compile time

## Performance Considerations

Both C++ and Rust data structures have similar algorithmic complexity and runtime performance since they're compiled to efficient machine code. The main differences are:

- **Rust's safety checks** are mostly compile-time, so runtime performance is comparable
- **C++ allows more low-level control** but at the risk of undefined behavior
- **Rust's ownership** prevents certain optimizations but enables fearless concurrency
- Both support **custom allocators** for specialized use cases

## When to Use Which Language

**Choose C++** when:
- Working with existing C++ codebases
- Need mature ecosystem with decades of libraries
- Require specific low-level control unavailable in Rust
- Working in domains like game engines with established C++ tooling

**Choose Rust** when:
- Building new systems requiring high reliability
- Need memory safety guarantees without garbage collection
- Developing concurrent/parallel systems
- Want compile-time prevention of common bugs (null pointers, data races)
- Building network services, CLIs, or embedded systems