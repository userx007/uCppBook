# C++ Generator Coroutine 

## C++ Generator Coroutine Source Code

```cpp
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
/*
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

## C++ Generator Coroutine - Visual Flow Diagram

### Component Structure
```
┌─────────────────────────────────────────────────────────────────┐
│                      Generator<int>                             │
│  ┌───────────────────────────────────────────────────────────┐  │
│  │  coroutine_handle<promise_type> handle                    │  │
│  │  ┌─────────────────────────────────────────────────────┐  │  │
│  │  │          promise_type                               │  │  │
│  │  │  ┌──────────────────────────────────────────────┐   │  │  │
│  │  │  │ current_value: int                           │   │  │  │
│  │  │  │ exception: std::exception_ptr                │   │  │  │
│  │  │  └──────────────────────────────────────────────┘   │  │  │
│  │  │                                                     │  │  │
│  │  │  Lifecycle Methods:                                 │  │  │
│  │  │  • get_return_object()                              │  │  │
│  │  │  • initial_suspend()  → suspend_always              │  │  │
│  │  │  • yield_value(T)     → suspend_always              │  │  │
│  │  │  • final_suspend()    → suspend_always              │  │  │
│  │  │  • return_void()                                    │  │  │
│  │  │  • unhandled_exception()                            │  │  │
│  │  └─────────────────────────────────────────────────────┘  │  │
│  └───────────────────────────────────────────────────────────┘  │
│                                                                 │
│  Public API:                                                    │
│  • next()  → bool    (resumes coroutine)                        │
│  • value() → T       (gets current_value)                       │
└─────────────────────────────────────────────────────────────────┘
```

### Execution Flow: counter(5)

#### Step 1: Initial Call
```
main()                          Coroutine Space
  │
  │  auto gen = counter(5);
  │────────────────────────────────────────────►
  │                              ┌───────────────────────┐
  │                              │ 1. Allocate coroutine │
  │                              │    state/frame        │
  │                              │ 2. Create promise     │
  │                              │ 3. get_return_object()│
  │◄─────────────────────────────│ 4. initial_suspend()  │
  │                              │    [SUSPENDED]        │
  │                              └───────────────────────┘
  │ Generator object              Coroutine State:
  │ returned                      ╔═══════════════════╗
  │                               ║ i: [uninitialized]║
  ▼                               ║ n: 5              ║
                                  ║ current_value: ?  ║
                                  ║ STATE: SUSPENDED  ║
                                  ╚═══════════════════╝
```

#### Step 2: First next() Call
```
gen.next()
  │
  │ handle.resume()
  │─────────────────────────►  ┌─────────────────────┐
  │                            │ Resume execution    │
  │                            │ for(int i=0; ...)   │
  │                            │ i = 0               │
  │                            │ co_yield 0          │
  │                            │   current_value = 0 │
  │                            │   [SUSPEND HERE]    │
  │◄───────────────────────────│                     │
  │                            └─────────────────────┘
  │ returns true                Coroutine State:
  │                             ╔═══════════════════╗
  │                             ║ i: 0              ║
  │                             ║ n: 5              ║
  │                             ║ current_value: 0  ║
  │                             ║ STATE: SUSPENDED  ║
  │                             ╚═══════════════════╝
```

#### Step 3: Get Value
```
gen.value()
  │
  │ return handle.promise().current_value
  │─────────────────────────►  ╔═══════════════════╗
  │                            ║ current_value: 0  ║
  │◄───────────────────────────║                   ║
  │                            ╚═══════════════════╝
  │ returns 0
  ▼
```

#### Step 4: Subsequent next() Calls
```
Iteration 1→2:
gen.next()
  │─────►  i++; i=1; co_yield 1  ─────► current_value=1, SUSPENDED
  │◄───── returns true

gen.value() ────► returns 1

Iteration 2→3:
gen.next()
  │─────►  i++; i=2; co_yield 2  ─────► current_value=2, SUSPENDED
  │◄───── returns true

... continues until i=5 ...

Iteration 5→6:
gen.next()
  │─────►  i++; i=6; 
  │        condition (i<=5) fails
  │        loop exits
  │        final_suspend() called ─────► DONE state
  │◄───── returns false (done!)
```

### Complete State Machine Diagram

```
                         counter(5) invoked
                                │
                                ▼
                    ┌───────────────────────┐
                    │  CREATE COROUTINE     │
                    │  • Allocate frame     │
                    │  • Initialize promise │
                    └───────────┬───────────┘
                                │
                                ▼
                    ┌───────────────────────┐
                    │  initial_suspend()    │
                    │  Returns Generator    │
                    └───────────┬───────────┘
                                │
                                ▼
        ╔═══════════════════════════════════════════════╗
        ║            SUSPENDED (Initial)                ║
        ║  Waiting for first next() call                ║
        ╚═══════════════════╦═══════════════════════════╝
                            │ next() called
                            ▼
        ╔═══════════════════════════════════════════════╗
        ║            RUNNING                            ║
        ║  Executing: for(int i=0; i<=n; ++i)           ║
        ╚═══════════╦═══════════════════╦═══════════════╝
                    │                   │
         co_yield i │                   │ loop ends
                    ▼                   ▼
        ╔═══════════════════╗   ╔═══════════════════════╗
        ║  SUSPENDED        ║   ║  final_suspend()      ║
        ║  (at co_yield)    ║   ║                       ║
        ╚═════╦═════════════╝   ╚═══════╦═══════════════╝
              │ next()                  │
              │ called                  ▼
              │             ╔═══════════════════════════╗
              └────────────►║  DONE                     ║
                            ║  next() returns false     ║
                            ╚═══════════╦═══════════════╝
                                        │ ~Generator()
                                        ▼
                            ┌───────────────────────┐
                            │  handle.destroy()     │
                            │  Free coroutine frame │
                            └───────────────────────┘
```

### Memory Layout

```
┌─────────────────────────────────────────────────────────────┐
│                    STACK (main function)                    │
│  ┌────────────────────────────────────────────────────┐     │
│  │  Generator<int> gen                                │     │
│  │  ┌──────────────────────────────────────────────┐  │     │
│  │  │  coroutine_handle* ──────────┐               │  │     │
│  │  └──────────────────────────────┼───────────────┘  │     │
│  └─────────────────────────────────┼──────────────────┘     │
└────────────────────────────────────┼────────────────────────┘
                                     │
                                     │ points to
                                     ▼
┌─────────────────────────────────────────────────────────────┐
│                    HEAP (coroutine frame)                   │
│  ┌────────────────────────────────────────────────────┐     │
│  │  Coroutine Frame (allocated by compiler)           │     │
│  │  ┌──────────────────────────────────────────────┐  │     │
│  │  │  Promise Object (promise_type)               │  │     │
│  │  │    • current_value: int                      │  │     │
│  │  │    • exception: exception_ptr                │  │     │
│  │  ├──────────────────────────────────────────────┤  │     │
│  │  │  Local Variables                             │  │     │
│  │  │    • int i  (loop counter)                   │  │     │
│  │  │    • int n  (parameter copy)                 │  │     │
│  │  ├──────────────────────────────────────────────┤  │     │
│  │  │  Suspension Point (program counter)          │  │     │
│  │  │    • Where to resume execution               │  │     │
│  │  └──────────────────────────────────────────────┘  │     │
│  └────────────────────────────────────────────────────┘     │
└─────────────────────────────────────────────────────────────┘
```

### Timeline of Execution

```
Time ──────────────────────────────────────────────────────────►

Call counter(5)    next()  value()  next()  value()  ...  next()
     │              │        │        │        │            │
     ▼              ▼        ▼        ▼        ▼            ▼
┌────┴───┐  ┌───────┴────┐   │   ┌────┴────┐   │      ┌─────┴─────┐
│ Create │  │ Resume to  │   │   │ Resume  │   │      │ Resume to │
│ & Init │  │ co_yield 0 │   │   │ to      │   │      │ loop end  │
│ Suspend│  │ Suspend    │   │   │co_yield │   │      │ Done!     │
└────────┘  └────────────┘   │   │ 1       │   │      └───────────┘
                             │   │ Suspend │   │
           Read current=0  ──┘   └─────────┘   │
                                               │
                               Read current=1 ─┘

[SUSP]     [SUSP]             [SUSP]           [DONE]
  ↑          ↑                  ↑                ↑
  │          │                  │                │
  └──────────┴──────────────────┴────────────────┘
            Coroutine State Transitions
```