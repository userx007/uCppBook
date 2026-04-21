## `transaction()` — Template Wrapper Function

This is a **generic RAII-style transaction wrapper** for SPI/I2C bus communication (or any protocol that requires chip-select toggling).

### What it does, step by step

```cpp
template<typename Func>
auto transaction(Func&& func) -> decltype(func())
```
- Accepts **any callable** (lambda, function pointer, functor) via a forwarding reference (`&&`)
- The return type is deduced as **whatever `func()` returns** using a trailing `decltype` — so if you pass a lambda returning `uint8_t`, the transaction returns `uint8_t`

```cpp
assert_cs();        // Pull Chip Select LOW  → select the device
auto result = func(); // Execute your read/write operation(s)
wait_not_busy();    // Block until the peripheral signals it's done
deassert_cs();      // Pull Chip Select HIGH → release the device
return result;      // Pass the return value back to the caller
```

### Why this pattern is useful

It **enforces correct CS framing** — you can't forget to deassert, and you can't accidentally interleave operations from two devices. Usage looks like:

```cpp
// Returns whatever your lambda returns
uint8_t val = transaction([&]() {
    spi_write(READ_CMD);
    return spi_read();
});
```

### A subtle gotcha — `void` return

If `func()` returns `void`, `auto result = func()` **won't compile** because you can't store `void`. This is a common issue with this pattern. The fix is a `void` specialization or `if constexpr`:

```cpp
template<typename Func>
auto transaction(Func&& func) -> decltype(func()) {
    assert_cs();
    if constexpr (std::is_void_v<decltype(func())>) {
        func();
        wait_not_busy();
        deassert_cs();
    } else {
        auto result = func();
        wait_not_busy();
        deassert_cs();
        return result;
    }
}
```

So the original code implicitly assumes `func` always returns a non-`void` value.