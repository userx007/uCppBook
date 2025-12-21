#include <iostream>
#include <memory>
#include <vector>
#include <string>

// ============================================================================
// MEMORY OWNERSHIP MODELS IN C++
// ============================================================================
// Ownership determines who is responsible for deleting dynamically allocated
// memory. Clear ownership prevents memory leaks and double-deletion bugs.

// ============================================================================
// 1. UNIQUE OWNERSHIP (std::unique_ptr)
// ============================================================================
// One owner at a time. Ownership can be transferred but not shared.
// Automatically deletes the object when the unique_ptr goes out of scope.

class Resource {
    std::string name;
public:
    Resource(const std::string& n) : name(n) {
        std::cout << "Resource '" << name << "' created\n";
    }
    ~Resource() {
        std::cout << "Resource '" << name << "' destroyed\n";
    }
    void use() { std::cout << "Using " << name << "\n"; }
};

void uniqueOwnershipExample() {
    std::cout << "\n=== UNIQUE OWNERSHIP ===\n";
    
    // Create unique_ptr - it owns the Resource
    std::unique_ptr<Resource> ptr1 = std::make_unique<Resource>("UniqueRes1");
    ptr1->use();
    
    // Transfer ownership with std::move
    std::unique_ptr<Resource> ptr2 = std::move(ptr1);
    // ptr1 is now nullptr, ptr2 owns the resource
    
    if (!ptr1) {
        std::cout << "ptr1 is now empty\n";
    }
    ptr2->use();
    
    // Resource automatically deleted when ptr2 goes out of scope
}

// ============================================================================
// 2. SHARED OWNERSHIP (std::shared_ptr)
// ============================================================================
// Multiple owners can share the same object. Uses reference counting.
// Object is deleted when the last shared_ptr is destroyed.

void sharedOwnershipExample() {
    std::cout << "\n=== SHARED OWNERSHIP ===\n";
    
    std::shared_ptr<Resource> shared1 = std::make_shared<Resource>("SharedRes");
    std::cout << "Reference count: " << shared1.use_count() << "\n";
    
    {
        // Create another shared_ptr - increases reference count
        std::shared_ptr<Resource> shared2 = shared1;
        std::cout << "Reference count: " << shared1.use_count() << "\n";
        shared2->use();
        
        // shared2 goes out of scope, reference count decreases
    }
    
    std::cout << "Reference count after scope: " << shared1.use_count() << "\n";
    // Resource deleted when last shared_ptr (shared1) goes out of scope
}

// ============================================================================
// 3. WEAK REFERENCES (std::weak_ptr)
// ============================================================================
// Non-owning reference to a shared_ptr. Doesn't affect reference count.
// Used to break circular references and observe without owning.

class Node {
public:
    std::string name;
    std::shared_ptr<Node> next;  // Owning reference
    std::weak_ptr<Node> prev;    // Non-owning reference (breaks cycle)
    
    Node(const std::string& n) : name(n) {
        std::cout << "Node '" << name << "' created\n";
    }
    ~Node() {
        std::cout << "Node '" << name << "' destroyed\n";
    }
};

void weakPtrExample() {
    std::cout << "\n=== WEAK REFERENCES ===\n";
    
    auto node1 = std::make_shared<Node>("Node1");
    auto node2 = std::make_shared<Node>("Node2");
    
    // Create bidirectional links
    node1->next = node2;        // shared_ptr (owning)
    node2->prev = node1;        // weak_ptr (non-owning, breaks cycle)
    
    // To use weak_ptr, convert to shared_ptr
    if (auto prevNode = node2->prev.lock()) {
        std::cout << "Node2's previous: " << prevNode->name << "\n";
    }
    
    // Both nodes properly destroyed without memory leak
}

// ============================================================================
// 4. NON-OWNING REFERENCES (Raw Pointers/References)
// ============================================================================
// Used when you need to refer to an object but don't own it.
// The owner is responsible for the object's lifetime.

class Engine {
public:
    void start() { std::cout << "Engine started\n"; }
};

class Car {
    std::unique_ptr<Engine> engine;  // Car owns the engine
public:
    Car() : engine(std::make_unique<Engine>()) {}
    
    // Return non-owning pointer for observation
    Engine* getEngine() { return engine.get(); }
    
    // Alternative: return reference
    Engine& getEngineRef() { return *engine; }
};

void nonOwningExample() {
    std::cout << "\n=== NON-OWNING REFERENCES ===\n";
    
    Car car;
    
    // Get non-owning pointer
    Engine* enginePtr = car.getEngine();
    enginePtr->start();
    
    // Get reference
    Engine& engineRef = car.getEngineRef();
    engineRef.start();
    
    // Car still owns and will delete the engine
}

// ============================================================================
// 5. FACTORY PATTERN WITH OWNERSHIP TRANSFER
// ============================================================================

std::unique_ptr<Resource> createResource(const std::string& name) {
    return std::make_unique<Resource>(name);
}

void factoryExample() {
    std::cout << "\n=== FACTORY PATTERN ===\n";
    
    // Factory transfers ownership to caller
    auto resource = createResource("FactoryRes");
    resource->use();
}

// ============================================================================
// 6. CONTAINER OWNERSHIP
// ============================================================================

class Manager {
    // Manager owns all resources
    std::vector<std::unique_ptr<Resource>> resources;
    
public:
    void addResource(std::unique_ptr<Resource> res) {
        resources.push_back(std::move(res));
    }
    
    // Return non-owning pointer for access
    Resource* getResource(size_t index) {
        if (index < resources.size()) {
            return resources[index].get();
        }
        return nullptr;
    }
    
    ~Manager() {
        std::cout << "Manager destroying all resources\n";
    }
};

void containerOwnershipExample() {
    std::cout << "\n=== CONTAINER OWNERSHIP ===\n";
    
    Manager manager;
    manager.addResource(std::make_unique<Resource>("Res1"));
    manager.addResource(std::make_unique<Resource>("Res2"));
    
    // Access without taking ownership
    if (Resource* res = manager.getResource(0)) {
        res->use();
    }
    
    // Manager automatically deletes all resources when destroyed
}

// ============================================================================
// 7. CIRCULAR REFERENCE PROBLEM AND SOLUTION
// ============================================================================

class Parent;
class Child {
public:
    std::weak_ptr<Parent> parent;  // Weak to break cycle
    std::string name;
    
    Child(const std::string& n) : name(n) {
        std::cout << "Child '" << name << "' created\n";
    }
    ~Child() {
        std::cout << "Child '" << name << "' destroyed\n";
    }
};

class Parent {
public:
    std::vector<std::shared_ptr<Child>> children;  // Strong ownership
    std::string name;
    
    Parent(const std::string& n) : name(n) {
        std::cout << "Parent '" << name << "' created\n";
    }
    ~Parent() {
        std::cout << "Parent '" << name << "' destroyed\n";
    }
};

void circularReferenceExample() {
    std::cout << "\n=== CIRCULAR REFERENCE SOLUTION ===\n";
    
    auto parent = std::make_shared<Parent>("Dad");
    auto child = std::make_shared<Child>("Son");
    
    parent->children.push_back(child);
    child->parent = parent;  // Weak pointer breaks cycle
    
    // Both properly destroyed when they go out of scope
}

// ============================================================================
// 8. CUSTOM DELETERS
// ============================================================================

void customDeleter(Resource* ptr) {
    std::cout << "Custom deleter called\n";
    delete ptr;
}

void customDeleterExample() {
    std::cout << "\n=== CUSTOM DELETERS ===\n";
    
    // unique_ptr with custom deleter
    std::unique_ptr<Resource, decltype(&customDeleter)> 
        ptr(new Resource("CustomDel"), customDeleter);
    
    // Can also use lambda
    auto deleter = [](Resource* p) {
        std::cout << "Lambda deleter called\n";
        delete p;
    };
    std::unique_ptr<Resource, decltype(deleter)> 
        ptr2(new Resource("LambdaDel"), deleter);
}

// ============================================================================
// MAIN DEMONSTRATION
// ============================================================================

int main() {
    std::cout << "C++ MEMORY OWNERSHIP MODELS DEMONSTRATION\n";
    std::cout << "=========================================\n";
    
    uniqueOwnershipExample();
    sharedOwnershipExample();
    weakPtrExample();
    nonOwningExample();
    factoryExample();
    containerOwnershipExample();
    circularReferenceExample();
    customDeleterExample();
    
    std::cout << "\n=== PROGRAM END ===\n";
    return 0;
}

/* ============================================================================
   KEY PRINCIPLES AND BEST PRACTICES:
   ============================================================================
   
   1. PREFER UNIQUE_PTR BY DEFAULT
      - Lowest overhead (zero cost abstraction)
      - Clear ownership semantics
      - Use shared_ptr only when truly needed
   
   2. USE MAKE_UNIQUE AND MAKE_SHARED
      - Exception safe
      - More efficient (single allocation for shared_ptr)
      - Syntax: auto ptr = std::make_unique<Type>(args);
   
   3. NEVER USE NEW/DELETE DIRECTLY
      - Always use smart pointers
      - RAII (Resource Acquisition Is Initialization)
   
   4. OWNERSHIP TRANSFER
      - Use std::move() for unique_ptr
      - Pass by value and move for transfer
      - Pass by const& for shared_ptr when not transferring
   
   5. NON-OWNING ACCESS
      - Use raw pointers or references for observation
      - Never delete non-owning raw pointers
      - Ensure lifetime is managed by owner
   
   6. AVOID CIRCULAR REFERENCES
      - Use weak_ptr for back-references
      - Parent owns child with shared_ptr
      - Child references parent with weak_ptr
   
   7. FUNCTION PARAMETERS
      - Pass by value for ownership transfer
      - Pass by const reference for sharing
      - Pass raw pointer/reference for non-owning access
   
   8. RETURN VALUES
      - Return unique_ptr for ownership transfer
      - Return shared_ptr for shared ownership
      - Return raw pointer/reference for non-owning access
   
   ============================================================================
   COMMON PITFALLS:
   ============================================================================
   
   - Mixing ownership models (raw ptr from unique_ptr then delete)
   - Creating multiple unique_ptr/shared_ptr to same raw pointer
   - Circular references with shared_ptr (use weak_ptr)
   - Storing weak_ptr without checking if valid (use lock())
   - Returning raw pointer to local unique_ptr
   - Using shared_ptr everywhere (performance overhead)
   
   ============================================================================
*/