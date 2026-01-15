# if statement with initializer introduced in C++17

```cpp
if (auto err = spi.init(); err) {
    std::cout << "Cannot init SPI for performance test\n";
    return;
}
```

### Step 1: The initializer

```cpp
auto err = spi.init();
```

* This runs **before the condition is checked**.
* `spi.init()` returns a value (in your code, probably `std::optional<std::string>`).
* `err` is **local to the `if` statement** — it only exists inside the `if` block and the `else` block (if any).
* This is useful to avoid polluting the surrounding scope with temporary variables.

### Step 2: The condition

```cpp
err
```

* In this case, `err` is an `std::optional<std::string>` (likely).
* When used in a boolean context, `std::optional` evaluates to `true` if it **contains a value** (i.e., there was an error).
* So the `if` block executes only if `init()` returned an error.

### Step 3: The block

```cpp
{
    std::cout << "Cannot init SPI for performance test\n";
    return;
}
```

* Handles the error locally.
* `err` is still accessible here if you wanted to print the actual error message:

```cpp
if (auto err = spi.init(); err) {
    std::cout << "Cannot init SPI: " << *err << "\n";
    return;
}
```

* `*err` dereferences the `std::optional` to get the stored string.

### ✅ Why use this syntax

1. **Keeps the error variable local** — no need for a separate line before the `if`.
2. **Makes code cleaner** — initialization and condition in one line.
3. Works with **any type**, not just `std::optional`.

### Equivalent older C++ (pre-C++17):

```cpp
auto err = spi.init();
if (err) {
    std::cout << "Cannot init SPI for performance test\n";
    return;
}
```

Works the same, but `err` exists **after the `if`** as well, which may clutter the scope.
This pattern is common in modern C++ for **error handling** and **resource management**.

