```cpp
// ─────────────────────────────────────────────────────────────────────────────
// sync_print()
//
// A variadic, thread-safe print helper. Accepts any number of arguments of
// any printable type and writes them atomically to std::cout — no interleaving
// with output from other threads is possible.
//
// Example calls:
//   sync_print("hello");
//   sync_print("Producer ", id, " pushed: ", value);
//   sync_print("x=", 1, " y=", 2.5f, " label=", "ok");
// ─────────────────────────────────────────────────────────────────────────────
template<typename... Args>   // Variadic template — Args is a "parameter pack",
                             // meaning zero or more types deduced at call site.
                             // Each call may deduce completely different types:
                             //   sync_print("x=", 42)  → Args = {const char*, int}
                             //   sync_print("done")    → Args = {const char*}

void sync_print(Args&&... args) {
    //          ^^^^^^^^^^^
    //          Universal references (forwarding references). The && here does NOT
    //          mean rvalue-only. Combined with a deduced template type it means:
    //            - lvalues  → deduced as T&   (bound by reference, not copied)
    //            - rvalues  → deduced as T&&  (moved or forwarded cheaply)
    //          This avoids unnecessary copies for every argument passed in.

    std::osyncstream out(std::cout);
    //  ^^^^^^^^^^^^
    //  Synchronised output stream (C++20, <syncstream>).
    //  Wraps std::cout with an internal buffer. All output written to `out`
    //  is accumulated locally in this object. When `out` goes out of scope
    //  (end of function), the destructor flushes the entire buffer to
    //  std::cout as ONE atomic operation — guaranteed by the standard to
    //  never interleave with output from other threads doing the same.
    //
    //  This replaces the old pattern of:
    //    std::lock_guard<std::mutex> lock(print_mtx);
    //    std::cout << ...;
    //  with zero manual locking.

    (out << ... << args) << '\n';
    //  ^^^^^^^^^^^^^^^^^^^
    //  C++17 binary left fold expression over the << operator.
    //
    //  For args = (a, b, c) this expands to:
    //    ((out << a) << b) << c) << '\n'
    //
    //  i.e. each argument is streamed left-to-right into `out`, exactly
    //  as if you had written them manually one by one. The compiler generates
    //  this chain at compile time — no runtime loop, no overhead.
    //
    //  The final << '\n' appends a newline after all arguments.
    //  '\n' is preferred over std::endl here because std::endl also flushes —
    //  but osyncstream's destructor already handles the flush atomically,
    //  so an extra flush would be redundant.

}   // ← out is destroyed here, triggering the atomic flush to std::cout
```

```cpp
template<typename... Args>
void sync_print(Args&&... args) {
    std::osyncstream out(std::cout);
    (out << ... << args) << '\n';   // C++17 fold expression
}

// Usage
sync_print("Producer ", id, " pushed:  ", value);
```
---

# Condition variable

```cpp
if (!cv_full.wait_for(lock, timeout, [this]() { return !is_full(); })) {
    return false;
}
```


## `wait_for` — The Three Arguments

```cpp
cv_full.wait_for(lock, timeout, predicate)
//               ───   ───────  ─────────
//                1       2         3
```

**1. `lock`** — the `unique_lock` currently held by this thread. `wait_for` will **atomically release it** while sleeping, so other threads (consumers) can acquire the mutex and make progress. When woken, it reacquires the lock before returning.

**2. `timeout`** — maximum time to wait. If nobody signals within this duration, the thread wakes up on its own.

**3. `[this]() { return !is_full(); }`** — the predicate (more below).

---

## The Predicate — Why It Exists

```cpp
[this]() { return !is_full(); }
//^^^
// captures `this` so the lambda can call is_full()
// which accesses  count and capacity — member variables
```

Without a predicate, `wait_for` would wake on **any** signal — including spurious wakeups (the OS can wake a thread for no reason at all). The predicate guards against that:

```
Thread wakes up
      ↓
Is predicate true?  (!is_full())
      ↓                   ↓
     YES                  NO
      ↓                   ↓
   proceed            go back to sleep   ← spurious wakeup avoided
```

`wait_for` re-checks the predicate **every time** the thread wakes, whether from a real signal or a spurious one.

---

## What `wait_for` Actually Returns

```cpp
// Internally, wait_for with a predicate behaves like:
//
//   while (!predicate()) {
//       if (timed_out) return false;   // timeout expired, predicate still false
//       sleep_and_wait_for_signal();
//   }
//   return true;   // predicate became true in time
```

So:

```
returns true  → predicate became true before timeout → space is available → safe to write
returns false → timeout expired, still full           → give up, tell caller
```

---

## The `!` Inversion and `return false`

```cpp
if (!cv_full.wait_for(...)) {   // if wait_for returned FALSE (timed out)
    return false;               // propagate failure to caller — push() failed
}
```

The `!` flips the logic: **if** `wait_for` returns `false` (timed out, still full), **then** we return `false` from `push()`. If it returns `true` (space is available), execution falls through and the write proceeds.

---

## Full Picture — Timeline

```
PRODUCER calls push(), buffer is FULL
         │
         ▼
  lock acquired
         │
         ▼
  cv_full.wait_for()
  ┌──────────────────────────────────────────────────────┐
  │  atomically releases lock                            │
  │  thread sleeps ──────────────────────────────────┐   │
  │                                                  │   │
  │  CONSUMER calls pop()                            │   │
  │    acquires lock (now available)                 │   │
  │    removes item, count--                         │   │
  │    calls cv_full.notify_one() ───────────────────┘   │
  │                                                      │
  │  producer wakes up                                   │
  │  reacquires lock                                     │
  │  checks predicate: !is_full() → true                 │
  └──────────────────────────────────────────────────────┘
         │
         ▼
  write proceeds safely
```