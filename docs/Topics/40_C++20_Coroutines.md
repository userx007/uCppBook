# Guide to C++20 Coroutines

## Key Concepts Demonstrated:

1. **Basic Generator** - Shows the fundamental structure with `co_yield` to produce values lazily
2. **Infinite Sequences** - Fibonacci generator demonstrating unbounded lazy evaluation
3. **Async Operations** - Using `co_await` for asynchronous workflows
4. **Return Values** - Coroutines that both yield values and return a final result with `co_return`
5. **Practical Use** - A string tokenizer showing real-world application

## Core Components:

- **promise_type**: The control structure that defines how the coroutine behaves (lifecycle, suspension points, value handling)
- **Coroutine Handle**: The low-level interface to resume, check status, and destroy coroutines
- **Awaitables**: Objects that control suspension behavior with `co_await`
- **Three Keywords**: `co_yield` (produce values), `co_await` (wait for operations), `co_return` (final return)

## Advantages:

- Write asynchronous code that reads sequentially
- Lazy evaluation - values computed only when needed
- Infinite sequences with finite memory
- No callback pyramids or complex state machines
- Compiler generates efficient state machines automatically

The examples compile with C++20 and demonstrate progressively complex patterns from simple counters to practical utilities like tokenizers. Each example is fully functional and includes detailed comments explaining the mechanics.

```
C++20 COROUTINES - COMPREHENSIVE GUIDE
======================================

WHAT ARE COROUTINES?
-------------------
Coroutines are functions that can suspend execution and resume later,
maintaining their state between suspensions. They enable:
- Asynchronous operations without callback hell
- Generators and lazy evaluation
- Cooperative multitasking

A function is a coroutine if it uses: co_await, co_yield, or co_return
```

```cpp
#include <coroutine>
#include <iostream>
#include <exception>
#include <optional>

// ============================================================================
// EXAMPLE 1: BASIC GENERATOR - Simple Coroutine
// ============================================================================

// ============================================================================
// Generator<T> - A coroutine-based lazy sequence generator
// ============================================================================
// This template implements a generator pattern using C++20 coroutines.
// Generators produce values on-demand (lazily) rather than computing all
// values upfront, making them memory-efficient for large or infinite sequences.

template<typename T>
struct Generator {
    // ========================================================================
    // promise_type - Controls coroutine lifecycle and suspension behavior
    // ========================================================================
    // The promise_type is a required nested type that defines how the coroutine
    // behaves at key points: creation, suspension, yielding values, and cleanup.
    // The compiler uses this to manage the coroutine's state machine.
    struct promise_type {
        // Storage for the most recently yielded value
        T current_value;
        
        // Captures any exception thrown inside the coroutine
        std::exception_ptr exception;
        
        // Called by the compiler to create the Generator object when the
        // coroutine is first invoked (before any coroutine code runs)
        Generator get_return_object() {
            // Create a coroutine handle from this promise and wrap it in Generator
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        // Called immediately after get_return_object()
        // Returns suspend_always to pause execution before the first line of
        // coroutine code, giving the caller control over when to start iteration
        std::suspend_always initial_suspend() { return {}; }
        
        // Called when coroutine execution completes (reaches end or co_return)
        // Returns suspend_always to keep the coroutine suspended so we can
        // detect completion via done() without destroying state prematurely
        // Must be noexcept per C++20 standard
        std::suspend_always final_suspend() noexcept { return {}; }
        
        // Called when the coroutine executes co_yield with a value
        // Stores the value and suspends execution, returning control to caller
        std::suspend_always yield_value(T value) {
            current_value = value;  // Store value for caller to retrieve
            return {};              // Suspend here (return to caller)
        }
        
        // Called if coroutine reaches end without co_return or has co_return
        // with no value. For generators, we typically don't return a final value.
        void return_void() {}
        
        // Called if an unhandled exception occurs in the coroutine body
        // Captures the exception so it can be rethrown when caller resumes
        void unhandled_exception() {
            exception = std::current_exception();
        }
    };
    
    // Handle to the coroutine's state machine and promise object
    std::coroutine_handle<promise_type> handle;
    
    // Constructor: takes ownership of the coroutine handle
    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    
    // Destructor: cleanup coroutine resources when Generator is destroyed
    // The coroutine handle must be explicitly destroyed to free its state
    ~Generator() { 
        if (handle) handle.destroy(); 
    }
    
    // ========================================================================
    // Move-only semantics (prevents double-destruction of coroutine handle)
    // ========================================================================
    
    // Delete copy constructor - generators can't be copied because we can't
    // duplicate coroutine state. Each handle must have unique ownership.
    Generator(const Generator&) = delete;
    
    // Move constructor - transfers ownership of the coroutine handle
    // Nullifies the source handle to prevent double-destruction
    Generator(Generator&& other) : handle(other.handle) {
        other.handle = nullptr;
    }
    
    // ========================================================================
    // Public API for consuming generated values
    // ========================================================================
    
    // Retrieves the most recently yielded value from the coroutine
    // Should only be called after a successful next() call
    T value() { 
        return handle.promise().current_value; 
    }
    
    // Advances the coroutine to the next yield point or completion
    // Returns true if a new value was yielded, false if coroutine finished
    bool next() {
        handle.resume();        // Resume coroutine execution
        return !handle.done();  // Check if coroutine has more values
    }
};

// ============================================================================
// Example: counter - A simple integer sequence generator
// ============================================================================
// Demonstrates basic coroutine usage with co_yield to generate values lazily.
// This function becomes a coroutine due to the presence of co_yield.
// When called, it returns a Generator<int> immediately without executing
// the loop - execution only proceeds when next() is called.

Generator<int> counter(int n) {
    // Loop executes incrementally, pausing at each co_yield
    for (int i = 0; i <= n; ++i) {
        co_yield i;  // Suspend execution and yield current value to caller
                     // Execution resumes here when next() is called again
    }
    // When loop completes, coroutine reaches final_suspend()
}

// ============================================================================
// Usage Example
// ============================================================================

#include <iostream>

int main() {
    // Create a generator for numbers 0 to 5
    // Note: No values are generated yet (lazy evaluation)
    auto gen = counter(5);
    
    // Iterate through generated values
    while (gen.next()) {  // Resume coroutine, get next value
        std::cout << gen.value() << " ";  // Print: 0 1 2 3 4 5
    }
    std::cout << std::endl;
    
    // Generator is automatically cleaned up when gen goes out of scope
    
    return 0;
}

/*
// Alternative: Manual iteration
auto gen2 = counter(3);
if (gen2.next()) {
    std::cout << gen2.value();  // 0
}
if (gen2.next()) {
    std::cout << gen2.value();  // 1
}
// ... and so on

// Memory efficiency example:
// auto huge = counter(1000000);  // No memory allocated for all values
// Generates each value only when requested via next()
*/
```
The comments explain:

1. **Overall purpose** - What generators are and why they're useful (lazy evaluation)
2. **promise_type mechanics** - Each method's role in the coroutine lifecycle
3. **Suspension points** - When and why the coroutine pauses execution
4. **Ownership model** - Why it's move-only and how resource cleanup works
5. **API usage** - How to consume values with `next()` and `value()`
6. **Practical examples** - Different ways to use the generator in real code

[Mode details here...](84_Coroutine_generator_explained.md)<br>


```cpp
// ============================================================================
// EXAMPLE 2: FIBONACCI GENERATOR - Infinite Sequence
// ============================================================================

Generator<unsigned long long> fibonacci() {
    unsigned long long a = 0, b = 1;
    while (true) {
        co_yield a;
        auto next = a + b;
        a = b;
        b = next;
    }
}

// ============================================================================
// EXAMPLE 3: TASK WITH CO_AWAIT - Async Operation
// ============================================================================

struct Task {
    struct promise_type {
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_never initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        void return_void() {}
        void unhandled_exception() {}
    };

    std::coroutine_handle<promise_type> handle;
    
    Task(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Task() { if (handle) handle.destroy(); }
    
    Task(const Task&) = delete;
    Task(Task&& other) : handle(other.handle) {
        other.handle = nullptr;
    }
};

// Awaitable type for demonstration
struct Awaiter {
    bool await_ready() { return false; }  // Should we suspend?
    void await_suspend(std::coroutine_handle<>) {
        std::cout << "  [Suspended - simulating async work]\n";
    }
    void await_resume() {
        std::cout << "  [Resumed]\n";
    }
};

Task async_example() {
    std::cout << "Starting async operation\n";
    co_await Awaiter{};  // Suspend here
    std::cout << "Async operation completed\n";
}

// ============================================================================
// EXAMPLE 4: GENERATOR WITH RETURN VALUE
// ============================================================================

template<typename T>
struct GeneratorWithReturn {
    struct promise_type {
        T current_value;
        std::optional<T> return_value;
        
        GeneratorWithReturn get_return_object() {
            return GeneratorWithReturn{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_value(T value) {
            return_value = value;
        }
        
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle;
    
    GeneratorWithReturn(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~GeneratorWithReturn() { if (handle) handle.destroy(); }
    
    GeneratorWithReturn(const GeneratorWithReturn&) = delete;
    GeneratorWithReturn(GeneratorWithReturn&& other) : handle(other.handle) {
        other.handle = nullptr;
    }
    
    T value() { return handle.promise().current_value; }
    bool next() {
        handle.resume();
        return !handle.done();
    }
    
    std::optional<T> get_return() {
        return handle.promise().return_value;
    }
};

// Generate squares and return the sum
GeneratorWithReturn<int> squares_with_sum(int n) {
    int sum = 0;
    for (int i = 1; i <= n; ++i) {
        int square = i * i;
        sum += square;
        co_yield square;
    }
    co_return sum;  // Final return value
}

// ============================================================================
// EXAMPLE 5: PRACTICAL USE CASE - String Tokenizer
// ============================================================================

Generator<std::string> tokenize(const std::string& str, char delimiter) {
    std::string token;
    for (char c : str) {
        if (c == delimiter) {
            if (!token.empty()) {
                co_yield token;
                token.clear();
            }
        } else {
            token += c;
        }
    }
    if (!token.empty()) {
        co_yield token;
    }
}

// ============================================================================
// MAIN - DEMONSTRATING ALL EXAMPLES
// ============================================================================

int main() {
    std::cout << "=== C++20 COROUTINES EXAMPLES ===\n\n";
    
    // Example 1: Basic counter
    std::cout << "1. BASIC COUNTER (0 to 5):\n   ";
    auto gen = counter(5);
    while (gen.next()) {
        std::cout << gen.value() << " ";
    }
    std::cout << "\n\n";
    
    // Example 2: Fibonacci sequence
    std::cout << "2. FIBONACCI SEQUENCE (first 10):\n   ";
    auto fib = fibonacci();
    for (int i = 0; i < 10 && fib.next(); ++i) {
        std::cout << fib.value() << " ";
    }
    std::cout << "\n\n";
    
    // Example 3: Async operation
    std::cout << "3. ASYNC OPERATION:\n";
    auto task = async_example();
    std::cout << "\n";
    
    // Example 4: Generator with return value
    std::cout << "4. SQUARES WITH SUM (1-5):\n   Squares: ";
    auto sq = squares_with_sum(5);
    while (sq.next()) {
        std::cout << sq.value() << " ";
    }
    std::cout << "\n   Sum: " << sq.get_return().value_or(0) << "\n\n";
    
    // Example 5: String tokenizer
    std::cout << "5. STRING TOKENIZER:\n   Input: \"Hello,World,C++20,Coroutines\"\n   Tokens: ";
    auto tokens = tokenize("Hello,World,C++20,Coroutines", ',');
    while (tokens.next()) {
        std::cout << "\"" << tokens.value() << "\" ";
    }
    std::cout << "\n\n";
    
    std::cout << "=== KEY CONCEPTS ===\n";
    std::cout << "• co_yield: Suspend and produce a value\n";
    std::cout << "• co_await: Suspend and wait for an operation\n";
    std::cout << "• co_return: Return final value and complete\n";
    std::cout << "• promise_type: Defines coroutine behavior\n";
    std::cout << "• Coroutines are lazy - they don't execute until resumed\n";
    
    return 0;
}

/*
 * IMPORTANT COROUTINE CONCEPTS:
 * =============================
 * 
 * 1. PROMISE TYPE:
 *    - Controls coroutine lifecycle
 *    - Must implement: get_return_object(), initial_suspend(), 
 *      final_suspend(), unhandled_exception()
 *    - Can implement: yield_value(), return_value(), return_void()
 * 
 * 2. COROUTINE HANDLE:
 *    - std::coroutine_handle<Promise> - type-erased handle to coroutine
 *    - Methods: resume(), done(), destroy()
 *    - Provides access to promise via promise()
 * 
 * 3. AWAITABLE:
 *    - Type used with co_await
 *    - Must implement: await_ready(), await_suspend(), await_resume()
 *    - Controls suspension behavior
 * 
 * 4. SUSPEND POINTS:
 *    - initial_suspend: Before coroutine body
 *    - co_await/co_yield: Explicit suspension points
 *    - final_suspend: After coroutine completes
 * 
 * 5. BENEFITS:
 *    - Write async code that looks synchronous
 *    - Lazy evaluation and infinite sequences
 *    - Better memory efficiency than callbacks
 *    - State machine generated by compiler
 * 
 * 6. COMPILE WITH:
 *    g++ -std=c++20 -fcoroutines filename.cpp
 *    clang++ -std=c++20 -stdlib=libc++ filename.cpp
 *    MSVC: /std:c++20
 */
```

## ASCII diagram showing how C++ 20 coroutines work:

```
C++ 20 COROUTINES FLOW
======================

┌─────────────────────────────────────────────────────────────────┐
│                         CALLER FUNCTION                         │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ calls coroutine
                              ▼
┌─────────────────────────────────────────────────────────────────┐
│                    COROUTINE FUNCTION                           │
│  Task<int> myCoroutine() {                                      │
│      co_await someOperation();  ◄─── suspension point           │
│      co_return 42;                                              │
│  }                                                              │
└─────────────────────────────────────────────────────────────────┘
                              │
                              │ creates
                              ▼
        ┌─────────────────────────────────────────┐
        │       COROUTINE STATE (heap)            │
        │  ┌───────────────────────────────────┐  │
        │  │  • Local variables                │  │
        │  │  • Suspend/resume point           │  │
        │  │  • Parameters                     │  │
        │  │  • Promise object                 │  │
        │  └───────────────────────────────────┘  │
        └─────────────────────────────────────────┘
                              │
                              ▼
        ┌─────────────────────────────────────────┐
        │        PROMISE OBJECT                   │
        │  ┌───────────────────────────────────┐  │
        │  │  get_return_object()              │  │
        │  │  initial_suspend()                │  │
        │  │  final_suspend()                  │  │
        │  │  return_value() / return_void()   │  │
        │  │  unhandled_exception()            │  │
        │  └───────────────────────────────────┘  │
        └─────────────────────────────────────────┘
                              │
                              │ returns
                              ▼
        ┌─────────────────────────────────────────┐
        │     COROUTINE HANDLE / TASK             │
        │          (returned to caller)           │
        └─────────────────────────────────────────┘


EXECUTION TIMELINE:
===================

Time ──────────────────────────────────────────────────────►

Caller:  [call]────[RUNNING]────────────[resume]────[done]
                      │                     │
                      │                     │
Coro:              [START]──[SUSPENDED]──[RESUME]──[FINISH]
                      │         │            │
                      │         │            │
                   co_await  (waiting)   continues
                             
                             
SUSPENSION POINTS (co_await):
=============================

┌──────────────┐
│   RUNNING    │
└──────┬───────┘
       │ encounters co_await
       ▼
┌──────────────┐         ┌─────────────────┐
│   SUSPEND    │────────►│  Save state to  │
│              │         │  coroutine      │
└──────┬───────┘         │  frame (heap)   │
       │                 └─────────────────┘
       │ return to caller
       ▼
┌──────────────┐
│   CALLER     │
│  continues   │
└──────┬───────┘
       │ later...
       │ calls resume()
       ▼
┌──────────────┐         ┌─────────────────┐
│   RESUME     │◄────────│  Restore state  │
│              │         │  from frame     │
└──────┬───────┘         └─────────────────┘
       │
       ▼
┌──────────────┐
│   RUNNING    │
│  (continues) │
└──────────────┘


KEY KEYWORDS:
=============
• co_await  → suspends execution, returns control to caller
• co_yield  → suspends and produces a value
• co_return → completes coroutine, destroys frame
```

The key concept is that coroutines can **suspend** their execution at specific points (using `co_await` or `co_yield`) without blocking the thread, save their state to the heap, return control to the caller, and later be **resumed** from exactly where they left off.

---

**What coroutines are:**
* functions that can be suspended and resumed. 
* an alternative to writing asynchronous code. 

**They help simplify:**
* asynchronous I/O code, 
* lazy computations 
* event-driven applications. 

**When a coroutine is suspended:**
* the execution returns to the caller, 
* the data necessary to resume the coroutine is stored separately from the stack. 
* for this reason, the C++20 coroutines are called stackless.

**What coroutines give you:**
* **Suspend / resume** functions
* Zero-cost state machines (no threads required)
* Fine-grained control over *when* and *where* execution continues
* A foundation for async abstractions

**What they *don’t* give you:**
* Threads
* Scheduling
* Parallelism
* I/O

**Coroutines answer:**
> “How can I *pause* this function and resume it later?”

**In real systems, the stack looks like this:**
```
┌─────────────┐
│ Coroutines  │  ← syntax + control flow
├─────────────┤
│ Awaitables  │
├─────────────┤
│ Scheduler   │  ← thread pool / event loop
├─────────────┤
│ OS threads  │
└─────────────┘
```

---

# C++ Coroutines: The Three Fundamental Keywords

C++ coroutines, introduced in C++20, provide a mechanism for writing asynchronous and generator-style code that can suspend and resume execution. The three keywords that define coroutine behavior are `co_await`, `co_return`, and `co_yield`. Here's a detailed explanation of each:

## 1. **co_await** - Suspend and Wait

The `co_await` operator is the primary mechanism for suspending a coroutine's execution until some condition is met or a result becomes available.

**How it works:**
- When a coroutine encounters `co_await`, it evaluates the expression that follows
- The coroutine's execution is suspended at that point, and control returns to the caller
- The coroutine can be resumed later when the awaited operation completes
- Upon resumption, execution continues from the point immediately after the `co_await` expression

**Key characteristics:**
- The operand of `co_await` must be an "awaitable" type (implementing the awaiter interface with methods: 
- - `await_ready()`, 
- - `await_suspend()`, 
- - `await_resume()`
- While suspended, the coroutine's state is preserved, including local variables and the execution position
- The suspension is cooperative — the coroutine voluntarily yields control rather than being preempted
- Can be used to await asynchronous I/O operations, timers, or any custom awaitable type

**Example use case:**
```cpp
Task<std::string> fetchData() {
    auto response = co_await httpRequest("https://api.example.com");
    co_return response.body();
}
```

**Same example hardly commented:**
```cpp
// Task<std::string> is the coroutine's return type
// - Task is a custom coroutine type that wraps the async operation
// - The template parameter <std::string> indicates this coroutine will eventually
//   produce a string value when it completes
// - Task must have an associated promise_type that implements the coroutine interface
// - This return type tells the compiler to treat this function as a coroutine
Task<std::string> fetchData() {
    
    // co_await suspends the coroutine until httpRequest completes
    // Here's what happens step by step:
    
    // 1. httpRequest() is called with the URL, returning an awaitable object
    //    (likely representing an async HTTP operation)
    
    // 2. The co_await operator checks if the operation is already complete
    //    by calling await_ready() on the awaitable object
    
    // 3. If not ready, the coroutine suspends:
    //    - The coroutine's current state (local variables, execution point) is saved
    //    - await_suspend() is called, which typically schedules the async operation
    //    - Control returns to the caller of fetchData()
    //    - The coroutine's stack frame remains alive but inactive
    
    // 4. When the HTTP request completes (possibly seconds later, on another thread):
    //    - The coroutine is resumed from where it suspended
    //    - await_resume() is called on the awaitable object
    //    - The result of await_resume() is assigned to 'response'
    
    // 5. Execution continues on the next line with 'response' now containing
    //    the HTTP response object
    auto response = co_await httpRequest("https://api.example.com");
    
    // co_return completes the coroutine and provides the final result
    // Here's what happens:
    
    // 1. response.body() is called to extract the body content from the HTTP response
    //    (this returns a std::string containing the response data)
    
    // 2. The co_return keyword triggers the promise object's return_value() method,
    //    passing the string from response.body() as the argument
    
    // 3. The promise stores this value so it can be retrieved by whoever is
    //    awaiting this fetchData() coroutine
    
    // 4. The coroutine transitions to its completed state:
    //    - Local variables (response) are destroyed in reverse order
    //    - final_suspend() is called on the promise object
    //    - Any code awaiting this Task<std::string> can now retrieve the result
    
    // 5. The coroutine cannot be resumed again after co_return
    //    - The coroutine frame may be destroyed (depending on final_suspend)
    co_return response.body();
    
    // Note: No code can appear after co_return as it terminates the coroutine
    // (similar to how regular return statements work)
}

// Overall execution flow from the caller's perspective:
// 
// auto task = fetchData();           // Coroutine starts, hits co_await, suspends
//                                     // Returns immediately with a Task object
// 
// // ... other work can happen here while HTTP request is in progress ...
// 
// std::string data = co_await task;  // Wait for completion, get the result
//                                     // This will contain response.body()
```

**Additional context comments:**

```
// Why use coroutines for this pattern?
// ---------------------------------
// Without coroutines, you'd need to:
// 1. Pass a callback function to httpRequest()
// 2. Handle the response in that callback
// 3. Deal with nested callbacks for sequential async operations (callback hell)
//
// With coroutines, you can write async code that LOOKS synchronous:
// - Linear, top-to-bottom flow
// - No callback nesting
// - Easy error handling with try/catch
// - Local variables persist across suspension points

// Memory and lifetime:
// -------------------
// - The coroutine frame (containing 'response' and execution state) is allocated
//   on the heap when fetchData() is first called
// - This frame persists while the coroutine is suspended
// - The frame is deallocated after co_return completes (or if the Task is destroyed)
// - This is why coroutines can safely suspend—their state lives beyond the stack

// Thread safety note:
// ------------------
// - The coroutine may resume on a DIFFERENT thread than it started on
// - This depends on the implementation of httpRequest's awaitable
// - Your code continues safely because all state is captured in the coroutine frame
```


## 2. **co_return** - Complete Execution

The `co_return` keyword marks the completion point of a coroutine and optionally returns a value to the caller.

**How it works:**
- Terminates the coroutine's execution
- Optionally provides a return value that becomes available to whoever is awaiting or using the coroutine
- Triggers cleanup of the coroutine's state
- Calls the promise type's `return_value()` or `return_void()` method

**Key characteristics:**
- Similar to the regular `return` statement but designed for coroutines
- Can be used with or without a value: `co_return;` or `co_return value;`
- After `co_return` executes, the coroutine cannot be resumed again
- The returned value is typically stored in the coroutine's promise object for retrieval by the caller
- Destructors for local variables are called in reverse order of construction

**Example use case:**
```cpp
Task<int> computeSum(int a, int b) {
    int result = a + b;
    co_return result;  // Completes the coroutine and returns the sum
}
```

## 3. **co_yield** - Suspend and Produce a Value

The `co_yield` keyword suspends the coroutine's execution while simultaneously producing a value that can be retrieved by the caller. This is the foundation for generator-style coroutines.

**How it works:**
- Evaluates the expression following `co_yield`
- Passes that value to the coroutine's promise object (via `yield_value()`)
- Suspends the coroutine's execution
- The caller can retrieve the yielded value
- The coroutine can later be resumed to continue producing more values

**Key characteristics:**
- Essentially shorthand for `co_await promise.yield_value(expression)`
- Unlike `co_return`, the coroutine can be resumed after `co_yield` to continue execution
- Perfect for implementing generators, sequences, or streams of values
- Multiple values can be yielded during a single coroutine's lifetime
- Each yield represents a suspension point where control returns to the caller

**Example use case:**
```cpp
Generator<int> fibonacci() {
    int a = 0, b = 1;
    while (true) {
        co_yield a;  // Produce the next Fibonacci number
        int next = a + b;
        a = b;
        b = next;
    }
}
```

## Important Distinctions

**co_await vs co_yield:**
- `co_await` suspends to wait for something (typically asynchronous completion)
- `co_yield` suspends to provide something (a value to the caller)

**co_yield vs co_return:**
- `co_yield` allows the coroutine to continue after resumption
- `co_return` terminates the coroutine permanently

**Presence of any of these keywords:**
- If a function contains any of these three keywords, it becomes a coroutine
- The function's return type must be a coroutine-compatible type (with an associated promise type)
- Regular `return` statements cannot be used in coroutines (must use `co_return`)

## Underlying Mechanism

All three keywords interact with the coroutine's **promise object**, which is an instance of a promise type determined by the coroutine's return type. The promise object controls the coroutine's behavior, manages its state, and interfaces between the coroutine and its caller. The compiler transforms coroutine functions into state machines that can suspend at these specific points and resume execution later while preserving all local state.

---

# Returned type of `co_await`

The exact type of `response` depends on the **return type of the `await_resume()` method** in the awaitable object returned by `httpRequest()`.

Here's the detailed breakdown:

## Step-by-Step Type Determination

```cpp
auto response = co_await httpRequest("https://api.example.com");
//   ^^^^^^^^           ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
//   This type          This expression
```

### What happens:

1. **`httpRequest("https://api.example.com")`** returns some type, let's call it `HttpAwaitable`

2. **`co_await HttpAwaitable`** performs these operations:
   - Calls `HttpAwaitable::await_ready()` → returns `bool`
   - Calls `HttpAwaitable::await_suspend(handle)` → handles suspension
   - **Calls `HttpAwaitable::await_resume()`** → **THIS determines `response`'s type**

3. **`response`** gets the exact type returned by `await_resume()`

## Concrete Example

```cpp
// Hypothetical HttpAwaitable implementation
struct HttpAwaitable {
    bool await_ready() { return false; }
    
    void await_suspend(std::coroutine_handle<> handle) {
        // Schedule async HTTP request
    }
    
    // THIS method determines the type of 'response'
    HttpResponse await_resume() {
        return HttpResponse{/* ... */};
    }
};

// Therefore:
auto response = co_await httpRequest("...");
// response is of type: HttpResponse
```

## Common Scenarios

### Scenario 1: Returns a custom response object
```cpp
struct HttpResponse {
    int status_code;
    std::string body();
    std::map<std::string, std::string> headers;
};

struct HttpAwaitable {
    HttpResponse await_resume() { /* ... */ }
};

// Result:
auto response = co_await httpRequest("...");
// response is: HttpResponse
```

### Scenario 2: Returns by reference
```cpp
struct HttpAwaitable {
    HttpResponse& await_resume() { /* ... */ }
};

// Result:
auto response = co_await httpRequest("...");
// response is: HttpResponse& (reference)
```

### Scenario 3: Returns void
```cpp
struct HttpAwaitable {
    void await_resume() { /* ... */ }
};

// Result:
auto response = co_await httpRequest("...");
// response is: void
// This line would actually be a compile error with 'auto'
// You'd need: co_await httpRequest("..."); // no assignment
```

### Scenario 4: Returns a smart pointer
```cpp
struct HttpAwaitable {
    std::unique_ptr<HttpResponse> await_resume() { /* ... */ }
};

// Result:
auto response = co_await httpRequest("...");
// response is: std::unique_ptr<HttpResponse>
```

### Scenario 5: Returns optional or expected
```cpp
struct HttpAwaitable {
    std::optional<HttpResponse> await_resume() { /* ... */ }
};

// Result:
auto response = co_await httpRequest("...");
// response is: std::optional<HttpResponse>
```

## The Complete Picture

```cpp
// Full type resolution chain:
auto response = co_await httpRequest("https://api.example.com");

// Expands conceptually to:
auto&& awaitable = httpRequest("https://api.example.com");
//     ^^^^^^^^^ Type: HttpAwaitable (or whatever httpRequest returns)

if (!awaitable.await_ready()) {
    awaitable.await_suspend(coroutine_handle);
    // <suspension happens here>
}

auto response = awaitable.await_resume();
//   ^^^^^^^^                ^^^^^^^^^^^^
//   This type               Return type of await_resume()
```

## How to Find Out

In practice, to determine the exact type:

### Method 1: Check the library documentation
```cpp
// Look up httpRequest's documentation to see what its awaitable returns
```

### Method 2: Use compiler diagnostics
```cpp
// Force a type error to see what the compiler thinks:
int* response = co_await httpRequest("..."); // Intentional error
// Error: cannot convert 'HttpResponse' to 'int*'
//                       ^^^^^^^^^^^^ This tells you the actual type
```

### Method 3: Use `decltype`
```cpp
auto response = co_await httpRequest("...");
static_assert(std::is_same_v<decltype(response), HttpResponse>);
```

### Method 4: IDE inspection
Most modern IDEs will show you the deduced type when you hover over `response`

## Key Principle

**The type of the variable assigned from `co_await expr` is ALWAYS the return type of `expr`'s awaiter's `await_resume()` method.**

```cpp
co_await expression
         ^^^^^^^^^^
         Must be "awaitable"
         
         Awaitable requirements:
         - await_ready() → bool
         - await_suspend(handle) → void/bool/coroutine_handle
         - await_resume() → T  ← THIS IS THE TYPE YOU GET
```
