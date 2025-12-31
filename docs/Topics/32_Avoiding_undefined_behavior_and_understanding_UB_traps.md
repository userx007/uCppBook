# Avoiding Undefined Behavior and Understanding UB Traps

Undefined behavior (UB) in C++ is one of the most dangerous aspects of the language. When your program invokes UB, the C++ standard makes **no guarantees** about what happens—your program might crash, produce wrong results, appear to work fine, or even format your hard drive (though unlikely, it's technically allowed by the standard).

## What Makes UB So Dangerous

The scary part about UB is that it can appear to work correctly during testing, then fail catastrophically in production. Compilers are allowed to assume UB never happens and optimize accordingly, leading to seemingly impossible bugs.

## Common UB Traps with Examples

### 1. **Signed Integer Overflow**

```cpp
int x = INT_MAX;
int y = x + 1;  // UB! Signed overflow
```

The compiler might optimize this assuming overflow never happens, leading to bizarre behavior. Use unsigned integers or check bounds explicitly:

```cpp
// Safe approach
if (x > INT_MAX - 1) {
    // Handle overflow
} else {
    int y = x + 1;
}
```

### 2. **Out-of-Bounds Array Access**

```cpp
int arr[5] = {1, 2, 3, 4, 5};
int value = arr[10];  // UB! Reading beyond array bounds
arr[-1] = 0;          // UB! Negative index
```

This might crash, return garbage, or even appear to work. Always validate indices:

```cpp
if (index >= 0 && index < 5) {
    value = arr[index];
}
// Or use std::vector with .at() for bounds checking
```

### 3. **Dereferencing Null or Dangling Pointers**

```cpp
int* ptr = nullptr;
*ptr = 42;  // UB! Dereferencing null pointer

int* dangling;
{
    int x = 10;
    dangling = &x;
}  // x destroyed here
*dangling = 20;  // UB! Dangling pointer
```

Always check pointers before dereferencing:

```cpp
if (ptr != nullptr) {
    *ptr = 42;
}
```

### 4. **Use After Free**

```cpp
int* ptr = new int(42);
delete ptr;
*ptr = 100;  // UB! Using freed memory
```

This is particularly insidious because it might work during testing. Set pointers to nullptr after deletion:

```cpp
delete ptr;
ptr = nullptr;  // Now can safely check
```

### 5. **Uninitialized Variables**

```cpp
int x;  // Not initialized
if (x == 5) {  // UB! Reading uninitialized value
    // ...
}

int arr[100];
for (int i = 0; i < 100; i++) {
    cout << arr[i];  // UB! Uninitialized array elements
}
```

Always initialize variables:

```cpp
int x = 0;  // Or let the compiler determine: int x{};
int arr[100] = {};  // Zero-initialize entire array
```

### 6. **Data Races (Multithreading)**

```cpp
int counter = 0;

void thread1() {
    for (int i = 0; i < 1000; i++) {
        counter++;  // UB if thread2 runs concurrently!
    }
}

void thread2() {
    for (int i = 0; i < 1000; i++) {
        counter++;  // Race condition!
    }
}
```

Use proper synchronization:

```cpp
std::mutex mtx;
void thread1() {
    for (int i = 0; i < 1000; i++) {
        std::lock_guard<std::mutex> lock(mtx);
        counter++;
    }
}
// Or use std::atomic<int>
```

### 7. **Modifying String Literals**

```cpp
char* str = "Hello";  // Should be const char*
str[0] = 'h';  // UB! Modifying string literal
```

String literals are stored in read-only memory:

```cpp
char str[] = "Hello";  // Creates mutable copy
str[0] = 'h';  // OK
```

### 8. **Violating Strict Aliasing Rule**

```cpp
int x = 42;
float* f = reinterpret_cast<float*>(&x);
float value = *f;  // UB! Type punning violation
```

Use memcpy or unions (carefully):

```cpp
float value;
memcpy(&value, &x, sizeof(float));  // Safe
```

### 9. **Invalid Iterator Usage**

```cpp
std::vector<int> vec = {1, 2, 3};
auto it = vec.begin();
vec.push_back(4);  // May reallocate!
*it = 10;  // UB! Iterator invalidated
```

Be aware of operations that invalidate iterators:

```cpp
for (auto it = vec.begin(); it != vec.end(); ) {
    if (*it == 2) {
        it = vec.erase(it);  // erase returns valid iterator
    } else {
        ++it;
    }
}
```

### 10. **Shifting by Invalid Amounts**

```cpp
int x = 1;
int y = x << 32;  // UB! Shift >= width of type
int z = x << -1;  // UB! Negative shift
```

Always validate shift amounts:

```cpp
if (shift >= 0 && shift < 32) {
    int y = x << shift;
}
```

## Tools to Detect UB

- **AddressSanitizer (ASan)**: Detects memory errors
- **UndefinedBehaviorSanitizer (UBSan)**: Catches many UB cases
- **Valgrind**: Memory debugging tool
- **Static analyzers**: Clang-Tidy, Cppcheck
- **Compile with warnings**: `-Wall -Wextra -Werror`

Example compilation:
```bash
g++ -fsanitize=undefined,address -Wall -Wextra program.cpp
```

## The Golden Rule

**Never assume UB "won't happen in practice."** Modern optimizers are aggressive and will exploit UB assumptions in ways that seem impossible. Always write defensive code, validate inputs, and use tools to catch UB early in development.