# Passing Arguments to Threads in C++

## Introduction

When working with multithreaded applications in C++, one of the fundamental skills is understanding how to properly pass arguments to threads. The `std::thread` constructor accepts a callable object (function, lambda, functor) followed by its arguments. However, the way these arguments are passed involves important subtleties related to copying, reference semantics, and move semantics that can lead to bugs if not properly understood.

## Basic Argument Passing

The simplest form of passing arguments to a thread involves passing by value. Arguments are copied into thread-local storage and then forwarded to the thread function.

```cpp
#include <iostream>
#include <thread>
#include <string>

void print_message(int id, std::string message) {
    std::cout << "Thread " << id << ": " << message << std::endl;
}

int main() {
    std::string msg = "Hello from thread";
    std::thread t1(print_message, 1, msg);
    std::thread t2(print_message, 2, "Direct string");
    
    t1.join();
    t2.join();
    
    return 0;
}
```

In this example, both `id` and `message` are copied into the thread's storage. Even if `msg` is modified in the main thread after creating `t1`, the thread will have its own copy.

## The Reference Problem

A common mistake is attempting to pass arguments by reference using the standard reference syntax. This doesn't work as expected because `std::thread` copies all arguments by default.

```cpp
#include <iostream>
#include <thread>
#include <chrono>

// This function expects a reference
void increment(int& value) {
    for (int i = 0; i < 5; ++i) {
        ++value;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    int counter = 0;
    
    // WRONG: This will not compile!
    // std::thread t(increment, counter);
    
    // The thread tries to copy counter, but increment expects a reference
    // This creates a type mismatch
    
    std::cout << "Counter: " << counter << std::endl;
    
    return 0;
}
```

The above code fails because `std::thread` attempts to copy `counter`, but the function `increment` expects a non-const reference, which cannot bind to the copied temporary value.

## Using std::ref and std::cref

To pass arguments by reference to a thread, you must explicitly use `std::ref` (for mutable references) or `std::cref` (for const references). These wrapper functions tell `std::thread` to pass references instead of making copies.

```cpp
#include <iostream>
#include <thread>
#include <functional>
#include <vector>

void increment(int& value) {
    for (int i = 0; i < 1000; ++i) {
        ++value;
    }
}

void print_info(const std::string& name, const std::vector<int>& data) {
    std::cout << name << " has " << data.size() << " elements" << std::endl;
}

int main() {
    int counter = 0;
    
    // CORRECT: Use std::ref to pass by reference
    std::thread t1(increment, std::ref(counter));
    t1.join();
    
    std::cout << "Counter after thread: " << counter << std::endl;
    
    // Using std::cref for const references
    std::string name = "MyVector";
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    std::thread t2(print_info, std::cref(name), std::cref(vec));
    t2.join();
    
    return 0;
}
```

**Important Note:** When using `std::ref`, you must ensure that the referenced object's lifetime extends beyond the thread's lifetime. Accessing a destroyed object from a thread leads to undefined behavior.

## Move Semantics with Threads

For expensive-to-copy objects or move-only types (like `std::unique_ptr`), you should use move semantics to transfer ownership to the thread efficiently.

```cpp
#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <string>

class LargeData {
    std::vector<int> data;
public:
    LargeData(size_t size) : data(size, 42) {
        std::cout << "LargeData created with " << size << " elements" << std::endl;
    }
    
    // Move constructor
    LargeData(LargeData&& other) noexcept : data(std::move(other.data)) {
        std::cout << "LargeData moved" << std::endl;
    }
    
    size_t size() const { return data.size(); }
};

void process_data(LargeData data) {
    std::cout << "Processing data with " << data.size() << " elements" << std::endl;
}

void process_unique(std::unique_ptr<std::string> ptr) {
    std::cout << "Processing: " << *ptr << std::endl;
}

int main() {
    // Moving a large object
    LargeData large(1000000);
    std::thread t1(process_data, std::move(large));
    // 'large' is now in a moved-from state and should not be used
    t1.join();
    
    // Moving a unique_ptr (move-only type)
    auto ptr = std::make_unique<std::string>("Important data");
    std::thread t2(process_unique, std::move(ptr));
    // 'ptr' is now nullptr
    t2.join();
    
    return 0;
}
```

Using `std::move` transfers ownership of the object to the thread, avoiding expensive copies and enabling the use of move-only types.

## Pointer Arguments and Implicit Conversions

Be careful with pointer arguments and implicit conversions. The thread constructor stores arguments by value, which can lead to unexpected behavior with pointers.

```cpp
#include <iostream>
#include <thread>
#include <string>
#include <chrono>

void process_string(const std::string& s) {
    std::cout << "Processing: " << s << std::endl;
}

int main() {
    char buffer[100] = "Hello Thread";
    
    // DANGEROUS: The pointer might dangle!
    // std::thread t(process_string, buffer);
    
    // The char* is stored as-is, and conversion to std::string
    // happens in the thread context. If buffer goes out of scope
    // before the conversion, we have undefined behavior.
    
    // SAFE: Explicitly convert to std::string in the calling thread
    std::thread t(process_string, std::string(buffer));
    
    t.join();
    
    return 0;
}
```

## Practical Example: Thread-Safe Counter

Here's a comprehensive example demonstrating various argument-passing techniques:

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>

class ThreadSafeCounter {
    int value;
    std::mutex mtx;
public:
    ThreadSafeCounter(int initial = 0) : value(initial) {}
    
    void increment(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        value += amount;
    }
    
    int get() {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }
};

// Pass by value - receives a copy
void worker_copy(int id, std::string name) {
    std::cout << "Worker " << id << " (" << name << ") starting" << std::endl;
}

// Pass by reference - modifies the original
void worker_ref(int id, ThreadSafeCounter& counter) {
    for (int i = 0; i < 100; ++i) {
        counter.increment(1);
    }
    std::cout << "Worker " << id << " completed" << std::endl;
}

// Pass by move - takes ownership
void worker_move(int id, std::unique_ptr<std::vector<int>> data) {
    std::cout << "Worker " << id << " received vector with " 
              << data->size() << " elements" << std::endl;
}

int main() {
    // Example 1: Pass by value
    std::string name = "Alpha";
    std::thread t1(worker_copy, 1, name);
    t1.join();
    
    // Example 2: Pass by reference with std::ref
    ThreadSafeCounter counter(0);
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker_ref, i, std::ref(counter));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter value: " << counter.get() << std::endl;
    
    // Example 3: Pass by move
    auto vec = std::make_unique<std::vector<int>>(100, 42);
    std::thread t2(worker_move, 2, std::move(vec));
    t2.join();
    
    return 0;
}
```

## Member Function Threads

When passing member functions to threads, you need to pass the object (or its pointer/reference) as an argument:

```cpp
#include <iostream>
#include <thread>

class Worker {
    int id;
public:
    Worker(int i) : id(i) {}
    
    void do_work(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            std::cout << "Worker " << id << " iteration " << i << std::endl;
        }
    }
};

int main() {
    Worker w1(1);
    Worker w2(2);
    
    // Pass member function with object reference
    std::thread t1(&Worker::do_work, &w1, 3);
    
    // Or use std::ref
    std::thread t2(&Worker::do_work, std::ref(w2), 3);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

## Summary

**Key Points:**

1. **Default Behavior:** Arguments are copied by default into thread-local storage before being forwarded to the thread function.

2. **Reference Semantics:** To pass by reference, explicitly use `std::ref()` for mutable references or `std::cref()` for const references. Without these, you cannot pass references to threads.

3. **Move Semantics:** Use `std::move()` to transfer ownership of expensive-to-copy or move-only types (like `std::unique_ptr`) to threads, avoiding unnecessary copies.

4. **Lifetime Management:** When using references (via `std::ref`), ensure the referenced object outlives the thread. Dangling references lead to undefined behavior.

5. **Implicit Conversions:** Be cautious with implicit type conversions and pointer arguments. Perform conversions explicitly in the calling thread to avoid dangling pointer issues.

6. **Member Functions:** When calling member functions, pass the member function pointer and the object (or its reference/pointer) as separate arguments.

Understanding these argument-passing semantics is crucial for writing correct and efficient multithreaded C++ code. Misusing references or failing to properly move objects can lead to bugs ranging from unnecessary copies to dangerous undefined behavior.

---

# Move Semantics with Threads

Reducing it to **one move** (or zero moves) using references

## Option A — **Zero moves** (pass by reference)
If the data already exists and you just want the thread to *use it*, don’t pass by value.

### Change the function signature

```cpp
void process_data(const LargeData& data) {
    std::cout << "Processing data with " << data.size() << " elements\n";
}
```

### Launch the thread with `std::cref`

```cpp
LargeData large(1000000);

std::thread t1(process_data, std::cref(large));
t1.join();
```

### What happens

* `std::thread` stores a `std::reference_wrapper<const LargeData>`
* **No `LargeData` move constructor is ever called**
* The thread accesses `large` directly

**Important rule**
You must guarantee `large` **outlives the thread**.


## Option B — **Exactly one move** (move once, then pass by reference)

If you want to *transfer ownership* to the thread but still avoid two moves:

### Step 1: Move into a local object

```cpp
LargeData large(1000000);
LargeData owned = std::move(large);  // ONE move here
```

### Step 2: Pass by reference to the thread

```cpp
void process_data(LargeData& data) {
    std::cout << "Processing data with " << data.size() << " elements\n";
}

std::thread t1(process_data, std::ref(owned));
t1.join();
```

### Result

* One move total
* Thread operates on the “owned” object
* Lifetime is explicit and clear

---

## Option C — **Heap ownership (common & safe)**

A very idiomatic solution is **heap ownership**, especially for threads:

```cpp
void process_data(std::unique_ptr<LargeData> data) {
    std::cout << "Processing data with " << data->size() << " elements\n";
}

std::thread t1(
    process_data,
    std::make_unique<LargeData>(1000000)
);
t1.join();
```

### Moves involved

* One move into `std::thread`
* One move into `process_data`

But:

* `unique_ptr` moves are trivial
* No large data movement

📌 **This is usually the best design**

The two-move behavior is **mandated by the standard**, not an implementation quirk.

## Key standard reference — `std::thread` constructor

### **[thread.thread.constr]**

The constructor is specified (simplified):

The new thread of execution executes:
```cpp
invoke(decay_copy(f), decay_copy(args)...)
```

### What `decay_copy` means

`decay_copy(x)` is defined as:

```cpp
std::decay_t<decltype(x)>(std::forward<decltype(x)>(x))
```

This means:

* References are stripped
* `const` is removed
* The value is **copied or moved** into internal storage

➡️ **This is the first move**

---

## Invocation of the function

Later, when the new thread starts:

```cpp
invoke(stored_f, stored_args...)
```
If your function parameter is **by value**:

```cpp
void process_data(LargeData data);
```
then:

* `stored_args` must be **moved again** to initialize `data`

➡️ **This is the second move**

## Why references change everything

When you use:

```cpp
std::ref(obj)
```
The type becomes:

```cpp
std::reference_wrapper<T>
```
Which:

* Is cheap to copy
* Decays to a reference at invocation
* Avoids object materialization

This is explicitly allowed by: **[func.require] + `std::invoke`**
`std::reference_wrapper<T>` is unwrapped into `T&` when invoking the function.

## Standard-mandated summary

| Step               | What the standard requires | Result      |
| ------------------ | -------------------------- | ----------- |
| Argument capture   | `decay_copy(args)`         | 1st move    |
| Thread invocation  | pass stored args           | 2nd move    |
| `std::ref` used    | reference_wrapper stored   | no move     |
| By-value parameter | new object created         | move occurs |

## Final mental model (standard-accurate)

**`std::thread` must first own *decayed copies* of its arguments, then later invoke the function with those arguments — value parameters therefore require two constructions.**
