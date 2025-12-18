#include <iostream>
#include <memory>
#include <vector>
#include <string>
#include <fstream>

// ============================================================================
// 1. BASIC UNIQUE_PTR USAGE
// ============================================================================

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

void uniquePtrBasics() {
    std::cout << "\n=== UNIQUE_PTR BASICS ===\n";

    // Preferred way: make_unique (C++14)
    auto ptr1 = std::make_unique<Resource>("ptr1");
    ptr1->use();

    // Old way (avoid)
    std::unique_ptr<Resource> ptr2(new Resource("ptr2"));

    // Moving ownership (unique_ptr is move-only)
    auto ptr3 = std::move(ptr1); // ptr1 is now nullptr
    if (!ptr1) {
        std::cout << "ptr1 is now null after move\n";
    }

    // Array support
    auto arr = std::make_unique<int[]>(5);
    arr[0] = 10;
    std::cout << "Array element: " << arr[0] << "\n";

    // reset() to change ownership or delete
    ptr3.reset(new Resource("ptr3-new"));

    // release() to give up ownership (rarely needed)
    Resource* raw = ptr3.release(); // ptr3 is now nullptr, we own raw
    delete raw; // Manual cleanup required

} // ptr2 automatically destroyed here

// ============================================================================
// 2. UNIQUE_PTR WITH CUSTOM DELETERS
// ============================================================================

void customDeleterExample() {
    std::cout << "\n=== CUSTOM DELETERS ===\n";

    // File handle with custom deleter
    auto fileDeleter = [](FILE* f) {
        if (f) {
            std::cout << "Closing file\n";
            fclose(f);
        }
    };

    std::unique_ptr<FILE, decltype(fileDeleter)> file(
        fopen("test.txt", "w"),
        fileDeleter
    );

    if (file) {
        fprintf(file.get(), "Hello from unique_ptr!\n");
    }

    // C++ resource with lambda deleter
    auto deleter = [](Resource* r) {
        std::cout << "Custom delete: ";
        delete r;
    };

    std::unique_ptr<Resource, decltype(deleter)> customPtr(
        new Resource("custom"),
        deleter
    );

} // File and resource automatically cleaned up

// ============================================================================
// 3. FACTORY PATTERN WITH UNIQUE_PTR
// ============================================================================

std::unique_ptr<Resource> createResource(const std::string& name) {
    return std::make_unique<Resource>(name);
}

void factoryPattern() {
    std::cout << "\n=== FACTORY PATTERN ===\n";
    auto res = createResource("factory-created");
    res->use();
}

// ============================================================================
// 4. BASIC SHARED_PTR USAGE
// ============================================================================

void sharedPtrBasics() {
    std::cout << "\n=== SHARED_PTR BASICS ===\n";

    // Preferred: make_shared (more efficient, single allocation)
    auto ptr1 = std::make_shared<Resource>("shared1");
    std::cout << "ptr1 use_count: " << ptr1.use_count() << "\n";

    {
        // Copying increases reference count
        auto ptr2 = ptr1;
        std::cout << "After copy, use_count: " << ptr1.use_count() << "\n";

        auto ptr3 = ptr1;
        std::cout << "After another copy: " << ptr1.use_count() << "\n";

    } // ptr2 and ptr3 destroyed, count decremented

    std::cout << "After scope exit: " << ptr1.use_count() << "\n";

    // Shared pointer with custom deleter
    auto customShared = std::shared_ptr<Resource>(
        new Resource("custom-shared"),
        [](Resource* r) {
            std::cout << "Custom shared deleter: ";
            delete r;
        }
    );

    // Convert unique_ptr to shared_ptr
    auto unique = std::make_unique<Resource>("converted");
    std::shared_ptr<Resource> converted = std::move(unique);
    std::cout << "Converted use_count: " << converted.use_count() << "\n";

} // All resources cleaned up

// ============================================================================
// 5. WEAK_PTR USAGE
// ============================================================================

void weakPtrBasics() {
    std::cout << "\n=== WEAK_PTR BASICS ===\n";

    std::weak_ptr<Resource> weak;

    {
        auto shared = std::make_shared<Resource>("observed");
        weak = shared; // weak doesn't increase ref count

        std::cout << "shared use_count: " << shared.use_count() << "\n";
        std::cout << "weak expired: " << weak.expired() << "\n";

        // Access through weak_ptr using lock()
        if (auto locked = weak.lock()) {
            locked->use();
            std::cout << "Locked use_count: " << locked.use_count() << "\n";
        }

    } // shared destroyed here

    std::cout << "After shared destroyed, weak expired: " << weak.expired() << "\n";

    if (auto locked = weak.lock()) {
        std::cout << "This won't print\n";
    } else {
        std::cout << "Object no longer exists\n";
    }
}

// ============================================================================
// 6. CIRCULAR REFERENCE PROBLEM AND SOLUTION
// ============================================================================

// PROBLEM: Circular reference causes memory leak
class NodeBad {
public:
    std::shared_ptr<NodeBad> next;
    std::shared_ptr<NodeBad> prev; // Creates cycle!
    std::string data;

    NodeBad(const std::string& d) : data(d) {
        std::cout << "NodeBad '" << data << "' created\n";
    }
    ~NodeBad() {
        std::cout << "NodeBad '" << data << "' destroyed\n";
    }
};

void circularReferenceProblem() {
    std::cout << "\n=== CIRCULAR REFERENCE PROBLEM ===\n";

    auto node1 = std::make_shared<NodeBad>("node1");
    auto node2 = std::make_shared<NodeBad>("node2");

    node1->next = node2;
    node2->prev = node1; // Creates circular reference!

    std::cout << "node1 use_count: " << node1.use_count() << "\n";
    std::cout << "node2 use_count: " << node2.use_count() << "\n";

    // Memory leak: nodes won't be destroyed!
}

// SOLUTION: Use weak_ptr to break the cycle
class NodeGood {
public:
    std::shared_ptr<NodeGood> next;
    std::weak_ptr<NodeGood> prev; // Weak breaks the cycle
    std::string data;

    NodeGood(const std::string& d) : data(d) {
        std::cout << "NodeGood '" << data << "' created\n";
    }
    ~NodeGood() {
        std::cout << "NodeGood '" << data << "' destroyed\n";
    }
};

void circularReferenceSolution() {
    std::cout << "\n=== CIRCULAR REFERENCE SOLUTION ===\n";

    auto node1 = std::make_shared<NodeGood>("node1");
    auto node2 = std::make_shared<NodeGood>("node2");

    node1->next = node2;
    node2->prev = node1; // weak_ptr doesn't increase ref count

    std::cout << "node1 use_count: " << node1.use_count() << "\n";
    std::cout << "node2 use_count: " << node2.use_count() << "\n";

    // Properly cleaned up!
}

// ============================================================================
// 7. PARENT-CHILD RELATIONSHIP WITH WEAK_PTR
// ============================================================================

class Child;

class Parent {
public:
    std::string name;
    std::vector<std::shared_ptr<Child>> children;

    Parent(const std::string& n) : name(n) {
        std::cout << "Parent '" << name << "' created\n";
    }
    ~Parent() {
        std::cout << "Parent '" << name << "' destroyed\n";
    }
};

class Child {
public:
    std::string name;
    std::weak_ptr<Parent> parent; // Weak to prevent cycle

    Child(const std::string& n) : name(n) {
        std::cout << "Child '" << name << "' created\n";
    }
    ~Child() {
        std::cout << "Child '" << name << "' destroyed\n";
    }

    void printParent() {
        if (auto p = parent.lock()) {
            std::cout << "My parent is: " << p->name << "\n";
        } else {
            std::cout << "Parent no longer exists\n";
        }
    }
};

void parentChildExample() {
    std::cout << "\n=== PARENT-CHILD RELATIONSHIP ===\n";

    auto parent = std::make_shared<Parent>("Dad");
    auto child1 = std::make_shared<Child>("Alice");
    auto child2 = std::make_shared<Child>("Bob");

    child1->parent = parent;
    child2->parent = parent;

    parent->children.push_back(child1);
    parent->children.push_back(child2);

    child1->printParent();

    // All properly cleaned up due to weak_ptr
}

// ============================================================================
// 8. ENABLE_SHARED_FROM_THIS
// ============================================================================

class Observable : public std::enable_shared_from_this<Observable> {
    std::vector<std::weak_ptr<Observable>> observers;
    std::string name;
public:
    Observable(const std::string& n) : name(n) {}

    void registerAsObserver(std::shared_ptr<Observable> other) {
        // Need to create shared_ptr to 'this'
        // WRONG: std::shared_ptr<Observable>(this) - creates new control block!
        // RIGHT: shared_from_this()
        other->observers.push_back(shared_from_this());
    }

    void notify() {
        std::cout << name << " notifying observers...\n";
        for (auto& weak : observers) {
            if (auto obs = weak.lock()) {
                std::cout << "  Observer still alive\n";
            }
        }
    }
};

void enableSharedFromThisExample() {
    std::cout << "\n=== ENABLE_SHARED_FROM_THIS ===\n";

    auto obj1 = std::make_shared<Observable>("obj1");
    auto obj2 = std::make_shared<Observable>("obj2");

    obj1->registerAsObserver(obj2);
    obj2->notify();
}

// ============================================================================
// 9. CACHE PATTERN WITH WEAK_PTR
// ============================================================================

class ExpensiveObject {
    int id;
public:
    ExpensiveObject(int i) : id(i) {
        std::cout << "ExpensiveObject " << id << " created\n";
    }
    ~ExpensiveObject() {
        std::cout << "ExpensiveObject " << id << " destroyed\n";
    }
    int getId() const { return id; }
};

class Cache {
    std::vector<std::weak_ptr<ExpensiveObject>> cache;
public:
    std::shared_ptr<ExpensiveObject> get(int id) {
        // Clean expired entries
        cache.erase(
            std::remove_if(cache.begin(), cache.end(),
                [](const auto& weak) { return weak.expired(); }),
            cache.end()
        );

        // Check if object exists in cache
        for (auto& weak : cache) {
            if (auto obj = weak.lock()) {
                if (obj->getId() == id) {
                    std::cout << "Cache hit for " << id << "\n";
                    return obj;
                }
            }
        }

        // Cache miss - create new
        std::cout << "Cache miss for " << id << "\n";
        auto obj = std::make_shared<ExpensiveObject>(id);
        cache.push_back(obj);
        return obj;
    }
};

void cachePatternExample() {
    std::cout << "\n=== CACHE PATTERN ===\n";

    Cache cache;

    {
        auto obj1 = cache.get(1);
        auto obj1_again = cache.get(1); // Cache hit

        std::cout << "obj1 use_count: " << obj1.use_count() << "\n";

    } // obj1 destroyed, but weak_ptr in cache remains

    std::cout << "After scope, trying to get object 1:\n";
    auto obj1_later = cache.get(1); // Cache miss (expired)
}

// ============================================================================
// 10. COMMON PITFALLS
// ============================================================================

void commonPitfalls() {
    std::cout << "\n=== COMMON PITFALLS ===\n";

    // PITFALL 1: Creating multiple shared_ptr from same raw pointer
    std::cout << "Pitfall 1: Multiple shared_ptr from raw pointer\n";
    // Resource* raw = new Resource("dangerous");
    // std::shared_ptr<Resource> ptr1(raw);
    // std::shared_ptr<Resource> ptr2(raw); // WRONG! Double delete!
    // Solution: Use make_shared or enable_shared_from_this

    // PITFALL 2: Forgetting to break cycles
    std::cout << "Pitfall 2: See circular reference examples above\n";

    // PITFALL 3: Thread safety misconception
    std::cout << "Pitfall 3: shared_ptr control block is thread-safe,\n";
    std::cout << "           but the object itself is NOT!\n";

    auto shared = std::make_shared<std::vector<int>>();
    // Multiple threads copying shared is safe
    // Multiple threads accessing shared->push_back() needs mutex!

    // PITFALL 4: Performance - don't use shared_ptr everywhere
    std::cout << "Pitfall 4: Use unique_ptr by default, shared_ptr only when needed\n";

    // PITFALL 5: Returning shared_ptr by value is fine (move semantics)
    std::cout << "Pitfall 5: Returning smart pointers by value is efficient (RVO/move)\n";
}

// ============================================================================
// 11. SMART POINTER SIZE COMPARISON
// ============================================================================

void sizeComparison() {
    std::cout << "\n=== SIZE COMPARISON ===\n";
    std::cout << "Raw pointer:   " << sizeof(int*) << " bytes\n";
    std::cout << "unique_ptr:    " << sizeof(std::unique_ptr<int>) << " bytes\n";
    std::cout << "shared_ptr:    " << sizeof(std::shared_ptr<int>) << " bytes\n";
    std::cout << "weak_ptr:      " << sizeof(std::weak_ptr<int>) << " bytes\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main() {
    uniquePtrBasics();
    customDeleterExample();
    factoryPattern();
    sharedPtrBasics();
    weakPtrBasics();
    circularReferenceProblem();
    circularReferenceSolution();
    parentChildExample();
    enableSharedFromThisExample();
    cachePatternExample();
    commonPitfalls();
    sizeComparison();

    std::cout << "\n=== PROGRAM END ===\n";
    return 0;
}