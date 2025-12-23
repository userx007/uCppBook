# Virtual Functions, VTables & Dynamic Dispatch in C++

## Overview

**Virtual functions** enable **polymorphism** in C++, allowing derived classes to override base class methods and have the correct version called at runtime based on the actual object type, not the pointer/reference type. This runtime resolution is called **dynamic dispatch**, and it's implemented through **vtables** (virtual tables).

## Key Concepts

### 1. Virtual Functions

A virtual function is a member function declared with the `virtual` keyword in a base class that can be overridden in derived classes.

```cpp
class Base {
public:
    virtual void speak() {
        std::cout << "Base speaking\n";
    }
};

class Derived : public Base {
public:
    void speak() override {  // override keyword is optional but recommended
        std::cout << "Derived speaking\n";
    }
};
```

### 2. Dynamic Dispatch

Dynamic dispatch means the function call is resolved at **runtime** rather than compile-time:

```cpp
Base* ptr = new Derived();
ptr->speak();  // Calls Derived::speak() - resolved at runtime!
```

Without `virtual`, this would call `Base::speak()` (static binding).

### 3. VTable (Virtual Table)

The compiler creates a **vtable** for each class with virtual functions. It's an array of function pointers:

```
Base vtable:          Derived vtable:
+------------+        +------------+
| &Base::speak |      | &Derived::speak |
+------------+        +------------+
```

Each object with virtual functions has a hidden **vptr** (virtual pointer) that points to its class's vtable.

## Comprehensive Example

```cpp
#include <iostream>
#include <vector>

class Animal {
protected:
    std::string name;
    
public:
    Animal(const std::string& n) : name(n) {}
    
    // Virtual function - enables polymorphism
    virtual void makeSound() const {
        std::cout << name << " makes a generic sound\n";
    }
    
    // Virtual destructor - ESSENTIAL for polymorphic base classes
    virtual ~Animal() {
        std::cout << "Animal destructor: " << name << "\n";
    }
    
    // Non-virtual function - statically dispatched
    void identify() const {
        std::cout << "I am an animal named " << name << "\n";
    }
};

class Dog : public Animal {
private:
    std::string breed;
    
public:
    Dog(const std::string& n, const std::string& b) 
        : Animal(n), breed(b) {}
    
    // Override virtual function
    void makeSound() const override {
        std::cout << name << " (a " << breed << ") barks: Woof!\n";
    }
    
    ~Dog() override {
        std::cout << "Dog destructor: " << name << "\n";
    }
};

class Cat : public Animal {
private:
    bool isIndoor;
    
public:
    Cat(const std::string& n, bool indoor) 
        : Animal(n), isIndoor(indoor) {}
    
    void makeSound() const override {
        std::cout << name << " meows: Meow!\n";
    }
    
    ~Cat() override {
        std::cout << "Cat destructor: " << name << "\n";
    }
};

class Bird : public Animal {
public:
    Bird(const std::string& n) : Animal(n) {}
    
    void makeSound() const override {
        std::cout << name << " chirps: Tweet tweet!\n";
    }
    
    ~Bird() override {
        std::cout << "Bird destructor: " << name << "\n";
    }
};

int main() {
    std::cout << "=== Creating animals ===\n";
    
    // Store different derived types through base class pointers
    std::vector<Animal*> animals;
    animals.push_back(new Dog("Buddy", "Golden Retriever"));
    animals.push_back(new Cat("Whiskers", true));
    animals.push_back(new Bird("Tweety"));
    animals.push_back(new Dog("Rex", "German Shepherd"));
    
    std::cout << "\n=== Making sounds (dynamic dispatch) ===\n";
    // Dynamic dispatch: correct derived class method called
    for (const auto& animal : animals) {
        animal->makeSound();  // Calls the appropriate derived class version
    }
    
    std::cout << "\n=== Demonstrating static vs dynamic binding ===\n";
    Animal* ptr = new Dog("Max", "Beagle");
    ptr->makeSound();   // Dynamic dispatch -> Dog::makeSound()
    ptr->identify();    // Static dispatch -> Animal::identify()
    
    std::cout << "\n=== Cleanup (virtual destructors in action) ===\n";
    delete ptr;  // Calls Dog::~Dog() then Animal::~Animal()
    
    std::cout << "\n=== Cleaning up vector ===\n";
    for (auto& animal : animals) {
        delete animal;  // Virtual destructor ensures proper cleanup
    }
    
    return 0;
}
```

**Output:**
```
=== Creating animals ===

=== Making sounds (dynamic dispatch) ===
Buddy (a Golden Retriever) barks: Woof!
Whiskers meows: Meow!
Tweety chirps: Tweet tweet!
Rex (a German Shepherd) barks: Woof!

=== Demonstrating static vs dynamic binding ===
Max (a Beagle) barks: Woof!
I am an animal named Max

=== Cleanup (virtual destructors in action) ===
Dog destructor: Max
Animal destructor: Max

=== Cleaning up vector ===
Dog destructor: Buddy
Animal destructor: Buddy
Cat destructor: Whiskers
Animal destructor: Whiskers
Bird destructor: Tweety
Animal destructor: Tweety
Dog destructor: Rex
Animal destructor: Rex
```

## How VTables Work (Memory Layout)

```cpp
// Simplified memory representation

Animal object:
+------------------+
| vptr (8 bytes)   | --> Points to Animal vtable
| name (24 bytes)  |
+------------------+

Dog object:
+------------------+
| vptr (8 bytes)   | --> Points to Dog vtable
| name (24 bytes)  |
| breed (24 bytes) |
+------------------+

Animal vtable:              Dog vtable:
+-----------------------+   +-----------------------+
| &Animal::makeSound    |   | &Dog::makeSound       |
| &Animal::~Animal      |   | &Dog::~Dog            |
+-----------------------+   +-----------------------+
```

When you call `ptr->makeSound()`:
1. Dereference `ptr` to get the object
2. Follow the `vptr` to the vtable
3. Index into the vtable to find the function pointer
4. Call the function through that pointer

## Important Points

### ✅ **Always Use Virtual Destructors**
```cpp
class Base {
public:
    virtual ~Base() {}  // ESSENTIAL!
};
```
Without this, `delete basePtr` won't call derived destructors, causing memory leaks.

### ✅ **The `override` Keyword**
```cpp
void makeSound() const override;  // Compiler checks you're actually overriding
```

### ✅ **The `final` Keyword**
```cpp
class Dog final : public Animal {  // Cannot be inherited from
    void makeSound() const final;   // Cannot be overridden further
};
```

### ⚠️ **Performance Cost**
- Virtual function calls are slightly slower (one extra indirection)
- Objects with virtual functions are slightly larger (vptr overhead)
- Usually negligible, but matters in tight loops

### ⚠️ **Pure Virtual Functions**
```cpp
class Animal {
public:
    virtual void makeSound() const = 0;  // Pure virtual - must override
};
```
Makes the class **abstract** (cannot instantiate).

## When to Use Virtual Functions

✅ **Use when:**
- You need polymorphic behavior
- Derived classes should customize base class behavior
- Working with collections of related types

❌ **Avoid when:**
- Performance is absolutely critical in tight loops
- You don't need polymorphism (consider templates instead)
- Class won't be used as a base class

This mechanism is fundamental to achieving runtime polymorphism in C++ and is one of the key features that enables object-oriented design patterns like Strategy, Factory, and Observer.