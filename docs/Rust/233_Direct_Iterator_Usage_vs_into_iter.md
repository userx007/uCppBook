# Direct Iterator Usage vs `into_iter()` in Rust

## Overview

In Rust, there are two common patterns for iterating over collections:

```rust
// Pattern 1: Direct iterator usage
for item in collection.iter() { }

// Pattern 2: Using into_iter()
for item in collection.into_iter() { }

// Pattern 3: Implicit into_iter() call
for item in collection { }
```

Understanding when these behave identically and when they differ is crucial for writing idiomatic Rust code.

## The Core Distinction

### Explicit Iterator Methods

Collections in Rust provide three explicit iterator methods:

- **`iter()`** - Borrows each element immutably, yielding `&T`
- **`iter_mut()`** - Borrows each element mutably, yielding `&mut T`
- **`into_iter()`** - Consumes the collection, yielding owned values `T`

### The `IntoIterator` Trait

The `IntoIterator` trait is the foundation of Rust's `for` loop syntax:

```rust
pub trait IntoIterator {
    type Item;
    type IntoIter: Iterator<Item = Self::Item>;
    
    fn into_iter(self) -> Self::IntoIter;
}
```

When you write `for item in collection`, Rust automatically calls `collection.into_iter()`.

## The Blanket Implementation

Here's where things get interesting. The standard library provides a **blanket implementation** of `IntoIterator` for all iterator types:

```rust
impl<I: Iterator> IntoIterator for I {
    type Item = I::Item;
    type IntoIter = I;
    
    fn into_iter(self) -> I {
        self
    }
}
```

### What This Means

This blanket implementation states: **any type that already implements `Iterator` automatically implements `IntoIterator` by simply returning itself**.

This is why the following two approaches are functionally identical:

```rust
let vec = vec![1, 2, 3];

// Direct iterator usage
for item in vec.iter() {
    println!("{}", item);
}

// Calling into_iter() on an iterator
for item in vec.iter().into_iter() {
    println!("{}", item);
}
```

Both produce the exact same result because `vec.iter()` returns an `Iter<'_, i32>`, which implements `Iterator`. When you call `.into_iter()` on it, the blanket implementation kicks in and just returns the iterator unchanged.

## When They Behave the Same

The patterns `collection.iter()` and `collection.iter().into_iter()` are functionally identical due to the blanket implementation. The extra `.into_iter()` call is a no-op that simply returns the iterator.

```rust
let numbers = vec![1, 2, 3, 4, 5];

// These three are equivalent
let sum1: i32 = numbers.iter().sum();
let sum2: i32 = numbers.iter().into_iter().sum();
let sum3: i32 = numbers.iter().sum();

assert_eq!(sum1, sum2);
assert_eq!(sum2, sum3);
```

## When They Behave Differently

The real distinction emerges when comparing direct collection usage with explicit iterator methods:

```rust
let vec = vec![String::from("a"), String::from("b")];

// Pattern 1: Implicit into_iter() - consumes the Vec
for item in vec {
    println!("{}", item); // item is String (owned)
}
// vec is no longer accessible here

let vec = vec![String::from("a"), String::from("b")];

// Pattern 2: Explicit iter() - borrows the Vec
for item in vec.iter() {
    println!("{}", item); // item is &String (borrowed)
}
// vec is still accessible here
println!("Vec still exists: {:?}", vec);
```

### The Three IntoIterator Implementations

Most collections implement `IntoIterator` three times, for different receiver types:

```rust
// For Vec<T> as an example:

// 1. Consuming the collection
impl<T> IntoIterator for Vec<T> {
    type Item = T;
    // Yields owned values
}

// 2. Borrowing immutably
impl<'a, T> IntoIterator for &'a Vec<T> {
    type Item = &'a T;
    // Yields immutable references
}

// 3. Borrowing mutably
impl<'a, T> IntoIterator for &'a mut Vec<T> {
    type Item = &'a mut T;
    // Yields mutable references
}
```

This is why these patterns work:

```rust
let mut vec = vec![1, 2, 3];

// Borrows immutably (calls IntoIterator for &Vec<T>)
for item in &vec {
    println!("{}", item); // &i32
}

// Borrows mutably (calls IntoIterator for &mut Vec<T>)
for item in &mut vec {
    *item += 10; // &mut i32
}

// Consumes (calls IntoIterator for Vec<T>)
for item in vec {
    println!("{}", item); // i32
}
```

## Practical Examples

### Example 1: Chain and Collect

```rust
let vec1 = vec![1, 2, 3];
let vec2 = vec![4, 5, 6];

// Both work identically due to blanket implementation
let combined1: Vec<_> = vec1.iter()
    .chain(vec2.iter())
    .collect();

let combined2: Vec<_> = vec1.iter().into_iter()
    .chain(vec2.iter().into_iter())
    .collect();

assert_eq!(combined1, combined2);
```

### Example 2: Filter and Map

```rust
let numbers = vec![1, 2, 3, 4, 5];

// Direct iterator usage (idiomatic)
let doubled: Vec<_> = numbers.iter()
    .filter(|&&x| x % 2 == 0)
    .map(|&x| x * 2)
    .collect();

// With explicit into_iter() (redundant but identical)
let doubled2: Vec<_> = numbers.iter().into_iter()
    .filter(|&&x| x % 2 == 0)
    .map(|&x| x * 2)
    .collect();

assert_eq!(doubled, doubled2);
```

### Example 3: Ownership Considerations

```rust
let strings = vec![
    String::from("hello"),
    String::from("world")
];

// Using iter() - doesn't consume
let lengths: Vec<_> = strings.iter()
    .map(|s| s.len())
    .collect();

println!("Original: {:?}", strings); // Still accessible

// Using into_iter() on the collection - consumes
let strings2 = vec![
    String::from("hello"),
    String::from("world")
];

let owned: Vec<_> = strings2.into_iter()
    .collect();

// println!("{:?}", strings2); // Error: value moved
```

## When to Use Each Pattern

### Use Direct Iterator Methods When:

1. **You need explicit control over ownership**: Choose between `iter()`, `iter_mut()`, or `into_iter()` based on whether you want to borrow or consume
2. **You're chaining iterator adaptors**: More explicit about what you're doing
3. **You want to preserve the collection**: Use `iter()` or `iter_mut()` to keep the original collection

### Use Implicit `into_iter()` (bare collection in `for` loop) When:

1. **You want the most concise code**: `for item in &collection` is idiomatic
2. **The semantics are clear from context**: Using `&collection` or `&mut collection` makes intent obvious
3. **You're consuming the collection**: `for item in collection` clearly shows ownership transfer

### Avoid:

Calling `.into_iter()` on something that's already an iterator (like `.iter().into_iter()`) is redundant and adds no value due to the blanket implementation.

## Key Takeaways

1. The blanket implementation `impl<I: Iterator> IntoIterator for I` makes calling `.into_iter()` on an iterator a no-op
2. Direct iterator methods (`iter()`, `iter_mut()`, `into_iter()`) give explicit control over borrowing vs consuming
3. The `for` loop syntax implicitly calls `into_iter()`, which behaves differently based on whether you pass a collection, `&collection`, or `&mut collection`
4. Collections typically implement `IntoIterator` three times: for `T`, `&T`, and `&mut T`, each yielding different item types
5. Understanding these patterns helps you write more idiomatic Rust code and avoid unnecessary ownership complications