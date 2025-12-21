Below is a **clean, practical, no-nonsense C++ cheat sheet** on when to use **`T*`**, **`T&`**, **`unique_ptr<T>`**, **`shared_ptr<T>`**, and **`weak_ptr<T>`**.

---

# 🧠 **C++ Pointer/Reference Cheat Sheet**

## ✅ **1. Use `T&` (references) when…**

### ✔ You want:

* A **guaranteed non-null**, **non-owning**, **non-reseatable** alias
* Clean, value-like syntax

### ✔ Best for:

* Function parameters
* Return values when you want to return an alias
* Overloaded operators (e.g., `operator<<`)
* Internal class members that must always refer to something valid

### ❌ Avoid `T&` when:

* You need optionality (use `T*`)
* You need reseating (use `T*`)
* You need long-term storage (use `T*` or `reference_wrapper`)

---

## ✅ **2. Use `T*` (raw pointers) when…**

### ✔ You need:

* **Optionality** (can be `nullptr`)
* **Reseatable alias**
* **Non-owning semantics**
* Low-level pointer arithmetic

### ✔ Best for:

* Function parameters that are **optional**
* Data structures that contain references to many objects (graphs, trees, ECS)
* Returning pointers from containers or APIs
* Interfacing with C APIs

### ❌ Don’t use raw pointers for ownership

Ownership should be expressed explicitly → smart pointers.

---

# 🔐 **Smart Pointer Cheat Sheet**

## 🔵 **3. Use `std::unique_ptr<T>` when…**

### ✔ You want:

* **Exclusive ownership**
* **Automatic deletion**
* **Clear ownership semantics**
* Cheap move, non-copyable

### ✔ Best for:

* RAII-managed resources
* Private class members that own resources
* Factory functions
* Polymorphic object ownership

### ❌ Don’t use `unique_ptr` when you need shared ownership

Use `shared_ptr` for that.

---

## 🟢 **4. Use `std::shared_ptr<T>` when…**

### ✔ You need:

* **Shared / reference-counted ownership**
* Objects with **multiple owners**
* Lifetime that depends on dynamic usage patterns

### ✔ Best for:

* Nodes in shared graphs
* UI widgets shared among systems
* Asynchronous callbacks

### ⚠️ Downsides:

* Reference counting overhead
* Possible cycles (use `weak_ptr` to break them)

---

## 🟡 **5. Use `std::weak_ptr<T>` when…**

### ✔ You want:

* A **non-owning** reference to a `shared_ptr` object
* Avoid ownership cycles
* Check validity dynamically (`lock()`)

### ✔ Best for:

* Parent pointers in trees
* Cache entries
* Observer lists
* Breaking `shared_ptr` cycles

---

# 🧩 Summary Table

| Situation                             | Use                                 |
| ------------------------------------- | ----------------------------------- |
| Non-null alias, no ownership          | `T&`                                |
| Nullable, reseatable, no ownership    | `T*`                                |
| Exclusive ownership                   | `unique_ptr<T>`                     |
| Shared ownership                      | `shared_ptr<T>`                     |
| Non-owning reference to shared object | `weak_ptr<T>`                       |
| Need container of references          | `std::reference_wrapper<T>` or `T*` |

---

# 🧪 Quick "What to use?" Rules

### Function parameters

* Needs alias, must not be null → **`T&`**
* Optional → **`T*`** or **`T const*`**
* Owning input → **`std::unique_ptr<T>`**

### Function return

* Alias to existing object → **`T&`**
* Might return nothing → **`T*`**
* Returning owned resource → **`std::unique_ptr<T>`**
* Returning shared resource → **`std::shared_ptr<T>`**

### Class members

* Owns → **`std::unique_ptr<T>`**
* Shared ownership → **`std::shared_ptr<T>`**
* Non-owning → **`T*`**
* Always valid alias → **`T&`**

### Containers

* Owning → store **`unique_ptr`**
* Shared relationships → **`shared_ptr`**
* Non-owning → store **`T*`** or `reference_wrapper`

---

If you want, I can turn this into a **one-page PDF**, a **printable diagram**, or a **flowchart-style decision tree**.
