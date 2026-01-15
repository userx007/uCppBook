# Memory Management Strategies: C++ vs. Rust

## Overview

Memory management is one of the most critical aspects of systems programming, and C++ and Rust take fundamentally different approaches to solving this problem. While C++ provides powerful manual control with optional safety features, Rust enforces memory safety at compile-time through its innovative ownership system.

## C++ Memory Management

### Manual Memory Management

C++ gives programmers direct control over memory allocation and deallocation using `new` and `delete` operators.

```cpp
#include <iostream>
#include <cstring>

class Player {
public:
    std::string name;
    int score;
    
    Player(const std::string& n, int s) : name(n), score(s) {
        std::cout << "Player created: " << name << std::endl;
    }
    
    ~Player() {
        std::cout << "Player destroyed: " << name << std::endl;
    }
};

void manualMemoryExample() {
    // Manual allocation on heap
    Player* player1 = new Player("Alice", 100);
    
    // Use the object
    std::cout << player1->name << " has score: " << player1->score << std::endl;
    
    // Manual deallocation - programmer's responsibility
    delete player1;
    
    // Array allocation
    int* numbers = new int[10];
    for(int i = 0; i < 10; i++) {
        numbers[i] = i * i;
    }
    delete[] numbers;  // Must use delete[] for arrays
    
    // Common pitfalls:
    // 1. Memory leak if delete is forgotten
    // 2. Double delete causes undefined behavior
    // 3. Use after delete (dangling pointer)
    // 4. Mismatched new/delete and new[]/delete[]
}
```

### RAII (Resource Acquisition Is Initialization)

RAII is C++'s primary idiom for automatic resource management, where resources are tied to object lifetime.

```cpp
#include <memory>
#include <fstream>
#include <vector>

// RAII with smart pointers
void raiiSmartPointerExample() {
    // unique_ptr: exclusive ownership
    std::unique_ptr<Player> player1 = std::make_unique<Player>("Bob", 200);
    // Automatically deleted when player1 goes out of scope
    
    // shared_ptr: shared ownership with reference counting
    std::shared_ptr<Player> player2 = std::make_shared<Player>("Charlie", 300);
    {
        std::shared_ptr<Player> player2_copy = player2;  // Reference count: 2
        std::cout << "Reference count: " << player2.use_count() << std::endl;
    }  // player2_copy destroyed, reference count: 1
    
    std::cout << "Reference count: " << player2.use_count() << std::endl;
    // player2 automatically deleted when last shared_ptr goes out of scope
}

// RAII with custom resources
class FileHandler {
private:
    std::FILE* file;
    
public:
    FileHandler(const char* filename, const char* mode) {
        file = std::fopen(filename, mode);
        if (!file) {
            throw std::runtime_error("Failed to open file");
        }
    }
    
    ~FileHandler() {
        if (file) {
            std::fclose(file);
            std::cout << "File closed automatically" << std::endl;
        }
    }
    
    // Delete copy operations to prevent double-close
    FileHandler(const FileHandler&) = delete;
    FileHandler& operator=(const FileHandler&) = delete;
    
    std::FILE* get() { return file; }
};

void raiiFileExample() {
    FileHandler handler("data.txt", "w");
    std::fprintf(handler.get(), "Hello, RAII!\n");
    // File automatically closed when handler goes out of scope
}
```

### Custom Allocators

C++ allows fine-grained control over memory allocation through custom allocators.

```cpp
#include <vector>
#include <memory>

// Simple custom allocator example
template<typename T>
class PoolAllocator {
private:
    static const size_t POOL_SIZE = 1024;
    char pool[POOL_SIZE];
    size_t offset = 0;
    
public:
    using value_type = T;
    
    PoolAllocator() noexcept {}
    
    template<typename U>
    PoolAllocator(const PoolAllocator<U>&) noexcept {}
    
    T* allocate(size_t n) {
        size_t bytes = n * sizeof(T);
        if (offset + bytes > POOL_SIZE) {
            throw std::bad_alloc();
        }
        T* result = reinterpret_cast<T*>(pool + offset);
        offset += bytes;
        return result;
    }
    
    void deallocate(T*, size_t) noexcept {
        // Simple pool doesn't support individual deallocation
    }
};

void customAllocatorExample() {
    // Vector using custom allocator
    std::vector<int, PoolAllocator<int>> vec;
    vec.push_back(42);
    vec.push_back(100);
}
```

## Rust Memory Management

### Ownership System

Rust's ownership system is its defining feature, providing memory safety without garbage collection.

```rust
// Ownership rules:
// 1. Each value has a single owner
// 2. When the owner goes out of scope, the value is dropped
// 3. Values can be moved or borrowed

fn ownership_basics() {
    // s1 owns the String
    let s1 = String::from("hello");
    
    // Ownership moved to s2, s1 is no longer valid
    let s2 = s1;
    // println!("{}", s1);  // ERROR: s1 no longer owns the data
    println!("{}", s2);
    
    // Automatic cleanup when s2 goes out of scope
}

struct Player {
    name: String,
    score: i32,
}

impl Drop for Player {
    fn drop(&mut self) {
        println!("Player {} is being dropped", self.name);
    }
}

fn ownership_with_structs() {
    let player1 = Player {
        name: String::from("Alice"),
        score: 100,
    };
    
    // Move ownership to function
    take_ownership(player1);
    // println!("{}", player1.name);  // ERROR: player1 was moved
}

fn take_ownership(player: Player) {
    println!("{} has score {}", player.name, player.score);
    // player is dropped here
}

// Returning ownership
fn create_player() -> Player {
    Player {
        name: String::from("Bob"),
        score: 200,
    }
}
```

### Borrowing and References

Rust allows temporary access to data without transferring ownership through borrowing.

```rust
fn borrowing_example() {
    let mut data = String::from("hello");
    
    // Immutable borrow
    let len = calculate_length(&data);
    println!("Length: {}, Data: {}", len, data);  // data still valid
    
    // Mutable borrow
    modify_string(&mut data);
    println!("Modified: {}", data);
}

fn calculate_length(s: &String) -> usize {
    s.len()
    // s is not dropped here because we don't own it
}

fn modify_string(s: &mut String) {
    s.push_str(", world");
}

// Borrowing rules prevent data races at compile time
fn borrowing_rules_demo() {
    let mut value = 42;
    
    // Multiple immutable borrows are OK
    let ref1 = &value;
    let ref2 = &value;
    println!("{} and {}", ref1, ref2);
    
    // But can't have mutable borrow while immutable borrows exist
    let mut_ref = &mut value;
    // println!("{}", ref1);  // ERROR: can't use ref1 here
    *mut_ref += 10;
    println!("{}", mut_ref);
}
```

### Box and Heap Allocation

`Box<T>` provides explicit heap allocation in Rust.

```rust
fn box_example() {
    // Allocate on heap
    let boxed_player = Box::new(Player {
        name: String::from("Charlie"),
        score: 300,
    });
    
    println!("{} has score {}", boxed_player.name, boxed_player.score);
    // Automatically deallocated when boxed_player goes out of scope
}

// Recursive types require indirection via Box
enum List {
    Cons(i32, Box<List>),
    Nil,
}

fn recursive_structure() {
    use List::{Cons, Nil};
    
    let list = Cons(1, 
                    Box::new(Cons(2, 
                        Box::new(Cons(3, 
                            Box::new(Nil))))));
}

// Moving large data efficiently
struct LargeStruct {
    data: [u8; 10000],
}

fn box_for_large_data() {
    // Allocate large struct on heap to avoid stack overflow
    let large = Box::new(LargeStruct {
        data: [0; 10000],
    });
    
    // Passing Box only moves pointer, not all data
    process_large(large);
}

fn process_large(data: Box<LargeStruct>) {
    // Work with data
}
```

### Reference Counting

Rust provides `Rc<T>` and `Arc<T>` for shared ownership scenarios.

```rust
use std::rc::Rc;
use std::sync::Arc;
use std::thread;

// Rc: Single-threaded reference counting
fn rc_example() {
    let player = Rc::new(Player {
        name: String::from("David"),
        score: 400,
    });
    
    // Clone increases reference count, doesn't clone data
    let player_ref1 = Rc::clone(&player);
    let player_ref2 = Rc::clone(&player);
    
    println!("Reference count: {}", Rc::strong_count(&player));
    
    {
        let player_ref3 = Rc::clone(&player);
        println!("Reference count: {}", Rc::strong_count(&player));
    }  // player_ref3 dropped, count decreases
    
    println!("Reference count: {}", Rc::strong_count(&player));
}

// Arc: Thread-safe reference counting
fn arc_example() {
    let shared_data = Arc::new(vec![1, 2, 3, 4, 5]);
    
    let mut handles = vec![];
    
    for i in 0..3 {
        let data_clone = Arc::clone(&shared_data);
        let handle = thread::spawn(move || {
            println!("Thread {} sees: {:?}", i, data_clone);
        });
        handles.push(handle);
    }
    
    for handle in handles {
        handle.join().unwrap();
    }
    
    println!("Main thread still has access: {:?}", shared_data);
}

// RefCell for interior mutability with Rc
use std::cell::RefCell;

fn rc_refcell_example() {
    let shared_value = Rc::new(RefCell::new(5));
    
    let a = Rc::clone(&shared_value);
    let b = Rc::clone(&shared_value);
    
    *a.borrow_mut() += 10;  // Runtime-checked mutable borrow
    *b.borrow_mut() += 20;
    
    println!("Final value: {}", shared_value.borrow());
}
```

### Memory Safety Guarantees

```rust
// No dangling pointers - compiler prevents this
fn no_dangling_pointers() {
    let reference;
    {
        let value = String::from("temporary");
        // reference = &value;  // ERROR: value doesn't live long enough
    }
    // println!("{}", reference);
}

// No data races - enforced at compile time
fn no_data_races() {
    let mut data = vec![1, 2, 3];
    
    // Can't have mutable and immutable references simultaneously
    let read_ref = &data;
    // data.push(4);  // ERROR: can't mutate while borrowed
    println!("{:?}", read_ref);
    
    data.push(4);  // OK now, read_ref no longer used
}

// No use-after-free
fn no_use_after_free() {
    let data = Box::new(42);
    let moved_data = data;
    // println!("{}", data);  // ERROR: value moved
    println!("{}", moved_data);
}
```

## Comparison Summary

| Feature | C++ | Rust |
|---------|-----|------|
| **Default Allocation** | Stack-based, manual heap with `new` | Stack-based, explicit heap with `Box` |
| **Deallocation** | Manual `delete` or automatic with RAII | Automatic when owner goes out of scope |
| **Safety Model** | Opt-in safety (smart pointers) | Enforced at compile-time |
| **Ownership** | Implicit, tracked by programmer | Explicit, tracked by compiler |
| **Shared Ownership** | `shared_ptr` (runtime overhead) | `Rc`/`Arc` (explicit, minimal overhead) |
| **Unique Ownership** | `unique_ptr` | Default behavior, `Box` for heap |
| **Dangling Pointers** | Possible, runtime errors | Prevented at compile-time |
| **Memory Leaks** | Possible with manual management | Possible with `Rc` cycles, rare otherwise |
| **Data Races** | Possible, detected at runtime | Prevented at compile-time |
| **Use-After-Free** | Possible, undefined behavior | Prevented at compile-time |
| **Performance** | Zero-cost abstractions, full control | Zero-cost abstractions, borrow checker overhead |
| **Learning Curve** | Moderate, pitfalls with manual management | Steep, fighting the borrow checker initially |
| **Custom Allocators** | Full support, standard interface | Limited, requires unsafe code |
| **Garbage Collection** | No (optional libraries exist) | No |
| **Runtime Overhead** | Minimal for manual, some for smart pointers | Minimal, no runtime checks for safety |

## Key Differences

**Philosophy**: C++ trusts programmers to manage memory correctly and provides tools to help, while Rust assumes programmers will make mistakes and uses the compiler to prevent them.

**Flexibility vs Safety**: C++ offers maximum flexibility with optional safety features. Rust enforces safety by default, requiring explicit use of `unsafe` for operations the compiler can't verify.

**Error Detection**: C++ catches memory errors at runtime (if at all), often resulting in undefined behavior. Rust catches most memory errors at compile-time, preventing them from ever occurring.

**Performance**: Both languages achieve zero-cost abstractions for their respective safety models. C++ may have an edge in scenarios requiring custom memory layouts, while Rust's safety guarantees come with no runtime cost but may require more upfront design consideration.