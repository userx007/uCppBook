# std::atomic

**Methods**

# `std::atomic<T>` — Complete Method Reference

---

## Construction & Assignment

### `atomic(T desired)` / `atomic()`
Constructs the atomic object. Default constructor leaves value uninitialized (for non-class types).
```cpp
std::atomic<int> a(42);
std::atomic<int> b{10};
```

### `operator=(T desired)`
Atomically assigns a value. Returns the assigned value.
```cpp
std::atomic<int> a;
a = 5; // equivalent to a.store(5)
```

---

## Core Load / Store

### `store(T desired, memory_order order = seq_cst)`
Atomically replaces the value.
```cpp
std::atomic<int> a(0);
a.store(99);
a.store(99, std::memory_order_relaxed);
```

### `load(memory_order order = seq_cst) -> T`
Atomically reads the value.
```cpp
std::atomic<int> a(42);
int v = a.load();                          // 42
int v2 = a.load(std::memory_order_acquire);
```

### `operator T()`
Implicit conversion — shorthand for `load()`.
```cpp
std::atomic<int> a(7);
int v = a; // calls a.load()
```

---

## Exchange & Compare-Exchange

### `exchange(T desired, memory_order order = seq_cst) -> T`
Atomically replaces value and **returns the old value**.
```cpp
std::atomic<int> a(1);
int old = a.exchange(2); // old == 1, a == 2
```

### `compare_exchange_weak(T& expected, T desired, ...) -> bool`
CAS (Compare-And-Swap). 
- If `a == expected`, sets `a = desired` and returns `true`. Otherwise, sets `expected = a` and returns `false`. May **spuriously fail** (use in a loop).
```cpp
std::atomic<int> a(1);
int expected = 1;
while (!a.compare_exchange_weak(expected, 2))
    ; // retry until success
```

### `compare_exchange_strong(T& expected, T desired, ...) -> bool`
Same as weak, but **never spuriously fails**. Prefer when a loop isn't needed.
```cpp
std::atomic<int> a(1);
int expected = 1;
bool ok = a.compare_exchange_strong(expected, 2); // ok == true
```

> Both CAS variants have overloads taking two `memory_order` arguments: one for success, one for failure.
> ```cpp
> a.compare_exchange_strong(exp, val,
>     std::memory_order_acq_rel,  // success
>     std::memory_order_acquire); // failure
> ```

---

## Arithmetic Operations *(integral & pointer types only)*

### `fetch_add(T arg, memory_order = seq_cst) -> T`
Atomically adds `arg`, returns the **old** value.
```cpp
std::atomic<int> a(10);
int old = a.fetch_add(5); // old == 10, a == 15
```

### `fetch_sub(T arg, memory_order = seq_cst) -> T`
Atomically subtracts, returns old value.
```cpp
std::atomic<int> a(10);
int old = a.fetch_sub(3); // old == 10, a == 7
```

### `operator+=` / `operator-=`
Shorthand for `fetch_add` / `fetch_sub`. Returns the **new** value.
```cpp
std::atomic<int> a(10);
a += 5; // a == 15
a -= 3; // a == 12
```

### `operator++` / `operator--` (pre & post)
Atomic increment/decrement.
```cpp
std::atomic<int> a(0);
a++;   // a == 1
++a;   // a == 2
a--;   // a == 1
--a;   // a == 0
```

---

## Bitwise Operations *(integral types only)*

### `fetch_and(T arg, memory_order = seq_cst) -> T`
Atomic bitwise AND, returns old value.
```cpp
std::atomic<int> a(0b1100);
int old = a.fetch_and(0b1010); // old == 0b1100, a == 0b1000
```

### `fetch_or(T arg, memory_order = seq_cst) -> T`
Atomic bitwise OR, returns old value.
```cpp
std::atomic<int> a(0b1100);
int old = a.fetch_or(0b0011); // old == 0b1100, a == 0b1111
```

### `fetch_xor(T arg, memory_order = seq_cst) -> T`
Atomic bitwise XOR, returns old value.
```cpp
std::atomic<int> a(0b1111);
int old = a.fetch_xor(0b0101); // old == 0b1111, a == 0b1010
```

### `operator&=` / `operator|=` / `operator^=`
Shorthand for fetch_and/or/xor. Returns the **new** value.
```cpp
std::atomic<int> a(0xFF);
a &= 0x0F; // a == 0x0F
a |= 0x30; // a == 0x3F
a ^= 0x11; // a == 0x2E
```

---

## Wait / Notify *(C++20)*

### `wait(T old, memory_order = seq_cst)`
Blocks until the value **changes** from `old`. Avoids busy-waiting.
```cpp
std::atomic<int> a(0);
// thread 1:
a.wait(0); // blocks while a == 0
// thread 2:
a.store(1);
a.notify_one();
```

### `notify_one()`
Unblocks **one** thread waiting on this atomic.
```cpp
a.store(1);
a.notify_one();
```

### `notify_all()`
Unblocks **all** threads waiting on this atomic.
```cpp
a.store(1);
a.notify_all();
```

---

## Utility

### `is_lock_free() -> bool`
Returns `true` if the atomic operations are truly lock-free (no internal mutex).
```cpp
std::atomic<int> a;
bool lf = a.is_lock_free(); // likely true for int
```

### `static constexpr bool is_always_lock_free` *(C++17)*
Compile-time check.
```cpp
static_assert(std::atomic<int>::is_always_lock_free);
```

---

## Memory Order Quick Reference

| Order | Meaning |
|---|---|
| `relaxed` | No sync, only atomicity |
| `consume` | Data-dependency ordering (rarely used directly) |
| `acquire` | No reads/writes can move **before** this load |
| `release` | No reads/writes can move **after** this store |
| `acq_rel` | Both acquire + release (for RMW ops) |
| `seq_cst` | Full sequential consistency (default, strongest) |

---

> **Key rule of thumb:** `fetch_*` and `exchange` return the **old** value. Compound operators (`+=`, `|=`, `++`) return the **new** value.

- `store`(val)
- val = `load`()

```cpp
#include <atomic>
#include <iostream>
#include <thread>

// Declare atomic variables
std::atomic<int> counter(0);           // Initialize to 0
std::atomic<bool> flag{false};         // Brace initialization
std::atomic<double> value;             // Default initialized

int main() {
    // Atomic operations are thread-safe
    counter.store(10);                 // Atomic write
    int current = counter.load();      // Atomic read
    
    std::cout << "Counter: " << current << std::endl;
    return 0;
}
```

```cpp
#include <atomic>
#include <syncstream>
#include <iostream>
#include <thread>

template<typename ... Args>
void sync_print(Args ... args)
{
    std::osyncstream out(std::cout);
    (out << ... << args) << "\n";
}

std::atomic<int> shared_counter{0};

void increment(int iterations)
{
    for (int i = 0; i < iterations; ++i) {
        shared_counter.fetch_add(1);
    }
}

int main(int argc, char const *argv[])
{

    std::thread t1(increment, 10);
    std::thread t2(increment, 20);
    std::thread t3(increment, 30);

    t1.join();
    t2.join();
    t3.join();

    sync_print("Final value: ", shared_counter.load());

    return 0;
}
```

```cpp
#include <atomic>
#include <syncstream>
#include <iostream>
#include <vector>
#include <thread>
#include <ranges>

template<typename ... Args>
void print_sync(Args ... args)
{
    std::osyncstream out(std::cout);
    (out << ... << args) << "\n";
}

std::atomic<int> atomic_value{0};

void safe_exchange(int maxval)
{
    int crtval = atomic_value.load();

    while(crtval < maxval){
        if(atomic_value.compare_exchange_weak(crtval, crtval + 1)){
            break;
        }
    }
}

int main(int argc, char const *argv[])
{
    std::vector<std::thread> threads;
    auto gen = std::views::iota(0,200);

    for(auto i : gen){
        threads.emplace_back(safe_exchange, 100);
    }

    for(auto& t : threads){
        t.join();
    }

    print_sync("Test result: ", atomic_value.load());
    return 0;
}
```
