# `std::shared_lock` in C++

`std::shared_lock` is a RAII wrapper (introduced in C++14, in `<shared_mutex>`) for **shared (read) ownership** of a `SharedMutex` — most commonly `std::shared_mutex` or `std::shared_timed_mutex`. It enables the **multiple-readers / single-writer** (readers-writer lock) pattern.

---

## The Two Lock Roles

| Lock type | Wrapper | Concurrency |
|---|---|---|
| **Exclusive** (write) | `std::unique_lock` / `std::lock_guard` | Only one thread at a time |
| **Shared** (read) | `std::shared_lock` | Many threads simultaneously |

The underlying mutex tracks both counts. A shared lock is only granted when **no exclusive lock is held**, and an exclusive lock is only granted when **no shared or exclusive lock is held**.

---

## Basic Usage

```cpp
#include <shared_mutex>
#include <thread>
#include <iostream>

std::shared_mutex rw_mutex;
int shared_data = 0;

void reader(int id) {
    std::shared_lock lock(rw_mutex);   // Many readers can hold this simultaneously
    std::cout << "Reader " << id << " sees: " << shared_data << "\n";
}   // Shared lock released here

void writer(int value) {
    std::unique_lock lock(rw_mutex);   // Exclusive: blocks all readers and writers
    shared_data = value;
}   // Exclusive lock released here
```

---

## Constructor Overloads & Locking Strategies

```cpp
// 1. Lock immediately on construction (default)
std::shared_lock<std::shared_mutex> lk(mtx);

// 2. Deferred — don't lock yet
std::shared_lock<std::shared_mutex> lk(mtx, std::defer_lock);
lk.lock();  // Lock later manually

// 3. Try to lock — non-blocking
std::shared_lock<std::shared_mutex> lk(mtx, std::try_to_lock);
if (lk.owns_lock()) { /* got it */ }

// 4. Adopt an already-locked mutex
mtx.lock_shared();
std::shared_lock<std::shared_mutex> lk(mtx, std::adopt_lock);

// 5. Timed (requires std::shared_timed_mutex)
std::shared_lock<std::shared_timed_mutex> lk(mtx, std::chrono::milliseconds(100));
if (lk.owns_lock()) { /* acquired within timeout */ }
```

---

## Key Member Functions

```cpp
std::shared_lock<std::shared_mutex> lk(mtx, std::defer_lock);

lk.lock();           // Acquire shared lock (blocks)
lk.try_lock();       // Non-blocking attempt → bool
lk.unlock();         // Release without destroying

lk.owns_lock();      // true if currently holding the lock
bool b = (bool)lk;   // Same as owns_lock()

lk.release();        // Detach from mutex WITHOUT unlocking — returns raw pointer
lk.mutex();          // Returns pointer to the managed mutex
```

---

## Movable, Not Copyable

`std::shared_lock` is **move-only**, just like `std::unique_lock`:

```cpp
std::shared_lock<std::shared_mutex> acquire_read(std::shared_mutex& m) {
    return std::shared_lock<std::shared_mutex>(m);  // Move out of function — valid
}
```

---

## Real-World Pattern: Thread-Safe Cache

```cpp
#include <shared_mutex>
#include <unordered_map>
#include <optional>
#include <string>

class Cache {
    mutable std::shared_mutex mtx_;
    std::unordered_map<std::string, std::string> store_;

public:
    // Multiple threads can read concurrently
    std::optional<std::string> get(const std::string& key) const {
        std::shared_lock lock(mtx_);
        auto it = store_.find(key);
        if (it == store_.end()) return std::nullopt;
        return it->second;
    }

    // Only one thread can write at a time; blocks all readers
    void set(const std::string& key, const std::string& value) {
        std::unique_lock lock(mtx_);
        store_[key] = value;
    }
};
```

---

## Upgrading a Lock (Manual Pattern)

C++ has no built-in upgrade mechanism, so you do it manually:

```cpp
// Start with a shared (read) lock
std::shared_lock s_lock(mtx);

if (needs_write) {
    s_lock.unlock();                   // Release shared
    std::unique_lock x_lock(mtx);      // Acquire exclusive
    // Re-validate state — another writer may have acted in the gap!
    modify_data();
}
```

> ⚠️ There is an **unavoidable gap** between releasing the shared lock and acquiring the exclusive lock. Always re-check your invariants after the upgrade.

---

## `std::shared_mutex` vs `std::shared_timed_mutex`

| Feature | `shared_mutex` | `shared_timed_mutex` |
|---|---|---|
| `lock_shared()` | ✅ | ✅ |
| `try_lock_shared()` | ✅ | ✅ |
| `try_lock_shared_for()` | ❌ | ✅ |
| `try_lock_shared_until()` | ❌ | ✅ |
| Overhead | Lower | Slightly higher |

---

## When to Use It

- ✅ **Read-heavy** workloads where writes are rare (caches, config stores, registries)
- ✅ Reads are **long enough** that the contention saving matters
- ❌ Don't bother if writes are as frequent as reads — the overhead of the RW mechanism cancels the benefit
- ❌ Don't use with `std::mutex` — it only works with types satisfying the `SharedMutex` named requirement

Great question — these two interfaces are niche but have legitimate uses. They're easy to confuse, so here's a clear breakdown:


## Usage of the `mutex()` and `release()` interfaces

```cpp
lk.mutex();     // Returns raw pointer — lock STILL owns the mutex
lk.release();   // Detaches and returns raw pointer — lock NO LONGER owns it
                // YOU are now responsible for calling unlock_shared()
```

---

## `mutex()` — Inspect Without Giving Up Ownership

The lock remains fully active. You're just peeking at the underlying mutex.

### Example 1: Verifying two locks share the same mutex

```cpp
void assert_same_mutex(const std::shared_lock<std::shared_mutex>& a,
                        const std::shared_lock<std::shared_mutex>& b)
{
    // Useful in debug/assert contexts to verify locking discipline
    assert(a.mutex() == b.mutex() && "Locks must protect the same resource!");
}
```

### Example 2: Passing the mutex to a deeper layer

```cpp
void log_with_lock(std::shared_lock<std::shared_mutex>& lk, const std::string& msg) {
    // We want to temporarily upgrade to exclusive for the log write,
    // but we need the mutex handle to do so.
    std::shared_mutex* mtx = lk.mutex();

    lk.unlock();                     // Release shared lock
    {
        std::unique_lock ex(*mtx);   // Grab exclusive — mtx() gave us the handle
        write_log(msg);
    }                                // Exclusive released
    lk.lock();                       // Re-acquire shared
}
```

> `mutex()` returns `nullptr` if the `shared_lock` was default-constructed or has been `release()`'d.

---

## `release()` — Transfer Responsibility to Caller

After calling `release()`, the lock object is disarmed — it will **not** unlock on destruction. You own the mutex now and must unlock it manually.

### Example 1: Passing lock ownership to a C-style API

```cpp
// Imagine a legacy C API that manages its own lock lifetime
void register_resource(std::shared_mutex* mtx);   // Takes ownership, unlocks later
void unregister_resource(std::shared_mutex* mtx); // Must be called to release

void handoff_to_legacy(std::shared_mutex& mtx) {
    std::shared_lock lock(mtx);       // Acquire shared lock via RAII

    std::shared_mutex* raw = lock.release(); // Disarm RAII — we hand off ownership
    register_resource(raw);                  // C API now owns the lock

    // lock destructor runs here — but does nothing, since release() was called
}

void cleanup(std::shared_mutex& mtx) {
    unregister_resource(&mtx);  // C API calls mtx.unlock_shared() internally
}
```

### Example 2: Conditional ownership transfer

```cpp
// Returns the raw locked mutex only if a condition is met,
// otherwise RAII cleans up normally.
std::shared_mutex* maybe_transfer(std::shared_mutex& mtx, bool condition) {
    std::shared_lock lock(mtx);

    if (!condition) {
        return nullptr;  // lock destructor fires → mutex cleanly unlocked
    }

    return lock.release();  // Caller is now responsible for unlock_shared()
}

// Caller:
std::shared_mutex* raw = maybe_transfer(mtx, true);
if (raw) {
    do_something_under_lock();
    raw->unlock_shared();   // Must not forget this!
}
```

### Example 3: Reconstructing a lock with `adopt_lock`

A pattern where you `release()` into one type and re-wrap it in another:

```cpp
std::shared_mutex mtx;
std::shared_lock s_lock(mtx);           // Acquire shared

// Decide we want unique_lock to manage the same already-locked mutex.
// (After manually upgrading to exclusive, for example.)
s_lock.unlock();
mtx.lock();                             // Now exclusive

std::shared_mutex* raw = s_lock.release();           // Disarm shared_lock
std::unique_lock u_lock(*raw, std::adopt_lock);      // Adopt into unique_lock
// unique_lock now manages the exclusive lock via RAII
```

---

## Are They Actually Useful?

| Interface | Verdict |
|---|---|
| `mutex()` | **Moderately useful** — clean way to inspect or pass the mutex handle to a helper without giving up RAII safety |
| `release()` | **Rarely needed** — mainly for C API interop, custom lock managers, or advanced ownership transfers. High risk of leaking a locked mutex if misused |

The golden rule with `release()`: if you call it, you **must** manually call `mtx->unlock_shared()` at some point, or you'll permanently block any future exclusive lock attempts on that mutex — a subtle and nasty deadlock.

---

Good question. `std::shared_lock` has a **default constructor** that creates a lock object not associated with any mutex at all:

```cpp
std::shared_lock<std::shared_mutex> lk;  // Default constructed — no mutex, not locked
lk.mutex();    // Returns nullptr
lk.owns_lock() // Returns false
```

This is in contrast to the normal construction where you pass a mutex:

```cpp
std::shared_mutex mtx;
std::shared_lock lk(mtx);  // Associated with mtx, and locked
lk.mutex();     // Returns &mtx
lk.owns_lock()  // Returns true
```

---

## Why Does Default Construction Exist?

It's useful in a few scenarios:

### 1. Deferred initialization — decide the mutex later

```cpp
std::shared_lock<std::shared_mutex> lk;  // Empty for now

if (use_mutex_a) {
    lk = std::shared_lock<std::shared_mutex>(mtx_a);
} else {
    lk = std::shared_lock<std::shared_mutex>(mtx_b);
}
// lk is now properly associated and locked
```

### 2. Optional locking — lock only under a condition

```cpp
std::shared_lock<std::shared_mutex> lk;  // Start empty

if (needs_thread_safety) {
    lk = std::shared_lock<std::shared_mutex>(mtx);  // Move-assign a real lock
}

read_data();  // Runs with or without the lock depending on context
```

### 3. Return value placeholder before assignment

```cpp
std::shared_lock<std::shared_mutex> get_lock(std::shared_mutex& mtx, bool readonly) {
    if (readonly) {
        return std::shared_lock<std::shared_mutex>(mtx);  // Real lock
    }
    return std::shared_lock<std::shared_mutex>{};  // Empty sentinel — caller won't use it
}
```

---

## The Full State Summary

| Construction | `mutex()` | `owns_lock()` |
|---|---|---|
| `shared_lock lk` | `nullptr` | `false` |
| `shared_lock lk(mtx, defer_lock)` | `&mtx` | `false` |
| `shared_lock lk(mtx)` | `&mtx` | `true` |
| After `lk.release()` | `nullptr` | `false` |
| After `lk.unlock()` | `&mtx` | `false` |

Note the important distinction between the last two rows — after `unlock()` the mutex association is preserved and you can call `lk.lock()` again, whereas after `release()` the association is gone entirely and the lock object is essentially reset to a default-constructed state.