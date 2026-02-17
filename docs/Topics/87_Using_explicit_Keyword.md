## The `explicit` Keyword — Why It Matters

Without `explicit`, C++ will silently use single-parameter constructors to perform **implicit conversions** on your behalf. That sounds helpful but creates sneaky bugs.

---

## The Problem — Implicit Conversion

```cpp
class Buffer {
public:
    Buffer(int size) {   // NO explicit
        data = new char[size];
    }
};

void process(Buffer b) { /* ... */ }

int main() {
    process(42);        // ← compiles! C++ silently converts int→Buffer
    process(999);       // ← also compiles! Is this intentional? Almost never.

    Buffer b = 100;     // ← copy-init also silently works
}
```

The compiler sees `process(42)` needs a `Buffer`, notices it can construct one from an `int`, and does so **without asking you**. No warning, no error.

---

## The Fix — Add `explicit`

```cpp
class Buffer {
public:
    explicit Buffer(int size) {   // ← now conversion must be deliberate
        data = new char[size];
    }
};

void process(Buffer b) { /* ... */ }

int main() {
    process(42);            // ✗ compile error — no implicit conversion
    process(Buffer(42));    // ✔ explicit, intention is clear
    
    Buffer b = 100;         // ✗ compile error
    Buffer b(100);          // ✔ direct initialization, fine
    Buffer b{100};          // ✔ brace initialization, fine
}
```

---

## A More Realistic Bug `explicit` Prevents

```cpp
class MyString {
public:
    MyString(int n);        // allocates string of n chars — NO explicit
    MyString(const char*);  // normal string constructor
};

bool operator==(MyString a, MyString b);

int main() {
    MyString s = "hello";
    
    if (s == 'x') {         // ← BUG: 'x' is char→int (120)→MyString(120)
        // compares s against a 120-char buffer, not the character 'x'
        // compiles silently, wrong result at runtime
    }
}
```

With `explicit MyString(int n)` that line becomes a **compile error** instead of a silent runtime bug.

---

## Multi-Param Constructors — Does It Matter?

For constructors with **2+ parameters**, `explicit` is less critical because C++ won't implicitly call them anyway. But it still guards against **brace-initialization** surprises:

```cpp
struct Point {
    explicit Point(int x, int y) {}
};

Point p = {1, 2};    // ✗ error with explicit
Point p{1, 2};       // ✗ error with explicit  
Point p(1, 2);       // ✔ always fine
```

So `explicit` on multi-param constructors is a stricter style choice — not always needed, but sometimes desired.

---

## The Rule of Thumb

```
┌────────────────────────────────────────────────────────────┐
│  Single-param constructor?  → explicit almost always       │
│                                                            │
│  Exception: when implicit conversion IS the point,         │
│  e.g. a wrapper type meant to be interchangeable:          │
│                                                            │
│    class Celsius {                                         │
│    public:                                                 │
│        Celsius(double val) : v(val) {}  // implicit ok     │
│    };                                                      │
│                                                            │
│  Multi-param constructor?   → explicit is optional,        │
│                               use it for extra strictness  │
└────────────────────────────────────────────────────────────┘
```

The core idea: **implicit conversions should be obvious and intentional**, not something the compiler infers silently. `explicit` forces the caller to say what they mean.