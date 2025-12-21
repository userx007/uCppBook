#include <iostream>
#include <optional>
#include <variant>
#include <any>
#include <string>
#include <vector>
#include <map>

/*
============================================================================
                            STD::OPTIONAL
============================================================================
Purpose: Represents a value that may or may not be present
Replaces: Null pointers, sentinel values, and error-prone patterns
Size: sizeof(T) + 1 byte (typically with padding)
*/

// Example 1: Basic std::optional usage
std::optional<int> divide(int a, int b) {
    if (b == 0) {
        return std::nullopt; // No value
    }
    return a / b;
}

// Example 2: Optional in data structures
struct User {
    std::string name;
    std::optional<std::string> middle_name;
    std::optional<int> age;
};

// Example 3: Configuration with optional values
class DatabaseConfig {
    std::string host;
    int port;
    std::optional<std::string> password;
    
public:
    DatabaseConfig(std::string h, int p) : host(h), port(p) {}
    
    void setPassword(const std::string& pwd) {
        password = pwd;
    }
    
    std::string getConnectionString() const {
        std::string conn = host + ":" + std::to_string(port);
        if (password.has_value()) {
            conn += " (authenticated)";
        }
        return conn;
    }
};

void demonstrateOptional() {
    std::cout << "=== STD::OPTIONAL EXAMPLES ===\n\n";
    
    // Basic usage
    auto result1 = divide(10, 2);
    auto result2 = divide(10, 0);
    
    if (result1.has_value()) {
        std::cout << "10/2 = " << result1.value() << "\n";
    }
    
    if (!result2) { // Implicit bool conversion
        std::cout << "10/0 = undefined\n";
    }
    
    // Value_or: provide default if empty
    std::cout << "Result with default: " << result2.value_or(-1) << "\n\n";
    
    // User with optional fields
    User u1{"John", "William", 30};
    User u2{"Jane", std::nullopt, std::nullopt};
    
    std::cout << "User 1 middle name: " 
              << u1.middle_name.value_or("(none)") << "\n";
    std::cout << "User 2 middle name: " 
              << u2.middle_name.value_or("(none)") << "\n\n";
    
    // Monadic operations (C++23)
    std::optional<int> opt_val = 42;
    auto doubled = opt_val.transform([](int x) { return x * 2; });
    std::cout << "Transformed value: " << doubled.value() << "\n\n";
    
    // Emplace: construct in-place
    std::optional<std::vector<int>> opt_vec;
    opt_vec.emplace(10, 5); // Vector of 10 elements, value 5
    std::cout << "Vector size: " << opt_vec->size() << "\n\n";
}

/*
============================================================================
                            STD::VARIANT
============================================================================
Purpose: Type-safe union that can hold one of several types
Replaces: C unions, void*, polymorphism in some cases
Size: sizeof(largest_type) + discriminator
*/

// Example 1: Basic variant usage
using IntOrString = std::variant<int, std::string>;

// Example 2: Result type pattern
template<typename T, typename E>
class Result {
    std::variant<T, E> data;
    
public:
    Result(T value) : data(std::move(value)) {}
    Result(E error) : data(std::move(error)) {}
    
    bool isOk() const { return std::holds_alternative<T>(data); }
    bool isErr() const { return std::holds_alternative<E>(data); }
    
    const T& value() const { return std::get<T>(data); }
    const E& error() const { return std::get<E>(data); }
};

// Example 3: AST/Expression tree
struct Add;
struct Multiply;
struct Number { double value; };

using Expr = std::variant<Number, 
                         std::unique_ptr<Add>, 
                         std::unique_ptr<Multiply>>;

struct Add {
    Expr left, right;
};

struct Multiply {
    Expr left, right;
};

// Visitor pattern with std::visit
struct ExprEvaluator {
    double operator()(const Number& n) const {
        return n.value;
    }
    
    double operator()(const std::unique_ptr<Add>& a) const {
        return std::visit(*this, a->left) + std::visit(*this, a->right);
    }
    
    double operator()(const std::unique_ptr<Multiply>& m) const {
        return std::visit(*this, m->left) * std::visit(*this, m->right);
    }
};

void demonstrateVariant() {
    std::cout << "=== STD::VARIANT EXAMPLES ===\n\n";
    
    // Basic usage
    std::variant<int, double, std::string> v1 = 42;
    std::variant<int, double, std::string> v2 = 3.14;
    std::variant<int, double, std::string> v3 = "hello";
    
    // Check which type is active
    std::cout << "v1 holds int: " << std::holds_alternative<int>(v1) << "\n";
    std::cout << "v1 index: " << v1.index() << "\n\n";
    
    // std::get: throws if wrong type
    try {
        std::cout << "v1 as int: " << std::get<int>(v1) << "\n";
        std::cout << "v1 as string: " << std::get<std::string>(v1) << "\n";
    } catch (const std::bad_variant_access& e) {
        std::cout << "Exception: " << e.what() << "\n\n";
    }
    
    // std::get_if: returns nullptr if wrong type
    if (auto* str = std::get_if<std::string>(&v3)) {
        std::cout << "v3 contains string: " << *str << "\n\n";
    }
    
    // std::visit: apply visitor to variant
    auto printer = [](const auto& val) {
        std::cout << "Value: " << val << "\n";
    };
    
    std::visit(printer, v1);
    std::visit(printer, v2);
    std::visit(printer, v3);
    std::cout << "\n";
    
    // Overloaded visitor pattern
    struct {
        void operator()(int i) const { std::cout << "Int: " << i << "\n"; }
        void operator()(double d) const { std::cout << "Double: " << d << "\n"; }
        void operator()(const std::string& s) const { 
            std::cout << "String: " << s << "\n"; 
        }
    } overloaded_printer;
    
    std::visit(overloaded_printer, v1);
    
    // Result type pattern
    Result<int, std::string> success(42);
    Result<int, std::string> failure("Error: division by zero");
    
    std::cout << "\nResult is ok: " << success.isOk() << "\n";
    std::cout << "Failure message: " << failure.error() << "\n\n";
}

/*
============================================================================
                             STD::ANY
============================================================================
Purpose: Type-safe container for single values of any type
Replaces: void*, boost::any
Size: Small object optimization or heap allocation
Performance: Slower than variant (uses type erasure)
*/

// Example 1: Heterogeneous container
void demonstrateAny() {
    std::cout << "=== STD::ANY EXAMPLES ===\n\n";
    
    // Basic usage
    std::any a1 = 42;
    std::any a2 = 3.14;
    std::any a3 = std::string("hello");
    
    // Check if any contains a value
    std::cout << "a1 has value: " << a1.has_value() << "\n";
    
    // Get type information
    std::cout << "a1 type: " << a1.type().name() << "\n\n";
    
    // any_cast: extract value
    try {
        int val = std::any_cast<int>(a1);
        std::cout << "a1 value: " << val << "\n";
        
        // Wrong type throws
        double d = std::any_cast<double>(a1);
    } catch (const std::bad_any_cast& e) {
        std::cout << "Exception: " << e.what() << "\n\n";
    }
    
    // Pointer version doesn't throw
    if (auto* ptr = std::any_cast<std::string>(&a3)) {
        std::cout << "a3 contains: " << *ptr << "\n\n";
    }
    
    // Example: Property bag / Generic container
    std::map<std::string, std::any> properties;
    properties["name"] = std::string("Widget");
    properties["count"] = 42;
    properties["price"] = 19.99;
    properties["active"] = true;
    
    std::cout << "Properties:\n";
    for (const auto& [key, value] : properties) {
        std::cout << "  " << key << ": ";
        
        if (value.type() == typeid(std::string)) {
            std::cout << std::any_cast<std::string>(value);
        } else if (value.type() == typeid(int)) {
            std::cout << std::any_cast<int>(value);
        } else if (value.type() == typeid(double)) {
            std::cout << std::any_cast<double>(value);
        } else if (value.type() == typeid(bool)) {
            std::cout << std::any_cast<bool>(value);
        }
        std::cout << "\n";
    }
    std::cout << "\n";
    
    // Emplace
    std::any a4;
    a4.emplace<std::vector<int>>(5, 100); // Vector of 5 elements, value 100
    auto& vec = std::any_cast<std::vector<int>&>(a4);
    std::cout << "Vector size: " << vec.size() << "\n";
    std::cout << "First element: " << vec[0] << "\n\n";
    
    // Reset
    a1.reset();
    std::cout << "After reset, a1 has value: " << a1.has_value() << "\n\n";
}

/*
============================================================================
                         COMPARISON & GUIDELINES
============================================================================

WHEN TO USE STD::OPTIONAL:
- Function may or may not return a value
- Data member that's legitimately optional
- Replacing null pointers or sentinel values
- Configuration or settings with defaults

WHEN TO USE STD::VARIANT:
- Value must be one of a fixed set of types (known at compile time)
- Type-safe discriminated unions
- State machines
- Expression trees / AST
- Return multiple error types
- Performance-critical code (no heap allocation)

WHEN TO USE STD::ANY:
- Type is not known at compile time
- Plugin systems
- Scripting language bindings
- Generic property bags
- Bridge between type-safe and dynamic code
- Type information needs to be preserved across API boundaries

PERFORMANCE COMPARISON:
- std::optional: Zero overhead when empty, size overhead when full
- std::variant: No heap allocation, size of largest alternative
- std::any: Often uses heap allocation (except small object optimization)

SAFETY:
- std::optional: Exception if accessing empty optional with .value()
- std::variant: Exception if std::get<T> used with wrong type
- std::any: Exception if any_cast<T> used with wrong type
============================================================================
*/

int main() {
    demonstrateOptional();
    std::cout << "\n";
    demonstrateVariant();
    std::cout << "\n";
    demonstrateAny();
    
    std::cout << "\n=== COMPARISON ===\n\n";
    
    std::cout << "Size comparisons:\n";
    std::cout << "sizeof(int): " << sizeof(int) << "\n";
    std::cout << "sizeof(std::optional<int>): " << sizeof(std::optional<int>) << "\n";
    std::cout << "sizeof(std::variant<int, double, std::string>): " 
              << sizeof(std::variant<int, double, std::string>) << "\n";
    std::cout << "sizeof(std::any): " << sizeof(std::any) << "\n";
    
    return 0;
}