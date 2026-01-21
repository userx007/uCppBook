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
    // ============================================
    // CREATION AND INITIALIZATION
    // ============================================
    // Initialize vector with 5 elements using initializer list
    // Memory is allocated on the heap, managed automatically
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // ============================================
    // ADDING ELEMENTS
    // ============================================
    // push_back() adds element to the end of the vector
    // May trigger reallocation if capacity is exceeded
    // Time complexity: O(1) amortized
    numbers.push_back(6);
    
    // ============================================
    // ACCESSING ELEMENTS - KEY DIFFERENCE
    // ============================================
    
    // OPERATOR[] - UNCHECKED ACCESS
    // - No bounds checking
    // - Undefined behavior if index is out of range
    // - Faster (no overhead)
    // - Use when you're certain the index is valid
    std::cout << "First: " << numbers[0] << std::endl;
    
    // AT() - CHECKED ACCESS
    // - Performs bounds checking
    // - Throws std::out_of_range exception if index is invalid
    // - Slightly slower (overhead of bounds check)
    // - Use when index validity is uncertain or in critical code
    std::cout << "At index 2: " << numbers.at(2) << std::endl;
    
    // Example demonstrating the difference:
    // numbers[100];      // Undefined behavior - may crash or return garbage
    // numbers.at(100);   // Throws std::out_of_range exception - safe
    
    // ============================================
    // ITERATION
    // ============================================
    // Range-based for loop (C++11)
    // const auto& avoids copying, auto deduces type (int)
    for (const auto& num : numbers) {
        std::cout << num << " ";
    }
    std::cout << std::endl;
    
    // ============================================
    // SIZE AND CAPACITY
    // ============================================
    // size() - number of elements currently stored
    std::cout << "Size: " << numbers.size() << std::endl;
    
    // capacity() - number of elements that can be stored without reallocation
    // capacity() >= size() always true
    // Vector typically over-allocates to minimize reallocations
    std::cout << "Capacity: " << numbers.capacity() << std::endl;
    
    // ============================================
    // REMOVING ELEMENTS
    // ============================================
    // pop_back() removes the last element
    // Decreases size by 1, but capacity remains unchanged
    // Time complexity: O(1)
    // Does NOT deallocate memory - use shrink_to_fit() for that
    numbers.pop_back();
    
    return 0;
}
```

#### Key Differences: `operator[]` vs `.at()`

| Feature | `operator[]` | `.at()` |
|---------|-------------|---------|
| **Bounds Checking** | ❌ No | ✅ Yes |
| **Out-of-range behavior** | Undefined behavior | Throws `std::out_of_range` |
| **Performance** | Faster (no checks) | Slightly slower |
| **Safety** | Unsafe | Safe |
| **Best for** | Performance-critical code with guaranteed valid indices | User input, uncertain indices, debugging |

#### When to use which:

**Use `operator[]` when:**
- You're certain the index is valid
- Performance is critical
- Index comes from controlled logic (loop counters, etc.)

**Use `.at()` when:**
- Index comes from user input or external sources
- You want to catch errors explicitly
- Debugging or developing new code
- Safety is more important than the small performance overhead


### Rust: `Vec<T>`

Similar to C++'s vector but with ownership semantics and guaranteed memory safety.

```rust
fn main() {
    // ============================================
    // CREATION AND INITIALIZATION
    // ============================================
    // vec! macro creates a Vec<i32> on the heap
    // 'mut' keyword makes the vector mutable (can be modified)
    // Without 'mut', you couldn't push, pop, or modify elements
    let mut numbers = vec![1, 2, 3, 4, 5];
    
    // ============================================
    // ADDING ELEMENTS
    // ============================================
    // push() adds element to the end of the vector
    // May trigger reallocation if capacity is exceeded
    // Time complexity: O(1) amortized
    // Ownership: takes ownership of the value (or copies if Copy trait)
    numbers.push(6);
    
    // ============================================
    // ACCESSING ELEMENTS - KEY DIFFERENCE
    // ============================================
    
    // INDEXED ACCESS: numbers[index]
    // - Direct access, no runtime bounds checking in release mode
    // - PANICS at runtime if index is out of bounds (even in release)
    // - Returns a reference to the element: &T
    // - Fast but can crash your program
    // - Use when you're certain the index is valid
    println!("First: {}", numbers[0]);
    
    // GET() METHOD: .get(index)
    // - Returns Option<&T> (Some(&value) or None)
    // - NEVER panics - returns None if out of bounds
    // - Requires handling the Option (using unwrap, unwrap_or, match, etc.)
    // - Slightly slower due to Option wrapping
    // - Safe and idiomatic for uncertain indices
    println!("At index 2: {}", numbers.get(2).unwrap());
    
    // Better alternatives to .get().unwrap():
    // Safe pattern matching:
    // match numbers.get(2) {
    //     Some(value) => println!("At index 2: {}", value),
    //     None => println!("Index out of bounds"),
    // }
    
    // Or with if let:
    // if let Some(value) = numbers.get(2) {
    //     println!("At index 2: {}", value);
    // }
    
    // Or with unwrap_or for default value:
    // println!("At index 2: {}", numbers.get(2).unwrap_or(&0));
    
    // Example demonstrating the difference:
    // numbers[100];        // PANICS: "index out of bounds"
    // numbers.get(100);    // Returns None - no panic, safe
    
    // ============================================
    // ITERATION
    // ============================================
    // Iterate over immutable references to avoid moving/copying
    // &numbers creates an iterator over &i32 (references)
    // 'num' has type &i32 (reference to integer)
    for num in &numbers {
        print!("{} ", num);
    }
    println!(); // newline
    
    // Alternative iteration patterns:
    // 1. Mutable references (to modify elements):
    //    for num in &mut numbers { *num += 1; }
    // 2. Take ownership (consumes the vector):
    //    for num in numbers { ... }
    // 3. Enumerate with indices:
    //    for (i, num) in numbers.iter().enumerate() { ... }
    
    // ============================================
    // SIZE AND CAPACITY
    // ============================================
    // len() - number of elements currently stored
    // Returns usize (platform-dependent unsigned integer)
    println!("Size: {}", numbers.len());
    
    // capacity() - number of elements that can be stored without reallocation
    // Vec over-allocates to minimize expensive reallocations
    // Typically grows by factor of 2 when capacity is exceeded
    println!("Capacity: {}", numbers.capacity());
    
    // ============================================
    // REMOVING ELEMENTS
    // ============================================
    // pop() removes and returns the last element as Option<T>
    // - Returns Some(value) if vector is not empty
    // - Returns None if vector is empty (doesn't panic!)
    // - Decreases len by 1, capacity remains unchanged
    // - Time complexity: O(1)
    let last = numbers.pop();
    println!("Popped: {:?}", last); // Prints Some(6)
    
    // If vector was empty:
    // let empty_vec: Vec<i32> = vec![];
    // let result = empty_vec.pop(); // Returns None, doesn't panic
    
    // To deallocate excess capacity:
    // numbers.shrink_to_fit();
}
```

#### Key Differences: Indexed Access `[]` vs `.get()`

| Feature | `numbers[index]` | `numbers.get(index)` |
|---------|------------------|----------------------|
| **Return Type** | `&T` (direct reference) | `Option<&T>` (Some or None) |
| **Out-of-bounds behavior** | **Panics** (crashes program) | Returns `None` (safe) |
| **Performance** | Faster (direct access) | Slightly slower (Option wrapping) |
| **Safety** | Can crash if index invalid | Never crashes |
| **Mutability** | `numbers[i]` or `&mut numbers[i]` | `.get(i)` or `.get_mut(i)` |
| **Best for** | Guaranteed valid indices | User input, uncertain indices |

##### When to use which:

**Use indexed access `[]` when:**
- You're certain the index is valid (e.g., iterating with known bounds)
- Performance is critical and bounds are guaranteed
- You want the code to panic on logic errors (fail-fast principle)
```rust
let first = numbers[0];  // Fine if vector is never empty
for i in 0..numbers.len() {
    println!("{}", numbers[i]);  // Safe: i is always valid
}
```

**Use `.get()` when:**
- Index comes from user input or external sources
- You want to handle the "not found" case gracefully
- The vector might be empty or index might be out of bounds
```rust
if let Some(value) = numbers.get(user_index) {
    println!("Found: {}", value);
} else {
    println!("Invalid index");
}
```

##### Important Rust-Specific Notes:

1. **`.get()` is more idiomatic** in Rust when dealing with potentially invalid indices
2. **Never use `.unwrap()` on `.get()`** unless you're absolutely certain - it defeats the safety purpose
3. Rust's **borrow checker prevents** many common C++ mistakes (dangling references, iterator invalidation)
4. **`pop()` returns `Option<T>`** (not void like C++), preventing errors when popping from empty vectors

**Final Key Differences:**
- Rust's `get()` returns `Option<&T>` for safe bounds checking
- Rust requires explicit mutability with `mut`
- Rust's ownership prevents iterator invalidation at compile time

---

## 2. Hash Maps (Key-Value Store)

### C++: `std::unordered_map<K, V>`

Hash table implementation providing average O(1) lookup time.

```cpp
#include <unordered_map>  // For hash map (unordered key-value storage)
#include <string>         // For std::string
#include <iostream>       // For console I/O

int main() {
    // Create an empty unordered_map with string keys and int values
    // unordered_map provides O(1) average-case lookup, insertion, and deletion
    std::unordered_map<std::string, int> ages;
    
    // Insert key-value pairs using the subscript operator
    // IMPORTANT: If the key doesn't exist, it's created; if it exists, the value is updated
    ages["Alice"] = 30;
    ages["Bob"] = 25;
    
    // Alternative insertion method using insert() with initializer list
    // IMPORTANT: This approach won't overwrite existing keys (insert fails silently if key exists)
    ages.insert({"Charlie", 35});
    
    // Access value using subscript operator
    // WARNING: Using [] on a non-existent key will create it with default value (0 for int)
    std::cout << "Alice's age: " << ages["Alice"] << std::endl;
    
    // Safe way to check if a key exists before accessing
    // find() returns an iterator; if key not found, returns ages.end()
    if (ages.find("Bob") != ages.end()) {
        std::cout << "Bob found!" << std::endl;
    }
    
    // Iterate through all key-value pairs using structured binding (C++17)
    // const auto& avoids copying; [name, age] unpacks the pair
    for (const auto& [name, age] : ages) {
        std::cout << name << ": " << age << std::endl;
    }
    
    // Remove an element by key
    // erase() returns the number of elements removed (0 or 1 for unordered_map)
    ages.erase("Charlie");
    
    return 0;
}
```

### Rust: `HashMap<K, V>`

Similar hash map with ownership semantics and no default initialization.

```rust
use std::collections::HashMap;  // Import HashMap from standard library

fn main() {
    // Create a new empty HashMap with type inference
    // 'mut' is required because we'll be modifying the HashMap
    // Type will be inferred as HashMap<String, i32> from first insertion
    let mut ages = HashMap::new();
    
    // Insert key-value pairs using insert()
    // IMPORTANT: to_string() converts &str literals to owned String (required for HashMap ownership)
    // IMPORTANT: insert() returns Option<V>: None if key was new, Some(old_value) if key existed
    ages.insert("Alice".to_string(), 30);
    ages.insert("Bob".to_string(), 25);
    ages.insert("Charlie".to_string(), 35);
    
    // Safe access using get(), which returns Option<&V>
    // if let pattern matching extracts the value if Some
    // IMPORTANT: &age pattern matches the reference, giving us the actual i32 value
    if let Some(&age) = ages.get("Alice") {
        println!("Alice's age: {}", age);
    }
    
    // Check if a key exists without retrieving the value
    // More efficient than get() when you only need existence check
    if ages.contains_key("Bob") {
        println!("Bob found!");
    }
    
    // Iterate over key-value pairs by borrowing the HashMap
    // &ages prevents moving/consuming the HashMap
    // (name, age) destructures each (&String, &i32) tuple
    for (name, age) in &ages {
        println!("{}: {}", name, age);
    }
    
    // Remove a key-value pair by key
    // remove() returns Option<V>: Some(value) if key existed, None otherwise
    // The removed value is dropped here since we don't capture it
    ages.remove("Charlie");
}
```

#### Destructuring details

```rust
for (&name, &age) in &ages {
    println!("{}: {}", name, age);
}
```

- **`&age` works**: You can dereference `&i32` to get `i32` because `i32` implements `Copy`. The value is copied, not moved.

- **`&name` fails**: You cannot dereference `&String` to get `String` because `String` does NOT implement `Copy`. Dereferencing would try to move the `String` out of the HashMap, which Rust doesn't allow since the HashMap still owns it.

**What actually works:**

```rust
// Option 1: Borrow both (most common)
for (name, age) in &ages {
    // name: &String, age: &i32
    println!("{}: {}", name, age);
}

// Option 2: Dereference only age
for (name, &age) in &ages {
    // name: &String, age: i32 (copied)
    println!("{}: {}", name, age);
}

// Option 3: Clone the String if you need ownership
for (name, age) in &ages {
    let owned_name: String = name.clone();
    println!("{}: {}", owned_name, age);
}
```

**Final Key Differences:**
- C++ `operator[]` creates default value if key doesn't exist; Rust's `get()` returns `Option`
- Rust requires explicit `to_string()` or ownership transfer
- Rust's `entry()` API provides elegant upsert patterns

---

## 3. Ordered Maps (Sorted Key-Value Store)

### C++: `std::map<K, V>`

Binary search tree (typically red-black tree) with O(log n) operations.


```cpp
#include <map>
#include <string>
#include <iostream>

int main() {
    // Creation (automatically sorted by key using std::less<std::string> by default)
    // Alternative: std::unordered_map for O(1) average lookup without sorting
    std::map<std::string, int> scores;
    
    // Insertion methods
    scores["Alice"] = 95;      // operator[] - creates if not exists, overwrites if exists
    scores["Charlie"] = 88;
    scores["Bob"] = 92;
    
    // Alternative insertion methods:
    // scores.insert({"David", 90});           // insert() - doesn't overwrite
    // scores.insert_or_assign("Eve", 87);     // C++17 - overwrites if exists
    // scores.emplace("Frank", 85);            // constructs in-place
    // scores.try_emplace("Grace", 93);        // C++17 - only inserts if key doesn't exist
    
    // Iteration (sorted order by key - alphabetically for strings)
    // Time complexity: O(n), where n is the number of elements
    for (const auto& [name, score] : scores) {  // C++17 structured binding
        std::cout << name << ": " << score << std::endl;
    }
    // Output: Alice: 95, Bob: 92, Charlie: 88
    
    // Alternative iteration methods:
    // for (auto it = scores.begin(); it != scores.end(); ++it)
    //     std::cout << it->first << ": " << it->second << std::endl;
    
    // Range queries - one of std::map's key advantages over std::unordered_map
    auto it = scores.lower_bound("Bob");  // Returns iterator to first element >= "Bob"
    std::cout << "First >= Bob: " << it->first << std::endl;
    
    // Other useful range query methods:
    // auto it2 = scores.upper_bound("Bob");   // First element > "Bob"
    // auto [low, high] = scores.equal_range("Bob"); // Range of elements == "Bob"
    
    // Lookup methods:
    // if (scores.count("Alice")) { }          // Returns 0 or 1
    // if (scores.find("Alice") != scores.end()) { } // Returns iterator
    // if (scores.contains("Alice")) { }       // C++20 - most readable
    
    // Access methods:
    // int val = scores["Alice"];              // Creates entry if not exists (use with caution)
    // int val = scores.at("Alice");           // Throws std::out_of_range if not exists (safer)
    
    // Performance characteristics:
    // - Insertion: O(log n)
    // - Lookup: O(log n)
    // - Deletion: O(log n)
    // - Maintains sorted order
    // Compare with std::unordered_map: O(1) average for insert/lookup/delete, no ordering
    
    return 0;
}
```

### Rust: `BTreeMap<K, V>`

B-tree implementation with similar O(log n) guarantees.

```rust
use std::collections::BTreeMap;

fn main() {
    // Creation (automatically sorted by key using Ord trait)
    // Alternative: HashMap for O(1) average lookup without sorting
    let mut scores = BTreeMap::new();
    
    // Insertion - returns Option<V> with old value if key existed
    scores.insert("Alice".to_string(), 95);
    scores.insert("Charlie".to_string(), 88);
    scores.insert("Bob".to_string(), 92);
    
    // Alternative insertion methods:
    // scores.entry("David".to_string()).or_insert(90);  // Insert only if not exists
    // scores.entry("Eve".to_string()).or_insert_with(|| 87); // Lazy initialization
    // *scores.entry("Alice".to_string()).or_insert(0) += 5;   // Update or insert pattern
    
    // Alternative: Use &str as keys to avoid allocations (requires different type)
    // let mut scores: BTreeMap<&str, i32> = BTreeMap::new();
    // scores.insert("Alice", 95);
    
    // Iteration (sorted order by key - alphabetically for strings)
    // Time complexity: O(n), where n is the number of elements
    for (name, score) in &scores {  // Borrows the map, iterates in sorted order
        println!("{}: {}", name, score);
    }
    // Output: Alice: 95, Bob: 92, Charlie: 88
    
    // Alternative iteration methods:
    // for (name, score) in scores.iter() { }       // Explicit iterator
    // for name in scores.keys() { }                 // Keys only
    // for score in scores.values() { }              // Values only
    // for score in scores.values_mut() { }          // Mutable values
    // for (name, score) in scores.into_iter() { }   // Consumes the map
    
    // Range queries - one of BTreeMap's key advantages over HashMap
    // Uses Rust's range syntax: .. (full), start.. (from), ..end (to), start..end
    for (name, score) in scores.range("Bob".to_string()..) {
        println!("Range: {}: {}", name, score);
    }
    // Output: Bob: 92, Charlie: 88
    
    // More efficient range query (avoids allocation):
    // for (name, score) in scores.range("Bob"..) {  // &str works for range bounds
    //     println!("Range: {}: {}", name, score);
    // }
    
    // Other useful range methods:
    // scores.range(.."Charlie")                     // All keys < "Charlie"
    // scores.range("Alice"..="Bob")                 // Inclusive range
    // scores.range_mut("Bob"..)                     // Mutable range iteration
    
    // Lookup methods:
    // if let Some(score) = scores.get("Alice") { } // Returns Option<&V>
    // if let Some(score) = scores.get_mut("Alice") { } // Returns Option<&mut V>
    // if scores.contains_key("Alice") { }           // Returns bool
    // let score = scores["Alice"];                  // Panics if key doesn't exist
    
    // Removal methods:
    // scores.remove("Alice");                       // Returns Option<V>
    // scores.remove_entry("Alice");                 // Returns Option<(K, V)>
    
    // Split operations (unique to BTreeMap):
    // let right = scores.split_off("Charlie");      // Splits at key, returns right half
    
    // Performance characteristics:
    // - Insertion: O(log n)
    // - Lookup: O(log n)
    // - Deletion: O(log n)
    // - Range queries: O(log n + k) where k is range size
    // - Maintains sorted order
    // Compare with HashMap: O(1) average for insert/lookup/delete, no ordering
    
    // Memory: BTreeMap is generally more cache-friendly than tree implementations
    // in other languages due to node compacting (stores multiple elements per node)
}
```

**Key Differences:**
- C++ uses red-black tree; Rust uses B-tree (better cache locality)
- Both maintain sorted order by key
- Rust's range API uses Rust's range syntax

---

## 4. Hash Sets (Unique Elements)

### C++: `std::unordered_set<T>`

Hash table storing unique elements with O(1) average lookup.

```cpp
#include <unordered_set>
#include <string>
#include <iostream>

int main() {
    // Creation - hash-based set with O(1) average insertion/lookup
    // Alternative: std::set for O(log n) operations with sorted order
    std::unordered_set<std::string> fruits = {"apple", "banana", "orange"};
    
    // Alternative creation methods:
    // std::unordered_set<std::string> fruits;          // Empty set
    // std::unordered_set<std::string> fruits({"a"});   // Initializer list
    // std::unordered_set<std::string> fruits(other);   // Copy constructor
    
    // Insertion - returns pair<iterator, bool> where bool indicates if inserted
    fruits.insert("grape");
    fruits.insert("apple"); // Duplicate, won't be added (insert returns false)
    
    // Alternative insertion methods:
    // auto [it, inserted] = fruits.insert("mango");    // C++17 structured binding
    // fruits.emplace("kiwi");                          // Construct in-place
    // fruits.insert(other.begin(), other.end());       // Insert range
    
    // Check membership - multiple approaches
    if (fruits.count("banana") > 0) {  // Returns 0 or 1 for unordered_set
        std::cout << "Banana exists" << std::endl;
    }
    
    // Alternative membership checks:
    // if (fruits.find("banana") != fruits.end()) { }   // Returns iterator
    // if (fruits.contains("banana")) { }               // C++20 - most readable
    
    // Iteration - order is NOT guaranteed (hash-based)
    // If you need sorted iteration, use std::set instead
    for (const auto& fruit : fruits) {
        std::cout << fruit << std::endl;
    }
    // Output order: unpredictable (depends on hash function)
    
    // Alternative iteration:
    // for (auto it = fruits.begin(); it != fruits.end(); ++it)
    //     std::cout << *it << std::endl;
    
    // Remove - returns number of elements removed (0 or 1)
    fruits.erase("orange");
    
    // Alternative removal methods:
    // auto it = fruits.find("apple");
    // if (it != fruits.end()) fruits.erase(it);        // Erase by iterator
    // fruits.clear();                                   // Remove all elements
    
    // Set operations (require algorithms or manual implementation):
    // std::unordered_set<std::string> other = {"apple", "grape", "mango"};
    // for (const auto& item : other) {
    //     fruits.insert(item);                         // Union
    // }
    
    // Size and capacity:
    // std::cout << "Size: " << fruits.size() << std::endl;
    // std::cout << "Empty: " << fruits.empty() << std::endl;
    // std::cout << "Bucket count: " << fruits.bucket_count() << std::endl;
    // std::cout << "Load factor: " << fruits.load_factor() << std::endl;
    
    // Performance characteristics:
    // - Insertion: O(1) average, O(n) worst case
    // - Lookup: O(1) average, O(n) worst case
    // - Deletion: O(1) average, O(n) worst case
    // - No ordering maintained
    // - Better cache performance than std::set for large datasets
    
    // When to use std::set instead:
    // - Need sorted/ordered iteration
    // - Need range queries (lower_bound, upper_bound)
    // - Predictable O(log n) performance (no worst-case O(n))
    
    return 0;
}
```

### Rust: `HashSet<T>`

Similar hash-based set with ownership semantics.

```rust
use std::collections::HashSet;

fn main() {
    // Creation - hash-based set with O(1) average insertion/lookup
    // Alternative: BTreeSet for O(log n) operations with sorted order
    let mut fruits = HashSet::new();
    fruits.insert("apple".to_string());
    fruits.insert("banana".to_string());
    fruits.insert("orange".to_string());
    
    // Alternative creation methods:
    // let fruits: HashSet<String> = HashSet::new();                    // Empty with type annotation
    // let fruits: HashSet<_> = ["apple", "banana"].iter()
    //     .map(|s| s.to_string()).collect();                           // From iterator
    // let fruits = HashSet::from(["apple".to_string(), "banana".to_string()]); // From array (Rust 2021)
    
    // More efficient: use &str to avoid allocations (requires different type)
    // let mut fruits: HashSet<&str> = HashSet::new();
    // fruits.insert("apple");
    
    // Insertion - returns bool indicating if value was newly inserted
    fruits.insert("grape".to_string());
    let was_inserted = fruits.insert("apple".to_string()); // Duplicate, returns false
    
    // Alternative insertion pattern:
    // if fruits.insert("mango".to_string()) {
    //     println!("Mango was added");
    // }
    
    // Check membership
    if fruits.contains("banana") {  // More idiomatic than C++'s count()
        println!("Banana exists");
    }
    
    // Alternative: pattern matching with get()
    // if fruits.get("banana").is_some() { }
    
    // Iteration - order is NOT guaranteed (hash-based)
    // If you need sorted iteration, use BTreeSet instead
    for fruit in &fruits {  // Borrows the set
        println!("{}", fruit);
    }
    // Output order: unpredictable (depends on hash function)
    
    // Alternative iteration methods:
    // for fruit in fruits.iter() { }           // Explicit iterator
    // for fruit in &fruits { }                 // Immutable borrow (same as above)
    // for fruit in fruits.into_iter() { }      // Consumes the set, takes ownership
    
    // Set operations - HashSet provides rich set algebra methods
    let vegetables: HashSet<_> = ["carrot", "lettuce"].iter().cloned().collect();
    
    // Union - combines all elements from both sets
    let union: HashSet<_> = fruits.union(&vegetables).collect();
    // Alternative: union returns references, use cloned() if needed
    // let union: HashSet<&str> = fruits.union(&vegetables).cloned().collect();
    
    // Other set operations:
    // let intersection: HashSet<_> = fruits.intersection(&vegetables).collect(); // Common elements
    // let difference: HashSet<_> = fruits.difference(&vegetables).collect();     // In fruits but not vegetables
    // let sym_diff: HashSet<_> = fruits.symmetric_difference(&vegetables).collect(); // In either but not both
    
    // Set relationship checks:
    // fruits.is_disjoint(&vegetables)          // No common elements
    // fruits.is_subset(&vegetables)            // All fruits in vegetables
    // fruits.is_superset(&vegetables)          // All vegetables in fruits
    
    // Remove - returns bool indicating if value was present
    let was_present = fruits.remove("orange");
    
    // Alternative removal:
    // fruits.take("orange");                   // Returns Option<T>, takes ownership
    // fruits.retain(|f| f != "apple");         // Keep elements matching predicate
    // fruits.clear();                          // Remove all elements
    
    // Size and capacity:
    // println!("Size: {}", fruits.len());
    // println!("Empty: {}", fruits.is_empty());
    // println!("Capacity: {}", fruits.capacity());
    // fruits.reserve(100);                     // Reserve capacity
    // fruits.shrink_to_fit();                  // Reduce capacity to fit
    
    // Replacement operation:
    // if let Some(old) = fruits.replace("apple".to_string()) {
    //     println!("Replaced: {}", old);       // Returns old value if it existed
    // }
    
    // Performance characteristics:
    // - Insertion: O(1) average, O(n) worst case
    // - Lookup: O(1) average, O(n) worst case
    // - Deletion: O(1) average, O(n) worst case
    // - No ordering maintained
    // - Memory efficient (single allocation for hash table)
    
    // When to use BTreeSet instead:
    // - Need sorted/ordered iteration
    // - Need range queries (range() method)
    // - Predictable O(log n) performance
    // - When T implements Ord but not Hash
    
    // Custom hash function (advanced):
    // use std::collections::hash_map::RandomState;
    // use std::hash::BuildHasher;
    // let s = RandomState::new();
    // let fruits: HashSet<String, _> = HashSet::with_hasher(s);
}
```

**Key Differences:**
- Rust provides rich set operations (union, intersection, difference)
- Similar performance characteristics
- Rust's `contains()` vs C++'s `count()`

---

## 5. Ordered Sets (Sorted Unique Elements)

### C++: `std::set<T>`

Binary search tree storing unique sorted elements.

```cpp
#include <set>
#include <iostream>

int main() {
    // Creation (automatically sorted using std::less<int> by default)
    // Alternative: std::unordered_set for O(1) average lookup without sorting
    std::set<int> numbers = {5, 2, 8, 1, 9};
    
    // Alternative creation methods:
    // std::set<int> numbers;                           // Empty set
    // std::set<int> numbers(other);                    // Copy constructor
    // std::set<int, std::greater<int>> desc{5,2,8};    // Descending order
    // std::set<int> numbers(vec.begin(), vec.end());   // From range
    
    // Insertion - returns pair<iterator, bool> where bool indicates if inserted
    numbers.insert(3);
    auto [it, inserted] = numbers.insert(5); // C++17 structured binding
    // Duplicate, won't be added (inserted == false)
    
    // Alternative insertion methods:
    // numbers.emplace(7);                              // Construct in-place
    // numbers.insert(other.begin(), other.end());      // Insert range
    // auto hint_it = numbers.insert(it, 4);            // Insert with hint (optimization)
    
    // Iteration (sorted order - ascending by default)
    // Time complexity: O(n)
    for (int num : numbers) {
        std::cout << num << " ";
    }
    // Output: 1 2 3 5 8 9
    
    // Alternative iteration:
    // for (auto it = numbers.begin(); it != numbers.end(); ++it)
    //     std::cout << *it << " ";
    // for (auto it = numbers.rbegin(); it != numbers.rend(); ++it) // Reverse iteration
    //     std::cout << *it << " ";
    
    // Range queries - one of std::set's key advantages over std::unordered_set
    auto it2 = numbers.lower_bound(5);  // Returns iterator to first element >= 5
    std::cout << "\nFirst >= 5: " << *it2 << std::endl;
    
    // Other useful range query methods:
    // auto it3 = numbers.upper_bound(5);               // First element > 5
    // auto [low, high] = numbers.equal_range(5);       // Range of elements == 5
    // if (it2 != numbers.end()) { }                    // Always check validity
    
    // Check membership
    // if (numbers.count(5) > 0) { }                    // Returns 0 or 1
    // if (numbers.find(5) != numbers.end()) { }        // Returns iterator
    // if (numbers.contains(5)) { }                     // C++20 - most readable
    
    // Removal methods:
    // numbers.erase(5);                                // Erase by value, returns count (0 or 1)
    // numbers.erase(it2);                              // Erase by iterator, returns next iterator
    // numbers.erase(numbers.begin(), it2);             // Erase range
    // numbers.clear();                                 // Remove all elements
    
    // Set operations (using algorithms):
    // std::set<int> other = {3, 4, 5, 6};
    // std::set<int> result;
    // std::set_union(numbers.begin(), numbers.end(),
    //                other.begin(), other.end(),
    //                std::inserter(result, result.begin()));        // Union
    // std::set_intersection(...);                      // Intersection
    // std::set_difference(...);                        // Difference
    // std::set_symmetric_difference(...);              // Symmetric difference
    
    // Note: set operations require sorted ranges (which std::set guarantees)
    
    // Size and bounds:
    // std::cout << "Size: " << numbers.size() << std::endl;
    // std::cout << "Empty: " << numbers.empty() << std::endl;
    // if (!numbers.empty()) {
    //     std::cout << "Min: " << *numbers.begin() << std::endl;
    //     std::cout << "Max: " << *numbers.rbegin() << std::endl;
    // }
    
    // Extract node (C++17) - remove without destroying
    // auto node = numbers.extract(5);                  // Returns node_type
    // if (!node.empty()) {
    //     node.value() = 10;                           // Modify the value
    //     numbers.insert(std::move(node));             // Reinsert
    // }
    
    // Merge (C++17) - transfer elements from another set
    // std::set<int> other = {10, 11};
    // numbers.merge(other);                            // other becomes empty
    
    // Performance characteristics:
    // - Insertion: O(log n)
    // - Lookup: O(log n)
    // - Deletion: O(log n)
    // - Range queries: O(log n) to find, O(k) to iterate k elements
    // - Maintains sorted order (automatically balanced tree, typically Red-Black)
    // - Predictable performance (no worst-case O(n) like unordered_set)
    
    // When to use std::unordered_set instead:
    // - Don't need sorted order
    // - Don't need range queries
    // - Want O(1) average case performance
    // - Have a good hash function for your type
    
    // When to use std::multiset:
    // - Need to store duplicate values
    // - All operations same as std::set
    
    return 0;
}
```

### Rust: `BTreeSet<T>`

B-tree based sorted set.

```rust
use std::collections::BTreeSet;

fn main() {
    // Creation (automatically sorted using Ord trait)
    // Alternative: HashSet for O(1) average lookup without sorting
    let mut numbers = BTreeSet::new();
    numbers.insert(5);
    numbers.insert(2);
    numbers.insert(8);
    numbers.insert(1);
    numbers.insert(9);
    
    // Alternative creation methods:
    // let numbers: BTreeSet<i32> = BTreeSet::new();               // Empty with type annotation
    // let numbers: BTreeSet<_> = [5, 2, 8, 1, 9].iter().copied().collect(); // From iterator
    // let numbers = BTreeSet::from([5, 2, 8, 1, 9]);              // From array (Rust 2021)
    
    // Insertion - returns bool indicating if value was newly inserted
    numbers.insert(3);
    let was_inserted = numbers.insert(5); // Duplicate, returns false
    
    // Alternative insertion pattern:
    // if numbers.insert(7) {
    //     println!("7 was added");
    // }
    
    // Iteration (sorted order - ascending)
    // Time complexity: O(n)
    for num in &numbers {  // Borrows the set
        print!("{} ", num);
    }
    // Output: 1 2 3 5 8 9
    println!();
    
    // Alternative iteration methods:
    // for num in numbers.iter() { }                    // Explicit iterator
    // for num in numbers.iter().rev() { }              // Reverse iteration
    // for num in &numbers { }                          // Immutable borrow (same as iter())
    // for num in numbers.into_iter() { }               // Consumes the set
    
    // Range queries - one of BTreeSet's key advantages over HashSet
    // Uses Rust's range syntax: .. (full), start.. (from), ..end (to), start..end
    for num in numbers.range(5..) {
        println!("Range >= 5: {}", num);
    }
    // Output: 5 8 9
    
    // Other useful range operations:
    // for num in numbers.range(..5) { }                // All elements < 5
    // for num in numbers.range(2..=5) { }              // Inclusive range: 2 <= x <= 5
    // for num in numbers.range(2..8) { }               // Exclusive end: 2 <= x < 8
    
    // Check membership
    // if numbers.contains(&5) { }                      // Returns bool (note: takes reference)
    // if numbers.get(&5).is_some() { }                 // Returns Option<&T>
    
    // First and last elements:
    // if let Some(first) = numbers.first() {           // O(log n) - smallest element
    //     println!("Min: {}", first);
    // }
    // if let Some(last) = numbers.last() {             // O(log n) - largest element
    //     println!("Max: {}", last);
    // }
    
    // Pop operations:
    // if let Some(first) = numbers.pop_first() {       // Remove and return smallest
    //     println!("Removed min: {}", first);
    // }
    // if let Some(last) = numbers.pop_last() {         // Remove and return largest
    //     println!("Removed max: {}", last);
    // }
    
    // Removal methods:
    // let was_present = numbers.remove(&5);            // Returns bool
    // numbers.take(&5);                                // Returns Option<T>, takes ownership
    // numbers.retain(|&n| n % 2 == 0);                 // Keep only even numbers
    // numbers.clear();                                 // Remove all elements
    
    // Set operations - BTreeSet provides rich set algebra methods
    // let other = BTreeSet::from([3, 4, 5, 6]);
    // let union: BTreeSet<_> = numbers.union(&other).copied().collect();           // All elements
    // let intersection: BTreeSet<_> = numbers.intersection(&other).copied().collect(); // Common elements
    // let difference: BTreeSet<_> = numbers.difference(&other).copied().collect(); // In numbers but not other
    // let sym_diff: BTreeSet<_> = numbers.symmetric_difference(&other).copied().collect(); // In either but not both
    
    // Set relationship checks:
    // numbers.is_disjoint(&other)                      // No common elements
    // numbers.is_subset(&other)                        // All numbers in other
    // numbers.is_superset(&other)                      // All other in numbers
    
    // Split operations (unique to BTreeSet):
    // let right = numbers.split_off(&5);               // Splits at value, returns elements >= 5
    
    // Append operation:
    // let mut other = BTreeSet::from([10, 11, 12]);
    // numbers.append(&mut other);                      // Moves all elements from other to numbers
    
    // Size and capacity:
    // println!("Size: {}", numbers.len());
    // println!("Empty: {}", numbers.is_empty());
    
    // Replacement operation:
    // if let Some(old) = numbers.replace(5) {          // Returns old value if it existed
    //     println!("Replaced: {}", old);
    // }
    
    // Take ownership of value:
    // if let Some(value) = numbers.take(&5) {          // Removes and returns the value
    //     println!("Took: {}", value);
    // }
    
    // Performance characteristics:
    // - Insertion: O(log n)
    // - Lookup: O(log n)
    // - Deletion: O(log n)
    // - Range queries: O(log n) to find start, O(k) to iterate k elements
    // - Maintains sorted order (B-tree implementation)
    // - Cache-friendly (stores multiple elements per node)
    // - Predictable performance (no worst-case O(n) like HashSet)
    
    // When to use HashSet instead:
    // - Don't need sorted order
    // - Don't need range queries
    // - Want O(1) average case performance
    // - Type implements Hash + Eq but not Ord
    
    // Memory considerations:
    // BTreeSet is generally more memory-efficient than traditional binary trees
    // because it stores multiple elements per node (better cache locality)
}
```

---

## 6. Double-Ended Queue

### C++: `std::deque<T>`

Double-ended queue allowing efficient insertion/removal at both ends.

```cpp
#include <deque>
#include <iostream>

int main() {
    // Creation - double-ended queue with O(1) insertion/deletion at both ends
    // Alternative: std::vector for O(1) back operations only, better cache locality
    // Alternative: std::list for O(1) insertion/deletion anywhere, but no random access
    std::deque<int> dq = {3, 4, 5};
    
    // Alternative creation methods:
    // std::deque<int> dq;                              // Empty deque
    // std::deque<int> dq(10);                          // 10 default-initialized elements
    // std::deque<int> dq(10, 42);                      // 10 elements, all initialized to 42
    // std::deque<int> dq(vec.begin(), vec.end());      // From range
    
    // Add to front and back - O(1) amortized time
    dq.push_front(2);  // Add to front
    dq.push_back(6);   // Add to back
    
    // Alternative insertion methods:
    // dq.emplace_front(1);                             // Construct in-place at front
    // dq.emplace_back(7);                              // Construct in-place at back
    // dq.insert(dq.begin() + 2, 99);                   // Insert at position (O(n))
    // dq.emplace(dq.begin() + 2, 99);                  // Emplace at position (O(n))
    // dq.insert(dq.begin(), 3, 42);                    // Insert 3 copies of 42
    // dq.insert(dq.begin(), vec.begin(), vec.end());   // Insert range
    
    // Access - O(1) random access
    std::cout << "Front: " << dq.front() << std::endl;  // First element
    std::cout << "Back: " << dq.back() << std::endl;    // Last element
    
    // Random access - O(1) but slightly slower than vector
    std::cout << "At index 2: " << dq[2] << std::endl;  // No bounds checking
    
    // Alternative access methods:
    // std::cout << "At index 2: " << dq.at(2) << std::endl; // With bounds checking (throws)
    // int* ptr = dq.data();                            // NOT available for deque (no contiguous memory)
    
    // Remove from both ends - O(1) time
    dq.pop_front();  // Remove from front
    dq.pop_back();   // Remove from back
    
    // Note: pop_front() and pop_back() don't return values, they just remove
    // To get and remove:
    // int front_val = dq.front();
    // dq.pop_front();
    
    // Other removal methods:
    // dq.erase(dq.begin() + 2);                        // Erase element at position (O(n))
    // dq.erase(dq.begin(), dq.begin() + 3);            // Erase range (O(n))
    // dq.clear();                                      // Remove all elements
    
    // Iteration
    // for (int val : dq) {
    //     std::cout << val << " ";
    // }
    // for (auto it = dq.begin(); it != dq.end(); ++it) {
    //     std::cout << *it << " ";
    // }
    // for (auto it = dq.rbegin(); it != dq.rend(); ++it) { // Reverse iteration
    //     std::cout << *it << " ";
    // }
    
    // Size operations:
    // std::cout << "Size: " << dq.size() << std::endl;
    // std::cout << "Empty: " << dq.empty() << std::endl;
    // std::cout << "Max size: " << dq.max_size() << std::endl;
    
    // Resize operations:
    // dq.resize(10);                                   // Resize to 10 elements
    // dq.resize(10, 42);                               // Resize and fill new elements with 42
    // dq.shrink_to_fit();                              // Request to reduce memory (non-binding)
    
    // Assign operations:
    // dq.assign(5, 100);                               // Replace with 5 copies of 100
    // dq.assign(vec.begin(), vec.end());               // Replace with range
    // dq.assign({1, 2, 3, 4, 5});                      // Replace with initializer list
    
    // Swap:
    // std::deque<int> other = {10, 20, 30};
    // dq.swap(other);                                  // Swap contents (O(1))
    
    // Performance characteristics:
    // - push_front/push_back: O(1) amortized
    // - pop_front/pop_back: O(1)
    // - Random access (operator[], at): O(1) but slower than vector
    // - Insert/erase in middle: O(n)
    // - Memory: Non-contiguous (typically array of arrays/chunks)
    // - Iterator invalidation: insert/erase invalidates all iterators
    
    // Key differences from std::vector:
    // - deque: O(1) front operations, slightly slower random access, non-contiguous memory
    // - vector: O(n) front operations (push_front not available), faster random access, contiguous memory
    
    // Key differences from std::list:
    // - deque: O(1) random access, O(n) middle insert/erase
    // - list: No random access, O(1) middle insert/erase (if you have iterator)
    
    // Use deque when:
    // - Need efficient insertion/deletion at both ends
    // - Need random access (but vector not suitable due to front operations)
    // - Don't need contiguous memory guarantee
    // - Memory usage pattern: typically uses less memory than vector for frequent push/pop at ends
    
    // Use vector when:
    // - Only need back operations (push_back/pop_back)
    // - Need fastest random access
    // - Need contiguous memory (e.g., for C API interop)
    // - Want best cache locality
    
    return 0;
}
```

### Rust: `VecDeque<T>`

Similar double-ended queue implementation.

```rust
use std::collections::VecDeque;

fn main() {
    // Creation - double-ended queue with O(1) insertion/deletion at both ends
    // Alternative: Vec for O(1) back operations only, better cache locality
    // Alternative: LinkedList for O(1) insertion/deletion anywhere, but no random access
    let mut dq = VecDeque::from(vec![3, 4, 5]);
    
    // Alternative creation methods:
    // let mut dq: VecDeque<i32> = VecDeque::new();              // Empty deque
    // let mut dq = VecDeque::with_capacity(10);                 // Pre-allocate capacity
    // let mut dq: VecDeque<_> = [3, 4, 5].iter().copied().collect(); // From iterator
    // let mut dq = VecDeque::from([3, 4, 5]);                   // From array (Rust 2021)
    
    // Add to front and back - O(1) amortized time
    dq.push_front(2);  // Add to front
    dq.push_back(6);   // Add to back
    
    // Access - returns Option<&T>
    println!("Front: {:?}", dq.front());  // First element (Some(&2) or None if empty)
    println!("Back: {:?}", dq.back());    // Last element (Some(&6) or None if empty)
    
    // Alternative access methods:
    // if let Some(front) = dq.front() {                         // Pattern matching
    //     println!("Front: {}", front);
    // }
    // let front_mut = dq.front_mut();                           // Mutable reference
    // let back_mut = dq.back_mut();                             // Mutable reference
    
    // Random access - O(1) but slightly slower than Vec
    println!("At index 2: {}", dq[2]);  // Panics if out of bounds
    
    // Alternative random access:
    // if let Some(val) = dq.get(2) {                            // Safe access, returns Option
    //     println!("At index 2: {}", val);
    // }
    // if let Some(val_mut) = dq.get_mut(2) {                    // Mutable access
    //     *val_mut = 99;
    // }
    
    // Remove from both ends - O(1) time, returns Option<T>
    let front_val = dq.pop_front();  // Remove and return from front (Some(2) or None)
    let back_val = dq.pop_back();    // Remove and return from back (Some(6) or None)
    
    // Pattern matching on pop:
    // if let Some(val) = dq.pop_front() {
    //     println!("Removed front: {}", val);
    // }
    
    // Insert at arbitrary position - O(n) where n is min(index, len - index)
    // dq.insert(2, 99);                                         // Insert 99 at index 2
    
    // Remove from arbitrary position - O(n) where n is min(index, len - index)
    // if let Some(val) = dq.remove(2) {                         // Remove and return element at index
    //     println!("Removed: {}", val);
    // }
    
    // Swap remove (doesn't preserve order, but O(1)):
    // dq.swap_remove_front(2);                                  // Remove index 2, swap with front
    // dq.swap_remove_back(2);                                   // Remove index 2, swap with back
    
    // Iteration
    // for val in &dq {                                          // Immutable iteration
    //     println!("{}", val);
    // }
    // for val in &mut dq {                                      // Mutable iteration
    //     *val *= 2;
    // }
    // for val in dq.iter() {                                    // Explicit iterator
    //     println!("{}", val);
    // }
    // for val in dq.into_iter() {                               // Consuming iteration
    //     println!("{}", val);
    // }
    
    // Rotation operations (unique to VecDeque):
    // dq.rotate_left(2);                                        // Rotate elements left by 2
    // dq.rotate_right(2);                                       // Rotate elements right by 2
    
    // Split operations:
    // let right = dq.split_off(3);                              // Split at index 3, returns right half
    
    // Drain operations:
    // let drained: Vec<_> = dq.drain(1..3).collect();           // Remove and return range
    // dq.drain(..);                                             // Drain all (same as clear but returns iterator)
    
    // Retain operation:
    // dq.retain(|&x| x % 2 == 0);                               // Keep only even numbers
    
    // Size and capacity:
    // println!("Length: {}", dq.len());
    // println!("Is empty: {}", dq.is_empty());
    // println!("Capacity: {}", dq.capacity());
    // dq.reserve(100);                                          // Reserve additional capacity
    // dq.reserve_exact(100);                                    // Reserve exact capacity
    // dq.shrink_to_fit();                                       // Reduce capacity to fit length
    // dq.shrink_to(10);                                         // Shrink capacity to at least 10
    
    // Clear:
    // dq.clear();                                               // Remove all elements
    
    // Extend:
    // dq.extend([7, 8, 9]);                                     // Add multiple elements to back
    // dq.extend(vec![10, 11]);
    
    // Append:
    // let mut other = VecDeque::from([10, 11, 12]);
    // dq.append(&mut other);                                    // Move all from other to back of dq
    
    // Make contiguous (unique to VecDeque):
    // let slice = dq.make_contiguous();                         // Reorganize into contiguous slice
    // This is useful because VecDeque's internal ring buffer may wrap around
    
    // As slices (after make_contiguous):
    // let (front, back) = dq.as_slices();                       // Get internal slices
    // let (front_mut, back_mut) = dq.as_mut_slices();           // Mutable slices
    
    // Swap:
    // dq.swap(0, 2);                                            // Swap elements at indices
    
    // Resize:
    // dq.resize(10, 0);                                         // Resize to 10, fill new with 0
    // dq.resize_with(10, Default::default);                     // Resize with closure
    
    // Truncate:
    // dq.truncate(5);                                           // Keep only first 5 elements
    
    // Binary search (requires sorted deque):
    // match dq.binary_search(&5) {
    //     Ok(index) => println!("Found at {}", index),
    //     Err(index) => println!("Not found, would insert at {}", index),
    // }
    
    // Performance characteristics:
    // - push_front/push_back: O(1) amortized
    // - pop_front/pop_back: O(1)
    // - Random access ([], get): O(1) but slightly slower than Vec
    // - Insert/remove in middle: O(min(index, len-index))
    // - Memory: Ring buffer (single contiguous allocation that wraps around)
    // - Better than Vec for front operations, worse cache locality than Vec
    
    // Key differences from Vec:
    // - VecDeque: O(1) front operations, ring buffer, slightly slower random access
    // - Vec: O(n) front operations (no push_front), contiguous memory, faster random access
    
    // Key differences from LinkedList:
    // - VecDeque: O(1) random access, better cache locality, O(n) middle operations
    // - LinkedList: No random access, O(1) middle operations (with cursor), poor cache locality
    
    // Use VecDeque when:
    // - Need efficient insertion/deletion at both ends
    // - Need random access (but Vec not suitable due to front operations)
    // - Implementing queues, sliding windows, or algorithms that need both-ended access
    
    // Use Vec when:
    // - Only need back operations (push/pop)
    // - Need fastest random access and iteration
    // - Want best cache locality
    
    // Use LinkedList when:
    // - Need O(1) insertion/deletion in middle (rare in Rust, usually VecDeque is better)
    // - Don't need random access at all
}
```
---

## 7. Linked Lists

### C++: `std::list<T>` and `std::forward_list<T>`

Doubly-linked list and singly-linked list respectively.

```cpp
#include <list>          // Doubly-linked list container
#include <forward_list>  // Singly-linked list container
#include <iostream>

int main() {
    // Doubly-linked list
    // - O(1) insertion/deletion at both ends
    // - Bidirectional iteration (forward and backward)
    // - Higher memory overhead (stores both next and prev pointers)
    std::list<int> dll = {1, 2, 3, 4};
    dll.push_front(0);  // Add to front: O(1)
    dll.push_back(5);   // Add to back: O(1)
    
    // Alternative: std::deque for similar functionality with better cache locality
    // std::deque<int> deq = {1, 2, 3, 4};
    // deq.push_front(0);
    // deq.push_back(5);
    
    // Singly-linked list
    // - O(1) insertion/deletion only at front
    // - Forward-only iteration
    // - Lower memory overhead (only next pointer)
    // - No push_back() method (would require O(n) traversal)
    std::forward_list<int> sll = {1, 2, 3};
    sll.push_front(0);  // Add to front: O(1)
    
    // To add to back efficiently, you'd need to track the tail separately
    // or use std::list instead
    
    // Note: Consider std::vector for most use cases
    // - Better cache performance due to contiguous memory
    // - Only use list/forward_list when you need:
    //   * Frequent insertions/deletions in the middle
    //   * Iterator stability (iterators remain valid after modifications)
    //   * No random access required
    
    return 0;
}
```

**Key differences and when to use each:**

- **`std::list`**: Use when you need bidirectional traversal or frequent insertions/deletions at both ends or in the middle
- **`std::forward_list`**: Use when memory is tight and you only need forward iteration (rarely used in practice)
- **`std::vector`**: Best default choice for most scenarios due to cache-friendly contiguous storage
- **`std::deque`**: Good middle ground - supports push/pop at both ends with better performance than `std::list`


### Rust: `LinkedList<T>`

Doubly-linked list (singly-linked list not in std).

```rust
use std::collections::LinkedList;

fn main() {
    // LinkedList is a doubly-linked list
    // - O(1) insertion/deletion at both ends
    // - O(n) indexing (no random access)
    // - Higher memory overhead and poor cache locality
    // NOTE: Rust docs explicitly discourage using LinkedList in most cases
    let mut list = LinkedList::new();
    list.push_back(1);   // Add to back: O(1)
    list.push_back(2);   // Add to back: O(1)
    list.push_front(0);  // Add to front: O(1)
    
    // Iteration: immutable reference
    for item in &list {
        println!("{}", item);
    }
    
    // Alternative iteration methods:
    // for item in list.iter() {           // Explicit iterator
    //     println!("{}", item);
    // }
    
    // for item in &mut list {             // Mutable iteration
    //     *item += 10;
    // }
    
    // for item in list.into_iter() {      // Consuming iteration (moves ownership)
    //     println!("{}", item);
    // }
    // // `list` is no longer accessible here
    
    // PREFERRED ALTERNATIVES:
    
    // 1. Vec<T> - Use this 99% of the time
    // - Contiguous memory = excellent cache performance
    // - O(1) random access and push_back (amortized)
    // - Only O(n) for push_front, but usually faster overall
    // let mut vec = vec![0, 1, 2];
    // vec.push(3);
    
    // 2. VecDeque<T> - Use when you need efficient operations at both ends
    // - Ring buffer implementation
    // - O(1) push/pop at both ends
    // - Better cache locality than LinkedList
    // use std::collections::VecDeque;
    // let mut deque = VecDeque::from([0, 1, 2]);
    // deque.push_front(0);
    // deque.push_back(3);
    
    // ONLY use LinkedList when:
    // - You need to split/merge lists frequently (split_off, append)
    // - You need cursor-based navigation (cursor_front_mut, cursor_back_mut)
    // - You're implementing something like LRU cache with pointer manipulation
    // Even then, consider if VecDeque would work instead
}
```

**Rust-specific notes:**

- **Rust's `LinkedList` is often slower than `Vec`** even for operations where linked lists theoretically excel, due to memory allocator overhead and cache misses
- The Rust documentation itself states: *"It is almost always better to use `Vec` or `VecDeque`"*
- **No singly-linked list** in std (you'd need an external crate or implement your own)
- **Ownership matters**: `into_iter()` consumes the list, `iter()` borrows it

**Note:** Linked lists are rarely needed in both languages due to cache-unfriendly performance. Use `Vec`/`vector` instead unless you specifically need O(1) insertion/removal in the middle.

---

## 8. Binary Heap (Priority Queue)

### C++: `std::priority_queue<T>`

Max-heap by default (can be customized to min-heap).

```cpp
#include <queue>       // For priority_queue
#include <vector>      // Default underlying container
#include <deque>       // Alternative underlying container
#include <functional>  // For std::greater
#include <iostream>

int main() {
    // Max heap (default behavior)
    // - Largest element at top
    // - Uses std::less<int> comparator by default
    // - Underlying container: std::vector<int> by default
    // - Operations: push O(log n), top O(1), pop O(log n)
    std::priority_queue<int> max_heap;
    max_heap.push(3);  // Insert: O(log n)
    max_heap.push(1);
    max_heap.push(5);
    max_heap.push(2);
    
    std::cout << "Max: " << max_heap.top() << std::endl; // 5 - O(1) access
    max_heap.pop();    // Remove top: O(log n)
    
    // Min heap
    // - Smallest element at top
    // - Uses std::greater<int> comparator to reverse order
    // - Template parameters: <Type, Container, Comparator>
    std::priority_queue<int, std::vector<int>, std::greater<int>> min_heap;
    min_heap.push(3);
    min_heap.push(1);
    min_heap.push(5);
    
    std::cout << "Min: " << min_heap.top() << std::endl; // 1
    
    // ALTERNATIVE: Using std::deque as underlying container
    // Slightly different performance characteristics
    // std::priority_queue<int, std::deque<int>, std::greater<int>> min_heap_deque;
    
    // ALTERNATIVE: Custom comparator with lambda (C++11+)
    // auto cmp = [](int a, int b) { return a > b; }; // Min heap behavior
    // std::priority_queue<int, std::vector<int>, decltype(cmp)> custom_heap(cmp);
    
    // ALTERNATIVE: Custom comparator with struct
    // struct Compare {
    //     bool operator()(int a, int b) {
    //         return a > b; // Min heap: return true if a has lower priority than b
    //     }
    // };
    // std::priority_queue<int, std::vector<int>, Compare> custom_heap2;
    
    // ALTERNATIVE: For complex types
    // struct Task {
    //     int priority;
    //     std::string name;
    // };
    // auto task_cmp = [](const Task& a, const Task& b) {
    //     return a.priority < b.priority; // Higher priority value = higher priority
    // };
    // std::priority_queue<Task, std::vector<Task>, decltype(task_cmp)> task_queue(task_cmp);
    
    // Common operations:
    // max_heap.empty();     // Check if empty: O(1)
    // max_heap.size();      // Get size: O(1)
    // No iterator access or ability to modify middle elements
    // (heap property must be maintained)
    
    // NOTE: priority_queue does NOT support:
    // - Random access
    // - Iteration
    // - Searching for specific elements
    // - Updating priorities (must pop and re-push)
    
    // WHEN TO USE:
    // - Need repeated access to min/max element
    // - Implementing algorithms like Dijkstra's, A*, or Huffman coding
    // - Task scheduling based on priority
    // - K-th largest/smallest problems
    
    // WHEN NOT TO USE:
    // - Need to access elements other than top → use std::set/std::multiset
    // - Need to update priorities efficiently → use custom data structure
    // - Need to iterate over all elements → use std::vector + std::make_heap
    
    return 0;
}
```

**Key points:**

- **Default is max heap** (largest element on top)
- **Template signature**: `priority_queue<Type, Container = vector<Type>, Compare = less<Type>>`
- **Min heap requires**: `std::greater<int>` comparator
- **No random access or iteration**: Only top element is accessible
- **Comparator semantics**: Return `true` if first argument has *lower* priority than second
- **Alternative for iteration**: Use `std::vector` with `std::make_heap`, `std::push_heap`, `std::pop_heap` algorithms if you need both heap operations and iteration


### Rust: `BinaryHeap<T>`

Max-heap by default (wrap in `Reverse` for min-heap).

```rust
use std::collections::BinaryHeap;
use std::cmp::Reverse;

fn main() {
    // Max heap (default behavior)
    // - Largest element at top
    // - Elements must implement Ord trait
    // - Underlying implementation: Vec<T> organized as binary heap
    // - Operations: push O(log n), peek O(1), pop O(log n)
    let mut max_heap = BinaryHeap::new();
    max_heap.push(3);  // Insert: O(log n)
    max_heap.push(1);
    max_heap.push(5);
    max_heap.push(2);
    
    println!("Max: {:?}", max_heap.peek()); // Some(5) - O(1) access, returns Option<&T>
    max_heap.pop();    // Remove top: O(log n), returns Option<T>
    
    // Min heap using Reverse wrapper
    // - Wraps each element in Reverse to invert comparison
    // - Need to unwrap Reverse when extracting values
    let mut min_heap = BinaryHeap::new();
    min_heap.push(Reverse(3));
    min_heap.push(Reverse(1));
    min_heap.push(Reverse(5));
    
    println!("Min: {:?}", min_heap.peek()); // Some(Reverse(1))
    
    // Extracting value from min heap
    if let Some(Reverse(min_value)) = min_heap.pop() {
        println!("Popped min: {}", min_value); // 1
    }
    
    // ALTERNATIVE: Custom type with custom Ord implementation
    // #[derive(Eq, PartialEq)]
    // struct Task {
    //     priority: i32,
    //     name: String,
    // }
    // 
    // impl Ord for Task {
    //     fn cmp(&self, other: &Self) -> std::cmp::Ordering {
    //         // Higher priority value = higher priority (max heap)
    //         self.priority.cmp(&other.priority)
    //         // For min heap behavior, reverse: other.priority.cmp(&self.priority)
    //     }
    // }
    // 
    // impl PartialOrd for Task {
    //     fn partial_cmp(&self, other: &Self) -> Option<std::cmp::Ordering> {
    //         Some(self.cmp(other))
    //     }
    // }
    // 
    // let mut task_heap = BinaryHeap::new();
    // task_heap.push(Task { priority: 5, name: "High".to_string() });
    
    // Common operations:
    let heap_size = max_heap.len();           // Get size: O(1)
    let is_empty = max_heap.is_empty();       // Check if empty: O(1)
    // max_heap.clear();                      // Remove all elements: O(n)
    
    // Creating from existing collection:
    let vec = vec![3, 1, 5, 2];
    let heap_from_vec: BinaryHeap<_> = vec.into_iter().collect(); // O(n) heapify
    // Or use BinaryHeap::from(vec);
    
    // Iteration (in arbitrary order, NOT sorted):
    // for item in &max_heap {
    //     println!("{}", item); // Items in heap order, not sorted
    // }
    
    // CONSUMING into sorted Vec:
    let mut sorted_desc: Vec<_> = max_heap.into_sorted_vec(); // O(n log n), consumes heap
    println!("Sorted (descending): {:?}", sorted_desc);
    
    // ALTERNATIVE: Manual drain for ascending order
    let mut new_heap = BinaryHeap::from([5, 2, 8, 1]);
    let mut sorted_asc = Vec::new();
    while let Some(val) = new_heap.pop() {
        sorted_asc.push(val);
    }
    sorted_asc.reverse(); // Now ascending
    
    // RUST-SPECIFIC FEATURES:
    
    // peek_mut() - get mutable reference to top element
    // Must maintain heap property manually if modified
    // let mut heap = BinaryHeap::from([1, 5, 3]);
    // if let Some(mut top) = heap.peek_mut() {
    //     *top += 10; // Modifying doesn't break heap property automatically
    // }
    
    // append() - move all elements from another heap: O(n log n)
    // let mut heap1 = BinaryHeap::from([1, 3]);
    // let mut heap2 = BinaryHeap::from([2, 4]);
    // heap1.append(&mut heap2); // heap2 is now empty
    
    // LIMITATIONS:
    // - No iterator over sorted elements (use into_sorted_vec() to consume)
    // - No efficient priority updates (must remove and re-insert)
    // - No search for specific elements
    // - Can't efficiently get k-th largest without popping k times
    
    // WHEN TO USE:
    // - Need repeated access to max (or min with Reverse) element
    // - Implementing Dijkstra's algorithm, A*, Huffman coding
    // - Priority-based task scheduling
    // - Finding k largest/smallest elements
    // - Streaming median algorithms
    
    // WHEN NOT TO USE:
    // - Need sorted iteration → use BTreeSet or sort a Vec
    // - Need to update priorities efficiently → use custom data structure
    // - Need to search for elements → use HashMap or BTreeMap
    // - Need both min and max efficiently → maintain two heaps or use BTreeSet
    
    // PERFORMANCE NOTES:
    // - Better cache locality than tree-based structures
    // - Worse constant factors than C++ priority_queue due to bounds checking
    // - from_iter()/collect() uses Floyd's O(n) heapify algorithm (very efficient)
}
```

**Key Rust-specific differences from C++:**

- **`peek()` returns `Option<&T>`** (not direct reference) - idiomatic Rust error handling
- **`Reverse` wrapper** for min heap (vs C++ `std::greater`)
- **`into_sorted_vec()`** - consumes heap and returns sorted Vec (unique to Rust)
- **`peek_mut()`** - allows modifying top element with mutable reference
- **Ownership semantics**: `pop()` moves value out, iteration borrows
- **No custom comparator parameter** - must implement `Ord` on type or use `Reverse`
- **Better type safety**: Can't accidentally create invalid heap state

**Min heap pattern comparison:**
- **Rust**: `BinaryHeap<Reverse<T>>` - wrap each value
- **C++**: `priority_queue<T, vector<T>, greater<T>>` - template parameter

---

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