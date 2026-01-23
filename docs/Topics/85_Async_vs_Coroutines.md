# Relationship between async/futures/promises and Coroutines

## Relationship: Complementary, Not Competing

That they're complementary rather than directly related. Think of it this way:

- **async/futures/promises** are about *task-based concurrency* - launching work and getting results back
- **Coroutines** are about *writing asynchronous code* - a language feature for pausable functions

Coroutines are a lower-level primitive that can actually *implement* async operations more elegantly.

## What Problems Do Coroutines Solve That async Cannot?

### 1. **Callback Hell / Pyramid of Doom**

**With async (traditional approach):**
```cpp
auto future1 = std::async([]{ return fetch_user(123); });
auto user = future1.get(); // blocking!

auto future2 = std::async([user]{ return fetch_orders(user.id); });
auto orders = future2.get(); // blocking again!

auto future3 = std::async([orders]{ return process_orders(orders); });
return future3.get();
```

**With coroutines:**
```cpp
task<result> process_pipeline() {
    auto user = co_await fetch_user_async(123);
    auto orders = co_await fetch_orders_async(user.id);
    auto result = co_await process_orders_async(orders);
    co_return result;
}
```

The coroutine version looks synchronous but is fully asynchronous - no blocking threads!

### 2. **Efficient Suspension Without Blocking Threads**

`std::async` with `.get()` blocks a thread while waiting. Coroutines can suspend without consuming thread resources:

```cpp
task<data> fetch_with_timeout() {
    auto data = co_await async_read();  // suspends, thread does other work
    co_await sleep_for(100ms);          // suspends, doesn't block thread
    co_return data;
}
```

### 3. **Generators and Lazy Sequences**

Coroutines enable generators, which async cannot do:

```cpp
generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

// Usage
for (int value : fibonacci()) {
    if (value > 100) break;
    std::cout << value << '\n';
}
```

This produces values on-demand without blocking or creating futures.

### 4. **State Machine Management**

Coroutines automatically manage state across suspension points. With callbacks/futures, you'd need to manually track state:

```cpp
// Coroutine: state is automatically preserved
task<void> multi_step_process() {
    int state = 0;
    state += co_await step1();  // state preserved across suspension
    state += co_await step2();
    state += co_await step3();
    co_return;
}
```

### 5. **Composability**

Coroutines compose naturally:

```cpp
task<int> level1() { co_return co_await level2() + 1; }
task<int> level2() { co_return co_await level3() + 1; }
task<int> level3() { co_return 42; }
```

With futures, composition gets messy with `.then()` chains or nested `.get()` calls.

## Runtime Requirements - Your Question

You're **partially correct**, but it's nuanced:

### Coroutines:
- **Language feature**: The compiler transforms coroutine code into state machines (no special runtime needed for the transformation)
- **Library requirement**: You still need a coroutine library (like `task<T>`, `generator<T>`) that implements the promise types and awaitables
- **No thread pool required**: Unlike `std::async`, coroutines don't automatically spawn threads. They're just pausable functions

### std::async:
- **Runtime overhead**: Typically launches threads or uses a thread pool (implementation-defined)
- **Thread resources**: Each async task may consume thread resources even when waiting

### Key Difference:

```cpp
// std::async: likely spawns a thread or uses thread pool
auto future = std::async(std::launch::async, expensive_work);

// Coroutine: just a pausable function, execution context is explicit
task<void> my_coroutine() {
    co_await some_io_operation();  // decides where to resume
}
```

**With coroutines, YOU control the execution model**. You can:
- Run them on a single thread with an event loop
- Use a thread pool explicitly
- Run them inline
- Mix and match

## Summary

| Feature | std::async/futures | Coroutines |
|---------|-------------------|------------|
| Purpose | Launch tasks, get results | Write pausable functions |
| Threading | Often spawns threads | No implicit threading |
| Blocking | `.get()` blocks | `co_await` suspends without blocking |
| State management | Manual | Automatic |
| Composability | Limited | Excellent |
| Use case | Simple parallel tasks | Complex async workflows, generators |

Think of coroutines as a more flexible, lower-level primitive that can implement what `std::async` does, plus much more - but requires more setup (you need a coroutine library/framework).

---

# Questions and answers

1. Can `generator<T>` be used with any type?
2. Is it reusable for any scenarios?
3. Why isn't it in the standard library?


## 1. Can `generator<T>` be used with any type?

**Yes, mostly!** The template works with any type `T` that is:
- **Move-constructible** (can be moved)
- **Copy-constructible** (can be copied) - for the `operator*()` that returns `const T&`

```cpp
generator<std::string> string_gen() {
    co_yield "hello";
    co_yield "world";
}

generator<std::vector<int>> vector_gen() {
    co_yield std::vector{1, 2, 3};
    co_yield std::vector{4, 5, 6};
}

struct MyClass {
    int x;
    std::string name;
};

generator<MyClass> object_gen() {
    co_yield MyClass{42, "Alice"};
    co_yield MyClass{99, "Bob"};
}
```

## 2. Is it reusable for any scenarios?

**Mostly, but with limitations.** The implementation I provided is a **basic generator** suitable for:
- ✅ Lazy sequences (Fibonacci, ranges, filters)
- ✅ Data streaming
- ✅ Tree/graph traversal
- ✅ Parsers that yield tokens

**However**, it doesn't support:
- ❌ Async I/O operations (you'd need `async_generator`)
- ❌ Bidirectional communication (sending values back into the coroutine)
- ❌ Exception handling from outside
- ❌ Cancellation
- ❌ Multi-threading safety

## 3. Why isn't it in the standard library?

**This is the million-dollar question!** Here's the situation:

### The Problem:

C++20 added **coroutine language support** (the `co_yield`, `co_await`, `co_return` keywords and the transformation mechanism), but it deliberately did **NOT** include standard library types like `generator<T>`, `task<T>`, or `async_generator<T>`.

### Why?

1. **No consensus on the "right" design** - Different use cases need different features:
   - Should it be sync or async?
   - Should it support cancellation?
   - Should it be copyable or move-only?
   - How should it handle exceptions?

2. **The committee ran out of time** - Getting the language features into C++20 was already a huge effort

3. **They wanted to get experience first** - Let the ecosystem experiment with different designs before standardizing

### Current Status:

- **C++23**: Still no `std::generator` 😞
- **C++26**: Finally coming! 🎉

The C++26 standard is expected to include:
- `std::generator<T>` - synchronous generator (like our example)
- `std::async_generator<T>` - for async scenarios

### What to Use Now?

Until C++26, you have several options:

**Option 1: Use a library**
```cpp
// cppcoro library (popular but no longer maintained)
#include <cppcoro/generator.hpp>

cppcoro::generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        int next = a + b;
        a = b;
        b = next;
    }
}
```

**Option 2: Use compiler-specific implementations**
```cpp
// MSVC has experimental support
#include <experimental/generator>

std::experimental::generator<int> fibonacci() {
    // ...
}
```

**Option 3: Roll your own** (like we did)
- Good for learning
- Full control over features
- Can be tailored to your needs

### The Future (C++26 Preview):

The proposed `std::generator<T>` will likely be similar to what we implemented, but with:
- Better exception safety
- Optimized performance
- Range adaptors integration
- Standardized behavior across compilers

```cpp
// C++26 (upcoming)
#include <generator>

std::generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;
        int next = a + b;
        a = b;
        b = next;
    }
}
```

## Summary:

| Question | Answer |
|----------|--------|
| Works with any type? | Yes, if move/copy-constructible |
| Reusable for all scenarios? | Most common ones, but not async operations |
| Why not in std? | It IS coming in C++26! Just took time to get the design right |

The lesson here is that **C++ moves cautiously** - rather than rush a generator into the standard with the wrong design, the committee waited to see how the community used coroutines first. This is both frustrating (we have to wait) and wise (we get a better design).

For now, the implementation I provided is perfectly usable for synchronous generators in production code, or you can use established libraries like cppcoro or wait for your compiler to support the experimental versions.