# Unlocking mutex from another thread

Rust handles this situation in a fundamentally different way that makes the problem **impossible at compile time**.

## Rust's Approach: Compile-Time Prevention

In Rust, you **cannot** unlock a mutex from the wrong thread because the mutex doesn't have an `unlock()` method at all!

```rust
use std::sync::Mutex;

let mtx = Mutex::new(0);

// This is how you use a Mutex in Rust:
{
    let mut guard = mtx.lock().unwrap(); // Acquire lock
    *guard += 1; // Modify protected data
    // Lock automatically released when `guard` goes out of scope
}

// There is NO mtx.unlock() method!
```

## How Rust Prevents Wrong-Thread Unlocking

**1. RAII (Scope-Based Locking)**
- `lock()` returns a `MutexGuard` that owns the lock
- The lock is automatically released when `MutexGuard` is dropped (goes out of scope)
- You cannot manually unlock or transfer the guard to another thread

```rust
let guard = mtx.lock().unwrap();
// guard owns the lock

// Trying to send guard to another thread:
std::thread::spawn(move || {
    drop(guard); // Try to unlock in different thread
}); // ❌ COMPILE ERROR: MutexGuard is not Send!
```

**Compile error:**
```
error[E0277]: `MutexGuard<'_, i32>` cannot be sent between threads safely
```

**2. Type System Enforcement**
- `MutexGuard` is marked as `!Send` (not sendable between threads)
- The compiler physically prevents you from moving it to another thread
- This is checked at **compile time**, not runtime

## Comparison: C++ vs Rust

### C++ (Runtime Problem)
```cpp
std::mutex mtx;

std::thread t1([&mtx]() {
    mtx.lock();
    // Oops, forgot to unlock or want to unlock elsewhere
});

std::thread t2([&mtx]() {
    mtx.unlock(); // Compiles fine, undefined behavior at runtime
});
```

### Rust (Compile-Time Prevention)
```rust
let mtx = Mutex::new(0);

std::thread::spawn(move || {
    let guard = mtx.lock().unwrap();
    // guard cannot be moved to another thread
    // guard automatically unlocks when it goes out of scope
});

// Cannot unlock from here - no unlock method exists!
// Cannot move guard to another thread - compiler prevents it!
```

## What If You Really Need Cross-Thread Unlocking?

Rust acknowledges that sometimes you need signaling patterns. For those cases, use the right tool:

### For signaling between threads, use Condvar or channels

```rust
use std::sync::{Arc, Condvar, Mutex};

// For signaling between threads, use Condvar or channels
let pair = Arc::new((Mutex::new(false), Condvar::new()));
let pair2 = Arc::clone(&pair);

// Thread 1: signals
std::thread::spawn(move || {
    let (lock, cvar) = &*pair2;
    let mut ready = lock.lock().unwrap();
    *ready = true;
    cvar.notify_one(); // Signal, not unlock
});

// Thread 2: waits for signal
let (lock, cvar) = &*pair;
let mut ready = lock.lock().unwrap();
while !*ready {
    ready = cvar.wait(ready).unwrap();
}
```

### Or use channels for producer-consumer patterns:

```rust
use std::sync::mpsc;

let (tx, rx) = mpsc::channel();

// Producer
std::thread::spawn(move || {
    tx.send(42).unwrap();
});

// Consumer
let value = rx.recv().unwrap();
```

## Summary Table

| Aspect | C++ `std::mutex` | Rust `Mutex` |
|--------|------------------|--------------|
| **Manual unlock** | `mtx.unlock()` exists | No `unlock()` method |
| **Wrong thread unlock** | Undefined behavior | **Compile error** |
| **When unlocked** | Manual call | Automatic (RAII/scope-based) |
| **Transferable to other thread** | N/A (no lock object) | ❌ `MutexGuard` is `!Send` |
| **Forget to unlock** | Deadlock possible | Impossible (automatic) |
| **Detection** | Runtime (maybe, UB) | **Compile time** |

**Key insight**: Rust's approach is "make incorrect code impossible to write" rather than "detect incorrect code at runtime." This is the core philosophy of Rust's memory and concurrency safety guarantees.