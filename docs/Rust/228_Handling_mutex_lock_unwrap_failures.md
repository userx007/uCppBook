# Approaches to handle the potential panic from mutex lock `unwrap()`

## 1. **Use `expect()` with a Descriptive Message** (Simple Improvement)

```rust
let mut num = counter_clone.lock().expect("Failed to acquire mutex lock");
*num += 1;
```

**Pros:** 
- Better error message when it fails
- Shows intent that this *should* succeed

**Cons:** 
- Still panics, but at least you know why

---

## 2. **Handle the Error Gracefully with `if let`**

```rust
for _ in 0..1000 {
    if let Ok(mut num) = counter_clone.lock() {
        *num += 1;
    } else {
        eprintln!("Warning: Failed to acquire lock, skipping increment");
        // Could also break, continue, or implement retry logic
    }
}
```

**Pros:** 
- No panic
- Thread continues running

**Cons:** 
- Silently skips increments (lost work)

---

## 3. **Use `match` for Explicit Error Handling**

```rust
for _ in 0..1000 {
    match counter_clone.lock() {
        Ok(mut num) => {
            *num += 1;
        }
        Err(poisoned) => {
            // Mutex was poisoned - a thread panicked while holding the lock
            eprintln!("Mutex poisoned! Recovering data anyway...");
            let mut num = poisoned.into_inner();  // Recover the data
            *num += 1;
        }
    }
}
```

**Pros:** 
- Handles poisoned mutex gracefully
- Can recover the data even after panic

**Cons:** 
- More verbose

---

## 4. **Panic the Thread with Context** (Recommended for Critical Operations)

```rust
use std::thread;

for i in 0..5 {
    let counter_clone = Arc::clone(&counter);
    thread::spawn(move || {
        for j in 0..1000 {
            let mut num = counter_clone.lock()
                .unwrap_or_else(|e| {
                    panic!("Thread {} iteration {}: Mutex lock failed: {}", i, j, e);
                });
            *num += 1;
        }
    });
}
```

**Pros:** 
- Clear context when things go wrong
- Thread panics with useful debug info

---

## 5. **Return Result from Thread** (Most Robust)

```rust
use std::thread;

let mut handles = vec![];

for _ in 0..5 {
    let counter_clone = Arc::clone(&counter);
    let handle = thread::spawn(move || -> Result<(), String> {
        for _ in 0..1000 {
            let mut num = counter_clone.lock()
                .map_err(|e| format!("Lock acquisition failed: {}", e))?;
            *num += 1;
        }
        Ok(())
    });
    handles.push(handle);
}

// Check for errors when joining
for (i, handle) in handles.into_iter().enumerate() {
    match handle.join() {
        Ok(Ok(())) => println!("Thread {} completed successfully", i),
        Ok(Err(e)) => eprintln!("Thread {} failed: {}", i, e),
        Err(_) => eprintln!("Thread {} panicked", i),
    }
}
```

**Pros:** 
- Proper error propagation
- Main thread knows if workers failed
- Can implement retry logic

**Cons:** 
- More complex

---

## 6. **Scoped Threads with Error Handling** (Modern Rust)

```rust
use std::thread;

thread::scope(|s| {
    for _ in 0..5 {
        let counter = &counter;  // Can borrow directly with scoped threads!
        s.spawn(move || {
            for _ in 0..1000 {
                match counter.lock() {
                    Ok(mut num) => *num += 1,
                    Err(e) => {
                        eprintln!("Lock failed: {}", e);
                        return;  // Exit this thread gracefully
                    }
                }
            }
        });
    }
});  // All threads guaranteed to finish here
```

**Pros:** 
- No Arc needed with scoped threads!
- Cleaner lifetime management
- All threads guaranteed to complete before scope ends

---

## 7. **Use a Poisoning-Aware Wrapper** (Production Pattern)

```rust
use std::sync::{Arc, Mutex, PoisonError};

fn safe_increment(counter: &Arc<Mutex<i32>>) -> Result<(), String> {
    let mut num = counter.lock()
        .or_else(|e: PoisonError<_>| {
            // Mutex was poisoned, but we can still use the data
            eprintln!("Warning: Mutex was poisoned, recovering...");
            Ok(e.into_inner())  // Extract the data anyway
        })
        .map_err(|_| "Unexpected lock error")?;
    
    *num += 1;
    Ok(())
}

// Usage:
for _ in 0..5 {
    let counter_clone = Arc::clone(&counter);
    thread::spawn(move || {
        for _ in 0..1000 {
            if let Err(e) = safe_increment(&counter_clone) {
                eprintln!("Increment failed: {}", e);
            }
        }
    });
}
```

---

## **Recommendation:**

For **simple cases** :
```rust
let mut num = counter_clone.lock().expect("Mutex poisoned");
```

For **production code**:
```rust
let mut num = match counter_clone.lock() {
    Ok(guard) => guard,
    Err(poisoned) => {
        eprintln!("Mutex poisoned, recovering data");
        poisoned.into_inner()  // Continue with recovered data
    }
};
*num += 1;
```

**Why?** 
- Mutex poisoning is **extremely rare** (only happens if a thread panics while holding the lock)
- In most cases, if it happens, it indicates a serious bug
- Recovering the data with `into_inner()` is usually safe for simple types like counters

The key insight: `unwrap()` is actually **not that bad** here because mutex poisoning is rare and usually indicates a critical bug. But `expect()` or proper error handling shows professionalism and makes debugging easier.