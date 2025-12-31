Perfect — here is the complete table you asked for:
**What can bind to what? (`T&`, `const T&`, `T&&`, `const T&&`)**

This is the cleanest and most correct reference binding table used in modern C++.

---

# 🔥 **Reference Binding Rules (Full Table)**

Let `x` be an **lvalue** of type `T`,
and `5` be an **rvalue** (temporary).

| Type of reference | Binds to **lvalue**? | Binds to **const lvalue**? | Binds to **rvalue**? | Notes                                   |
| ----------------- | -------------------- | -------------------------- | -------------------- | --------------------------------------- |
| **T&**            | ✔️ Yes               | ❌ No                       | ❌ No                 | Non-const lvalue ref — most restrictive |
| **const T&**      | ✔️ Yes               | ✔️ Yes                     | ✔️ Yes               | Most flexible — binds to everything     |
| **T&&**           | ❌ No                 | ❌ No                       | ✔️ Yes               | Only rvalue refs bind to rvalues        |
| **const T&&**     | ❌ No                 | ❌ No                       | ✔️ Yes               | Rarely useful                           |

---

# 📌 **Examples**

## 1. **Non-const lvalue reference (`T&`)**

```cpp
int x = 5;
int& a = x;     // OK
int& b = 5;     // ❌ ERROR, 5 is rvalue
```

---

## 2. **Const lvalue reference (`const T&`)**

```cpp
int x = 5;
const int& a = x;     // OK
const int& b = 5;     // OK
const int& c = x + 1; // OK
```

This is why `const T&` is used everywhere in C++.

---

## 3. **Rvalue reference (`T&&`)**

```cpp
int&& a = 5;   // OK
int&& b = x;   // ❌ ERROR, x is an lvalue
```

---

## 4. **Const rvalue reference (`const T&&`)**

```cpp
const int&& a = 5;  // OK, but rarely useful
```

Not commonly used except in weird template cases.

---

# ✔️ **Bonus: What about assignment expressions?**

Because this matters for your earlier questions:

```cpp
(x = 10)     // lvalue (int&)
```

So all of these are valid:

```cpp
int x = 0;

int& a = (x = 10);        // OK
const int& b = (x = 10);  // OK
int&& c = (x = 10);       // ❌ ERROR — lvalue cannot bind to T&&
```

---

# 📌 **Ultimate Summary**

* **lvalues** bind to: `T&` and `const T&`
* **rvalues** bind to: `T&&` and `const T&`
* **const references** are the most flexible
* **assignment and compound assignment return lvalues**, so they behave like variables

---

If you want, I can also give you the **xvalues vs glvalues vs prvalues** table (modern C++ value category classification), which explains all of this rigorously.
