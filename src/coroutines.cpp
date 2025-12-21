/*
 * C++20 COROUTINES - COMPREHENSIVE GUIDE
 * ======================================
 * 
 * WHAT ARE COROUTINES?
 * -------------------
 * Coroutines are functions that can suspend execution and resume later,
 * maintaining their state between suspensions. They enable:
 * - Asynchronous operations without callback hell
 * - Generators and lazy evaluation
 * - Cooperative multitasking
 * 
 * A function is a coroutine if it uses: co_await, co_yield, or co_return
 */

#include <coroutine>
#include <iostream>
#include <exception>
#include <optional>

// ============================================================================
// EXAMPLE 1: BASIC GENERATOR - Simple Coroutine
// ============================================================================

// The promise_type defines the coroutine's behavior
template<typename T>
struct Generator {
    // Promise type is required for all coroutines
    struct promise_type {
        T current_value;
        std::exception_ptr exception;

        Generator get_return_object() {
            return Generator{std::coroutine_handle<promise_type>::from_promise(*this)};
        }
        
        std::suspend_always initial_suspend() { return {}; }
        std::suspend_always final_suspend() noexcept { return {}; }
        
        std::suspend_always yield_value(T value) {
            current_value = value;
            return {};
        }
        
        void return_void() {}
        
        void unhandled_exception() {
            exception = std::current_exception();
        }
    };

    std::coroutine_handle<promise_type> handle;
    
    Generator(std::coroutine_handle<promise_type> h) : handle(h) {}
    ~Generator() { if (handle) handle.destroy(); }
    
    // Move only
    Generator(const Generator&) = delete;
    Generator(Generator&& other) : handle(other.handle) {
        other.handle = nullptr;
    }
    
    T value() { return handle.promise().current_value; }
    bool next() {
        handle.resume();
        return !handle.done();
    }
};

// Simple generator coroutine - generates numbers from 0 to n
Generator<int> counter(int n) {
    for (int i = 0; i <= n; ++i) {
        co_yield i;  // Suspend and return value
    }
}

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