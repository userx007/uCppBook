# Smart pointers in C++ and Rust

The key differences to remember are:

- **Ownership semantics:** Rust enforces ownership at compile-time, while C++ relies on conventions
- **Thread safety:** Rust separates single-threaded (`Rc`) from multi-threaded (`Arc`) pointers for performance, while C++ `shared_ptr` is always atomic
- **Mutability:** Rust requires explicit interior mutability types like `RefCell` or `Mutex`, while C++ allows direct mutation
- **Safety guarantees:** Rust prevents data races and dangling pointers at compile-time

# Smart Pointers: C++ vs Rust Cheat Sheet

## Basic Equivalents

| C++ | Rust | Purpose |
|-----|------|---------|
| `std::unique_ptr<T>` | `Box<T>` | Unique ownership, heap allocation |
| `std::shared_ptr<T>` | `Rc<T>` | Shared ownership (single-threaded) |
| `std::shared_ptr<T>` | `Arc<T>` | Shared ownership (thread-safe) |
| `std::weak_ptr<T>` | `Weak<T>` | Non-owning reference to `Rc<T>` |
| `std::weak_ptr<T>` | `std::sync::Weak<T>` | Non-owning reference to `Arc<T>` |
| Raw pointer `T*` | `*const T` / `*mut T` | Unsafe raw pointers |
| Reference `T&` | `&T` | Borrowed reference (immutable) |
| Reference `T&` | `&mut T` | Borrowed reference (mutable) |

## Unique Ownership

### C++ `std::unique_ptr<T>`
```cpp
auto ptr = std::make_unique<int>(42);
auto moved = std::move(ptr); // Explicit move required
// ptr is now null
```

### Rust `Box<T>`
```rust
let ptr = Box::new(42);
let moved = ptr; // Automatic move
// ptr can no longer be used
```

**Key Difference:** Rust enforces move semantics at compile-time through ownership rules.

## Shared Ownership (Single-threaded)

### C++ `std::shared_ptr<T>`
```cpp
auto ptr1 = std::make_shared<int>(42);
auto ptr2 = ptr1; // Reference count: 2
// Both can access the value
```

### Rust `Rc<T>`
```rust
use std::rc::Rc;

let ptr1 = Rc::new(42);
let ptr2 = Rc::clone(&ptr1); // Reference count: 2
// Both can access the value
```

**Key Difference:** Rust's `Rc` is explicitly not thread-safe, preventing accidental data races.

## Shared Ownership (Thread-safe)

### C++ `std::shared_ptr<T>` (always atomic)
```cpp
auto ptr = std::make_shared<int>(42);
// Thread-safe reference counting (always atomic overhead)
```

### Rust `Arc<T>` (Atomic Reference Counted)
```rust
use std::sync::Arc;

let ptr = Arc::new(42);
let ptr2 = Arc::clone(&ptr);
// Thread-safe reference counting
```

**Key Difference:** Rust separates single-threaded (`Rc`) from multi-threaded (`Arc`) to avoid unnecessary atomic overhead.

## Weak Pointers

### C++ `std::weak_ptr<T>`
```cpp
auto shared = std::make_shared<int>(42);
std::weak_ptr<int> weak = shared;
if (auto locked = weak.lock()) {
    // Use locked as shared_ptr
}
```

### Rust `Weak<T>`
```rust
use std::rc::{Rc, Weak};

let shared = Rc::new(42);
let weak: Weak<i32> = Rc::downgrade(&shared);
if let Some(locked) = weak.upgrade() {
    // Use locked as Rc
}
```

**Key Difference:** Similar semantics, but Rust's version is tied to either `Rc` or `Arc`.

## Interior Mutability

C++ allows mutation through shared pointers directly, while Rust requires explicit interior mutability types:

### C++ (mutable through shared_ptr)
```cpp
auto ptr = std::make_shared<int>(42);
*ptr = 100; // Can mutate through shared pointer
```

### Rust (requires RefCell or Mutex)
```rust
use std::cell::RefCell;
use std::rc::Rc;

let ptr = Rc::new(RefCell::new(42));
*ptr.borrow_mut() = 100; // Runtime borrow checking

// For Arc (thread-safe):
use std::sync::{Arc, Mutex};
let ptr = Arc::new(Mutex::new(42));
*ptr.lock().unwrap() = 100; // Thread-safe mutation
```

## Reference Counting Patterns

| Pattern | C++ | Rust (single-thread) | Rust (multi-thread) |
|---------|-----|---------------------|---------------------|
| Shared immutable data | `shared_ptr<const T>` | `Rc<T>` | `Arc<T>` |
| Shared mutable data | `shared_ptr<T>` | `Rc<RefCell<T>>` | `Arc<Mutex<T>>` |
| Weak reference | `weak_ptr<T>` | `Weak<T>` | `sync::Weak<T>` |

## Key Philosophical Differences

1. **Default Mutability:** C++ allows mutation by default; Rust requires explicit opt-in via `RefCell` or `Mutex`.

2. **Thread Safety:** C++ `shared_ptr` is always atomic (performance cost); Rust separates `Rc` (fast, single-threaded) from `Arc` (atomic, thread-safe).

3. **Borrow Checking:** Rust enforces borrowing rules at compile-time for references, runtime for `RefCell`, making many memory errors impossible.

4. **Nullability:** C++ smart pointers can be null; Rust uses `Option<Box<T>>` to make nullability explicit.

5. **Memory Safety:** Rust guarantees no data races and no dangling pointers at compile-time, while C++ relies on programmer discipline.

## Quick Decision Guide

**Use `Box<T>` when:**
- You need heap allocation with unique ownership
- Equivalent to `unique_ptr<T>`

**Use `Rc<T>` when:**
- Multiple owners needed (single-threaded only)
- Equivalent to `shared_ptr<T>` in single-threaded code

**Use `Arc<T>` when:**
- Multiple owners across threads needed
- Equivalent to `shared_ptr<T>` in multi-threaded code

**Add `RefCell` or `Mutex` when:**
- You need mutation through shared ownership
- `RefCell` for single-threaded, `Mutex` for multi-threaded