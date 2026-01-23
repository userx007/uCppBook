/*
g++ -std=c++20 -fcoroutines -o fibonacci fibonacci.cpp
*/

#include <iostream>
#include <coroutine>
#include <exception>

// ==============================================================================
// GENERATOR IMPLEMENTATION
// ==============================================================================
// This is the "promise_type" and iterator infrastructure needed to make
// a coroutine work as a generator (lazy sequence producer)

template<typename T>
class generator {
public:
    // -------------------------------------------------------------------------
    // PROMISE TYPE - Required by C++20 coroutines
    // -------------------------------------------------------------------------
    // The promise_type is how the compiler communicates with your coroutine.
    // It's called "promise" but it's different from std::promise!
    struct promise_type {
        T current_value;                    // Stores the yielded value
        std::exception_ptr exception;       // Stores any exception thrown
        
        // Called when coroutine is created - returns the generator object
        generator get_return_object() {
            return generator{
                std::coroutine_handle<promise_type>::from_promise(*this)
            };
        }
        
        // Called at the start of the coroutine
        // suspend_always means "don't run until asked"
        std::suspend_always initial_suspend() { return {}; }
        
        // Called at the end of the coroutine
        // suspend_always means "stay suspended so we can check if done"
        std::suspend_always final_suspend() noexcept { return {}; }
        
        // Called when co_yield is used - stores the value and suspends
        std::suspend_always yield_value(T value) {
            current_value = std::move(value);
            return {};  // Suspend after storing the value
        }
        
        // Called if an exception escapes the coroutine
        void unhandled_exception() {
            exception = std::current_exception();
        }
        
        // Called if co_return is used (we don't use it in this example)
        void return_void() {}
    };
    
    // -------------------------------------------------------------------------
    // ITERATOR - Makes generator work with range-based for loops
    // -------------------------------------------------------------------------
    struct iterator {
        std::coroutine_handle<promise_type> handle;
        
        // Advance to next value
        iterator& operator++() {
            handle.resume();  // Resume coroutine to get next value
            if (handle.done()) {
                handle = nullptr;  // Mark as end iterator
            }
            return *this;
        }
        
        // Get current value
        const T& operator*() const {
            return handle.promise().current_value;
        }
        
        // Compare iterators (for loop termination)
        bool operator==(const iterator& other) const {
            return handle == other.handle;
        }
        
        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }
    };
    
    // -------------------------------------------------------------------------
    // GENERATOR INTERFACE
    // -------------------------------------------------------------------------
    
    // Constructor - takes ownership of the coroutine handle
    explicit generator(std::coroutine_handle<promise_type> h) 
        : handle(h) {}
    
    // Destructor - must destroy the coroutine to free resources
    ~generator() {
        if (handle) {
            handle.destroy();
        }
    }
    
    // Move-only type (coroutines shouldn't be copied)
    generator(const generator&) = delete;
    generator& operator=(const generator&) = delete;
    
    generator(generator&& other) noexcept : handle(other.handle) {
        other.handle = nullptr;
    }
    
    generator& operator=(generator&& other) noexcept {
        if (this != &other) {
            if (handle) {
                handle.destroy();
            }
            handle = other.handle;
            other.handle = nullptr;
        }
        return *this;
    }
    
    // Begin iterator - starts the coroutine
    iterator begin() {
        if (handle) {
            handle.resume();  // Start/resume coroutine to first yield
            if (handle.done()) {
                return iterator{nullptr};  // Coroutine finished immediately
            }
            return iterator{handle};
        }
        return iterator{nullptr};
    }
    
    // End iterator - sentinel value
    iterator end() {
        return iterator{nullptr};
    }
    
private:
    std::coroutine_handle<promise_type> handle;
};

// ==============================================================================
// FIBONACCI GENERATOR COROUTINE
// ==============================================================================
// This function is a coroutine because it uses co_yield
// The compiler transforms it into a state machine automatically

generator<int> fibonacci() {
    int a = 0, b = 1;
    
    // Infinite loop - but it's lazy! Only generates values when requested
    while (true) {
        // co_yield suspends the coroutine and returns 'a' to the caller
        // When resumed, execution continues right after this line
        co_yield a;
        
        // Calculate next Fibonacci number
        int next = a + b;
        a = b;
        b = next;
    }
    
    // This line is never reached because of the infinite loop
    // But if it were, co_return would end the generator
}

// ==============================================================================
// ADDITIONAL EXAMPLES
// ==============================================================================

// Example: Finite generator with a limit
generator<int> fibonacci_limited(int max_value) {
    int a = 0, b = 1;
    
    while (a <= max_value) {
        co_yield a;
        int next = a + b;
        a = b;
        b = next;
    }
    // When function ends, the generator is done (no more values)
}

// Example: Generator that takes parameters and has state
generator<int> range(int start, int end, int step = 1) {
    for (int i = start; i < end; i += step) {
        co_yield i;
    }
}

// Example: Generator that squares its input sequence
generator<int> squares(int count) {
    for (int i = 0; i < count; ++i) {
        co_yield i * i;
    }
}

// ==============================================================================
// MAIN - DEMONSTRATION
// ==============================================================================

int main() {
    std::cout << "=== Example 1: Infinite Fibonacci (first 10 under 100) ===\n";
    // The coroutine only runs when we iterate
    // Each iteration resumes the coroutine from where it last suspended
    for (int value : fibonacci()) {
        if (value > 100) break;  // Stop when we exceed 100
        std::cout << value << " ";
    }
    std::cout << "\n\n";
    
    std::cout << "=== Example 2: Limited Fibonacci (up to 1000) ===\n";
    // This generator automatically stops when the condition is met
    for (int value : fibonacci_limited(1000)) {
        std::cout << value << " ";
    }
    std::cout << "\n\n";
    
    std::cout << "=== Example 3: Range generator (0 to 20, step 3) ===\n";
    for (int value : range(0, 20, 3)) {
        std::cout << value << " ";
    }
    std::cout << "\n\n";
    
    std::cout << "=== Example 4: First 10 squares ===\n";
    for (int value : squares(10)) {
        std::cout << value << " ";
    }
    std::cout << "\n\n";
    
    std::cout << "=== Example 5: Manual iteration (more control) ===\n";
    // You can also manually control the iteration
    auto fib = fibonacci();
    auto it = fib.begin();
    auto end_it = fib.end();
    
    int count = 0;
    while (it != end_it && count < 8) {
        std::cout << *it << " ";
        ++it;  // This calls handle.resume() internally
        ++count;
    }
    std::cout << "\n\n";
    
    std::cout << "=== Key Points ===\n";
    std::cout << "1. Coroutines are LAZY - fibonacci() creates infinite sequence\n";
    std::cout << "   but only generates values when iterated\n";
    std::cout << "2. No threads involved - just pausable functions\n";
    std::cout << "3. State (a, b) is preserved between co_yield calls\n";
    std::cout << "4. Very memory efficient - only stores current state\n";
    std::cout << "5. The compiler transforms this into a state machine\n";
    
    return 0;
}

// ==============================================================================
// COMPILATION
// ==============================================================================
// Compile with C++20 support:
//   g++ -std=c++20 -fcoroutines -o fibonacci fibonacci.cpp
//   clang++ -std=c++20 -stdlib=libc++ -o fibonacci fibonacci.cpp
//   
// On some compilers you might need:
//   g++ -std=c++20 -fcoroutines -O2 -o fibonacci fibonacci.cpp
//
// ==============================================================================
// HOW IT WORKS UNDER THE HOOD
// ==============================================================================
// When you write:
//   for (int value : fibonacci()) { ... }
//
// The compiler roughly transforms it to:
//   1. auto gen = fibonacci();           // Creates coroutine, doesn't run it
//   2. auto it = gen.begin();            // Calls handle.resume() -> runs until first co_yield
//   3. while (it != gen.end()) {
//        int value = *it;                // Gets current_value from promise
//        { your loop body }
//        ++it;                           // Calls handle.resume() -> runs until next co_yield
//      }
//   4. gen destructor called             // Calls handle.destroy() to clean up
//
// Each co_yield:
//   - Stores the value in promise.current_value
//   - Saves the coroutine state (local variables, position)
//   - Returns control to the caller
//   - When resumed, continues right after co_yield
// ==============================================================================