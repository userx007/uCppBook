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

