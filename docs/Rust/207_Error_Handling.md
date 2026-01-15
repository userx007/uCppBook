# Error Handling: C++ vs Rust

## Overview

Error handling is one of the most significant philosophical differences between C++ and Rust. C++ offers multiple mechanisms that have evolved over decades, while Rust was designed from the ground up with a unified, explicit approach to error handling that makes errors part of the type system.

## C++ Error Handling

C++ provides several error handling mechanisms, each with different use cases and trade-offs.

### 1. Exceptions

Exceptions are C++'s traditional mechanism for handling runtime errors. They allow errors to propagate up the call stack until caught.

```cpp
#include <iostream>
#include <stdexcept>
#include <fstream>
#include <string>

// Function that throws an exception
double divide(double a, double b) {
    if (b == 0.0) {
        throw std::invalid_argument("Division by zero");
    }
    return a / b;
}

// Reading a file with exception handling
std::string readFile(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file: " + filename);
    }
    
    std::string content;
    std::string line;
    while (std::getline(file, line)) {
        content += line + "\n";
    }
    return content;
}

int main() {
    try {
        double result = divide(10.0, 0.0);
        std::cout << "Result: " << result << std::endl;
    } catch (const std::invalid_argument& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    
    try {
        std::string content = readFile("nonexistent.txt");
        std::cout << content << std::endl;
    } catch (const std::runtime_error& e) {
        std::cerr << "File error: " << e.what() << std::endl;
    }
    
    return 0;
}
```

### 2. Error Codes

Error codes are a traditional C-style approach, still widely used in modern C++ for performance-critical code or when exceptions are disabled.

```cpp
#include <iostream>
#include <cmath>

enum class ErrorCode {
    SUCCESS = 0,
    INVALID_INPUT,
    DIVISION_BY_ZERO,
    OVERFLOW
};

// Function returning error code
ErrorCode safeDivide(double a, double b, double& result) {
    if (b == 0.0) {
        return ErrorCode::DIVISION_BY_ZERO;
    }
    result = a / b;
    return ErrorCode::SUCCESS;
}

// Function with multiple error conditions
ErrorCode safeSqrt(double value, double& result) {
    if (value < 0) {
        return ErrorCode::INVALID_INPUT;
    }
    result = std::sqrt(value);
    return ErrorCode::SUCCESS;
}

int main() {
    double result;
    
    ErrorCode err = safeDivide(10.0, 0.0, result);
    if (err != ErrorCode::SUCCESS) {
        std::cerr << "Division failed with error code: " 
                  << static_cast<int>(err) << std::endl;
    } else {
        std::cout << "Result: " << result << std::endl;
    }
    
    err = safeSqrt(-4.0, result);
    if (err != ErrorCode::SUCCESS) {
        std::cerr << "Square root failed" << std::endl;
    }
    
    return 0;
}
```

### 3. std::optional (C++17)

`std::optional` represents a value that may or may not be present, useful for operations that might not produce a result.

```cpp
#include <iostream>
#include <optional>
#include <string>
#include <vector>

// Find element in vector
std::optional<int> findElement(const std::vector<int>& vec, int target) {
    for (size_t i = 0; i < vec.size(); ++i) {
        if (vec[i] == target) {
            return static_cast<int>(i);
        }
    }
    return std::nullopt;
}

// Parse integer from string
std::optional<int> parseInt(const std::string& str) {
    try {
        size_t pos;
        int value = std::stoi(str, &pos);
        if (pos == str.length()) {
            return value;
        }
        return std::nullopt;
    } catch (...) {
        return std::nullopt;
    }
}

int main() {
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    auto result = findElement(numbers, 3);
    if (result.has_value()) {
        std::cout << "Found at index: " << result.value() << std::endl;
    } else {
        std::cout << "Not found" << std::endl;
    }
    
    // Using value_or for default values
    auto index = findElement(numbers, 10).value_or(-1);
    std::cout << "Index: " << index << std::endl;
    
    // Parsing examples
    if (auto num = parseInt("42")) {
        std::cout << "Parsed: " << *num << std::endl;
    } else {
        std::cout << "Failed to parse" << std::endl;
    }
    
    return 0;
}
```

### 4. std::expected (C++23)

`std::expected` represents a value that can be either a successful result or an error, similar to Rust's `Result` type.

```cpp
#include <expected>
#include <string>
#include <iostream>

enum class ParseError {
    EMPTY_STRING,
    INVALID_FORMAT,
    OUT_OF_RANGE
};

// Function returning expected
std::expected<int, ParseError> parseInteger(const std::string& str) {
    if (str.empty()) {
        return std::unexpected(ParseError::EMPTY_STRING);
    }
    
    try {
        size_t pos;
        int value = std::stoi(str, &pos);
        if (pos != str.length()) {
            return std::unexpected(ParseError::INVALID_FORMAT);
        }
        return value;
    } catch (const std::out_of_range&) {
        return std::unexpected(ParseError::OUT_OF_RANGE);
    } catch (...) {
        return std::unexpected(ParseError::INVALID_FORMAT);
    }
}

std::expected<double, std::string> safeDivideExpected(double a, double b) {
    if (b == 0.0) {
        return std::unexpected("Division by zero");
    }
    return a / b;
}

int main() {
    auto result = parseInteger("42");
    if (result.has_value()) {
        std::cout << "Parsed: " << result.value() << std::endl;
    } else {
        std::cout << "Parse error occurred" << std::endl;
    }
    
    // Using and_then for chaining operations
    auto divResult = safeDivideExpected(10.0, 2.0);
    if (divResult) {
        std::cout << "Division result: " << *divResult << std::endl;
    } else {
        std::cout << "Error: " << divResult.error() << std::endl;
    }
    
    return 0;
}
```

## Rust Error Handling

Rust's error handling is built into the type system and enforces explicit handling of errors at compile time.

### 1. Result<T, E>

The `Result` type is Rust's primary mechanism for recoverable errors. It forces you to handle errors explicitly.

```rust
use std::fs::File;
use std::io::{self, Read};
use std::num::ParseIntError;

// Function returning Result
fn divide(a: f64, b: f64) -> Result<f64, String> {
    if b == 0.0 {
        Err("Division by zero".to_string())
    } else {
        Ok(a / b)
    }
}

// Reading a file with Result
fn read_file(filename: &str) -> Result<String, io::Error> {
    let mut file = File::open(filename)?;  // ? operator propagates errors
    let mut content = String::new();
    file.read_to_string(&mut content)?;
    Ok(content)
}

// Custom error type
#[derive(Debug)]
enum MathError {
    DivisionByZero,
    NegativeSquareRoot,
    Overflow,
}

fn safe_divide(a: f64, b: f64) -> Result<f64, MathError> {
    if b == 0.0 {
        Err(MathError::DivisionByZero)
    } else {
        Ok(a / b)
    }
}

fn safe_sqrt(value: f64) -> Result<f64, MathError> {
    if value < 0.0 {
        Err(MathError::NegativeSquareRoot)
    } else {
        Ok(value.sqrt())
    }
}

fn main() {
    // Basic Result handling
    match divide(10.0, 0.0) {
        Ok(result) => println!("Result: {}", result),
        Err(e) => eprintln!("Error: {}", e),
    }
    
    // Using if let
    if let Ok(result) = divide(10.0, 2.0) {
        println!("Division result: {}", result);
    }
    
    // Pattern matching with custom errors
    match safe_sqrt(-4.0) {
        Ok(result) => println!("Square root: {}", result),
        Err(MathError::NegativeSquareRoot) => {
            eprintln!("Cannot compute square root of negative number");
        }
        Err(e) => eprintln!("Math error: {:?}", e),
    }
}
```

### 2. Option<T>

`Option` represents the presence or absence of a value, similar to `std::optional` in C++ but more deeply integrated into the language.

```rust
// Find element in vector
fn find_element(vec: &[i32], target: i32) -> Option<usize> {
    vec.iter().position(|&x| x == target)
}

// Parse integer from string
fn parse_int(s: &str) -> Option<i32> {
    s.parse().ok()
}

// Chaining operations with Option
fn get_first_word_length(text: &str) -> Option<usize> {
    text.split_whitespace()
        .next()
        .map(|word| word.len())
}

fn main() {
    let numbers = vec![1, 2, 3, 4, 5];
    
    // Basic Option handling
    match find_element(&numbers, 3) {
        Some(index) => println!("Found at index: {}", index),
        None => println!("Not found"),
    }
    
    // Using unwrap_or for defaults
    let index = find_element(&numbers, 10).unwrap_or(usize::MAX);
    println!("Index: {}", index);
    
    // Using if let
    if let Some(num) = parse_int("42") {
        println!("Parsed: {}", num);
    } else {
        println!("Failed to parse");
    }
    
    // Chaining with map and and_then
    let text = "Hello world";
    let length = get_first_word_length(text).unwrap_or(0);
    println!("First word length: {}", length);
}
```

### 3. The ? Operator

The `?` operator provides elegant error propagation, automatically converting errors and returning early if an error occurs.

```rust
use std::fs::File;
use std::io::{self, Read};

// Multiple operations with ? operator
fn read_username_from_file() -> Result<String, io::Error> {
    let mut file = File::open("username.txt")?;
    let mut username = String::new();
    file.read_to_string(&mut username)?;
    Ok(username)
}

// Chaining with ?
fn process_file(filename: &str) -> Result<usize, io::Error> {
    let content = std::fs::read_to_string(filename)?;
    let word_count = content.split_whitespace().count();
    Ok(word_count)
}

// Converting between error types
fn parse_and_double(s: &str) -> Result<i32, Box<dyn std::error::Error>> {
    let num: i32 = s.parse()?;  // ParseIntError converted to Box<dyn Error>
    Ok(num * 2)
}

fn main() -> Result<(), Box<dyn std::error::Error>> {
    let word_count = process_file("data.txt")?;
    println!("Word count: {}", word_count);
    
    let doubled = parse_and_double("21")?;
    println!("Doubled: {}", doubled);
    
    Ok(())
}
```

### 4. panic! for Unrecoverable Errors

`panic!` is used for unrecoverable errors and programming bugs. It unwinds the stack and terminates the thread.

```rust
fn must_be_positive(value: i32) {
    if value < 0 {
        panic!("Value must be positive, got: {}", value);
    }
    println!("Value is: {}", value);
}

fn divide_unwrap(a: f64, b: f64) -> f64 {
    if b == 0.0 {
        panic!("Attempted to divide by zero");
    }
    a / b
}

// Using unwrap() and expect()
fn parse_config(s: &str) -> i32 {
    // unwrap panics if parsing fails
    s.parse().unwrap()
}

fn parse_config_better(s: &str) -> i32 {
    // expect provides custom panic message
    s.parse().expect("Failed to parse configuration value")
}

fn main() {
    must_be_positive(42);  // OK
    
    // This would panic:
    // must_be_positive(-1);
    
    let result = divide_unwrap(10.0, 2.0);
    println!("Result: {}", result);
    
    // Using expect for better error messages
    let config = parse_config_better("123");
    println!("Config value: {}", config);
}
```

### 5. Error Type Conversions and Custom Errors

Rust provides powerful mechanisms for creating and converting between error types.

```rust
use std::fmt;
use std::error::Error;
use std::num::ParseIntError;
use std::io;

// Custom error type
#[derive(Debug)]
enum AppError {
    Io(io::Error),
    Parse(ParseIntError),
    Custom(String),
}

impl fmt::Display for AppError {
    fn fmt(&self, f: &mut fmt::Formatter) -> fmt::Result {
        match self {
            AppError::Io(e) => write!(f, "IO error: {}", e),
            AppError::Parse(e) => write!(f, "Parse error: {}", e),
            AppError::Custom(msg) => write!(f, "Error: {}", msg),
        }
    }
}

impl Error for AppError {}

// Implementing From for automatic conversions
impl From<io::Error> for AppError {
    fn from(error: io::Error) -> Self {
        AppError::Io(error)
    }
}

impl From<ParseIntError> for AppError {
    fn from(error: ParseIntError) -> Self {
        AppError::Parse(error)
    }
}

// Now ? operator can automatically convert errors
fn read_and_parse_number(filename: &str) -> Result<i32, AppError> {
    let content = std::fs::read_to_string(filename)?;  // io::Error -> AppError
    let number: i32 = content.trim().parse()?;         // ParseIntError -> AppError
    
    if number < 0 {
        return Err(AppError::Custom("Number must be positive".to_string()));
    }
    
    Ok(number)
}

fn main() {
    match read_and_parse_number("number.txt") {
        Ok(num) => println!("Number: {}", num),
        Err(e) => eprintln!("Error occurred: {}", e),
    }
}
```

## Philosophical Differences

### C++ Philosophy
C++ provides flexibility and multiple approaches, allowing developers to choose based on context. This comes with trade-offs: exceptions can be disabled for embedded systems or performance-critical code, error codes offer zero-overhead but require discipline, and the newer `std::optional` and `std::expected` provide type-safe alternatives. However, nothing enforces error handling at compile time, and different codebases often mix various strategies.

### Rust Philosophy
Rust enforces explicit error handling through its type system. Errors must be handled or explicitly ignored (using `unwrap()` or `expect()`), making error paths visible in the code. The `Result` and `Option` types make the possibility of failure part of the function signature, and the `?` operator provides concise error propagation without sacrificing explicitness. This approach eliminates entire classes of bugs related to unhandled errors.

## Summary Table

| Feature | C++ | Rust |
|---------|-----|------|
| **Primary Mechanism** | Multiple: exceptions, error codes, optional, expected | `Result<T, E>` for recoverable errors, `panic!` for unrecoverable |
| **Compile-time Enforcement** | No, errors can be ignored | Yes, must handle or explicitly ignore |
| **Exception Overhead** | Can have performance cost, can be disabled | No exceptions; zero-cost abstractions |
| **Error Propagation** | `throw` (exceptions) or manual checking | `?` operator for automatic propagation |
| **Absence of Value** | `std::optional<T>` (C++17) | `Option<T>` (built-in, deeply integrated) |
| **Result + Error** | `std::expected<T, E>` (C++23) | `Result<T, E>` (core language feature) |
| **Stack Unwinding** | Automatic with exceptions | `panic!` unwinds by default (can be disabled) |
| **Error Composition** | Manual or library support | Built-in with traits (`From`, `Error`) |
| **Visibility** | Errors not always visible in signatures | Always visible in function signatures |
| **Mixed Approaches** | Common (exceptions + error codes + optional) | Uniform (`Result`/`Option` everywhere) |
| **Performance** | Varies by mechanism; exceptions have overhead | Zero-cost abstractions for `Result`/`Option` |
| **Error Context** | Exception hierarchies, error messages | Error types, pattern matching, custom errors |
| **Learning Curve** | Multiple patterns to learn | Single, consistent pattern |
| **Library Ecosystem** | Mixed approaches across libraries | Consistent `Result`/`Option` usage |
| **Debugging** | Stack traces with exceptions | Backtrace with `RUST_BACKTRACE=1` |
| **Default Behavior** | Unchecked errors can propagate silently | Compiler warns/errors on unhandled `Result` |

## Conclusion

C++ offers flexibility with multiple error-handling mechanisms that have evolved over time, allowing developers to optimize for specific contexts but requiring careful discipline. Rust takes an opinionated, type-driven approach that makes errors explicit and forces handling at compile time, eliminating many common bugs at the cost of initial verbosity. The choice reflects deeper language philosophies: C++'s trust-the-programmer flexibility versus Rust's safety-first guarantees.