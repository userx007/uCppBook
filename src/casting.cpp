#include <iostream>
#include <string>
#include <typeinfo>
#include <memory>

// ============================================================================
// 1. STATIC_CAST EXAMPLES
// ============================================================================

class Animal {
public:
    virtual ~Animal() {}
    virtual void makeSound() { std::cout << "Some generic sound\n"; }
};

class Dog : public Animal {
public:
    void makeSound() override { std::cout << "Woof!\n"; }
    void fetch() { std::cout << "Fetching ball...\n"; }
};

class Cat : public Animal {
public:
    void makeSound() override { std::cout << "Meow!\n"; }
    void climb() { std::cout << "Climbing tree...\n"; }
};

void demonstrateStaticCast() {
    std::cout << "\n=== STATIC_CAST EXAMPLES ===\n";

    // 1. Numeric conversions
    int intVal = 42;
    double doubleVal = static_cast<double>(intVal);
    std::cout << "Int to double: " << doubleVal << "\n";

    double pi = 3.14159;
    int truncated = static_cast<int>(pi);
    std::cout << "Double to int (truncated): " << truncated << "\n";

    // 2. Upcast (always safe)
    Dog* dog = new Dog();
    Animal* animal = static_cast<Animal*>(dog);
    animal->makeSound();  // Polymorphic call works

    // 3. Downcast (unsafe without guarantees!)
    Animal* animalPtr = new Dog();
    Dog* dogPtr = static_cast<Dog*>(animalPtr);  // Compiles, but risky!
    dogPtr->fetch();  // Works because we know it's actually a Dog

    // DANGER: This compiles but is WRONG!
    Animal* actualCat = new Cat();
    Dog* wrongCast = static_cast<Dog*>(actualCat);  // No error!
    // wrongCast->fetch();  // UNDEFINED BEHAVIOR - don't uncomment!

    // 4. Void pointer conversion
    int value = 100;
    void* voidPtr = &value;
    int* intPtr = static_cast<int*>(voidPtr);
    std::cout << "Value from void*: " << *intPtr << "\n";

    delete dog;
    delete animalPtr;
    delete actualCat;
}

// ============================================================================
// 2. DYNAMIC_CAST EXAMPLES
// ============================================================================

void demonstrateDynamicCast() {
    std::cout << "\n=== DYNAMIC_CAST EXAMPLES ===\n";

    // 1. Safe downcast with pointer
    Animal* animal1 = new Dog();
    Dog* dog = dynamic_cast<Dog*>(animal1);
    if (dog != nullptr) {
        std::cout << "Successfully cast to Dog\n";
        dog->fetch();
    } else {
        std::cout << "Failed to cast to Dog\n";
    }

    // 2. Failed downcast
    Animal* animal2 = new Cat();
    Dog* notADog = dynamic_cast<Dog*>(animal2);
    if (notADog == nullptr) {
        std::cout << "Correctly failed to cast Cat to Dog\n";
    }

    // 3. Cross-cast example
    Cat* cat = dynamic_cast<Cat*>(animal2);
    if (cat != nullptr) {
        std::cout << "Successfully cast to Cat\n";
        cat->climb();
    }

    // 4. Reference cast (throws on failure)
    try {
        Animal& animalRef = *animal1;
        Dog& dogRef = dynamic_cast<Dog&>(animalRef);
        std::cout << "Reference cast succeeded\n";
        dogRef.fetch();
    } catch (std::bad_cast& e) {
        std::cout << "Reference cast failed: " << e.what() << "\n";
    }

    // 5. This will throw
    try {
        Animal& catRef = *animal2;
        Dog& wrongRef = dynamic_cast<Dog&>(catRef);
        wrongRef.fetch();
    } catch (std::bad_cast& e) {
        std::cout << "Caught bad_cast exception: " << e.what() << "\n";
    }

    delete animal1;
    delete animal2;
}

// ============================================================================
// 3. CONST_CAST EXAMPLES
// ============================================================================

void legacyFunction(char* str) {
    // Old C API that doesn't use const but doesn't modify
    std::cout << "Legacy function: " << str << "\n";
}

class DataContainer {
    mutable int accessCount = 0;
    int data[5] = {1, 2, 3, 4, 5};

public:
    // Const version does the work
    const int& operator[](size_t index) const {
        ++accessCount;  // mutable member
        if (index >= 5) throw std::out_of_range("Index out of range");
        return data[index];
    }

    // Non-const version reuses const version
    int& operator[](size_t index) {
        return const_cast<int&>(
            static_cast<const DataContainer&>(*this)[index]
        );
    }

    int getAccessCount() const { return accessCount; }
};

void demonstrateConstCast() {
    std::cout << "\n=== CONST_CAST EXAMPLES ===\n";

    // 1. Interfacing with legacy API
    const char* message = "Hello, World!";
    // legacyFunction(message);  // Won't compile
    legacyFunction(const_cast<char*>(message));  // Works if function doesn't modify

    // 2. Adding const (always safe)
    int value = 42;
    int* ptr = &value;
    const int* constPtr = const_cast<const int*>(ptr);
    std::cout << "Const value: " << *constPtr << "\n";

    // 3. Code reuse pattern
    DataContainer container;
    std::cout << "Non-const access: " << container[2] << "\n";
    const DataContainer& constContainer = container;
    std::cout << "Const access: " << constContainer[3] << "\n";
    std::cout << "Access count: " << container.getAccessCount() << "\n";

    // 4. DANGER: Modifying originally const data
    const int original = 100;
    // int* dangerous = const_cast<int*>(&original);
    // *dangerous = 200;  // UNDEFINED BEHAVIOR - don't do this!

    std::cout << "Original value (still safe): " << original << "\n";
}

// ============================================================================
// 4. REINTERPRET_CAST EXAMPLES
// ============================================================================

struct Hardware {
    uint32_t status;
    uint32_t control;
    uint32_t data;
};

void demonstrateReinterpretCast() {
    std::cout << "\n=== REINTERPRET_CAST EXAMPLES ===\n";

    // 1. Pointer to integer conversion
    int value = 42;
    int* ptr = &value;
    uintptr_t address = reinterpret_cast<uintptr_t>(ptr);
    std::cout << "Pointer address: 0x" << std::hex << address << std::dec << "\n";

    int* ptrBack = reinterpret_cast<int*>(address);
    std::cout << "Value from reconstructed pointer: " << *ptrBack << "\n";

    // 2. Byte-level access to data
    double pi = 3.14159;
    unsigned char* bytes = reinterpret_cast<unsigned char*>(&pi);
    std::cout << "First 4 bytes of double: ";
    for (int i = 0; i < 4; ++i) {
        std::cout << std::hex << (int)bytes[i] << " ";
    }
    std::cout << std::dec << "\n";

    // 3. Simulated hardware register access
    // In real embedded systems: 0xFFFF0000 might be a hardware address
    // Here we just demonstrate the syntax
    uint32_t simulatedHardware[3] = {0x01, 0x02, 0x03};
    volatile Hardware* hwRegs = reinterpret_cast<volatile Hardware*>(simulatedHardware);
    std::cout << "Hardware status: " << hwRegs->status << "\n";

    // 4. Function pointer casting (advanced, rarely needed)
    void (*funcPtr)() = reinterpret_cast<void(*)()>(address);
    // Don't call it! Just showing the syntax
    std::cout << "Function pointer created (not calling it)\n";

    // 5. Converting between unrelated types (DANGEROUS)
    struct Point { int x, y; };
    struct Color { int r, g; };

    Point point{10, 20};
    Color* color = reinterpret_cast<Color*>(&point);
    std::cout << "Point as Color (nonsensical): r=" << color->r
              << ", g=" << color->g << "\n";
}

// ============================================================================
// 5. COMPARISON AND BEST PRACTICES
// ============================================================================

void demonstrateComparison() {
    std::cout << "\n=== CAST COMPARISON ===\n";

    double d = 3.14;

    // C-style cast (avoid!)
    int cStyle = (int)d;
    std::cout << "C-style cast: " << cStyle << "\n";

    // C++ style cast (preferred)
    int cppStyle = static_cast<int>(d);
    std::cout << "static_cast: " << cppStyle << "\n";

    // C-style cast can do dangerous things silently
    const int* constPtr = &cStyle;
    // int* badCast = (int*)constPtr;  // Compiles, removes const!

    // C++ requires explicit const_cast
    int* explicitCast = const_cast<int*>(constPtr);
    std::cout << "Explicit const_cast makes intent clear\n";

    // Polymorphic casting comparison
    Animal* animal = new Cat();

    // static_cast: no runtime check, compiles even if wrong
    Dog* staticDog = static_cast<Dog*>(animal);
    // staticDog->fetch();  // UNDEFINED BEHAVIOR!

    // dynamic_cast: runtime check, safe
    Dog* dynamicDog = dynamic_cast<Dog*>(animal);
    if (dynamicDog == nullptr) {
        std::cout << "dynamic_cast safely detected wrong type\n";
    }

    delete animal;
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "C++ CASTING OPERATORS - COMPREHENSIVE EXAMPLES\n";
    std::cout << "==============================================\n";

    try {
        demonstrateStaticCast();
        demonstrateDynamicCast();
        demonstrateConstCast();
        demonstrateReinterpretCast();
        demonstrateComparison();

        std::cout << "\n=== SUMMARY ===\n";
        std::cout << "1. static_cast: Compile-time, related types\n";
        std::cout << "2. dynamic_cast: Runtime, polymorphic types only\n";
        std::cout << "3. const_cast: Add/remove const/volatile\n";
        std::cout << "4. reinterpret_cast: Low-level bit reinterpretation\n";
        std::cout << "\nAlways prefer C++ casts over C-style casts!\n";

    } catch (const std::exception& e) {
        std::cerr << "Exception: " << e.what() << "\n";
        return 1;
    }

    return 0;
}