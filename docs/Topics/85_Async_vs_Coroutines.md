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