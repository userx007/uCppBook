#include <iostream>
#include <memory>
#include <vector>
#include <chrono>
using namespace std;

// ============================================
// DYNAMIC POLYMORPHISM (Runtime - Virtual Functions)
// ============================================
class AnimalDynamic {
public:
    virtual void makeSound() const = 0;
    virtual string getType() const = 0;
    virtual ~AnimalDynamic() = default;
};

class DogDynamic : public AnimalDynamic {
public:
    void makeSound() const override {
        cout << "Woof!" << endl;
    }
    string getType() const override {
        return "Dog";
    }
};

class CatDynamic : public AnimalDynamic {
public:
    void makeSound() const override {
        cout << "Meow!" << endl;
    }
    string getType() const override {
        return "Cat";
    }
};

// ============================================
// STATIC POLYMORPHISM (Compile-time - Templates/CRTP)
// ============================================
template <typename Derived>
class AnimalStatic {
public:
    void makeSound() const {
        static_cast<const Derived*>(this)->makeSoundImpl();
    }
    
    string getType() const {
        return static_cast<const Derived*>(this)->getTypeImpl();
    }
};

class DogStatic : public AnimalStatic<DogStatic> {
public:
    void makeSoundImpl() const {
        cout << "Woof!" << endl;
    }
    string getTypeImpl() const {
        return "Dog";
    }
};

class CatStatic : public AnimalStatic<CatStatic> {
public:
    void makeSoundImpl() const {
        cout << "Meow!" << endl;
    }
    string getTypeImpl() const {
        return "Cat";
    }
};

// ============================================
// DEMONSTRATION
// ============================================

// Function using dynamic polymorphism
void useDynamicPolymorphism() {
    cout << "=== DYNAMIC POLYMORPHISM ===" << endl;
    
    // Can store different types in same container
    vector<unique_ptr<AnimalDynamic>> animals;
    animals.push_back(make_unique<DogDynamic>());
    animals.push_back(make_unique<CatDynamic>());
    animals.push_back(make_unique<DogDynamic>());
    
    for (const auto& animal : animals) {
        cout << animal->getType() << " says: ";
        animal->makeSound();
    }
    cout << endl;
}

// Function using static polymorphism
template <typename T>
void processAnimal(const AnimalStatic<T>& animal) {
    cout << animal.getType() << " says: ";
    animal.makeSound();
}

void useStaticPolymorphism() {
    cout << "=== STATIC POLYMORPHISM ===" << endl;
    
    DogStatic dog;
    CatStatic cat;
    
    // Type known at compile time
    processAnimal(dog);
    processAnimal(cat);
    processAnimal(dog);
    cout << endl;
}

// Performance comparison
void performanceTest() {
    cout << "=== PERFORMANCE TEST ===" << endl;
    const int iterations = 10000000;
    
    // Dynamic polymorphism test
    auto start = chrono::high_resolution_clock::now();
    {
        unique_ptr<AnimalDynamic> animal = make_unique<DogDynamic>();
        for (int i = 0; i < iterations; ++i) {
            animal->getType(); // Virtual function call
        }
    }
    auto end = chrono::high_resolution_clock::now();
    auto dynamicTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    // Static polymorphism test
    start = chrono::high_resolution_clock::now();
    {
        DogStatic dog;
        for (int i = 0; i < iterations; ++i) {
            dog.getType(); // Direct call, inlined
        }
    }
    end = chrono::high_resolution_clock::now();
    auto staticTime = chrono::duration_cast<chrono::milliseconds>(end - start).count();
    
    cout << "Dynamic polymorphism: " << dynamicTime << "ms" << endl;
    cout << "Static polymorphism:  " << staticTime << "ms" << endl;
    cout << "Speedup: " << (double)dynamicTime / staticTime << "x" << endl;
}

int main() {
    useDynamicPolymorphism();
    useStaticPolymorphism();
    performanceTest();
    
    cout << "\nKey Differences:" << endl;
    cout << "- Dynamic: Runtime resolution, virtual table overhead, " 
         << "supports heterogeneous containers" << endl;
    cout << "- Static: Compile-time resolution, no overhead, " 
         << "faster but less flexible" << endl;
    
    return 0;
}