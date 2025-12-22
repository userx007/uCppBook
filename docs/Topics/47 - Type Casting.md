# Type casting in C++

## Key Topics Covered:

**1. Implicit Casting** - Automatic conversions that the compiler performs (like int to float)

**2. C-Style Casts** - The older `(type)value` syntax (generally avoided in modern C++)

**3. static_cast** - The most commonly used C++ cast for compile-time conversions between related types

**4. dynamic_cast** - Runtime type checking for polymorphic class hierarchies, returns nullptr on failure

**5. const_cast** - Removes or adds const qualifiers (use cautiously!)

**6. reinterpret_cast** - Low-level bit pattern reinterpretation (most dangerous)

**Best Practices Highlighted:**
- Prefer C++ style casts over C-style casts for clarity and safety
- Use `static_cast` for most everyday conversions
- Use `dynamic_cast` when you need runtime type safety in inheritance hierarchies
- Avoid `const_cast` and `reinterpret_cast` unless absolutely necessary

The code includes working examples for each casting type, shows common pitfalls (like data loss when casting float to int), and demonstrates practical use cases like percentage calculations and working with polymorphic objects.

```cpp
/*
================================================================================
                    C++ TYPE CASTING - COMPREHENSIVE GUIDE
================================================================================

Type casting is the process of converting a value from one data type to another.
C++ provides both C-style casts and four specialized C++ casting operators.

================================================================================
1. IMPLICIT TYPE CASTING (Automatic Conversion)
================================================================================
*/

#include <iostream>
using namespace std;

void demonstrateImplicitCasting() {
    cout << "\n=== IMPLICIT TYPE CASTING ===" << endl;
    
    // Integer to float (widening conversion - safe)
    int i = 42;
    float f = i;  // Automatic conversion
    cout << "int to float: " << i << " -> " << f << endl;
    
    // Float to double (widening conversion)
    float f2 = 3.14f;
    double d = f2;
    cout << "float to double: " << f2 << " -> " << d << endl;
    
    // WARNING: Float to int (narrowing conversion - data loss!)
    double d2 = 9.99;
    int i2 = d2;  // Fractional part is truncated
    cout << "double to int: " << d2 << " -> " << i2 << " (DATA LOSS!)" << endl;
    
    // Char to int
    char c = 'A';
    int ascii = c;
    cout << "char to int: '" << c << "' -> " << ascii << endl;
}

/*
================================================================================
2. EXPLICIT TYPE CASTING - C-STYLE CAST
================================================================================
Syntax: (type) expression  or  type(expression)

WARNING: C-style casts are powerful but dangerous because they can perform
multiple types of conversions without clear intent.
*/

void demonstrateCStyleCast() {
    cout << "\n=== C-STYLE CASTING ===" << endl;
    
    // Basic numeric conversion
    double pi = 3.14159;
    int truncated = (int)pi;
    cout << "C-style cast: " << pi << " -> " << truncated << endl;
    
    // Pointer conversion (dangerous!)
    int num = 65;
    char* charPtr = (char*)&num;
    cout << "Pointer cast result: " << *charPtr << endl;
    
    // Alternative syntax
    float value = float(100);
    cout << "Function-style cast: " << value << endl;
}

/*
================================================================================
3. STATIC_CAST - Compile-Time Type Conversion
================================================================================
Syntax: static_cast<new_type>(expression)

Use for:
- Converting between numeric types
- Converting pointers in class hierarchy (upcast/downcast)
- Explicit conversions that would otherwise be implicit
- Converting void* to typed pointer
*/

void demonstrateStaticCast() {
    cout << "\n=== STATIC_CAST ===" << endl;
    
    // Numeric conversion
    double d = 3.14159;
    int i = static_cast<int>(d);
    cout << "static_cast<int>: " << d << " -> " << i << endl;
    
    // Pointer to void and back
    int value = 42;
    void* voidPtr = &value;
    int* intPtr = static_cast<int*>(voidPtr);
    cout << "void* to int*: " << *intPtr << endl;
    
    // Enum to int
    enum Color { RED = 1, GREEN = 2, BLUE = 3 };
    Color c = RED;
    int colorValue = static_cast<int>(c);
    cout << "enum to int: " << colorValue << endl;
    
    // Integer to enum (not implicit!)
    int num = 2;
    Color c2 = static_cast<Color>(num);
    cout << "int to enum: " << static_cast<int>(c2) << endl;
}

/*
================================================================================
4. DYNAMIC_CAST - Runtime Type-Safe Casting
================================================================================
Syntax: dynamic_cast<new_type>(expression)

Use for:
- Safe downcasting in polymorphic class hierarchies
- Requires at least one virtual function in base class
- Returns nullptr for pointers or throws bad_cast for references on failure
*/

class Animal {
public:
    virtual ~Animal() {}  // Must have virtual function for dynamic_cast
    virtual void speak() { cout << "Some sound" << endl; }
};

class Dog : public Animal {
public:
    void speak() override { cout << "Woof!" << endl; }
    void fetch() { cout << "Fetching ball..." << endl; }
};

class Cat : public Animal {
public:
    void speak() override { cout << "Meow!" << endl; }
    void purr() { cout << "Purring..." << endl; }
};

void demonstrateDynamicCast() {
    cout << "\n=== DYNAMIC_CAST ===" << endl;
    
    Animal* animal1 = new Dog();
    Animal* animal2 = new Cat();
    
    // Safe downcast - SUCCESS
    Dog* dog = dynamic_cast<Dog*>(animal1);
    if (dog) {
        cout << "Successfully cast to Dog: ";
        dog->fetch();
    }
    
    // Safe downcast - FAILURE (returns nullptr)
    Dog* notADog = dynamic_cast<Dog*>(animal2);
    if (!notADog) {
        cout << "Cannot cast Cat to Dog (nullptr returned)" << endl;
    }
    
    // Using with references (throws exception on failure)
    try {
        Dog& dogRef = dynamic_cast<Dog&>(*animal2);
        dogRef.fetch();
    } catch (bad_cast& e) {
        cout << "bad_cast exception: " << e.what() << endl;
    }
    
    delete animal1;
    delete animal2;
}

/*
================================================================================
5. CONST_CAST - Add or Remove const/volatile Qualifiers
================================================================================
Syntax: const_cast<new_type>(expression)

Use for:
- Removing const qualifier (use with extreme caution!)
- Adding const qualifier (rarely needed)
- Only works with pointers and references

WARNING: Modifying a truly const object through const_cast is undefined behavior!
*/

void modifyValue(int* ptr) {
    *ptr = 100;
}

void demonstrateConstCast() {
    cout << "\n=== CONST_CAST ===" << endl;
    
    // Removing const to pass to legacy API
    const int value = 42;
    cout << "Original const value: " << value << endl;
    
    // WARNING: This is generally bad practice!
    // Only do this if you know the function won't actually modify the value
    // or if the original object wasn't truly const
    int* modifiable = const_cast<int*>(&value);
    // Uncommenting the next line would be undefined behavior:
    // *modifiable = 100;  // DON'T DO THIS with truly const objects!
    
    // More legitimate use: working with legacy C APIs
    int nonConstValue = 50;
    const int* constPtr = &nonConstValue;
    int* normalPtr = const_cast<int*>(constPtr);
    *normalPtr = 75;  // Safe because original wasn't const
    cout << "Modified non-const value through const_cast: " << nonConstValue << endl;
}

/*
================================================================================
6. REINTERPRET_CAST - Low-Level Bit Pattern Reinterpretation
================================================================================
Syntax: reinterpret_cast<new_type>(expression)

Use for:
- Converting between unrelated pointer types
- Converting pointers to integers and vice versa
- Casting function pointers

WARNING: Most dangerous cast! No type checking. Use only when absolutely necessary.
*/

void demonstrateReinterpretCast() {
    cout << "\n=== REINTERPRET_CAST ===" << endl;
    
    // Pointer to integer conversion
    int value = 42;
    int* ptr = &value;
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    cout << "Pointer as integer: 0x" << hex << address << dec << endl;
    
    // Integer to pointer (system-level programming)
    int* ptrFromInt = reinterpret_cast<int*>(address);
    cout << "Value from reconstructed pointer: " << *ptrFromInt << endl;
    
    // Reinterpreting bytes of a float as an int
    float f = 3.14f;
    int* intView = reinterpret_cast<int*>(&f);
    cout << "Float bits as int: " << *intView << endl;
    
    // Converting between unrelated pointer types
    struct Data { int x, y; };
    Data data = {10, 20};
    char* bytePtr = reinterpret_cast<char*>(&data);
    cout << "First byte of struct: " << static_cast<int>(bytePtr[0]) << endl;
}

/*
================================================================================
7. COMPARISON AND BEST PRACTICES
================================================================================

Casting Type       | Safety | Use Case
-------------------|--------|------------------------------------------
Implicit           | High   | Automatic safe conversions
static_cast        | Medium | Most general-purpose conversions
dynamic_cast       | High   | Polymorphic type checking at runtime
const_cast         | Low    | Working with const (use sparingly!)
reinterpret_cast   | Very Low| Low-level bit manipulation
C-style cast       | Low    | Avoid - unclear intent

BEST PRACTICES:
1. Prefer implicit conversions when possible
2. Use static_cast for most explicit conversions
3. Use dynamic_cast for safe polymorphic downcasting
4. Avoid const_cast unless working with legacy APIs
5. Avoid reinterpret_cast unless doing system-level work
6. Never use C-style casts in modern C++ code
7. Consider if your design needs redesigning if you need many casts
*/

/*
================================================================================
8. PRACTICAL EXAMPLES
================================================================================
*/

void practicalExamples() {
    cout << "\n=== PRACTICAL EXAMPLES ===" << endl;
    
    // Example 1: Percentage calculation
    int correct = 45;
    int total = 50;
    // Without cast: integer division gives 0!
    double percentage = static_cast<double>(correct) / total * 100;
    cout << "Score: " << percentage << "%" << endl;
    
    // Example 2: Working with enums
    enum Status { IDLE = 0, RUNNING = 1, STOPPED = 2 };
    Status status = RUNNING;
    if (static_cast<int>(status) > 0) {
        cout << "System is active" << endl;
    }
    
    // Example 3: Base class array
    Animal* animals[3] = { new Dog(), new Cat(), new Dog() };
    cout << "\nProcessing animals:" << endl;
    for (int i = 0; i < 3; i++) {
        animals[i]->speak();
        
        // Try to cast to Dog
        if (Dog* dog = dynamic_cast<Dog*>(animals[i])) {
            dog->fetch();
        }
        
        delete animals[i];
    }
}

/*
================================================================================
MAIN FUNCTION - RUN ALL DEMONSTRATIONS
================================================================================
*/

int main() {
    cout << "C++ TYPE CASTING DEMONSTRATION" << endl;
    cout << "==============================" << endl;
    
    demonstrateImplicitCasting();
    demonstrateCStyleCast();
    demonstrateStaticCast();
    demonstrateDynamicCast();
    demonstrateConstCast();
    demonstrateReinterpretCast();
    practicalExamples();
    
    cout << "\n==============================" << endl;
    cout << "All demonstrations complete!" << endl;
    
    return 0;
}
```
