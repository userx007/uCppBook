# Guide to C++ containers

## Container Categories:

**Sequential Containers** (store elements in sequence):
- **Vector**: Most commonly used, efficient random access
- **List**: Best for frequent insertions/deletions in the middle
- **Deque**: Efficient operations at both ends
- **Array**: Fixed size, compile-time constant

**Associative Containers** (store sorted key-value or key-only data):
- **Map**: Ordered key-value pairs, maintains sorted order
- **Set**: Ordered unique values

**Unordered Associative** (hash-based, faster but unordered):
- **Unordered_map**: Hash table for key-value pairs
- **Unordered_set**: Hash table for unique values

**Container Adaptors** (provide specific interfaces):
- **Stack**: LIFO (Last-In-First-Out)
- **Queue**: FIFO (First-In-First-Out)
- **Priority_queue**: Heap-based priority ordering

**Special Mention**:
- **Variant**: Not actually a container, but a type-safe union that can hold one of several types

## Quick Selection Guide:

- **Need random access?** → Use `vector`
- **Inserting/deleting in middle often?** → Use `list`
- **Need fast lookups by key?** → Use `unordered_map` (or `map` if you need ordering)
- **Need unique elements in order?** → Use `set`
- **LIFO/FIFO behavior?** → Use `stack`/`queue`
- **Need priority ordering?** → Use `priority_queue`

The code is fully compilable and demonstrates all major operations for each container type!

```cpp
#include <iostream>
#include <vector>
#include <list>
#include <map>
#include <unordered_map>
#include <set>
#include <queue>
#include <stack>
#include <deque>
#include <array>
#include <variant>
#include <string>

using namespace std;

// ============================================================================
// 1. VECTOR - Dynamic Array
// ============================================================================
void vectorExample() {
    cout << "\n=== VECTOR (Dynamic Array) ===\n";

    // Declaration and initialization
    vector<int> v1;                        // Empty vector
    vector<int> v2(5, 10);                 // 5 elements, all value 10
    vector<int> v3 = {1, 2, 3, 4, 5};      // Initializer list

    // Adding elements
    v1.push_back(100);
    v1.push_back(200);
    v1.emplace_back(300);  // Constructs in-place (more efficient)

    // Accessing elements
    cout << "First element: " << v1[0] << endl;
    cout << "Last element: " << v1.back() << endl;
    cout << "Using at(): " << v1.at(1) << endl;  // Bounds checking

    // Size and capacity
    cout << "Size: " << v1.size() << endl;
    cout << "Capacity: " << v1.capacity() << endl;

    // Iterating
    cout << "Elements: ";
    for (const auto& val : v1) {
        cout << val << " ";
    }
    cout << endl;

    // Inserting and erasing
    v1.insert(v1.begin() + 1, 150);  // Insert 150 at position 1
    v1.erase(v1.begin());            // Remove first element

    // When to use: Random access needed, frequent push_back operations
    // Time Complexity: Access O(1), Insert/Delete at end O(1), Insert/Delete middle O(n)
}

// ============================================================================
// 2. LIST - Doubly Linked List
// ============================================================================
void listExample() {
    cout << "\n=== LIST (Doubly Linked List) ===\n";

    list<int> lst = {10, 20, 30, 40, 50};

    // Adding elements
    lst.push_front(5);   // Add to front
    lst.push_back(60);   // Add to back

    // Accessing (no random access with [])
    cout << "First: " << lst.front() << endl;
    cout << "Last: " << lst.back() << endl;

    // Iterating
    cout << "Elements: ";
    for (const auto& val : lst) {
        cout << val << " ";
    }
    cout << endl;

    // Inserting in middle
    auto it = lst.begin();
    advance(it, 3);
    lst.insert(it, 25);

    // Removing elements
    lst.remove(30);  // Remove all elements with value 30
    lst.pop_front();
    lst.pop_back();

    // Sorting
    lst.sort();

    // When to use: Frequent insertions/deletions in middle, no random access needed
    // Time Complexity: Access O(n), Insert/Delete O(1) with iterator
}

// ============================================================================
// 3. DEQUE - Double-Ended Queue
// ============================================================================
void dequeExample() {
    cout << "\n=== DEQUE (Double-Ended Queue) ===\n";

    deque<int> dq = {10, 20, 30};

    // Add to both ends efficiently
    dq.push_front(5);
    dq.push_back(40);

    // Random access like vector
    cout << "Element at index 2: " << dq[2] << endl;

    // Remove from both ends
    dq.pop_front();
    dq.pop_back();

    cout << "Elements: ";
    for (const auto& val : dq) {
        cout << val << " ";
    }
    cout << endl;

    // When to use: Need efficient insertion/deletion at both ends with random access
    // Time Complexity: Access O(1), Insert/Delete at ends O(1)
}

// ============================================================================
// 4. MAP - Ordered Key-Value Pairs (Red-Black Tree)
// ============================================================================
void mapExample() {
    cout << "\n=== MAP (Ordered Associative Container) ===\n";

    map<string, int> ages;

    // Inserting elements
    ages["Alice"] = 25;
    ages["Bob"] = 30;
    ages.insert({"Charlie", 28});
    ages.emplace("David", 35);

    // Accessing elements
    cout << "Alice's age: " << ages["Alice"] << endl;

    // Check if key exists
    if (ages.find("Bob") != ages.end()) {
        cout << "Bob found with age: " << ages["Bob"] << endl;
    }

    // Iterating (sorted by key)
    cout << "All ages (sorted by name):\n";
    for (const auto& [name, age] : ages) {
        cout << name << ": " << age << endl;
    }

    // Erasing
    ages.erase("Charlie");

    // Count elements with key
    cout << "Count of 'Alice': " << ages.count("Alice") << endl;

    // When to use: Need ordered key-value pairs, frequent lookups
    // Time Complexity: Insert/Delete/Find O(log n)
}

// ============================================================================
// 5. UNORDERED_MAP - Hash Table
// ============================================================================
void unorderedMapExample() {
    cout << "\n=== UNORDERED_MAP (Hash Table) ===\n";

    unordered_map<string, int> scores;

    // Inserting
    scores["Alice"] = 95;
    scores["Bob"] = 87;
    scores["Charlie"] = 92;

    // Fast lookup
    cout << "Bob's score: " << scores["Bob"] << endl;

    // Iterating (no guaranteed order)
    cout << "All scores (unordered):\n";
    for (const auto& [name, score] : scores) {
        cout << name << ": " << score << endl;
    }

    // Check existence
    if (scores.find("David") == scores.end()) {
        cout << "David not found\n";
    }

    // When to use: Need fast lookup, order doesn't matter
    // Time Complexity: Insert/Delete/Find O(1) average, O(n) worst case
}

// ============================================================================
// 6. SET - Ordered Unique Elements
// ============================================================================
void setExample() {
    cout << "\n=== SET (Ordered Unique Elements) ===\n";

    set<int> s = {5, 2, 8, 2, 1, 8};  // Duplicates automatically removed

    // Insert
    s.insert(3);
    s.insert(5);  // Won't be added (duplicate)

    // Check if element exists
    if (s.find(3) != s.end()) {
        cout << "3 is in the set\n";
    }

    // Iterate (sorted order)
    cout << "Set elements (sorted): ";
    for (const auto& val : s) {
        cout << val << " ";
    }
    cout << endl;

    // Remove
    s.erase(2);

    // When to use: Need unique elements in sorted order
    // Time Complexity: Insert/Delete/Find O(log n)
}

// ============================================================================
// 7. STACK - LIFO (Last In First Out)
// ============================================================================
void stackExample() {
    cout << "\n=== STACK (LIFO) ===\n";

    stack<int> stk;

    // Push elements
    stk.push(10);
    stk.push(20);
    stk.push(30);

    cout << "Top element: " << stk.top() << endl;
    cout << "Size: " << stk.size() << endl;

    // Pop and display
    cout << "Popping elements: ";
    while (!stk.empty()) {
        cout << stk.top() << " ";
        stk.pop();
    }
    cout << endl;

    // When to use: Need LIFO behavior (undo operations, expression evaluation)
}

// ============================================================================
// 8. QUEUE - FIFO (First In First Out)
// ============================================================================
void queueExample() {
    cout << "\n=== QUEUE (FIFO) ===\n";

    queue<string> q;

    // Enqueue
    q.push("First");
    q.push("Second");
    q.push("Third");

    cout << "Front: " << q.front() << endl;
    cout << "Back: " << q.back() << endl;

    // Dequeue and display
    cout << "Processing queue: ";
    while (!q.empty()) {
        cout << q.front() << " ";
        q.pop();
    }
    cout << endl;

    // When to use: Need FIFO behavior (task scheduling, BFS)
}

// ============================================================================
// 9. PRIORITY_QUEUE - Heap
// ============================================================================
void priorityQueueExample() {
    cout << "\n=== PRIORITY_QUEUE (Max Heap by default) ===\n";

    priority_queue<int> pq;

    // Insert elements
    pq.push(30);
    pq.push(10);
    pq.push(50);
    pq.push(20);

    cout << "Processing by priority: ";
    while (!pq.empty()) {
        cout << pq.top() << " ";
        pq.pop();
    }
    cout << endl;

    // Min heap version
    priority_queue<int, vector<int>, greater<int>> minHeap;
    minHeap.push(30);
    minHeap.push(10);
    minHeap.push(50);

    cout << "Min heap: ";
    while (!minHeap.empty()) {
        cout << minHeap.top() << " ";
        minHeap.pop();
    }
    cout << endl;

    // When to use: Need elements in priority order (Dijkstra's, task scheduling)
}

// ============================================================================
// 10. ARRAY - Fixed-Size Array
// ============================================================================
void arrayExample() {
    cout << "\n=== ARRAY (Fixed Size) ===\n";

    array<int, 5> arr = {1, 2, 3, 4, 5};

    // Access
    cout << "First: " << arr[0] << endl;
    cout << "Last: " << arr.back() << endl;

    // Size is compile-time constant
    cout << "Size: " << arr.size() << endl;

    // Fill all with value
    arr.fill(10);

    cout << "After fill: ";
    for (const auto& val : arr) {
        cout << val << " ";
    }
    cout << endl;

    // When to use: Fixed size known at compile time, need bounds checking
}

// ============================================================================
// 11. VARIANT - Type-Safe Union
// ============================================================================
void variantExample() {
    cout << "\n=== VARIANT (Type-Safe Union) ===\n";

    // Can hold int, double, or string
    variant<int, double, string> v;

    v = 42;
    cout << "Holds int: " << get<int>(v) << endl;

    v = 3.14;
    cout << "Holds double: " << get<double>(v) << endl;

    v = "Hello";
    cout << "Holds string: " << get<string>(v) << endl;

    // Check which type is held
    if (holds_alternative<string>(v)) {
        cout << "Currently holds a string\n";
    }

    // Visit pattern
    auto visitor = [](auto&& arg) {
        using T = decay_t<decltype(arg)>;
        if constexpr (is_same_v<T, int>)
            cout << "Visitor found int: " << arg << endl;
        else if constexpr (is_same_v<T, double>)
            cout << "Visitor found double: " << arg << endl;
        else if constexpr (is_same_v<T, string>)
            cout << "Visitor found string: " << arg << endl;
    };

    visit(visitor, v);

    // When to use: Need a variable that can be one of several types
    // Note: Variant is NOT a container but a type-safe union
}

// ============================================================================
// MAIN - Run all examples
// ============================================================================
int main() {
    cout << "C++ CONTAINERS COMPREHENSIVE GUIDE\n";
    cout << "==================================\n";

    vectorExample();
    listExample();
    dequeExample();
    mapExample();
    unorderedMapExample();
    setExample();
    stackExample();
    queueExample();
    priorityQueueExample();
    arrayExample();
    variantExample();

    cout << "\n=== QUICK REFERENCE ===\n";
    cout << "Vector: Dynamic array, random access O(1)\n";
    cout << "List: Doubly linked list, insert/delete O(1) with iterator\n";
    cout << "Deque: Double-ended queue, efficient at both ends\n";
    cout << "Map: Ordered key-value, search O(log n)\n";
    cout << "Unordered_map: Hash table, search O(1) average\n";
    cout << "Set: Ordered unique elements, search O(log n)\n";
    cout << "Stack: LIFO, push/pop O(1)\n";
    cout << "Queue: FIFO, push/pop O(1)\n";
    cout << "Priority_queue: Heap, top element in O(1)\n";
    cout << "Array: Fixed size, bounds checking\n";
    cout << "Variant: Type-safe union, not a container\n";

    return 0;
}
```
---

# std::vector

## Construction and Initialization

```cpp
#include <vector>

// Empty vector
std::vector<int> v1;

// Vector with size
std::vector<int> v2(5);  // 5 elements, default-initialized to 0

// Vector with size and value
std::vector<int> v3(5, 10);  // {10, 10, 10, 10, 10}

// Initializer list
std::vector<int> v4 = {1, 2, 3, 4, 5};

// Copy constructor
std::vector<int> v5(v4);

// Range constructor
std::vector<int> v6(v4.begin(), v4.end());
```

## Capacity Methods

```cpp
std::vector<int> v = {1, 2, 3};

v.size();      // Returns 3 (number of elements)
v.capacity();  // Returns capacity (may be >= size)
v.empty();     // Returns false
v.max_size();  // Returns maximum possible size

v.reserve(100);  // Reserves space for 100 elements
v.shrink_to_fit();  // Reduces capacity to fit size
```

## Element Access

```cpp
std::vector<int> v = {10, 20, 30, 40, 50};

v[2];        // Returns 30 (no bounds checking)
v.at(2);     // Returns 30 (with bounds checking, throws if out of range)
v.front();   // Returns 10 (first element)
v.back();    // Returns 50 (last element)
v.data();    // Returns pointer to underlying array
```

## Modifiers - Adding Elements

```cpp
std::vector<int> v = {1, 2, 3};

v.push_back(4);  // Adds 4 to end: {1, 2, 3, 4}
v.emplace_back(5);  // Constructs 5 in-place at end: {1, 2, 3, 4, 5}

v.insert(v.begin() + 1, 99);  // Insert 99 at position 1: {1, 99, 2, 3, 4, 5}
v.insert(v.end(), 3, 7);  // Insert three 7s at end: {1, 99, 2, 3, 4, 5, 7, 7, 7}

std::vector<int> extra = {100, 200};
v.insert(v.begin(), extra.begin(), extra.end());  // Insert range at beginning
```

## Modifiers - Removing Elements

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};

v.pop_back();  // Removes last element: {1, 2, 3, 4}
v.erase(v.begin() + 1);  // Removes element at index 1: {1, 3, 4}
v.erase(v.begin(), v.begin() + 2);  // Removes range: {4}

v.clear();  // Removes all elements (size becomes 0)
```

## Resizing

```cpp
std::vector<int> v = {1, 2, 3};

v.resize(5);  // Increases size to 5: {1, 2, 3, 0, 0}
v.resize(7, 99);  // Increases size to 7, filling with 99: {1, 2, 3, 0, 0, 99, 99}
v.resize(2);  // Decreases size to 2: {1, 2}
```

## Other Useful Methods

```cpp
std::vector<int> v1 = {1, 2, 3};
std::vector<int> v2 = {4, 5, 6};

v1.swap(v2);  // Swaps contents: v1 = {4, 5, 6}, v2 = {1, 2, 3}

v1.assign(3, 10);  // Replaces content: {10, 10, 10}
v1.assign({7, 8, 9});  // Replaces with initializer list: {7, 8, 9}
```

## Iterators

```cpp
std::vector<int> v = {1, 2, 3, 4, 5};

// Forward iteration
for (auto it = v.begin(); it != v.end(); ++it) {
    std::cout << *it << " ";
}

// Reverse iteration
for (auto it = v.rbegin(); it != v.rend(); ++it) {
    std::cout << *it << " ";
}

// Const iterators
auto cit = v.cbegin();  // const iterator
auto crit = v.crbegin();  // const reverse iterator
```

## Practical Example: Complete Program

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

int main() {
    // Create and populate vector
    std::vector<int> numbers = {5, 2, 8, 1, 9};
    
    // Add elements
    numbers.push_back(3);
    numbers.emplace_back(7);
    
    // Access elements
    std::cout << "First: " << numbers.front() << "\n";
    std::cout << "Last: " << numbers.back() << "\n";
    std::cout << "Size: " << numbers.size() << "\n";
    
    // Sort the vector
    std::sort(numbers.begin(), numbers.end());
    
    // Iterate and print
    std::cout << "Sorted: ";
    for (int n : numbers) {
        std::cout << n << " ";
    }
    std::cout << "\n";
    
    // Remove specific element
    numbers.erase(std::remove(numbers.begin(), numbers.end(), 5), numbers.end());
    
    // Check if empty
    if (!numbers.empty()) {
        std::cout << "Vector still has " << numbers.size() << " elements\n";
    }
    
    return 0;
}
```

The key differences to remember:
- `push_back()` vs `emplace_back()`: emplace constructs in-place (more efficient for complex types)
- `[]` vs `at()`: at() provides bounds checking
- `size()` vs `capacity()`: size is actual elements, capacity is allocated space
- `erase()` vs `clear()`: erase removes specific elements, clear removes all

---

# C++ std::vector<T>::erase() - All Forms

The `erase()` method has **two overloads** for removing elements from a vector:

## 1. Single Element Erase

```cpp
iterator erase(const_iterator pos);
```

Removes the element at position `pos`.

**Parameters:**
- `pos` - iterator to the element to remove

**Return value:**
- Iterator following the last removed element (or `end()` if the last element was removed)

**Example:**

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v = {10, 20, 30, 40, 50};
    
    // Erase element at index 2 (value 30)
    auto it = v.erase(v.begin() + 2);
    
    // v is now: {10, 20, 40, 50}
    std::cout << "After erase, next element: " << *it << "\n";  // 40
    
    // Erase the first element
    v.erase(v.begin());
    // v is now: {20, 40, 50}
    
    // Erase the last element
    v.erase(v.end() - 1);
    // v is now: {20, 40}
    
    for (int val : v) {
        std::cout << val << " ";
    }
    
    return 0;
}
```

## 2. Range Erase

```cpp
iterator erase(const_iterator first, const_iterator last);
```

Removes elements in the range `[first, last)` (first is inclusive, last is exclusive).

**Parameters:**
- `first` - iterator to the beginning of the range
- `last` - iterator to the end of the range (not included)

**Return value:**
- Iterator following the last removed element

**Example:**

```cpp
#include <iostream>
#include <vector>

int main() {
    std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // Erase elements from index 2 to 5 (removes 3, 4, 5)
    auto it = v.erase(v.begin() + 2, v.begin() + 5);
    
    // v is now: {1, 2, 6, 7, 8, 9, 10}
    std::cout << "After erase, next element: " << *it << "\n";  // 6
    
    // Erase first 2 elements
    v.erase(v.begin(), v.begin() + 2);
    // v is now: {6, 7, 8, 9, 10}
    
    // Erase last 2 elements
    v.erase(v.end() - 2, v.end());
    // v is now: {6, 7, 8}
    
    // Erase all elements
    v.erase(v.begin(), v.end());  // Same as v.clear()
    // v is now: {}
    
    std::cout << "Size: " << v.size() << "\n";  // 0
    
    return 0;
}
```

## Important Characteristics

### Iterator Invalidation
```cpp
std::vector<int> v = {1, 2, 3, 4, 5};

auto it = v.begin() + 2;  // Points to 3
v.erase(v.begin());       // Erase first element

// it is now INVALID! Don't use it.
// Always use the returned iterator:
auto valid_it = v.erase(v.begin());
```

### Complexity
- **Single element erase:** Linear in the distance to the end - O(n)
- **Range erase:** Linear in the distance to the end plus the number of elements erased - O(n)

### Safe Iteration While Erasing
```cpp
std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// WRONG: Iterator invalidation
for (auto it = v.begin(); it != v.end(); ++it) {
    if (*it % 2 == 0) {
        v.erase(it);  // Invalidates iterator!
    }
}

// CORRECT: Use returned iterator
for (auto it = v.begin(); it != v.end(); ) {
    if (*it % 2 == 0) {
        it = v.erase(it);  // Update iterator
    } else {
        ++it;
    }
}
```

## Common Patterns

### Remove-Erase Idiom
To remove all elements with a specific value:

```cpp
#include <algorithm>

std::vector<int> v = {1, 2, 3, 2, 4, 2, 5};

// Remove all occurrences of 2
v.erase(std::remove(v.begin(), v.end(), 2), v.end());
// v is now: {1, 3, 4, 5}
```

### Remove-Erase-If Idiom
To remove elements based on a condition:

```cpp
#include <algorithm>

std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Remove all even numbers
v.erase(std::remove_if(v.begin(), v.end(), 
    [](int x) { return x % 2 == 0; }), v.end());
// v is now: {1, 3, 5, 7, 9}
```

### Conditional Erase in Loop
```cpp
std::vector<std::string> words = {"apple", "banana", "apricot", "cherry", "avocado"};

// Remove all words starting with 'a'
for (auto it = words.begin(); it != words.end(); ) {
    if ((*it)[0] == 'a') {
        it = words.erase(it);
    } else {
        ++it;
    }
}
// words is now: {"banana", "cherry"}
```

### Erase Multiple Ranges
```cpp
std::vector<int> v = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};

// Erase elements 2-3 and 6-7
v.erase(v.begin() + 6, v.begin() + 8);  // Erase 7, 8
// v is now: {1, 2, 3, 4, 5, 6, 9, 10}

v.erase(v.begin() + 2, v.begin() + 4);  // Erase 3, 4
// v is now: {1, 2, 5, 6, 9, 10}
```

## Complete Example Program

```cpp
#include <iostream>
#include <vector>
#include <algorithm>

void print(const std::vector<int>& v, const std::string& label) {
    std::cout << label << ": ";
    for (int x : v) std::cout << x << " ";
    std::cout << "\n";
}

int main() {
    std::vector<int> v1 = {1, 2, 3, 4, 5};
    print(v1, "Original");
    
    // Single element erase
    v1.erase(v1.begin() + 2);
    print(v1, "After erasing index 2");
    
    std::vector<int> v2 = {1, 2, 3, 4, 5, 6, 7, 8};
    print(v2, "\nOriginal v2");
    
    // Range erase
    v2.erase(v2.begin() + 2, v2.begin() + 5);
    print(v2, "After erasing range [2, 5)");
    
    // Remove-erase idiom
    std::vector<int> v3 = {1, 2, 3, 2, 4, 2, 5};
    print(v3, "\nOriginal v3");
    v3.erase(std::remove(v3.begin(), v3.end(), 2), v3.end());
    print(v3, "After removing all 2s");
    
    // Conditional erase
    std::vector<int> v4 = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    print(v4, "\nOriginal v4");
    for (auto it = v4.begin(); it != v4.end(); ) {
        if (*it % 2 == 0) {
            it = v4.erase(it);
        } else {
            ++it;
        }
    }
    print(v4, "After removing evens");
    
    return 0;
}
```

**Key Takeaways:**
1. Always use the returned iterator from `erase()`
2. `erase()` invalidates iterators at or after the erase point
3. For multiple removals, prefer the remove-erase idiom for efficiency
4. Range erase uses half-open intervals: `[first, last)`