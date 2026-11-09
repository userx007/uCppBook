# Closure parameter type


- The closure parameter type depends on the specific method and how it's designed.
- The difference in the patterns `|&&x|` vs `|&x|` is just about **how much we choose to unpack** in the pattern, not what they receive.

## EXPLANATION OF `|&&x|` PATTERN MATCHING


```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5, 6, 7, 8, 9, 10];
    
    // EXPLANATION OF |&&x| PATTERN MATCHING
    
    // numbers.iter() produces an iterator of &i32 (references to i32)
    // find() takes a closure that receives each item from the iterator
    // So the closure receives: &&i32 (reference to reference)
    
    // Let's break down why we have double references:
    // 1. iter() yields &i32 (references to elements in the Vec)
    // 2. find()'s closure receives &T where T is what iter() yields
    // 3. So closure receives &(&i32) which is &&i32
    
    let found = numbers.iter().find(|&&x| x > 5);
    //                               ^^
    //                               ||
    //                               |+-- Second & dereferences &i32 to i32
    //                               +--- First & pattern matches &&i32
    // Result: x has type i32
    
    match found {
        Some(&value) => println!("Found: {}", value),
        //    ^
        //    Pattern matches &i32 to extract i32
        None => println!("Not found"),
    }
    
    // ALTERNATIVE WAYS TO WRITE THE SAME THING:
    
    // 1. Using single reference pattern with explicit dereference
    let found = numbers.iter().find(|&x| *x > 5);
    //                               ^    ^
    //                               |    Dereference &i32 to i32
    //                               Pattern matches &&i32 to &i32
    
    // 2. Using double reference in the body (no pattern matching)
    let found = numbers.iter().find(|x| **x > 5);
    //                               ^   ^^
    //                               |   Double dereference &&i32 to i32
    //                               x has type &&i32
    
    // 3. Using no pattern matching and comparing references (works due to auto-deref)
    let found = numbers.iter().find(|x| *x > &5);
    //                                  ^    ^
    //                                  |    Comparing &i32 with &i32
    //                                  Single deref &&i32 to &i32
    
    // 4. Most explicit version - showing all types
    let found: Option<&i32> = numbers.iter().find(|item: &&i32| -> bool {
        let x: i32 = **item;  // Double dereference to get the actual i32
        x > 5
    });
    
    // VISUAL BREAKDOWN OF |&&x|:
    //
    // numbers.iter() → Iterator<Item = &i32>
    //                           ↓
    //        find() closure receives &(&i32) = &&i32
    //                           ↓
    //        Pattern |&&x| unpacks:
    //        - First & matches the outer reference
    //        - Second & matches the inner reference  
    //        - x is bound to the actual i32 value
    
    // COMPARISON WITH OTHER ITERATOR METHODS:
    
    // into_iter() - consumes the Vec, yields owned values
    let v1 = vec![1, 2, 3];
    let found1 = v1.into_iter().find(|&x| x > 1);  // Only one & needed
    //                                ^
    //                                Closure receives &i32 (not &&i32)
    // v1 is no longer usable here
    
    // iter() - borrows the Vec, yields references
    let v2 = vec![1, 2, 3];
    let found2 = v2.iter().find(|&&x| x > 1);  // Two && needed
    //                           ^^
    //                           Closure receives &&i32
    // v2 is still usable here
    
    // iter_mut() - mutably borrows the Vec, yields mutable references
    let mut v3 = vec![1, 2, 3];
    let found3 = v3.iter_mut().find(|x| **x > 1);  // Receives &&mut i32
    //                               ^   ^^
    //                               |   Need to deref twice
    //                               x has type &mut &mut i32
    
    // WHY DOES find() TAKE A REFERENCE TO EACH ITEM?
    // Answer: So the closure can inspect items without taking ownership
    // This allows the iterator to continue being used after find()
    
    let v4 = vec![1, 2, 3, 4, 5];
    let first_even = v4.iter().find(|&&x| x % 2 == 0);
    let first_odd = v4.iter().find(|&&x| x % 2 != 0);
    // v4 is still valid and can be used again
    
    // COMMON MISTAKE:
    // let found = numbers.iter().find(|&x| x > 5);
    //                                      ^ ERROR: can't compare &i32 with i32
    // This fails because x would be &i32, not i32
    
    // WHEN TO USE EACH PATTERN:
    // - |&&x| → When you want the actual value (most common)
    // - |&x|  → When you want a reference to the value
    // - |x|   → When you want to work with the double reference directly
    
    println!("Found: {:?}", found);
}
```

**Summary:**

The `|&&x|` syntax is **pattern matching** that unpacks nested references:

1. **`numbers.iter()`** yields `&i32` (references to elements)
2. **`find()`** passes `&T` to its closure, where `T = &i32`
3. So the closure receives **`&&i32`** (reference to a reference)
4. **`|&&x|`** unpacks both layers: first `&` matches outer ref, second `&` matches inner ref
5. **`x`** ends up as type `i32` (the actual value)

**Think of it like nested pattern matching:**
```rust
// These are equivalent:
|&&x| x > 5
|item| match item {
    &&x => x > 5
}
```

The double reference exists to prevent the closure from taking ownership of items from the iterator, allowing the iterator to remain valid for further operations.

---

## The closure parameter type depends on the specific method and how it's designed.

```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5];

    // RULE: each iterator method decides what it passes to the closure.
    // There is NO universal rule that closures receive &T.

    // ============================================================
    // PREDICATES THAT TAKE A REFERENCE TO THE ITEM: FnMut(&Self::Item)
    //   find, filter, take_while, skip_while, inspect, partition
    // With iter() the item is &i32, so the closure receives &&i32.
    // ============================================================
    let found = numbers.iter().find(|&&x| x > 3);                    // x: i32
    let evens: Vec<&i32> = numbers.iter().filter(|&&x| x % 2 == 0).collect();
    let small: Vec<&i32> = numbers.iter().take_while(|&&x| x < 3).collect();
    println!("find: {:?}, filter: {:?}, take_while: {:?}", found, evens, small);

    // ============================================================
    // METHODS THAT TAKE THE ITEM BY VALUE: FnMut(Self::Item)
    //   map, for_each, fold, any, all, position, filter_map, find_map
    // With iter() the item is &i32, so the closure receives &i32.
    // ============================================================
    let doubled: Vec<i32> = numbers.iter().map(|&x| x * 2).collect();
    let sum = numbers.iter().fold(0, |acc, &x| acc + x);
    let has_even = numbers.iter().any(|&x| x % 2 == 0);
    let all_positive = numbers.iter().all(|&x| x > 0);
    let pos = numbers.iter().position(|&x| x == 3);
    numbers.iter().for_each(|&x| print!("{} ", x));
    println!();
    println!("map: {:?}, fold: {}, any: {}, all: {}, position: {:?}",
             doubled, sum, has_even, all_positive, pos);

    // ============================================================
    // THE SAME METHODS WITH into_iter(): the item is an owned i32
    // ============================================================
    let found = numbers.clone().into_iter().find(|&x| x > 3);        // find gets &i32
    let doubled: Vec<i32> = numbers.clone().into_iter().map(|x| x * 2).collect(); // map gets i32
    let all_positive = numbers.clone().into_iter().all(|x| x > 0);   // all gets i32
    println!("into_iter find: {:?}, map: {:?}, all: {}", found, doubled, all_positive);

    // ============================================================
    // COMPARISON TABLE: closure parameter type when using iter()
    // ============================================================
    // Method        | Closure receives
    // --------------|-------------------------------------------
    // find()        | &&i32   (FnMut(&Self::Item))
    // filter()      | &&i32
    // take_while()  | &&i32
    // skip_while()  | &&i32
    // inspect()     | &&i32
    // --------------|-------------------------------------------
    // map()         | &i32    (FnMut(Self::Item))
    // for_each()    | &i32
    // any()         | &i32
    // all()         | &i32
    // position()    | &i32
    // fold()        | (acc, &i32)
}
```

**Key Takeaway:**

The closure parameter type is **determined by each method's signature**, not a universal rule:

- **`find()`, `filter()`, `take_while()`, `skip_while()`, `inspect()`** → take **`&Self::Item`** (a reference to the iterator item)
- **`map()`, `for_each()`, `fold()`, `any()`, `all()`, `position()`, `filter_map()`, `find_map()`** → take **`Self::Item`** (the iterator item by value)

When using **`.iter()`** (which yields `&T`):
- Methods taking `&Self::Item` → closure gets `&&T`
- Methods taking `Self::Item` → closure gets `&T`

When using **`.into_iter()`** (which yields `T`):
- Methods taking `&Self::Item` → closure gets `&T`
- Methods taking `Self::Item` → closure gets `T`

---

## `find()` vs `all()`: same pattern syntax, different parameter types

`find()` and `all()` do **not** receive the same type. With `.iter()` (which yields `&i32`):

- **`find()`** takes `&Self::Item`, so its closure receives **`&&i32`**.
- **`all()`** (and `any()`, `position()`) take `Self::Item` by value, so their closure receives **`&i32`**.

That is why `|&&x|` works for `find()` but is a type error for `all()`.

```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5];

    // find(): the predicate takes &Self::Item  -> &&i32 with iter()
    let found1 = numbers.iter().find(|&&x| x > 3);   // x: i32   (unpack both references)
    let found2 = numbers.iter().find(|&x| *x > 3);   // x: &i32  (unpack one reference)
    let found3 = numbers.iter().find(|x| **x > 3);   // x: &&i32 (no unpacking)
    println!("find: {:?} {:?} {:?}", found1, found2, found3);

    // all(): the predicate takes Self::Item by value -> &i32 with iter()
    let all1 = numbers.iter().all(|&x| x > 0);       // x: i32   (unpack the single reference)
    let all2 = numbers.iter().all(|x| *x > 0);       // x: &i32  (no unpacking)
    println!("all: {} {}", all1, all2);

    // |&&x| does NOT compile with all(): the item is only one reference deep.
    // let bad = numbers.iter().all(|&&x| x > 0);    // error[E0308]: mismatched types

    // Return types differ too:
    match numbers.iter().find(|&&x| x > 3) {         // Option<&i32>
        Some(&value) => println!("Found value: {}", value),
        None => println!("Not found"),
    }
    let all_positive: bool = numbers.iter().all(|&x| x > 0);
    println!("All positive: {}", all_positive);

    // Step by step for find():
    let result = numbers.iter().find(|item: &&i32| {
        let &&x = item;   // x: i32
        x > 1
    });
    println!("{:?}", result);
}
```

**TL;DR:**

- `find(|&&x| ...)` → unpacks both `&` layers, `x` is `i32`; `find(|&x| ...)` → `x` is `&i32`; `find(|x| ...)` → `x` is `&&i32`.
- `all(|&x| ...)` → unpacks the single `&` layer, `x` is `i32`; `all(|x| ...)` → `x` is `&i32`. `all(|&&x| ...)` does not compile.
- The patterns are only different ways to destructure the input *that each method actually provides*; which patterns are legal depends on the method's signature.
- Return types also differ: `find()` → `Option<&T>` (the found item), `all()` → `bool`.
