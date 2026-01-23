/*
g++ -std=c++20 -o coroutines coroutines.cpp
*/

#include <coroutine>
#include <iostream>
#include <exception>

// ============================================================================
// TASK - The return type for our coroutine
// ============================================================================
template<typename T>
struct Task {
    // Promise type - defines coroutine behavior
    struct promise_type {
        T value_;
        std::exception_ptr exception_;

        // Called to create the Task object returned to caller
        Task get_return_object() {
            return Task{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        // Should coroutine suspend at initial call?
        std::suspend_never initial_suspend() { 
            std::cout << "  [Promise] initial_suspend - starting immediately\n";
            return {}; 
        }

        // Should coroutine suspend at final return?
        std::suspend_always final_suspend() noexcept { 
            std::cout << "  [Promise] final_suspend - suspending at end\n";
            return {}; 
        }

        // Handle co_return <value>
        void return_value(T val) {
            std::cout << "  [Promise] return_value - storing result: " << val << "\n";
            value_ = val;
        }

        // Handle exceptions thrown in coroutine
        void unhandled_exception() {
            std::cout << "  [Promise] unhandled_exception\n";
            exception_ = std::current_exception();
        }
    };

    // Coroutine handle
    std::coroutine_handle<promise_type> handle_;

    // Constructor
    Task(std::coroutine_handle<promise_type> h) : handle_(h) {
        std::cout << "  [Task] Constructor - coroutine handle created\n";
    }

    // Destructor - clean up coroutine frame
    ~Task() {
        std::cout << "  [Task] Destructor - destroying coroutine\n";
        if (handle_) handle_.destroy();
    }

    // Move semantics
    Task(Task&& t) noexcept : handle_(t.handle_) { t.handle_ = {}; }
    Task& operator=(Task&& t) noexcept {
        if (this != &t) {
            if (handle_) handle_.destroy();
            handle_ = t.handle_;
            t.handle_ = {};
        }
        return *this;
    }

    // Delete copy
    Task(const Task&) = delete;
    Task& operator=(const Task&) = delete;

    // Get the result
    T get_result() {
        if (handle_.promise().exception_) {
            std::rethrow_exception(handle_.promise().exception_);
        }
        return handle_.promise().value_;
    }

    // Check if coroutine is done
    bool done() const { return handle_.done(); }

    // Resume coroutine
    void resume() {
        if (!handle_.done()) {
            std::cout << "  [Task] Resuming coroutine...\n";
            handle_.resume();
        }
    }
};

// ============================================================================
// AWAITABLE - Custom awaitable type for co_await
// ============================================================================
struct SimpleAwaitable {
    int await_value_;

    // Should we suspend when co_await is called?
    bool await_ready() { 
        std::cout << "    [Awaitable] await_ready - check if ready (returning false = will suspend)\n";
        return false; // Always suspend for demonstration
    }

    // Called when suspending - can customize behavior
    void await_suspend(std::coroutine_handle<> h) {
        std::cout << "    [Awaitable] await_suspend - coroutine suspended, could schedule resume\n";
        // In real async code, you'd schedule h.resume() to be called later
        // For this example, we'll resume immediately in the caller
    }

    // Called when resuming - return value becomes result of co_await expression
    int await_resume() {
        std::cout << "    [Awaitable] await_resume - returning value: " << await_value_ << "\n";
        return await_value_;
    }
};

// ============================================================================
// GENERATOR - Example using co_yield
// ============================================================================
template<typename T>
struct Generator {
    struct promise_type {
        T current_value_;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }

        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }

        // Handle co_yield <value>
        std::suspend_always yield_value(T value) {
            std::cout << "  [Generator] yield_value - yielding: " << value << "\n";
            current_value_ = value;
            return {};
        }

        void return_void() {}
        void unhandled_exception() { std::terminate(); }
    };

    std::coroutine_handle<promise_type> handle_;

    Generator(std::coroutine_handle<promise_type> h) : handle_(h) {}
    ~Generator() { if (handle_) handle_.destroy(); }

    Generator(Generator&& g) noexcept : handle_(g.handle_) { g.handle_ = {}; }
    Generator& operator=(Generator&&) = delete;
    Generator(const Generator&) = delete;
    Generator& operator=(const Generator&) = delete;

    T current_value() { return handle_.promise().current_value_; }
    bool done() { return handle_.done(); }
    
    bool next() {
        handle_.resume();
        return !handle_.done();
    }
};

// ============================================================================
// COROUTINE FUNCTIONS
// ============================================================================

// Coroutine using co_await and co_return
Task<int> computeAsync(int x) {
    std::cout << "\n[Coroutine] computeAsync started with x=" << x << "\n";
    
    std::cout << "[Coroutine] About to co_await...\n";
    int result = co_await SimpleAwaitable{x * 10};
    
    std::cout << "[Coroutine] Resumed after co_await, got: " << result << "\n";
    
    std::cout << "[Coroutine] About to co_return...\n";
    co_return result + 5;
}

// Coroutine using co_yield
Generator<int> generateNumbers(int count) {
    std::cout << "\n[Generator] Starting to generate " << count << " numbers\n";
    for (int i = 0; i < count; ++i) {
        std::cout << "[Generator] About to co_yield " << i << "\n";
        co_yield i;
        std::cout << "[Generator] Resumed after yield\n";
    }
    std::cout << "[Generator] Finished generating\n";
}

// ============================================================================
// MAIN - Demonstrates coroutine usage
// ============================================================================
int main() {
    std::cout << "=== EXAMPLE 1: co_await and co_return ===\n";
    std::cout << "\n[Main] Calling computeAsync(7)...\n";
    
    Task<int> task = computeAsync(7);
    
    std::cout << "\n[Main] Back in main, coroutine is suspended\n";
    std::cout << "[Main] Coroutine done? " << (task.done() ? "yes" : "no") << "\n";
    
    std::cout << "\n[Main] Manually resuming coroutine...\n";
    task.resume();
    
    std::cout << "\n[Main] After resume, done? " << (task.done() ? "yes" : "no") << "\n";
    std::cout << "[Main] Final result: " << task.get_result() << "\n";
    
    std::cout << "\n\n=== EXAMPLE 2: co_yield (Generator) ===\n";
    std::cout << "\n[Main] Creating generator...\n";
    
    auto gen = generateNumbers(3);
    
    std::cout << "\n[Main] Starting iteration...\n";
    while (gen.next()) {
        std::cout << "[Main] Got value: " << gen.current_value() << "\n\n";
    }
    
    std::cout << "\n[Main] Generator exhausted\n";
    
    std::cout << "\n=== Program ending (destructors will be called) ===\n";
    return 0;
}


/*
Comprehensive example that demonstrates all the key components of C++20 coroutines:

**What this example shows:**

1. **Promise Type Methods** (all required):
   - `get_return_object()` - Creates the Task/Generator returned to caller
   - `initial_suspend()` - Controls if coroutine starts immediately or suspended
   - `final_suspend()` - Controls behavior when coroutine completes
   - `return_value()` - Handles `co_return <value>`
   - `unhandled_exception()` - Handles exceptions in coroutine
   - `yield_value()` - Handles `co_yield <value>` (for generators)

2. **Awaitable Type Methods**:
   - `await_ready()` - Check if we need to suspend
   - `await_suspend()` - Called when suspending (can schedule resume)
   - `await_resume()` - Called when resuming (returns value to `co_await` expression)

3. **Keywords in Action**:
   - `co_await` - Suspends and waits for a value
   - `co_return` - Returns final value and ends coroutine
   - `co_yield` - Suspends and produces a value (for generators)

The extensive logging shows you exactly when each method is called and the flow of execution. When you compile and run this, you'll see the coroutine suspend, return control to main, get resumed, and finally complete.
*/