# Sharing data between threads

## 1. **Shared Mutable Data: `Arc<Mutex<T>>`**

```rust
let counter = Arc::new(Mutex::new(0));  // Read-write =>  Mutex needed

for _ in 0..5 {
    let counter_clone = Arc::clone(&counter);
    thread::spawn(move || {
        for _ in 0..1000 {
            let mut num = counter_clone.lock().unwrap();
            *num += 1;  
        }
    }); 
}
```

**When to use:**
- Multiple threads need **read AND write** access
- Data changes during execution

---

## 2. **Shared Immutable Data: Just `Arc<T>`**

```rust
let config = Arc::new(Config { max_retries: 3, timeout: 30 });  // Read-only => no Mutex

for _ in 0..5 {
    let config_clone = Arc::clone(&config);
    thread::spawn(move || {
        println!("Max retries: {}", config_clone.max_retries);  // Only reading
        // No Mutex needed - data never changes!
    });
}
```

**When to use:**
- Multiple threads only **read** the data
- Data is immutable (doesn't change)
- No Mutex overhead!

---

## 3. **Single Owner, Moved Data: Just `move`**

```rust
let data = vec![1, 2, 3, 4, 5];

thread::spawn(move || {
    // This thread OWNS the data now
    println!("Sum: {}", data.iter().sum::<i32>());
});

// data is no longer accessible here - it was moved
```

**When to use:**
- Only **one thread** needs the data
- Transfer ownership completely
- No Arc or Mutex needed!

---

## 4. **Read-Write Lock: `Arc<RwLock<T>>`**

```rust
use std::sync::{Arc, RwLock};

let data = Arc::new(RwLock::new(vec![1, 2, 3]));

// Multiple readers can access simultaneously
let data1 = Arc::clone(&data);
thread::spawn(move || {
    let r = data1.read().unwrap();  // Shared read access
    println!("{:?}", *r);
});

// Writers get exclusive access
let data2 = Arc::clone(&data);
thread::spawn(move || {
    let mut w = data2.write().unwrap();  // Exclusive write access
    w.push(4);
});
```

**When to use:**
- Many reads, few writes
- Better performance than Mutex for read-heavy workloads

---

## 5. **Atomic Types: No Mutex Needed!**

```rust
use std::sync::Arc;
use std::sync::atomic::{AtomicUsize, Ordering};

let counter = Arc::new(AtomicUsize::new(0));

for _ in 0..5 {
    let counter_clone = Arc::clone(&counter);
    thread::spawn(move || {
        for _ in 0..1000 {
            counter_clone.fetch_add(1, Ordering::SeqCst);  // Lock-free!
        }
    });
}
```

**When to use:**
- Simple types (integers, booleans)
- Lock-free atomic operations
- Better performance than Mutex for simple counters

---

## Decision Tree:

```
Do multiple threads need access?
│
├─ NO  → Just move the data (no Arc, no Mutex)
│
└─ YES → Do threads need to modify it?
    │
    ├─ NO (read-only) → Arc<T>
    │
    └─ YES (mutable) → What kind of data?
        │
        ├─ Simple type (int, bool) → Arc<AtomicT>
        │
        ├─ Many reads, few writes → Arc<RwLock<T>>
        │
        └─ General case → Arc<Mutex<T>>
```

## Quick Reference:

| Pattern | Use Case | Example |
|---------|----------|---------|
| `move` | Single owner | `thread::spawn(move \|\| { use data })` |
| `Arc<T>` | Shared, immutable | Config, lookup tables |
| `Arc<Mutex<T>>` | Shared, mutable | Counters, shared state |
| `Arc<RwLock<T>>` | Many reads, few writes | Caches, databases |
| `Arc<AtomicT>` | Simple atomic ops | Flags, simple counters |

