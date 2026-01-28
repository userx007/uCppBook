# Collection of hints/tricks to remember..

## Error Handling

```rust

// IGNORE ERRORS USING FILTER_MAP

// filter_map combines map + filter in one operation
// It's perfect for Option/Result types where you want to ignore failures
let parsed: Vec<i32> = strings
    .iter()                                  // Iterator over &str
    .filter_map(|s| s.parse::<i32>().ok())   // parse() returns Result<i32, ParseIntError>
                                             // .ok() converts Result to Option:
                                             //   Ok(value) -> Some(value)
                                             //   Err(_) -> None
                                             // filter_map automatically filters out None values
                                             // and unwraps Some values
    .collect();                              // Collect the successful parses into Vec<i32>

```

---

```rust

// STOPS AT THE FIRST ERROR ENCOUNTERED

let result: Result<Vec<i32>, _> = strings
    .iter()
    .map(|s| s.parse::<i32>())               // Returns Iterator<Item = Result<i32, ParseIntError>>
                                             // Each element is a Result, not unwrapped
    .collect();                              // collect() is smart! When collecting Result items,
                                             // it returns Result<Vec<T>, E>
                                             // If ANY element is Err, entire result is Err
                                             // This is called "transpose" or "short-circuiting"
```

---

```rust

// SEPARATE SUCCESSES AND FAILURES

// Sometimes you want BOTH the successes AND the errors
// This requires the itertools crate (add to Cargo.toml: itertools = "0.12")
use itertools::Itertools;  // External crate providing extra iterator methods

// partition_map splits an iterator into two collections based on Either type
let (successes, failures): (Vec<i32>, Vec<_>) = strings
    .iter()
    .map(|s| s.parse::<i32>())               // Returns Iterator<Item = Result<i32, ParseIntError>>
    .partition_map(|r| match r {             // partition_map processes ALL elements
        Ok(n) => itertools::Either::Left(n), // Successful parses go to the Left (first tuple element)
        Err(e) => itertools::Either::Right(e), // Errors go to the Right (second tuple element)
    });                                      // Returns tuple: (Vec<Left>, Vec<Right>)
```    
