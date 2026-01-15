# Strings & Text Processing: C++ vs. Rust

## Overview

String handling is a critical aspect of modern programming, and both C++ and Rust provide powerful tools for text processing. However, they differ significantly in their approaches to safety, ownership, and encoding guarantees. This comprehensive guide explores these differences with practical examples.

## C++ String Types

### 1. `std::string` - Owned String

`std::string` is a dynamically allocated, mutable string container that owns its data.

```cpp
#include <iostream>
#include <string>

void demonstrateStdString() {
    // Construction
    std::string s1 = "Hello, World!";
    std::string s2(5, 'A');  // "AAAAA"
    
    // Modification
    s1 += " How are you?";
    s1.append(" Fine!");
    
    // Access
    char first = s1[0];  // No bounds checking - undefined behavior if out of bounds
    char safe = s1.at(0);  // Bounds checking - throws exception
    
    // Iteration
    for (char c : s1) {
        std::cout << c;
    }
    
    // Substring
    std::string sub = s1.substr(0, 5);  // "Hello"
    
    // Find operations
    size_t pos = s1.find("World");
    if (pos != std::string::npos) {
        std::cout << "Found at position: " << pos << std::endl;
    }
}
```

### 2. `std::string_view` - Non-Owning String View (C++17)

`std::string_view` is a lightweight, non-owning reference to a string sequence.

```cpp
#include <string_view>
#include <iostream>

void processText(std::string_view sv) {
    // No copying occurs - just a pointer and length
    std::cout << "Processing: " << sv << std::endl;
}

void demonstrateStringView() {
    std::string owned = "Hello, World!";
    const char* c_str = "C-style string";
    
    // Can be constructed from various sources
    std::string_view sv1 = owned;
    std::string_view sv2 = c_str;
    std::string_view sv3 = owned.substr(0, 5);  // Returns string, then converts
    
    // Efficient substring operations (no allocation)
    std::string_view sub = sv1.substr(0, 5);
    
    // DANGER: Dangling reference
    std::string_view dangling;
    {
        std::string temp = "Temporary";
        dangling = temp;  // temp's data will be destroyed!
    }
    // Using 'dangling' here is undefined behavior
}

// Efficient function that doesn't copy
size_t countSpaces(std::string_view text) {
    size_t count = 0;
    for (char c : text) {
        if (c == ' ') count++;
    }
    return count;
}
```

### 3. UTF-8 in C++

C++ doesn't guarantee UTF-8 encoding, and handling it requires care:

```cpp
#include <string>
#include <iostream>
#include <locale>
#include <codecvt>

void demonstrateUTF8Issues() {
    // UTF-8 string literal (C++11)
    std::string utf8_str = u8"Hello, 世界! 🌍";
    
    // PROBLEM: Length is in bytes, not characters
    std::cout << "Byte length: " << utf8_str.length() << std::endl;
    // Output: 19 (not the number of characters)
    
    // PROBLEM: Indexing gives bytes, not characters
    char byte = utf8_str[8];  // Gets a byte from "世"
    
    // Iterating over bytes (not characters)
    for (char c : utf8_str) {
        // 'c' is a byte, multi-byte characters will be split
        std::cout << c;
    }
}

// Manual UTF-8 character counting
size_t utf8_length(const std::string& str) {
    size_t len = 0;
    for (size_t i = 0; i < str.length(); ) {
        unsigned char c = str[i];
        if (c < 0x80) {
            i += 1;
        } else if (c < 0xE0) {
            i += 2;
        } else if (c < 0xF0) {
            i += 3;
        } else {
            i += 4;
        }
        len++;
    }
    return len;
}

void utf8Processing() {
    std::string text = u8"Hello, 世界!";
    std::cout << "Byte length: " << text.length() << std::endl;
    std::cout << "Character length: " << utf8_length(text) << std::endl;
    
    // C++ allows invalid UTF-8
    std::string invalid = "Hello\xFF\xFE";  // Invalid UTF-8 bytes
    // No error, but will cause issues if treated as UTF-8
}
```

## Rust String Types

### 1. `String` - Owned, Growable String

`String` is a heap-allocated, growable, UTF-8 encoded string.

```rust
fn demonstrate_string() {
    // Construction
    let mut s1 = String::from("Hello, World!");
    let s2 = "Hello".to_string();
    let s3 = String::new();
    
    // Modification
    s1.push_str(" How are you?");
    s1.push('!');
    
    // Access - always safe
    let first = s1.chars().next();  // Returns Option<char>
    
    // CANNOT index directly: s1[0] - compile error!
    // This prevents byte-vs-character confusion
    
    // Iteration over characters (not bytes)
    for c in s1.chars() {
        println!("{}", c);
    }
    
    // Iteration over bytes if needed
    for b in s1.bytes() {
        println!("{}", b);
    }
    
    // Substring operations
    let hello = &s1[0..5];  // Byte slice - panics if not on char boundary
    
    // Safe substring
    let safe_sub = s1.get(0..5);  // Returns Option<&str>
}
```

### 2. `&str` - String Slice (Borrowed String)

`&str` is an immutable reference to a UTF-8 string slice.

```rust
fn process_text(s: &str) {
    // No ownership, just borrows the data
    println!("Processing: {}", s);
}

fn demonstrate_str_slice() {
    let owned = String::from("Hello, World!");
    let literal = "String literal";  // &str type
    
    // String slices from owned strings
    let slice1: &str = &owned;
    let slice2: &str = &owned[0..5];
    
    // Efficient - no allocation
    process_text(&owned);      // String -> &str (deref coercion)
    process_text(literal);     // &str -> &str
    process_text("inline");    // &str
    
    // Lifetime safety prevents dangling references
    let slice: &str;
    {
        let temp = String::from("Temporary");
        slice = &temp;  // Compile error! temp doesn't live long enough
    }
    // Cannot use 'slice' here - compiler prevents it
}

// Efficient function signature
fn count_spaces(text: &str) -> usize {
    text.chars().filter(|c| *c == ' ').count()
}
```

### 3. UTF-8 Guarantees in Rust

Rust guarantees all strings are valid UTF-8:

```rust
fn demonstrate_utf8_safety() {
    // UTF-8 guaranteed
    let text = "Hello, 世界! 🌍";
    
    // Character counting is accurate
    let char_count = text.chars().count();
    println!("Characters: {}", char_count);  // Correct count
    
    // Byte length
    let byte_count = text.len();
    println!("Bytes: {}", byte_count);
    
    // Safe iteration over characters
    for (i, c) in text.chars().enumerate() {
        println!("Character {}: {}", i, c);
    }
    
    // Cannot create invalid UTF-8
    // This won't compile:
    // let invalid = String::from_utf8(vec![0xFF, 0xFE]).unwrap();
    // Will panic at runtime if bytes are invalid
    
    // Safe way to handle potentially invalid UTF-8
    let bytes = vec![0xFF, 0xFE];
    match String::from_utf8(bytes) {
        Ok(s) => println!("Valid: {}", s),
        Err(e) => println!("Invalid UTF-8: {:?}", e),
    }
}

// String slicing safety
fn safe_slicing() {
    let text = "Hello, 世界!";
    
    // This panics - not on character boundary:
    // let bad = &text[0..8];  // Panics at runtime
    
    // Safe slicing
    if let Some(sub) = text.get(0..5) {
        println!("Safe: {}", sub);
    }
    
    // Or use char_indices
    for (i, _) in text.char_indices() {
        println!("Valid boundary at: {}", i);
    }
}
```

## Ownership and Safety Comparison

### C++ Example: Potential Issues

```cpp
#include <string>
#include <string_view>
#include <iostream>

// Dangerous: Returns view to temporary
std::string_view getGreeting() {
    std::string temp = "Hello";
    return temp;  // Dangling pointer! Compiles but dangerous
}

void ownershipIssues() {
    // Use after free (undefined behavior)
    std::string_view sv = getGreeting();
    std::cout << sv << std::endl;  // Undefined behavior
    
    // Buffer overflow possible
    std::string str = "Hello";
    char& c = str[100];  // No bounds check, undefined behavior
    
    // Invalid UTF-8 allowed
    std::string invalid;
    invalid.push_back(0xFF);  // Creates invalid UTF-8
    
    // Iterator invalidation
    std::string text = "Hello";
    for (char& c : text) {
        if (c == 'l') {
            text += "World";  // Undefined behavior! Invalidates iterator
        }
    }
}
```

### Rust Example: Compile-Time Safety

```rust
// Won't compile: Returns reference to temporary
// fn get_greeting() -> &str {
//     let temp = String::from("Hello");
//     &temp  // Compile error: borrowed value doesn't live long enough
// }

// Correct version
fn get_greeting() -> String {
    String::from("Hello")  // Returns owned data
}

fn ownership_safety() {
    // Bounds checking
    let s = String::from("Hello");
    // let c = s[100];  // Compile error: cannot index String
    let c = s.chars().nth(100);  // Returns None safely
    
    // Cannot create invalid UTF-8 without using unsafe
    // let invalid = String::from_utf8_unchecked(vec![0xFF]);  // Requires unsafe
    
    // Safe UTF-8 construction
    let bytes = vec![0xFF];
    match String::from_utf8(bytes) {
        Ok(_) => println!("Valid"),
        Err(e) => println!("Invalid: {:?}", e),
    }
    
    // Iterator safety
    let mut text = String::from("Hello");
    // This won't compile:
    // for c in text.chars() {
    //     if c == 'l' {
    //         text.push_str("World");  // Compile error: cannot borrow as mutable
    //     }
    // }
    
    // Correct approach
    let positions: Vec<_> = text
        .char_indices()
        .filter(|(_, c)| *c == 'l')
        .map(|(i, _)| i)
        .collect();
    // Now safe to modify
}
```

## Performance Comparison

### C++ String Operations

```cpp
#include <string>
#include <string_view>
#include <chrono>

void performanceComparison() {
    std::string large_text(1000000, 'a');
    
    // Copying strings (expensive)
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        std::string copy = large_text;  // Full copy
    }
    auto end = std::chrono::high_resolution_clock::now();
    
    // Using string_view (cheap)
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < 1000; i++) {
        std::string_view view = large_text;  // Just pointer + length
    }
    end = std::chrono::high_resolution_clock::now();
    
    // Substring with string_view (no allocation)
    std::string_view sub = std::string_view(large_text).substr(0, 100);
    
    // Substring with string (allocation)
    std::string sub_copy = large_text.substr(0, 100);
}
```

### Rust String Operations

```rust
use std::time::Instant;

fn performance_comparison() {
    let large_text = "a".repeat(1_000_000);
    
    // Cloning strings (expensive)
    let start = Instant::now();
    for _ in 0..1000 {
        let _copy = large_text.clone();  // Full copy
    }
    let duration = start.elapsed();
    
    // Using &str (cheap)
    let start = Instant::now();
    for _ in 0..1000 {
        let _view: &str = &large_text;  // Just reference
    }
    let duration = start.elapsed();
    
    // Substring with &str (no allocation)
    let sub: &str = &large_text[0..100];
    
    // Substring with String (allocation)
    let sub_owned: String = large_text[0..100].to_string();
}
```

## Advanced Text Processing Examples

### C++ Text Processing

```cpp
#include <string>
#include <vector>
#include <sstream>
#include <algorithm>

// Split string
std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream ss(str);
    std::string token;
    while (std::getline(ss, token, delimiter)) {
        tokens.push_back(token);
    }
    return tokens;
}

// Trim whitespace
std::string trim(const std::string& str) {
    auto start = std::find_if_not(str.begin(), str.end(),
                                   [](unsigned char c) { return std::isspace(c); });
    auto end = std::find_if_not(str.rbegin(), str.rend(),
                                 [](unsigned char c) { return std::isspace(c); }).base();
    return (start < end) ? std::string(start, end) : std::string();
}

// Replace all occurrences
std::string replace_all(std::string str, const std::string& from,
                        const std::string& to) {
    size_t pos = 0;
    while ((pos = str.find(from, pos)) != std::string::npos) {
        str.replace(pos, from.length(), to);
        pos += to.length();
    }
    return str;
}

void textProcessing() {
    std::string text = "  Hello, World! How are you?  ";
    
    // Trim
    std::string trimmed = trim(text);
    
    // Split
    std::vector<std::string> words = split(trimmed, ' ');
    
    // Replace
    std::string replaced = replace_all(text, "Hello", "Hi");
    
    // Case conversion (requires algorithm)
    std::string upper = text;
    std::transform(upper.begin(), upper.end(), upper.begin(), ::toupper);
}
```

### Rust Text Processing

```rust
fn text_processing() {
    let text = "  Hello, World! How are you?  ";
    
    // Trim (built-in)
    let trimmed = text.trim();
    
    // Split (returns iterator, lazy)
    let words: Vec<&str> = text.split_whitespace().collect();
    
    // Replace (built-in)
    let replaced = text.replace("Hello", "Hi");
    
    // Case conversion (built-in, Unicode-aware)
    let upper = text.to_uppercase();
    let lower = text.to_lowercase();
    
    // More complex splitting
    let parts: Vec<&str> = text.split(|c: char| c.is_whitespace() || c == ',')
        .filter(|s| !s.is_empty())
        .collect();
    
    // Pattern matching
    if text.contains("World") {
        println!("Found 'World'");
    }
    
    // Unicode-aware operations
    let emoji = "Hello 👋 World 🌍";
    println!("Chars: {}", emoji.chars().count());  // Correct count
    println!("Bytes: {}", emoji.len());
}

// Efficient string building
fn build_string() {
    let mut result = String::new();
    for i in 0..1000 {
        result.push_str(&format!("Item {}, ", i));
    }
    
    // Or use collect
    let result: String = (0..1000)
        .map(|i| format!("Item {}, ", i))
        .collect();
}
```

## Summary Table

| Feature | C++ | Rust |
|---------|-----|------|
| **Owned String Type** | `std::string` | `String` |
| **Borrowed/View Type** | `std::string_view` (C++17) | `&str` |
| **UTF-8 Guarantee** | ❌ No (depends on encoding) | ✅ Yes (always valid UTF-8) |
| **Memory Safety** | ⚠️ Manual (UB possible) | ✅ Compile-time guaranteed |
| **Bounds Checking** | Optional (`at()` throws, `[]` doesn't) | ✅ Always (compile error for direct indexing) |
| **Dangling References** | ⚠️ Possible (UB at runtime) | ✅ Prevented at compile time |
| **Character Iteration** | Byte iteration by default | UTF-8 character iteration by default |
| **Invalid UTF-8** | ✅ Allowed (no validation) | ❌ Prevented (validation enforced) |
| **String Indexing** | `str[i]` returns byte/char | ❌ Compile error (use `.chars().nth(i)`) |
| **Length Method** | Returns byte count | Returns byte count (use `.chars().count()` for chars) |
| **Substring Safety** | ⚠️ No validation | ✅ Panics if not on char boundary |
| **Ownership Model** | Manual management | Borrow checker enforced |
| **Performance (Views)** | Excellent with `string_view` | Excellent with `&str` |
| **Mutable Iteration** | ✅ Allowed (can cause UB) | ✅ Allowed (safe, exclusive access) |
| **Zero-Copy Operations** | ✅ With `string_view` | ✅ With `&str` slicing |
| **Standard Library Support** | Extensive | Extensive with iterator focus |
| **Encoding Flexibility** | ✅ Any encoding possible | ❌ UTF-8 only |
| **Learning Curve** | Moderate | Steeper (ownership concepts) |

## Key Takeaways

**C++ Strengths:**
- Flexible encoding support
- Performance control with `string_view`
- Backward compatibility
- Direct byte manipulation when needed

**C++ Weaknesses:**
- No UTF-8 guarantees
- Easy to create dangling references
- Undefined behavior with out-of-bounds access
- Manual lifetime management

**Rust Strengths:**
- UTF-8 guaranteed at compile time
- Memory safety guaranteed by borrow checker
- No dangling references possible
- Clear owned vs borrowed distinction
- Unicode-correct operations by default

**Rust Weaknesses:**
- Steeper learning curve for ownership
- Cannot directly index strings (must use methods)
- UTF-8 only (no arbitrary encodings)
- Stricter compiler requirements

Both languages offer powerful string handling, but Rust provides stronger safety guarantees at compile time, while C++ offers more flexibility at the cost of potential runtime errors.