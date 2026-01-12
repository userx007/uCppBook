# Recursive Mutex in C++

## Introduction

A **recursive mutex** (`std::recursive_mutex`) is a synchronization primitive that allows the same thread to acquire the same mutex multiple times without causing a deadlock. This is particularly useful in scenarios where a thread needs to re-enter a locked section of code, such as in recursive function calls or when calling other synchronized methods from within a locked method.

Unlike a regular `std::mutex`, which will deadlock if the same thread tries to lock it twice, a `std::recursive_mutex` keeps track of how many times it has been locked by the owning thread and requires an equal number of unlocks before another thread can acquire it.

## When to Use Recursive Mutex

Recursive mutexes are appropriate in several scenarios:

1. **Recursive Functions**: When a function calls itself and both the outer and inner calls need to lock the same resource
2. **Method Chaining**: When synchronized methods call other synchronized methods within the same class
3. **Complex Class Hierarchies**: When derived class methods call base class methods that both require locking
4. **Legacy Code Refactoring**: When converting non-thread-safe code to thread-safe without major restructuring

However, recursive mutexes should be used sparingly. They often indicate code that could be better structured, and they have slightly higher overhead than regular mutexes.

## Basic Usage

Here's a simple example demonstrating the difference between regular and recursive mutexes:

```cpp
#include <iostream>
#include <mutex>
#include <thread>

class Counter {
private:
    std::recursive_mutex mtx;
    int value = 0;

public:
    // Public method that locks the mutex
    void increment() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        value++;
        std::cout << "Value incremented to: " << value << std::endl;
    }
    
    // Another public method that locks the mutex AND calls increment
    void incrementBy(int n) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        std::cout << "Incrementing by " << n << std::endl;
        for (int i = 0; i < n; i++) {
            increment(); // This would deadlock with std::mutex!
        }
    }
    
    int getValue() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return value;
    }
};

int main() {
    Counter counter;
    
    // This works fine because recursive_mutex allows re-locking
    counter.incrementBy(3);
    
    std::cout << "Final value: " << counter.getValue() << std::endl;
    
    return 0;
}
```

## Recursive Function Example

Recursive mutexes are particularly useful with recursive algorithms:

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

class TreeNode {
public:
    int data;
    std::vector<TreeNode*> children;
    mutable std::recursive_mutex mtx;
    
    TreeNode(int val) : data(val) {}
    
    void addChild(TreeNode* child) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        children.push_back(child);
    }
    
    // Recursive function that needs to lock at each level
    int sumTree() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        int sum = data;
        
        for (const auto* child : children) {
            sum += child->sumTree(); // Recursive call, locks again
        }
        
        return sum;
    }
    
    // Count total nodes in tree
    int countNodes() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        int count = 1; // Count this node
        
        for (const auto* child : children) {
            count += child->countNodes(); // Recursive locking
        }
        
        return count;
    }
};

int main() {
    TreeNode root(10);
    TreeNode* child1 = new TreeNode(5);
    TreeNode* child2 = new TreeNode(15);
    TreeNode* grandchild = new TreeNode(3);
    
    child1->addChild(grandchild);
    root.addChild(child1);
    root.addChild(child2);
    
    std::cout << "Sum of tree: " << root.sumTree() << std::endl;
    std::cout << "Node count: " << root.countNodes() << std::endl;
    
    // Cleanup
    delete grandchild;
    delete child1;
    delete child2;
    
    return 0;
}
```

## Lock Counting Mechanism

The recursive mutex maintains a lock count. Here's an example that demonstrates this concept:

```cpp
#include <iostream>
#include <mutex>
#include <thread>

class LockCounter {
private:
    std::recursive_mutex mtx;
    int lockDepth = 0;

public:
    void nestedLocking(int depth) {
        if (depth <= 0) return;
        
        mtx.lock();
        lockDepth++;
        std::cout << "Thread " << std::this_thread::get_id() 
                  << " locked at depth: " << lockDepth << std::endl;
        
        // Recursive call with the mutex already locked
        nestedLocking(depth - 1);
        
        lockDepth--;
        std::cout << "Thread " << std::this_thread::get_id() 
                  << " unlocking at depth: " << lockDepth + 1 << std::endl;
        mtx.unlock();
    }
    
    void demonstrateNesting() {
        nestedLocking(3);
    }
};

int main() {
    LockCounter counter;
    counter.demonstrateNesting();
    
    return 0;
}
```

## Practical Example: Bank Account with Transfer

Here's a real-world scenario where recursive mutex helps with method chaining:

```cpp
#include <iostream>
#include <mutex>
#include <thread>
#include <chrono>

class BankAccount {
private:
    std::recursive_mutex mtx;
    double balance;
    std::string accountNumber;

public:
    BankAccount(const std::string& accNum, double initialBalance) 
        : accountNumber(accNum), balance(initialBalance) {}
    
    void deposit(double amount) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        balance += amount;
        std::cout << accountNumber << ": Deposited $" << amount 
                  << ", New balance: $" << balance << std::endl;
    }
    
    bool withdraw(double amount) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        if (balance >= amount) {
            balance -= amount;
            std::cout << accountNumber << ": Withdrew $" << amount 
                      << ", New balance: $" << balance << std::endl;
            return true;
        }
        std::cout << accountNumber << ": Insufficient funds" << std::endl;
        return false;
    }
    
    // This method calls other methods that also lock the mutex
    bool transfer(BankAccount& toAccount, double amount) {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        
        std::cout << accountNumber << ": Initiating transfer of $" 
                  << amount << " to " << toAccount.accountNumber << std::endl;
        
        if (withdraw(amount)) { // Locks again (recursive)
            toAccount.deposit(amount);
            std::cout << accountNumber << ": Transfer completed" << std::endl;
            return true;
        }
        
        std::cout << accountNumber << ": Transfer failed" << std::endl;
        return false;
    }
    
    double getBalance() const {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        return balance;
    }
};

int main() {
    BankAccount account1("ACC-001", 1000.0);
    BankAccount account2("ACC-002", 500.0);
    
    // Perform operations
    account1.deposit(200.0);
    account1.transfer(account2, 300.0);
    
    std::cout << "\nFinal Balances:" << std::endl;
    std::cout << "Account 1: $" << account1.getBalance() << std::endl;
    std::cout << "Account 2: $" << account2.getBalance() << std::endl;
    
    return 0;
}
```

## Comparison: Regular Mutex vs Recursive Mutex

Here's a demonstration showing what happens with each type:

```cpp
#include <iostream>
#include <mutex>

class RegularMutexExample {
private:
    std::mutex mtx;
    
public:
    void method1() {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Method1 executing" << std::endl;
    }
    
    void method2() {
        std::lock_guard<std::mutex> lock(mtx);
        std::cout << "Method2 executing" << std::endl;
        // method1(); // DEADLOCK! Would hang forever
    }
};

class RecursiveMutexExample {
private:
    std::recursive_mutex mtx;
    
public:
    void method1() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        std::cout << "Method1 executing" << std::endl;
    }
    
    void method2() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        std::cout << "Method2 executing" << std::endl;
        method1(); // No problem! Works fine
    }
};

int main() {
    std::cout << "Regular Mutex Example:" << std::endl;
    RegularMutexExample regular;
    regular.method1();
    // regular.method2(); // Don't call this - it will deadlock!
    
    std::cout << "\nRecursive Mutex Example:" << std::endl;
    RecursiveMutexExample recursive;
    recursive.method1();
    recursive.method2(); // Safe to call
    
    return 0;
}
```

## Best Practices and Considerations

### Performance Overhead

Recursive mutexes have slightly more overhead than regular mutexes because they must track ownership and lock count:

```cpp
#include <iostream>
#include <mutex>
#include <chrono>

void performanceComparison() {
    const int iterations = 1000000;
    
    // Regular mutex
    std::mutex regular_mtx;
    auto start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        std::lock_guard<std::mutex> lock(regular_mtx);
    }
    auto end = std::chrono::high_resolution_clock::now();
    auto regular_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    // Recursive mutex
    std::recursive_mutex recursive_mtx;
    start = std::chrono::high_resolution_clock::now();
    for (int i = 0; i < iterations; i++) {
        std::lock_guard<std::recursive_mutex> lock(recursive_mtx);
    }
    end = std::chrono::high_resolution_clock::now();
    auto recursive_time = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
    
    std::cout << "Regular mutex: " << regular_time.count() << " μs" << std::endl;
    std::cout << "Recursive mutex: " << recursive_time.count() << " μs" << std::endl;
}
```

### When NOT to Use Recursive Mutex

Consider refactoring instead of using recursive mutex:

```cpp
// BAD: Using recursive mutex as a band-aid
class BadDesign {
private:
    std::recursive_mutex mtx;
    int value;
    
public:
    void publicMethod() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        // ... do work
        helperMethod(); // Needs lock
    }
    
    void helperMethod() {
        std::lock_guard<std::recursive_mutex> lock(mtx);
        // ... do work
    }
};

// BETTER: Separate locked and unlocked methods
class BetterDesign {
private:
    std::mutex mtx;
    int value;
    
    // Internal method assumes lock is held
    void helperMethodUnsafe() {
        // ... do work (no locking)
    }
    
public:
    void publicMethod() {
        std::lock_guard<std::mutex> lock(mtx);
        // ... do work
        helperMethodUnsafe(); // No additional lock needed
    }
    
    void helperMethod() {
        std::lock_guard<std::mutex> lock(mtx);
        helperMethodUnsafe();
    }
};
```

## Summary

**Recursive Mutex** (`std::recursive_mutex`) is a synchronization primitive that allows the same thread to lock a mutex multiple times without deadlocking. Key points include:

- **Use Cases**: Recursive functions, method chaining in classes, complex call hierarchies where methods call other synchronized methods
- **Mechanism**: Maintains a lock count and thread ownership, requiring an equal number of unlocks to fully release the mutex
- **Advantages**: Simplifies code in scenarios with reentrant locking needs, prevents deadlocks when the same thread needs multiple locks
- **Disadvantages**: Slightly higher overhead than regular mutexes, can mask poor design decisions, may indicate code that needs refactoring
- **Best Practice**: Use sparingly and consider whether code restructuring with internal unsafe methods would be cleaner
- **Thread Safety**: Only the owning thread can lock and unlock the recursive mutex; it provides no cross-thread reentrancy

While recursive mutexes solve specific problems elegantly, they should be a deliberate choice rather than a default. Well-structured code with clear separation between locked public interfaces and unlocked internal helpers often eliminates the need for recursive locking altogether.