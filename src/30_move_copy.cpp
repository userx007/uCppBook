#include <iostream>
#include <vector>
#include <string>
#include <chrono>
#include <memory>

using namespace std;
using namespace std::chrono;

// ============================================================================
// PART 1: Understanding Copy Semantics (Pre-C++11)
// ============================================================================

class HeavyResource_CopyOnly {
private:
    int* data;
    size_t size;
    
public:
    // Constructor
    HeavyResource_CopyOnly(size_t n) : size(n) {
        data = new int[size];
        for(size_t i = 0; i < size; i++) {
            data[i] = i;
        }
        cout << "  [Constructor] Allocated " << size << " integers\n";
    }
    
    // Copy Constructor - EXPENSIVE operation
    HeavyResource_CopyOnly(const HeavyResource_CopyOnly& other) : size(other.size) {
        data = new int[size];
        for(size_t i = 0; i < size; i++) {
            data[i] = other.data[i];  // Deep copy
        }
        cout << "  [Copy Constructor] Copied " << size << " integers\n";
    }
    
    // Copy Assignment - EXPENSIVE operation
    HeavyResource_CopyOnly& operator=(const HeavyResource_CopyOnly& other) {
        if(this != &other) {
            delete[] data;
            size = other.size;
            data = new int[size];
            for(size_t i = 0; i < size; i++) {
                data[i] = other.data[i];
            }
            cout << "  [Copy Assignment] Copied " << size << " integers\n";
        }
        return *this;
    }
    
    // Destructor
    ~HeavyResource_CopyOnly() {
        delete[] data;
        cout << "  [Destructor] Freed memory\n";
    }
    
    size_t getSize() const { return size; }
};

// ============================================================================
// PART 2: Move Semantics (C++11 and later)
// ============================================================================

class HeavyResource_WithMove {
private:
    int* data;
    size_t size;
    
public:
    // Constructor
    HeavyResource_WithMove(size_t n) : size(n) {
        data = new int[size];
        for(size_t i = 0; i < size; i++) {
            data[i] = i;
        }
        cout << "  [Constructor] Allocated " << size << " integers\n";
    }
    
    // Copy Constructor
    HeavyResource_WithMove(const HeavyResource_WithMove& other) : size(other.size) {
        data = new int[size];
        for(size_t i = 0; i < size; i++) {
            data[i] = other.data[i];
        }
        cout << "  [Copy Constructor] Copied " << size << " integers\n";
    }
    
    // Move Constructor - CHEAP operation (just pointer swap)
    HeavyResource_WithMove(HeavyResource_WithMove&& other) noexcept 
        : data(other.data), size(other.size) {
        other.data = nullptr;  // Leave source in valid state
        other.size = 0;
        cout << "  [Move Constructor] Moved " << size << " integers (just pointer)\n";
    }
    
    // Copy Assignment
    HeavyResource_WithMove& operator=(const HeavyResource_WithMove& other) {
        if(this != &other) {
            delete[] data;
            size = other.size;
            data = new int[size];
            for(size_t i = 0; i < size; i++) {
                data[i] = other.data[i];
            }
            cout << "  [Copy Assignment] Copied " << size << " integers\n";
        }
        return *this;
    }
    
    // Move Assignment - CHEAP operation
    HeavyResource_WithMove& operator=(HeavyResource_WithMove&& other) noexcept {
        if(this != &other) {
            delete[] data;
            data = other.data;
            size = other.size;
            other.data = nullptr;
            other.size = 0;
            cout << "  [Move Assignment] Moved (just pointer swap)\n";
        }
        return *this;
    }
    
    // Destructor
    ~HeavyResource_WithMove() {
        delete[] data;
        if(size > 0) cout << "  [Destructor] Freed memory\n";
    }
    
    size_t getSize() const { return size; }
};

// ============================================================================
// PART 3: Factory Functions to Demonstrate Move Semantics
// ============================================================================

HeavyResource_CopyOnly createResource_CopyOnly() {
    cout << "Creating resource (copy-only)...\n";
    return HeavyResource_CopyOnly(1000000);  // Should copy on return (without RVO)
}

HeavyResource_WithMove createResource_WithMove() {
    cout << "Creating resource (with move)...\n";
    return HeavyResource_WithMove(1000000);  // Will be moved (if RVO doesn't apply)
}

// ============================================================================
// PART 4: Performance Comparison
// ============================================================================

void demonstrateCopyPerformance() {
    cout << "\n=== COPY SEMANTICS (Expensive) ===\n";
    
    auto start = high_resolution_clock::now();
    
    vector<HeavyResource_CopyOnly> vec;
    for(int i = 0; i < 5; i++) {
        HeavyResource_CopyOnly temp(100000);
        vec.push_back(temp);  // Copies into vector
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Time taken: " << duration.count() << "ms\n";
}

void demonstrateMovePerformance() {
    cout << "\n=== MOVE SEMANTICS (Cheap) ===\n";
    
    auto start = high_resolution_clock::now();
    
    vector<HeavyResource_WithMove> vec;
    for(int i = 0; i < 5; i++) {
        HeavyResource_WithMove temp(100000);
        vec.push_back(move(temp));  // Moves into vector (explicit)
    }
    
    auto end = high_resolution_clock::now();
    auto duration = duration_cast<milliseconds>(end - start);
    cout << "Time taken: " << duration.count() << "ms\n";
}

// ============================================================================
// PART 5: std::move and rvalue references explained
// ============================================================================

void explainStdMove() {
    cout << "\n=== Understanding std::move ===\n";
    
    // std::move doesn't actually move anything!
    // It just casts an lvalue to an rvalue reference
    
    HeavyResource_WithMove resource1(1000);
    
    // resource1 is an lvalue (has a name, persists)
    // std::move(resource1) casts it to rvalue reference
    // This enables move constructor/assignment
    HeavyResource_WithMove resource2 = move(resource1);
    
    cout << "After move:\n";
    cout << "  resource1 size: " << resource1.getSize() << " (moved-from state)\n";
    cout << "  resource2 size: " << resource2.getSize() << " (now owns the data)\n";
}

// ============================================================================
// PART 6: When Moves Happen Automatically
// ============================================================================

void automaticMoves() {
    cout << "\n=== Automatic Moves (No explicit std::move needed) ===\n";
    
    // 1. Returning from function (if not RVO'd)
    HeavyResource_WithMove r1 = createResource_WithMove();
    
    // 2. Temporary objects (rvalues)
    vector<HeavyResource_WithMove> vec;
    vec.push_back(HeavyResource_WithMove(1000));  // Temporary is automatically moved
    
    // 3. After std::move (explicit)
    HeavyResource_WithMove r2(5000);
    HeavyResource_WithMove r3 = move(r2);
}

// ============================================================================
// PART 7: Common Pitfalls
// ============================================================================

void commonPitfalls() {
    cout << "\n=== Common Pitfalls ===\n";
    
    // PITFALL 1: Using moved-from object
    HeavyResource_WithMove resource(1000);
    HeavyResource_WithMove moved = move(resource);
    // resource is now in "valid but unspecified state"
    // Accessing resource.getSize() is OK (returns 0), but relying on its data is not
    cout << "Moved-from object size: " << resource.getSize() << " (valid but empty)\n";
    
    // PITFALL 2: Moving const objects (copies instead!)
    const HeavyResource_WithMove constResource(1000);
    HeavyResource_WithMove shouldMove = move(constResource);  // Actually copies!
    // Can't move from const because move modifies the source
    
    // PITFALL 3: Forgetting std::move on named rvalue references
    HeavyResource_WithMove&& rvalueRef = HeavyResource_WithMove(1000);
    // rvalueRef is an rvalue reference, but it's a NAMED rvalue reference
    // Named things are lvalues! Need std::move to actually move it
}

// ============================================================================
// PART 8: Move-Only Types
// ============================================================================

class MoveOnlyResource {
private:
    unique_ptr<int[]> data;
    size_t size;
    
public:
    MoveOnlyResource(size_t n) : size(n), data(make_unique<int[]>(n)) {
        cout << "  [MoveOnly] Created\n";
    }
    
    // Delete copy operations
    MoveOnlyResource(const MoveOnlyResource&) = delete;
    MoveOnlyResource& operator=(const MoveOnlyResource&) = delete;
    
    // Default move operations (compiler-generated)
    MoveOnlyResource(MoveOnlyResource&&) = default;
    MoveOnlyResource& operator=(MoveOnlyResource&&) = default;
    
    ~MoveOnlyResource() {
        cout << "  [MoveOnly] Destroyed\n";
    }
};

void demonstrateMoveOnlyTypes() {
    cout << "\n=== Move-Only Types (like unique_ptr) ===\n";
    
    MoveOnlyResource resource(1000);
    // MoveOnlyResource copy = resource;  // ERROR: would not compile
    MoveOnlyResource moved = move(resource);  // OK: can move
    
    vector<MoveOnlyResource> vec;
    vec.push_back(MoveOnlyResource(500));  // OK: temporary moved in
    // vec.push_back(moved);  // ERROR: would try to copy
    vec.push_back(move(moved));  // OK: explicit move
}

// ============================================================================
// MAIN PROGRAM
// ============================================================================

int main() {
    cout << "C++ MOVE vs COPY SEMANTICS - Performance Guide\n";
    cout << "================================================\n";
    
    // Demonstrate performance difference
    demonstrateCopyPerformance();
    demonstrateMovePerformance();
    
    // Explain std::move
    explainStdMove();
    
    // Show automatic moves
    automaticMoves();
    
    // Common mistakes
    commonPitfalls();
    
    // Move-only types
    demonstrateMoveOnlyTypes();
    
    cout << "\n=== KEY TAKEAWAYS ===\n";
    cout << "1. Copy: Creates independent duplicate (expensive for large objects)\n";
    cout << "2. Move: Transfers ownership (cheap - just pointer swap)\n";
    cout << "3. Use std::move() to explicitly enable move semantics\n";
    cout << "4. Temporaries and return values often move automatically\n";
    cout << "5. Implement move constructor/assignment with noexcept\n";
    cout << "6. Leave moved-from objects in valid but unspecified state\n";
    cout << "7. Rule of Five: If you define one, consider all five special members\n";
    
    return 0;
}

/*
PERFORMANCE CHARACTERISTICS SUMMARY:

COPY SEMANTICS:
- Time Complexity: O(n) where n = size of data
- Space: Allocates new memory
- Use when: You need independent objects
- Example: vector.push_back(obj) where obj is lvalue

MOVE SEMANTICS:
- Time Complexity: O(1) - just pointer assignments
- Space: No new allocation
- Use when: Source object no longer needed
- Example: vector.push_back(std::move(obj))

TYPICAL SPEEDUPS:
- Small objects (< 16 bytes): Minimal difference
- Medium objects (KB range): 10-100x faster
- Large objects (MB range): 100-1000x faster
- Containers: Proportional to contained elements

RULE OF FIVE:
If you define any of these, consider defining all:
1. Destructor
2. Copy constructor
3. Copy assignment operator
4. Move constructor
5. Move assignment operator
*/