# Closure pattern matching

## EPLANATION OF `|&&x|` PATTERN MATCHING

***The closure parameter type depends on the specific method and how it's designed.***

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
    
    // RULE: Each iterator method decides what it passes to the closure
    // There's NO universal rule that closures must receive &T
    
    // ============================================================
    // METHODS THAT PASS REFERENCES (&T)
    // ============================================================
    
    // find() - passes &T to avoid taking ownership
    // Signature: fn find<P>(&mut self, predicate: P) -> Option<Self::Item>
    //            where P: FnMut(&Self::Item) -> bool
    let found = numbers.iter().find(|&&x| x > 3);
    //          iter() yields &i32
    //          find() passes &&i32 to closure
    println!("find: {:?}", found);
    
    // any() - passes &T (doesn't need to return the item)
    let has_even = numbers.iter().any(|&x| x % 2 == 0);
    //             Receives &&i32, but return value doesn't matter
    println!("any: {}", has_even);
    
    // all() - passes &T
    let all_positive = numbers.iter().all(|&x| x > 0);
    println!("all: {}", all_positive);
    
    // position() - passes &T
    let pos = numbers.iter().position(|&x| x == 3);
    println!("position: {:?}", pos);
    
    // ============================================================
    // METHODS THAT PASS VALUES DIRECTLY (Self::Item)
    // ============================================================
    
    // map() - passes the actual item (Self::Item), NOT &T
    // Signature: fn map<B, F>(self, f: F) -> Map<Self, F>
    //            where F: FnMut(Self::Item) -> B
    let doubled = numbers.iter().map(|&x| x * 2).collect::<Vec<_>>();
    //            iter() yields &i32
    //            map() passes &i32 to closure (NOT &&i32)
    println!("map: {:?}", doubled);
    
    // filter() - passes the actual item
    let evens = numbers.iter().filter(|&x| x % 2 == 0).collect::<Vec<_>>();
    //          Receives &i32 (NOT &&i32)
    println!("filter: {:?}", evens);
    
    // for_each() - passes the actual item
    numbers.iter().for_each(|&x| {
        //         Receives &i32 (NOT &&i32)
        println!("for_each: {}", x);
    });
    
    // fold() - passes the actual item
    let sum = numbers.iter().fold(0, |acc, &x| acc + x);
    //                                    ^^
    //                                    Receives &i32 (NOT &&i32)
    println!("fold: {}", sum);
    
    // ============================================================
    // WHY THE DIFFERENCE?
    // ============================================================
    
    // Methods that pass &T:
    // - Need to return or reference the original item (find, position)
    // - Only need to inspect, not transform (any, all)
    
    // Methods that pass Self::Item directly:
    // - Transform or consume items (map, filter, fold)
    // - More flexible - you can choose to move or clone
    
    // ============================================================
    // EXAMPLES WITH into_iter() - YIELDS OWNED VALUES
    // ============================================================
    
    let v1 = vec![1, 2, 3, 4, 5];
    
    // find() with into_iter() - receives &i32 (reference to owned value)
    let found = v1.clone().into_iter().find(|&x| x > 3);
    //          into_iter() yields i32 (owned)
    //          find() passes &i32 to closure (only one &)
    println!("into_iter find: {:?}", found);
    
    // map() with into_iter() - receives i32 (owned value directly)
    let doubled = v1.clone().into_iter().map(|x| x * 2).collect::<Vec<_>>();
    //            into_iter() yields i32
    //            map() passes i32 to closure (no &)
    println!("into_iter map: {:?}", doubled);
    
    // ============================================================
    // COMPARISON TABLE
    // ============================================================
    
    // Method    | What closure receives when using iter()
    // ----------|------------------------------------------
    // find()    | &&T (reference to iterator item)
    // any()     | &&T
    // all()     | &&T
    // position()| &&T
    // ----------|------------------------------------------
    // map()     | &T (iterator item directly)
    // filter()  | &T
    // for_each()| &T
    // fold()    | &T (for accumulator) + &T (for item)
    
    // ============================================================
    // WHY THIS DESIGN?
    // ============================================================
    
    // find() needs to pass &T because:
    // 1. It returns Option<Self::Item> - needs to return the actual item
    // 2. The closure shouldn't consume items while searching
    // 3. If closure took ownership, iterator couldn't return the found item
    
    // map() passes Self::Item directly because:
    // 1. It's transforming items - more flexible with ownership
    // 2. You can choose to clone, move, or just reference
    // 3. The transformation might need to consume the item
    
    // ============================================================
    // PRACTICAL RULE OF THUMB:
    // ============================================================
    
    // If you see &&x in the pattern:
    //   - You're using iter() AND
    //   - The method passes &T to closure (like find, any, all)
    
    // If you see &x in the pattern:
    //   - You're using iter() and method passes T directly (like map, filter)
    //   OR
    //   - You're using into_iter() and method passes &T (like find)
    
    // If you see x (no &):
    //   - You're using into_iter() and method passes T directly (like map)
}
```

**Key Takeaway:**

The closure parameter type is **determined by each method's signature**, not a universal rule:

- **`find()`, `any()`, `all()`, `position()`** → Pass **`&Self::Item`** (reference to iterator item)
- **`map()`, `filter()`, `for_each()`, `fold()`** → Pass **`Self::Item`** (iterator item directly)

When using **`.iter()`** (which yields `&T`):
- Methods passing `&Self::Item` → closure gets `&&T`
- Methods passing `Self::Item` → closure gets `&T`

When using **`.into_iter()`** (which yields `T`):
- Methods passing `&Self::Item` → closure gets `&T`
- Methods passing `Self::Item` → closure gets `T`


---

**Both `find()` and `all()` actually receive the same type `&&i32`**. 
The difference in the patterns `|&&x|` vs `|&x|` is just about **how much we choose to unpack** in the pattern, not what they receive.

```rust
fn main() {
    let numbers = vec![1, 2, 3, 4, 5];
    
    // BOTH find() AND all() RECEIVE THE SAME TYPE: &&i32
    // The difference is just in how we write the closure pattern
    
    // ============================================================
    // find() - ALL THESE ARE EQUIVALENT
    // ============================================================
    
    // Option 1: Unpack both references → get i32
    let found1 = numbers.iter().find(|&&x| x > 3);
    //                                ^^   ^
    //                                ||   x is i32
    //                                |+-- unpacks inner &i32
    //                                +--- unpacks outer &
    
    // Option 2: Unpack one reference → get &i32
    let found2 = numbers.iter().find(|&x| *x > 3);
    //                                ^   ^^
    //                                |   x is &i32, so dereference to compare
    //                                +-- unpacks outer &
    
    // Option 3: No unpacking → work with &&i32 directly
    let found3 = numbers.iter().find(|x| **x > 3);
    //                                ^  ^^
    //                                |  x is &&i32, so double dereference
    //                                +- no unpacking
    
    println!("find: {:?} {:?} {:?}", found1, found2, found3);
    // All three produce the same result!
    
    // ============================================================
    // all() - ALL THESE ARE ALSO EQUIVALENT
    // ============================================================
    
    // Option 1: Unpack both references → get i32
    let all1 = numbers.iter().all(|&&x| x > 0);
    //                             ^^   ^
    //                             ||   x is i32
    //                             |+-- unpacks inner &i32
    //                             +--- unpacks outer &
    
    // Option 2: Unpack one reference → get &i32
    let all2 = numbers.iter().all(|&x| *x > 0);
    //                             ^   ^^
    //                             |   x is &i32, so dereference to compare
    //                             +-- unpacks outer &
    
    // Option 3: No unpacking → work with &&i32 directly
    let all3 = numbers.iter().all(|x| **x > 0);
    //                             ^  ^^
    //                             |  x is &&i32, so double dereference
    //                             +- no unpacking
    
    println!("all: {} {} {}", all1, all2, all3);
    // All three produce the same result!
    
    // ============================================================
    // SO WHY DID I USE DIFFERENT PATTERNS IN MY EXAMPLE?
    // ============================================================
    
    // Answer: Just personal preference/convention! There's NO technical difference.
    // I could have written both as |&&x| or both as |&x|
    
    // Many Rust programmers prefer:
    // - |&&x| when you want the actual value (cleaner, x is just i32)
    // - |&x| when you need a reference to work with
    
    // ============================================================
    // WHICH PATTERN TO CHOOSE?
    // ============================================================
    
    // Most common/readable: |&&x| - fully unpack to get the value
    let found = numbers.iter().find(|&&x| x > 3);
    let all_pos = numbers.iter().all(|&&x| x > 0);
    
    // When you actually need a reference (less common):
    let found_ref = numbers.iter().find(|&x| x == &3);  // comparing references
    
    // ============================================================
    // THE REAL DIFFERENCE: find() vs all() RETURN TYPES
    // ============================================================
    
    // find() returns Option<&T> - gives you back the found item
    let found = numbers.iter().find(|&&x| x > 3);
    //  ^^^^^
    //  found has type Option<&&i32> = Option<&i32>
    match found {
        Some(&value) => println!("Found value: {}", value),
        //    ^
        //    Must pattern match to extract i32 from &i32
        None => println!("Not found"),
    }
    
    // all() returns bool - just true/false, doesn't return items
    let all_positive = numbers.iter().all(|&&x| x > 0);
    //  ^^^^^^^^^^^^
    //  all_positive has type bool
    println!("All positive: {}", all_positive);
    // No need to pattern match, just use the boolean directly
    
    // ============================================================
    // VISUALIZING WHAT HAPPENS
    // ============================================================
    
    println!("\n=== Step-by-step breakdown ===");
    
    let vec = vec![1, 2, 3];
    
    // Step 1: iter() produces Iterator<Item = &i32>
    let mut iter = vec.iter();  // yields &i32
    
    // Step 2: find() signature says closure receives &Self::Item
    // Since Self::Item = &i32, closure receives &(&i32) = &&i32
    
    // Step 3: Pattern matching options
    let result = vec.iter().find(|item: &&i32| {
        // item has type &&i32
        
        // Option A: Pattern match to unpack
        let &&x = item;  // x has type i32
        println!("  Option A: x = {}", x);
        x > 1
        
        // Option B: Manual dereference
        // let x = **item;  // x has type i32
        // x > 1
    });
    
    // ============================================================
    // SUMMARY
    // ============================================================
    
    // 1. Both find() and all() receive &&i32 when using iter()
    // 2. |&&x| vs |&x| is just about how much you unpack in the pattern
    // 3. Use |&&x| when you want the actual value (most common)
    // 4. The real difference between find() and all() is:
    //    - find() returns Option<&T> (the found item)
    //    - all() returns bool (no item returned)
}
```

**TL;DR:**

Both `find()` and `all()` **receive exactly the same type** (`&&i32` when using `.iter()`).

The patterns `|&&x|` vs `|&x|` are **just different ways to destructure the same input**:
- `|&&x|` → Unpack both `&` layers, `x` is `i32`
- `|&x|` → Unpack one `&` layer, `x` is `&i32`  
- `|x|` → No unpacking, `x` is `&&i32`

All three work identically! It's purely a style choice. Most Rust code uses `|&&x|` because it's cleaner to work with the actual value rather than references.

The **actual difference** between `find()` and `all()` is their **return type**, not what the closure receives:
- `find()` → `Option<&T>` (returns the found item)
- `all()` → `bool` (just true/false)