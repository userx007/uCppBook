# Module Systems & Namespaces: C++ vs. Rust

## Overview

Both C++ and Rust provide mechanisms for organizing code into logical units, but they take fundamentally different approaches. C++ evolved from a header-based system to introduce modules in C++20, while Rust was designed from the ground up with a modern module system based on crates and modules.

## C++ Module System

### Traditional Header-Based Approach

C++ traditionally uses a separation between header files (.h/.hpp) and implementation files (.cpp):

**math_utils.h**
```cpp
#ifndef MATH_UTILS_H
#define MATH_UTILS_H

namespace MathUtils {
    int add(int a, int b);
    int multiply(int a, int b);
    
    namespace Advanced {
        double sqrt_approx(double x);
    }
}

#endif
```

**math_utils.cpp**
```cpp
#include "math_utils.h"

namespace MathUtils {
    int add(int a, int b) {
        return a + b;
    }
    
    int multiply(int a, int b) {
        return a * b;
    }
    
    namespace Advanced {
        double sqrt_approx(double x) {
            return x / 2.0; // Simplified
        }
    }
}
```

**main.cpp**
```cpp
#include "math_utils.h"
#include <iostream>

using namespace MathUtils;

int main() {
    std::cout << add(5, 3) << std::endl;
    std::cout << Advanced::sqrt_approx(16.0) << std::endl;
    return 0;
}
```

### C++20 Modules (Modern Approach)

**math_utils.cppm** (module interface)
```cpp
export module math_utils;

export namespace MathUtils {
    int add(int a, int b) {
        return a + b;
    }
    
    int multiply(int a, int b) {
        return a * b;
    }
    
    namespace Advanced {
        double sqrt_approx(double x) {
            return x / 2.0;
        }
    }
}

// Private implementation (not exported)
namespace Internal {
    void helper_function() {
        // Only visible within this module
    }
}
```

**main.cpp**
```cpp
import math_utils;
import std;

int main() {
    std::cout << MathUtils::add(5, 3) << std::endl;
    std::cout << MathUtils::Advanced::sqrt_approx(16.0) << std::endl;
    return 0;
}
```

### Namespace Features in C++

```cpp
// Nested namespaces (C++17)
namespace Company::Project::Utils {
    void do_something() {}
}

// Anonymous namespaces (internal linkage)
namespace {
    int internal_variable = 42; // Only visible in this translation unit
}

// Namespace aliases
namespace CP = Company::Project;

// Using declarations
using std::cout;
using std::endl;

// Using directives (imports entire namespace)
using namespace std;

// Inline namespaces (for versioning)
namespace MyLib {
    inline namespace v2 {
        void new_api() {}
    }
    namespace v1 {
        void old_api() {}
    }
}
// MyLib::new_api() works directly
```

## Rust Module System

### Crate Structure

Rust organizes code into crates (compilation units) and modules. A crate can be a binary or library.

**Cargo.toml**
```toml
[package]
name = "math_utils"
version = "0.1.0"

[dependencies]
```

**src/lib.rs** (library crate root)
```rust
// Public module
pub mod math {
    // Public function
    pub fn add(a: i32, b: i32) -> i32 {
        a + b
    }
    
    pub fn multiply(a: i32, b: i32) -> i32 {
        a * b
    }
    
    // Public nested module
    pub mod advanced {
        pub fn sqrt_approx(x: f64) -> f64 {
            x / 2.0
        }
        
        // Private function (not exported)
        fn internal_helper() {
            // Only visible within this module
        }
    }
    
    // Private function
    fn private_function() {
        // Only accessible within the math module
    }
}

// Private module (not exported from crate)
mod internal {
    pub fn helper() {
        // 'pub' makes it visible to parent, but module itself is private
    }
}
```

### File-Based Module Organization

**src/lib.rs**
```rust
// Declare modules from separate files
pub mod math;
pub mod geometry;

// Re-export specific items
pub use math::add;
pub use geometry::Point;
```

**src/math.rs**
```rust
pub fn add(a: i32, b: i32) -> i32 {
    a + b
}

pub fn subtract(a: i32, b: i32) -> i32 {
    a - b
}

pub mod advanced {
    pub fn power(base: f64, exp: f64) -> f64 {
        base.powf(exp)
    }
}
```

**src/geometry/mod.rs** (directory module)
```rust
// Items in geometry module
pub struct Point {
    pub x: f64,
    pub y: f64,
}

// Submodules
pub mod shapes;
pub mod transforms;
```

**src/geometry/shapes.rs**
```rust
use super::Point; // Import from parent module

pub struct Circle {
    pub center: Point,
    pub radius: f64,
}

impl Circle {
    pub fn new(center: Point, radius: f64) -> Self {
        Circle { center, radius }
    }
}
```

### Using the Rust Library

**src/main.rs** (binary crate)
```rust
// Import from external crate
use math_utils::math;
use math_utils::math::advanced;
use math_utils::{add, Point}; // Re-exported items

// Import with aliasing
use math_utils::geometry::shapes::Circle as Circ;

// Glob imports (use sparingly)
use math_utils::math::*;

fn main() {
    println!("{}", math::add(5, 3));
    println!("{}", advanced::sqrt_approx(16.0));
    
    // Using re-exported items
    println!("{}", add(10, 20));
    
    let point = Point { x: 1.0, y: 2.0 };
    let circle = Circ::new(point, 5.0);
}
```

### Visibility Modifiers in Rust

```rust
pub mod my_module {
    // Public - accessible from anywhere
    pub fn public_function() {}
    
    // Private - only within this module
    fn private_function() {}
    
    // Visible within crate only
    pub(crate) fn crate_visible() {}
    
    // Visible within parent module
    pub(super) fn parent_visible() {}
    
    // Visible within specific path
    pub(in crate::my_module) fn path_visible() {}
    
    pub mod nested {
        // Can access parent_visible and path_visible
        pub fn use_parent() {
            super::parent_visible();
        }
    }
}
```

## Compilation Model Differences

### C++ Compilation Model

The traditional C++ compilation model processes each translation unit independently:

1. **Preprocessing**: Headers are textually included via `#include`, potentially multiple times
2. **Compilation**: Each .cpp file is compiled to an object file separately
3. **Linking**: Object files are linked together to form the executable

**Problems with traditional approach:**
- Header files included multiple times (solved with include guards)
- Long compilation times due to repeated parsing
- Order-dependent includes
- No true encapsulation (macros leak across boundaries)

**C++20 Modules improve this:**
- Compiled once and cached
- No need for include guards
- Faster compilation
- Better encapsulation
- Order-independent imports

### Rust Compilation Model

Rust uses a more integrated compilation model:

1. **Crate-based**: Entire crate is the compilation unit
2. **Incremental**: Rust compiler tracks dependencies at a fine-grained level
3. **Module tree**: All modules form a single tree from the crate root
4. **No header files**: Interface and implementation together

**Advantages:**
- No header/implementation split
- Strong encapsulation by default
- Incremental recompilation based on actual changes
- No include guards needed
- Built-in dependency management via Cargo

### Compilation Example Comparison

**C++ Traditional Build:**
```bash
# Each file compiled separately
g++ -c math_utils.cpp -o math_utils.o
g++ -c geometry.cpp -o geometry.o
g++ -c main.cpp -o main.o
# Link everything
g++ math_utils.o geometry.o main.o -o program
```

**C++ Modules Build:**
```bash
# Compile module interfaces first
g++ -std=c++20 -fmodules-ts -xc++-module math_utils.cppm -c
g++ -std=c++20 -fmodules-ts main.cpp math_utils.o -o program
```

**Rust Build:**
```bash
# Cargo handles entire dependency tree
cargo build
# Or for release optimization
cargo build --release
```

## Summary Table

| Feature | C++ | Rust |
|---------|-----|------|
| **Organization Unit** | Namespaces, files, modules (C++20) | Crates and modules |
| **File Structure** | Header (.h) + Implementation (.cpp) | Single file per module or mod.rs |
| **Compilation Unit** | Translation unit (single .cpp + includes) | Entire crate |
| **Visibility Default** | Public (within translation unit) | Private |
| **Visibility Control** | `public:`, `private:`, `protected:`, anonymous namespace | `pub`, `pub(crate)`, `pub(super)`, private (default) |
| **Code Reuse** | `#include`, `import` (C++20), `using` | `use`, `pub use` (re-exports) |
| **Encapsulation** | Limited (macros leak, header exposure) | Strong (explicit pub required) |
| **Include Guards** | Required for headers (`#ifndef`) | Not needed |
| **Nested Organization** | Nested namespaces | Nested modules with hierarchy |
| **Aliasing** | Namespace aliases, using declarations | `use` with `as` keyword |
| **Glob Imports** | `using namespace` | `use module::*` (discouraged) |
| **Build System** | External (Make, CMake, etc.) | Integrated (Cargo) |
| **Dependency Management** | External (pkg-config, vcpkg, etc.) | Built-in (Cargo.toml) |
| **Compilation Speed** | Slow (traditional), Better (modules) | Fast incremental compilation |
| **Recompilation** | Often recompiles unnecessarily | Fine-grained dependency tracking |
| **Symbol Visibility** | Complex linkage rules (internal/external) | Explicit pub keywords |
| **Circular Dependencies** | Possible (with forward declarations) | Prevented at module level |
| **Module Interface** | Header files or module interfaces | Implicit from pub items |
| **Versioning** | Manual or inline namespaces | Semantic versioning via Cargo |

## Key Takeaways

**C++ Philosophy**: Maximum flexibility with namespaces and traditional headers, evolving toward modules for better compilation performance. The system offers great power but requires careful management.

**Rust Philosophy**: Safety and clarity through explicit visibility, integrated tooling, and a modern module system designed from scratch. The crate-module system enforces good practices and makes dependencies explicit.

**When choosing**: C++ modules are still being adopted (limited compiler support), while Rust's module system is mature and integral to the language. Rust's compilation model is generally faster for incremental builds, while C++'s new modules aim to catch up.