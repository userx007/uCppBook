// Lock-free stack implementation with safe memory reclamation
// Solves ABA problem and prevents use-after-free bugs in concurrent access

/*

g++ -pthread --std=c++20 lock_free_stack.cpp -o app -latomic

*/

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