# Atomic Types Fundamentals in C++

## Key Topics Covered:

1. **Introduction to Atomics** - What they are and why they matter
2. **Basic Usage** - Declaration, initialization, and common operations
3. **Lock-Free Operations** - Understanding and verifying lock-free behavior
4. **Compare-and-Swap (CAS)** - The foundation of lock-free algorithms
5. **Memory Ordering** - The six memory order options and when to use them
6. **Practical Examples**:
   - Thread-safe counter
   - Lock-free stack implementation
   - Producer-consumer synchronization
7. **Best Practices** - Choosing the right memory ordering and avoiding pitfalls
8. **Performance Considerations** - Atomic vs mutex comparison, false sharing
9. **Common Pitfalls** - What to watch out for

Each section includes working code examples that demonstrate the concepts in practice. The examples progress from simple atomic operations to more complex lock-free data structures, giving you a solid foundation in atomic programming.

The guide emphasizes that atomic types are excellent for simple shared state (flags, counters) but require careful consideration of memory ordering semantics for correct synchronization between threads.

# Atomic Types Fundamentals in C++

## Introduction

Atomic types in C++ provide thread-safe operations on shared data without the overhead of explicit locking mechanisms. The `std::atomic` template class, introduced in C++11, enables lock-free programming for primitive types and guarantees that operations on atomic variables are indivisible—they either complete entirely or not at all, with no possibility of another thread observing a partially completed operation.

## What Are Atomic Operations?

An atomic operation is one that appears to occur instantaneously from the perspective of other threads. When multiple threads access an atomic variable:

- **No data races occur**: The operation is inherently thread-safe
- **No torn reads/writes**: You never observe partial state changes
- **Memory ordering guarantees**: The operation provides specific synchronization semantics

## Basic std::atomic Usage

### Declaration and Initialization

```cpp
#include <atomic>
#include <iostream>
#include <thread>

// Declare atomic variables
std::atomic<int> counter(0);           // Initialize to 0
std::atomic<bool> flag{false};         // Brace initialization
std::atomic<double> value;             // Default initialized

int main() {
    // Atomic operations are thread-safe
    counter.store(10);                 // Atomic write
    int current = counter.load();      // Atomic read
    
    std::cout << "Counter: " << current << std::endl;
    return 0;
}
```

### Common Atomic Operations

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

// Shared atomic counter - safe for concurrent access from multiple threads
// Initialized to 0
std::atomic<int> shared_counter(0);

// Function executed by each thread to increment the shared counter
void increment_counter(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        // Atomic increment operation - thread-safe without locks
        // fetch_add atomically adds 1 and returns the previous value
        // memory_order_relaxed: provides atomicity but no synchronization/ordering guarantees
        // - Fastest memory order for simple counters where order doesn't matter
        // - Only ensures this operation itself is atomic
        // - Does not prevent reordering with other operations
        shared_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

int main() {
    // Configuration:  
    const int num_threads = 10;     // number of threads
    const int iterations = 1000;    // iterations per thread
    
    // Container to hold all thread objects
    std::vector<std::thread> threads;
    
    // Launch multiple threads that will run concurrently
    // Each thread will increment the counter 'iterations' times
    for (int i = 0; i < num_threads; ++i) {
        // emplace_back constructs thread in-place, passing 
        // increment_counter function and iterations parameter
        threads.emplace_back(increment_counter, iterations);
    }
    
    // Wait for all threads to complete execution
    // join() blocks until the thread finishes
    for (auto& t : threads) {
        t.join();
    }
    
    // Display results
    // load() atomically reads the counter value
    // Result is always exactly num_threads * iterations because atomic operations
    // guarantee that no increments are lost due to race conditions
    std::cout << "Final counter: " << shared_counter.load() << std::endl;
    std::cout << "Expected: " << (num_threads * iterations) << std::endl;
    
    return 0;
}
```

## Key Atomic Operations

### Read-Modify-Write Operations

```cpp
#include <atomic>
#include <iostream>
#include <thread>

void demonstrate_atomic_operations() {
    std::atomic<int> value(100);
    
    // fetch_add: returns old value, adds to atomic
    int old = value.fetch_add(5);
    std::cout << "Old: " << old << ", New: " << value.load() << std::endl;
    // Output: Old: 100, New: 105
    
    // fetch_sub: returns old value, subtracts from atomic
    old = value.fetch_sub(3);
    std::cout << "Old: " << old << ", New: " << value.load() << std::endl;
    // Output: Old: 105, New: 102
    
    // fetch_and, fetch_or, fetch_xor for bitwise operations
    std::atomic<unsigned int> flags(0b1010);
    old = flags.fetch_or(0b0101);
    std::cout << "Flags after OR: " << flags.load() << std::endl;
    // Output: Flags after OR: 15 (0b1111)
}

int main() {
    demonstrate_atomic_operations();
    return 0;
}
```

### Compare-and-Swap (CAS)

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>

// Shared atomic value - will be safely incremented up to a maximum by multiple threads
std::atomic<int> atomic_value(0);

// Atomic compare-and-swap (CAS) implementation
// Safely increments the atomic value without exceeding max_value
// Multiple threads can call this concurrently without data races
void safe_increment_to_max(int max_value) {
    // Load the current value - this is our initial "expected" value
    int current = atomic_value.load();
    
    // Loop until we've either successfully incremented or reached the max
    while (current < max_value) {
        // compare_exchange_weak attempts to atomically:
        // 1. Compare atomic_value with 'current' (expected value)
        // 2. If equal: set atomic_value to 'current + 1' and return true
        // 3. If not equal: update 'current' to actual atomic_value and return false
        //
        // Why "weak"? 
        // - Can spuriously fail (return false even when values match)
        // - Faster on some architectures (e.g., ARM, PowerPC with LL/SC instructions)
        // - Acceptable here because we're in a loop that will retry
        // - Use compare_exchange_strong if spurious failures are problematic
        //
        // This is a lock-free algorithm: no thread can block another
        if (atomic_value.compare_exchange_weak(current, current + 1)) {
            break;  // Success - we incremented the value, our work is done
        }
        // Failure case (another thread changed the value first):
        // - 'current' is automatically updated to the actual current value
        // - Loop continues with the updated 'current' value
        // - Will retry if still below max_value
        // - This is the CAS retry loop pattern
    }
    // Note: If current >= max_value when we enter or after update, 
    // the loop exits without incrementing (preventing overflow)
}

int main() {
    // Maximum value the atomic counter should reach
    const int max_val = 100;
    
    std::vector<std::thread> threads;
    
    // Create 200 threads all trying to increment to max_val
    // Since max_val is 100, only the first 100 successful increments will occur
    // The remaining 100+ threads will find the value already at max and exit
    for (int i = 0; i < 200; ++i) {
        threads.emplace_back(safe_increment_to_max, max_val);
    }
    
    // Wait for all threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    // Display final value
    std::cout << "Final value: " << atomic_value.load() << std::endl;
    // Output: Final value: 100 (never exceeds max)
    // Guaranteed: despite 200 threads, the value stops at exactly max_val
    // because compare_exchange ensures no thread can increment past the limit
    
    return 0;
}
```

```
Main Thread    Thread 1         Thread 2         Thread 3             atomic_value = 0
    |              |                |                |                        |
    |--create----->|                |                |                        | 
    |--create---------------------->|                |                        |
    |--create--------------------------------------->|                        |
    |              |                |                |                        |
    |              | load()         |                |                        |
    |              |--------------->|--------------->|----------------------->|
    |              | current=0      | current=0      | current=0              |
    |              |                |                |                        |
    |       [while: 0 < 100]        |                |                        |
    |              |                |                |                        |
    |              | compare_exchange_weak(0, 1)     |                        |
    |              |--------------------------------------------------------->|         
    |              |                |                | [CAS: 0==0?]           |
    |              |                |                | YES, set to 1          |
    |              |<---------------------------------------------------------| true 
    |              | break          |                |                        |
    |              |                |                |                        |
    |              |             [while: 0 < 100]    |                        |
    |              |                |                |                        |
    |              |                | compare_exchange_weak(0, 1)             |
    |              |                |---------------------------------------->|
    |              |                |                |         [CAS: 1==0?]   |
    |              |                |                |         NO, current=1  |
    |              |                |<----------------------------------------| false
    |              |                | current=1      |                        | = 1
    |              |                |                |                        |
    |              |                |         [while: 1 < 100]                |
    |              |                |                |                        |
    |              |                | compare_exchange_weak(1, 2)             |
    |              |                |---------------------------------------->|
    |              |                |                | [CAS: 1==1?]           |
    |              |                |                | YES, set to 2          |
    |              |                |<----------------------------------------| true
    |              |                | break          |                        | = 2
    |              |                |                |                        |
    |              |                |           [while: 0 < 100]              |
    |              |                |                |                        |
    |              |                |                | compare_exchange_weak(0, 1)
    |              |                |                |----------------------->|
    |              |                |                | [CAS: 2==0?]           |
    |              |                |                | NO, current=2          |
    |              |                |                |<-----------------------| false
    |              |                |                | current=2              | = 2
    |              |                |                |                        |
    |              |                |          [while: 2 < 100]               |
    |              |                |                |                        |
    |              |                |                | compare_exchange_weak(2, 3)
    |              |                |                |----------------------->|
    |              |                |                | [CAS: 2==2?]           |
    |              |                |                | YES, set to 3          |
    |              |                |                |<-----------------------| true
    |              |                |                | break                  | = 3
    |              |                |                |                        |
    |              |                |                |                        |
    |         ... (continues until atomic_value reaches 100) ...              |
    |              |                |                |                        |
    |              |                |                |                        | = 99
    |              |                |                |                        |
    |              | load()         |                |                        |
    |              |--------------------------------------------------------->|
    |              | current=99     |                |                        |
    |              |                |                |                        |
    |        [while: 99 < 100]      |                |                        |
    |              |                |                |                        |
    |              | compare_exchange_weak(99, 100)  |                        |
    |              |--------------------------------------------------------->|
    |              |                |                | [CAS: 99==99?]         |
    |              |                |                | YES, set to 100        |
    |              |<---------------------------------------------------------| true     
    |              | break          |                |                        | = 100
    |              X                |                |                        |
    |              |                |                |                        |
    |              |             load()              |                        |
    |              |                |---------------------------------------->|
    |              |                | current=100    |                        |
    |              |                |                |                        |
    |              |     [while: 100 < 100 = FALSE]  |                        |
    |              |                | exit loop      |                        |
    |              |                X                |                        |
    |              |                |                |                        |
    |              |                |             load()                      |
    |              |                |                |----------------------->|
    |              |                |                | current=100            |
    |              |                |                |                        |
    |              |                |     [while: 100 < 100 = FALSE]          |
    |              |                |                | exit loop              |
    |              |                |                X                        |
    |              |                |                |                        |
    | join()------>|                |                |                        |
    |<-------------|                |                |                        |
    |              |                |                |                        |
    | join()----------------------->|                |                        |
    |<------------------------------|                |                        |
    |              |                |                |                        |
    | join()---------------------------------------->|                        |
    |<-----------------------------------------------|                        |
    |              |                |                |                        |
    | load() final value                             |                        |
    |<------------------------------------------------------------------------|  
    | 100          |                |                |                        |
    |              |                |                |                        |
    | "Final value: 100"            |                |                        |
    |              |                |                |                        |
    V

Key CAS Scenarios Shown:
------------------------
1. Thread 1: CAS(0,1) succeeds - first increment (value: 0→1)
2. Thread 2: CAS(0,1) fails because Thread 1 changed it to 1
   - 'current' auto-updates to 1, retries with CAS(1,2)
3. Thread 3: CAS(0,1) fails, updates to current=2, retries with CAS(2,3)
4. Eventually reaches 100, remaining threads exit without incrementing
```

## Lock-Free Operations

### Checking Lock-Free Capability

```cpp
#include <atomic>
#include <iostream>

int main() {
    std::atomic<int> atomic_int;
    std::atomic<double> atomic_double;
    std::atomic<long long> atomic_longlong;
    
    // Check if operations are lock-free
    std::cout << "int is lock-free: " 
              << atomic_int.is_lock_free() << std::endl;
    std::cout << "double is lock-free: " 
              << atomic_double.is_lock_free() << std::endl;
    std::cout << "long long is lock-free: " 
              << atomic_longlong.is_lock_free() << std::endl;
    
    // Compile-time check (C++17)
    if constexpr (std::atomic<int>::is_always_lock_free) {
        std::cout << "int is ALWAYS lock-free on this platform" << std::endl;
    }
    
    return 0;
}
```

### Lock-Free Stack Example

```cpp
// Compile: g++ -pthread --std=c++20 lock_free_stack.cpp -o app -latomic

// Lock-free stack implementation with safe memory reclamation
// Solves ABA problem and prevents use-after-free bugs in concurrent access

#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <array>
#include <algorithm>

template<typename T>
class LockFreeStack {
private:
    // Stack node structure
    // Contains data and pointer to next node in the stack
    struct Node {
        T data;                      // User data stored in this node
        Node* next;                  // Pointer to next node (towards bottom of stack)
        std::atomic<int> ref_count;  // Reference counter (currently unused, kept for future extensions)
        
        Node(const T& value) : data(value), next(nullptr), ref_count(0) {}
    };
    
    // ===== HAZARD POINTER SYSTEM =====
    // Hazard pointers prevent premature deletion of nodes that other threads might be accessing
    // Each thread can "claim" a node by storing its address in a hazard pointer slot
    // A node can only be deleted when no hazard pointer references it
    
    static constexpr size_t MAX_THREADS = 128;         // Maximum concurrent threads supported
    static constexpr size_t HAZARDS_PER_THREAD = 1;    // Number of hazard pointers per thread
    
    // Hazard pointer structure - one slot to protect one node at a time
    struct HazardPointer {
        std::atomic<Node*> pointer;  // Atomic pointer to the node being protected
        HazardPointer() : pointer(nullptr) {}
    };
    
    // Global array of hazard pointers shared by all threads
    // Each thread gets HAZARDS_PER_THREAD consecutive slots
    static std::array<HazardPointer, MAX_THREADS * HAZARDS_PER_THREAD> hazard_pointers;
    
    // ===== RETIRED NODES LIST =====
    // Nodes that have been popped but can't be deleted yet (might still be accessed)
    // These are stored in a thread-local list and deleted when safe
    
    struct RetiredNode {
        Node* node;           // Pointer to the retired node
        RetiredNode* next;    // Next retired node in the list
        RetiredNode(Node* n) : node(n), next(nullptr) {}
    };
    
    // Thread-local storage for retired nodes (each thread maintains its own list)
    thread_local static RetiredNode* retired_list;   // Head of retired nodes list
    thread_local static size_t retired_count;        // Count of retired nodes
    
    // When retired list reaches this threshold, trigger garbage collection
    static constexpr size_t RETIRED_THRESHOLD = 10;
    
    // ===== TAGGED POINTER FOR ABA PREVENTION =====
    // Combines a pointer with a version tag to prevent the ABA problem
    // ABA Problem: Thread T1 reads A, gets suspended. Thread T2 changes A→B→A.
    // T1 resumes and CAS succeeds thinking nothing changed, but A is different now.
    // Solution: Increment tag on every change, so A(tag=1) != A(tag=2)
    
    struct TaggedPointer {
        Node* ptr;        // Actual pointer to the top node
        uintptr_t tag;    // Version counter, incremented on each modification
        
        TaggedPointer() : ptr(nullptr), tag(0) {}
        TaggedPointer(Node* p, uintptr_t t) : ptr(p), tag(t) {}
        
        // Two tagged pointers are equal only if both pointer AND tag match
        bool operator==(const TaggedPointer& other) const {
            return ptr == other.ptr && tag == other.tag;
        }
    };
    
    // Atomic head pointer with tag - the core of the lock-free stack
    std::atomic<TaggedPointer> head;
    
    // ===== HELPER FUNCTIONS =====
    
    // Assign and retrieve a hazard pointer slot for the current thread
    // Each thread gets a unique slot to claim nodes it's accessing
    HazardPointer* get_hazard_pointer() {
        // Thread-local variables to cache thread ID (avoids repeated atomic operations)
        static thread_local size_t thread_id = 0;
        static thread_local bool initialized = false;
        
        if (!initialized) {
            // First time this thread calls - assign it a unique ID
            thread_id = next_thread_id.fetch_add(1);
            initialized = true;
        }
        
        // Return pointer to this thread's hazard pointer slot
        return &hazard_pointers[thread_id * HAZARDS_PER_THREAD];
    }
    
    // Atomic counter for assigning unique thread IDs
    static std::atomic<size_t> next_thread_id;
    
    // Check if any thread is currently protecting this node via a hazard pointer
    // Returns true if node is "hazardous" (unsafe to delete)
    bool is_hazardous(Node* node) {
        // Scan all hazard pointer slots across all threads
        for (size_t i = 0; i < MAX_THREADS * HAZARDS_PER_THREAD; ++i) {
            if (hazard_pointers[i].pointer.load() == node) {
                return true;  // At least one thread is accessing this node
            }
        }
        return false;  // No thread is protecting this node - safe to delete
    }
    
    // Add a popped node to the retired list for later deletion
    // We can't delete immediately because another thread might still be accessing it
    void retire_node(Node* node) {
        // Create a retired node wrapper
        RetiredNode* retired = new RetiredNode(node);
        
        // Add to front of thread-local retired list
        retired->next = retired_list;
        retired_list = retired;
        retired_count++;
        
        // Periodically clean up retired nodes to prevent unbounded memory growth
        if (retired_count >= RETIRED_THRESHOLD) {
            scan_and_delete();
        }
    }
    
    // Scan the retired list and delete nodes that are no longer protected
    // This is the garbage collection mechanism for our lock-free structure
    void scan_and_delete() {
        RetiredNode* current = retired_list;
        RetiredNode* prev = nullptr;
        
        // Walk through the retired list
        while (current) {
            // Check if this node is still being accessed by any thread
            if (!is_hazardous(current->node)) {
                // Safe to delete - no thread has a hazard pointer to it
                Node* to_delete = current->node;
                RetiredNode* to_free = current;
                
                // Remove from retired list
                if (prev) {
                    prev->next = current->next;
                    current = current->next;
                } else {
                    retired_list = current->next;
                    current = retired_list;
                }
                
                // Actually free the memory
                delete to_delete;      // Delete the stack node
                delete to_free;        // Delete the retired node wrapper
                retired_count--;
            } else {
                // Still hazardous - keep in retired list and try again later
                prev = current;
                current = current->next;
            }
        }
    }
    
public:
    // Constructor - initialize with empty stack
    LockFreeStack() {
        head.store(TaggedPointer(nullptr, 0));
    }
    
    // ===== PUSH OPERATION =====
    // Thread-safe push operation using compare-and-swap (CAS) with ABA protection
    // Multiple threads can push concurrently without locks
    void push(const T& value) {
        // Allocate new node on heap
        Node* new_node = new Node(value);
        
        // Load current head with acquire semantics
        // acquire ensures we see all previous writes to nodes in the stack
        TaggedPointer old_head = head.load(std::memory_order_acquire);
        
        // Retry loop - keep trying until we successfully update head
        do {
            // Make new node point to current top of stack
            new_node->next = old_head.ptr;
            
            // Create new head with incremented tag to prevent ABA problem
            // Even if we pop this node and push it again, the tag will be different
            TaggedPointer new_head(new_node, old_head.tag + 1);
            
            // Attempt atomic compare-and-swap:
            // - If head still equals old_head, update to new_head and return
            // - If head changed, old_head is updated with current value, retry
            // release ensures all writes to new_node are visible before head is updated
            if (head.compare_exchange_weak(old_head, new_head,
                                          std::memory_order_release,
                                          std::memory_order_acquire)) {
                return;  // Success! Node is now on the stack
            }
            // CAS failed - another thread modified head, old_head is updated, loop again
        } while (true);
    }
    
    // ===== POP OPERATION =====
    // Thread-safe pop with hazard pointers for safe memory reclamation
    // Returns true and sets result if pop succeeded, false if stack was empty
    bool pop(T& result) {
        // Get this thread's hazard pointer slot
        HazardPointer* hp = get_hazard_pointer();
        TaggedPointer old_head;
        
        // Retry loop until we successfully pop or determine stack is empty
        do {
            // Load current head with acquire semantics
            old_head = head.load(std::memory_order_acquire);
            
            // Check if stack is empty
            if (!old_head.ptr) {
                // Clear any previous hazard pointer and return false
                hp->pointer.store(nullptr, std::memory_order_release);
                return false;
            }
            
            // CRITICAL: Protect the node with hazard pointer BEFORE accessing it
            // This tells other threads "don't delete this node, I'm using it"
            hp->pointer.store(old_head.ptr, std::memory_order_release);
            
            // Double-check that head wasn't changed while we set the hazard pointer
            // This handles the race: T1 loads head, T2 pops and deletes it, T1 sets hazard
            // Without this check, T1 would protect an already-deleted node
            TaggedPointer current_head = head.load(std::memory_order_acquire);
            if (current_head.ptr != old_head.ptr) {
                // Head changed between load and hazard pointer set - retry
                continue;
            }
            
            // Node is now safely protected - we can access it
            Node* next = old_head.ptr->next;
            
            // Create new head pointing to next node, with incremented tag
            TaggedPointer new_head(next, old_head.tag + 1);
            
            // Attempt to atomically update head to remove top node
            if (head.compare_exchange_weak(old_head, new_head,
                                          std::memory_order_release,
                                          std::memory_order_acquire)) {
                // Successfully popped! Copy data from node (still protected)
                result = old_head.ptr->data;
                
                // Clear our hazard pointer - we're done accessing this node
                hp->pointer.store(nullptr, std::memory_order_release);
                
                // Retire the node for deferred deletion
                // Can't delete immediately - other threads might still have hazard pointers to it
                retire_node(old_head.ptr);
                return true;
            }
            // CAS failed - another thread modified head, retry
        } while (true);
        
        return false;  // Unreachable, but keeps compiler happy
    }
    
    // ===== DESTRUCTOR =====
    // Clean up all remaining nodes in stack and retired lists
    ~LockFreeStack() {
        // Pop and discard all remaining elements from the stack
        T dummy;
        while (pop(dummy)) {}
        
        // Attempt to clean up retired nodes normally
        scan_and_delete();
        
        // Force delete any remaining retired nodes
        // At this point no other threads should be accessing the stack
        RetiredNode* current = retired_list;
        while (current) {
            RetiredNode* next = current->next;
            delete current->node;      // Delete the stack node
            delete current;            // Delete the retired node wrapper
            current = next;
        }
    }
};

// ===== STATIC MEMBER INITIALIZATION =====
// These must be defined outside the class template

// Global hazard pointer array - shared across all threads
template<typename T>
std::array<typename LockFreeStack<T>::HazardPointer, 
           LockFreeStack<T>::MAX_THREADS * LockFreeStack<T>::HAZARDS_PER_THREAD> 
LockFreeStack<T>::hazard_pointers;

// Atomic counter for assigning thread IDs
template<typename T>
std::atomic<size_t> LockFreeStack<T>::next_thread_id{0};

// Thread-local retired list head - each thread maintains its own
template<typename T>
thread_local typename LockFreeStack<T>::RetiredNode* LockFreeStack<T>::retired_list = nullptr;

// Thread-local count of retired nodes
template<typename T>
thread_local size_t LockFreeStack<T>::retired_count = 0;



// ===== TEST PROGRAM =====
int main() {
    LockFreeStack<int> stack;
    std::vector<std::thread> threads;
    
    std::cout << "Starting concurrent push operations..." << std::endl;
    
    // ===== TEST 1: Concurrent Pushes =====
    // Spawn 5 producer threads, each pushing 10 values
    // This tests the thread-safety of the push operation
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back([&stack, i]() {
            for (int j = 0; j < 10; ++j) {
                // Each thread pushes values like: 0-9, 10-19, 20-29, 30-39, 40-49
                stack.push(i * 10 + j);
            }
        });
    }
    
    // Wait for all producer threads to complete
    for (auto& t : threads) {
        t.join();
    }
    threads.clear();
    
    std::cout << "Push operations completed." << std::endl;
    std::cout << "Starting concurrent pop operations..." << std::endl;
    
    // ===== TEST 2: Concurrent Pops =====
    // Test that multiple threads can safely pop from the same stack
    // This stresses the hazard pointer system and memory reclamation
    std::atomic<int> pop_count{0};
    for (int i = 0; i < 3; ++i) {
        threads.emplace_back([&stack, &pop_count]() {
            int value;
            // Each thread pops until the stack is empty
            while (stack.pop(value)) {
                pop_count++;  // Count successful pops
            }
        });
    }
    
    // Wait for all consumer threads to complete
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Successfully popped " << pop_count.load() 
              << " elements from stack." << std::endl;
    
    // ===== VERIFICATION =====
    // Verify that the stack is now empty
    int value;
    if (!stack.pop(value)) {
        std::cout << "Stack is now empty (as expected)." << std::endl;
    }
    
    // When main() exits, ~LockFreeStack() will clean up any remaining memory
    return 0;
}
```

## Memory Ordering

Atomic operations support different memory ordering semantics:

```cpp
#include <atomic>
#include <thread>
#include <cassert>

// Atomic flag to signal when data is ready for consumption
std::atomic<bool> ready(false);

// Atomic integer to hold the shared data
std::atomic<int> data(0);

void producer() {
    // Write data with relaxed ordering (no synchronization guarantees on its own)
    // This is safe because the release fence below will ensure visibility
    data.store(42, std::memory_order_relaxed);
    
    // Release fence: ensures all previous writes (including data.store above)
    // are visible to any thread that performs an acquire operation on 'ready'
    // This creates a happens-before relationship with the consumer
    ready.store(true, std::memory_order_release);
}

void consumer() {
    // Acquire fence: synchronizes with the release store in producer
    // Once this load sees 'true', all writes that happened-before the
    // release store (including data.store(42)) are guaranteed visible
    while (!ready.load(std::memory_order_acquire)) {
        // Spin-wait until producer signals ready
        // Note: In production code, consider using std::this_thread::yield()
        // or a condition variable to reduce CPU usage
    }
    
    // By this point, we're guaranteed to see data == 42 due to the
    // acquire-release synchronization above, even though we use relaxed ordering here
    assert(data.load(std::memory_order_relaxed) == 42);
}

int main() {
    // Create producer thread
    std::thread t1(producer);
    
    // Create consumer thread
    std::thread t2(consumer);
    
    // Wait for both threads to complete
    t1.join();
    t2.join();
    
    return 0;
}
```

**Key concepts illustrated:**
- **Release-Acquire ordering**: Creates synchronization between threads without the overhead of sequential consistency
- **Relaxed ordering**: Used for `data` because the release-acquire pair on `ready` provides the necessary synchronization
- **Happens-before relationship**: The release store guarantees that all prior writes are visible after the acquire load succeeds


### Memory Order Options

- **memory_order_relaxed**: No synchronization, only atomicity guaranteed
- **memory_order_acquire**: Synchronizes with a release operation
- **memory_order_release**: Synchronizes with an acquire operation
- **memory_order_acq_rel**: Both acquire and release semantics
- **memory_order_seq_cst**: Sequential consistency (default, strongest)

## Practical Example: Thread-Safe Counter

```cpp
#include <atomic>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>

/**
 * Thread-safe counter using atomic operations
 * Demonstrates relaxed memory ordering for independent atomic operations
 */
class ThreadSafeCounter {
private:
    // Atomic integer for lock-free, thread-safe counting
    // Note: atomic<long long> ensures operations are indivisible across threads
    std::atomic<long long> value;
    
public:
    // Initialize counter to zero
    ThreadSafeCounter() : value(0) {}
    
    /**
     * Atomically increment the counter by 1
     * Uses relaxed ordering because:
     * - We only need atomicity of the increment itself
     * - We don't need synchronization with other variables
     * - Order of increments across threads doesn't matter for correctness
     */
    void increment() {
        value.fetch_add(1, std::memory_order_relaxed);
    }
    
    /**
     * Atomically decrement the counter by 1
     * Relaxed ordering is sufficient for the same reasons as increment
     */
    void decrement() {
        value.fetch_sub(1, std::memory_order_relaxed);
    }
    
    /**
     * Read the current counter value
     * Relaxed ordering is safe because:
     * - We're only reading a single atomic variable
     * - No happens-before relationship is required
     * - The returned value represents some valid state of the counter
     *   (though it may be immediately outdated in concurrent scenarios)
     */
    long long get() const {
        return value.load(std::memory_order_relaxed);
    }
    
    /**
     * Atomically reset counter to 0 and return the previous value
     * exchange() is atomic: read old value, write new value, return old value
     * Relaxed ordering is appropriate as this is a single atomic operation
     */
    long long reset() {
        return value.exchange(0, std::memory_order_relaxed);
    }
};

int main() {
    ThreadSafeCounter counter;
    std::vector<std::thread> threads;
    
    // Start timing the concurrent operations
    auto start = std::chrono::high_resolution_clock::now();
    
    // Spawn 10 threads, each incrementing the counter 100,000 times
    // Total expected increments: 10 × 100,000 = 1,000,000
    for (int i = 0; i < 10; ++i) {
        threads.emplace_back([&counter]() {
            // Each thread performs 100,000 increments
            // The atomic operations guarantee no lost updates despite
            // multiple threads modifying the same variable concurrently
            for (int j = 0; j < 100000; ++j) {
                counter.increment();
            }
        });
    }
    
    // Wait for all threads to complete their work
    // join() blocks until the thread finishes execution
    for (auto& t : threads) {
        t.join();
    }
    
    // Stop timing
    auto end = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    // Display results
    // The final count should always be exactly 1,000,000
    // because atomic operations prevent race conditions
    std::cout << "Final count: " << counter.get() << std::endl;
    std::cout << "Expected: 1000000" << std::endl;
    std::cout << "Time: " << duration.count() << "ms" << std::endl;
    
    return 0;
}
```

## Best Practices

### 1. Use Atomic Types for Simple Shared State

```cpp
// Good: Simple flag
std::atomic<bool> shutdown_requested(false);

// Avoid: Complex types (may not be lock-free)
struct BigStruct { int data[100]; };
std::atomic<BigStruct> big_atomic;  // Likely uses locks internally
```

### 2. Choose Appropriate Memory Ordering

```cpp
// For independent operations: relaxed
std::atomic<int> counter(0);
counter.fetch_add(1, std::memory_order_relaxed);

// For synchronization: acquire-release
std::atomic<bool> data_ready(false);
data_ready.store(true, std::memory_order_release);
while (!data_ready.load(std::memory_order_acquire)) {}

// When in doubt: sequential consistency (default)
counter.store(value);  // Uses memory_order_seq_cst
```

### 3. Verify Lock-Free Behavior

```cpp
// Check at runtime for critical paths
if (!my_atomic.is_lock_free()) {
    std::cerr << "Warning: atomic operations are not lock-free!" << std::endl;
}
```

## Performance Considerations

### Atomic vs Mutex Comparison

```cpp
#include <atomic>
#include <mutex>
#include <chrono>
#include <iostream>

// Atomic version
std::atomic<long> atomic_counter(0);

void atomic_increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        atomic_counter.fetch_add(1, std::memory_order_relaxed);
    }
}

// Mutex version
long mutex_counter = 0;
std::mutex counter_mutex;

void mutex_increment(int iterations) {
    for (int i = 0; i < iterations; ++i) {
        std::lock_guard<std::mutex> lock(counter_mutex);
        ++mutex_counter;
    }
}

// Atomics are typically 2-10x faster for simple operations
```

## Common Pitfalls

### 1. False Sharing

```cpp
// Bad: Atomic variables on same cache line
struct Counters {
    std::atomic<int> counter1;  // Same cache line
    std::atomic<int> counter2;  // Causes false sharing
};

// Good: Pad to separate cache lines
struct PaddedCounters {
    alignas(64) std::atomic<int> counter1;
    alignas(64) std::atomic<int> counter2;
};
```

### 2. Not Understanding Memory Ordering

```cpp
// Problematic code
std::atomic<int> x(0), y(0);

// Thread 1
x.store(1, std::memory_order_relaxed);
y.store(1, std::memory_order_relaxed);

// Thread 2
while (y.load(std::memory_order_relaxed) == 0) {}
assert(x.load(std::memory_order_relaxed) == 1);  // May fail!
```

## Summary

**Atomic types** provide thread-safe operations without explicit locks, enabling efficient concurrent programming:

- **std::atomic<T>** guarantees indivisible operations on primitive types
- **Lock-free operations** avoid kernel-level synchronization overhead when supported
- **Read-modify-write operations** like `fetch_add`, `compare_exchange` enable complex atomic updates
- **Memory ordering** controls visibility and ordering guarantees between threads
- **Performance benefit** significant for high-contention scenarios with simple operations

Atomic types are ideal for flags, counters, and simple synchronization primitives. For complex shared data structures, consider higher-level synchronization primitives or lock-free data structures built on atomic operations.

**Key Takeaway**: Use atomic types for lock-free coordination of simple shared state between threads, but understand their memory ordering semantics and verify lock-free behavior for performance-critical code.