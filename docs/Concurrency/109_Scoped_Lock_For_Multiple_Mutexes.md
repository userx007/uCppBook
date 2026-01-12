# Scoped Lock for Multiple Mutexes

## Introduction

In multithreaded programming, it's common to need protection for multiple shared resources simultaneously. When multiple mutexes must be locked together, there's a significant risk of deadlock if threads acquire locks in different orders. `std::scoped_lock` (introduced in C++17) provides an elegant, deadlock-free solution for locking multiple mutexes simultaneously.

## The Deadlock Problem

Deadlock occurs when two or more threads are waiting for each other to release locks, creating a circular dependency. Consider this problematic scenario:

```cpp
#include <iostream>
#include <thread>
#include <mutex>

std::mutex mutex1, mutex2;
int account1 = 1000;
int account2 = 2000;

// Thread 1: Transfer from account1 to account2
void transfer_1_to_2(int amount) {
    std::lock_guard<std::mutex> lock1(mutex1);  // Lock mutex1 first
    std::this_thread::sleep_for(std::chrono::milliseconds(10)); // Simulate work
    std::lock_guard<std::mutex> lock2(mutex2);  // Then lock mutex2
    
    account1 -= amount;
    account2 += amount;
    std::cout << "Transferred " << amount << " from account1 to account2\n";
}

// Thread 2: Transfer from account2 to account1
void transfer_2_to_1(int amount) {
    std::lock_guard<std::mutex> lock2(mutex2);  // Lock mutex2 first
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    std::lock_guard<std::mutex> lock1(mutex1);  // Then lock mutex1
    
    account2 -= amount;
    account1 += amount;
    std::cout << "Transferred " << amount << " from account2 to account1\n";
}

// DANGER: This can deadlock!
void dangerous_example() {
    std::thread t1(transfer_1_to_2, 100);
    std::thread t2(transfer_2_to_1, 50);
    
    t1.join();
    t2.join();
}
```

In this example, if thread 1 locks `mutex1` while thread 2 locks `mutex2`, both will wait indefinitely for the other's mutex, creating a deadlock.

## The Solution: std::scoped_lock

`std::scoped_lock` uses a deadlock avoidance algorithm to lock multiple mutexes simultaneously without risk of deadlock. It's essentially a variadic version of `std::lock_guard` that can handle multiple mutexes.

### Basic Syntax

```cpp
#include <mutex>

std::mutex m1, m2, m3;

void safe_function() {
    std::scoped_lock lock(m1, m2, m3);
    // All mutexes are now locked
    // Protected code here
    
    // All mutexes automatically unlock when lock goes out of scope
}
```

### Safe Bank Transfer Example

Here's the corrected version using `std::scoped_lock`:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <vector>

class BankAccount {
private:
    mutable std::mutex m_mutex;
    int m_balance;
    std::string m_name;

public:
    BankAccount(std::string name, int initial_balance)
        : m_name(std::move(name)), m_balance(initial_balance) {}
    
    int get_balance() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_balance;
    }
    
    std::string get_name() const {
        std::lock_guard<std::mutex> lock(m_mutex);
        return m_name;
    }
    
    // Friend function needs access to mutexes
    friend void safe_transfer(BankAccount& from, BankAccount& to, int amount);
};

void safe_transfer(BankAccount& from, BankAccount& to, int amount) {
    // Lock both mutexes simultaneously - deadlock-free!
    std::scoped_lock lock(from.m_mutex, to.m_mutex);
    
    if (from.m_balance >= amount) {
        from.m_balance -= amount;
        to.m_balance += amount;
        std::cout << "Transferred " << amount << " from " 
                  << from.m_name << " to " << to.m_name << "\n";
    } else {
        std::cout << "Insufficient funds in " << from.m_name << "\n";
    }
}

int main() {
    BankAccount alice("Alice", 1000);
    BankAccount bob("Bob", 2000);
    
    std::vector<std::thread> threads;
    
    // Multiple concurrent transfers in both directions
    threads.emplace_back(safe_transfer, std::ref(alice), std::ref(bob), 100);
    threads.emplace_back(safe_transfer, std::ref(bob), std::ref(alice), 200);
    threads.emplace_back(safe_transfer, std::ref(alice), std::ref(bob), 150);
    threads.emplace_back(safe_transfer, std::ref(bob), std::ref(alice), 50);
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nFinal balances:\n";
    std::cout << alice.get_name() << ": $" << alice.get_balance() << "\n";
    std::cout << bob.get_name() << ": $" << bob.get_balance() << "\n";
    
    return 0;
}
```

## Advanced Usage: Graph Operations

Here's a more complex example with multiple mutexes protecting a graph structure:

```cpp
#include <iostream>
#include <thread>
#include <mutex>
#include <map>
#include <vector>
#include <string>

class ThreadSafeGraph {
private:
    struct Node {
        std::string data;
        std::vector<int> edges;
        mutable std::mutex mutex;
        
        Node(std::string d) : data(std::move(d)) {}
    };
    
    std::map<int, Node> nodes;
    mutable std::mutex graph_mutex;

public:
    void add_node(int id, const std::string& data) {
        std::lock_guard<std::mutex> lock(graph_mutex);
        nodes.emplace(std::piecewise_construct,
                     std::forward_as_tuple(id),
                     std::forward_as_tuple(data));
    }
    
    // Add edge between two nodes - requires locking both nodes
    void add_edge(int from_id, int to_id) {
        std::lock_guard<std::mutex> graph_lock(graph_mutex);
        
        auto from_it = nodes.find(from_id);
        auto to_it = nodes.find(to_id);
        
        if (from_it == nodes.end() || to_it == nodes.end()) {
            std::cout << "Node not found\n";
            return;
        }
        
        // Lock both node mutexes simultaneously
        std::scoped_lock lock(from_it->second.mutex, to_it->second.mutex);
        from_it->second.edges.push_back(to_id);
        
        std::cout << "Added edge: " << from_id << " -> " << to_id << "\n";
    }
    
    // Swap data between two nodes - requires locking both
    void swap_node_data(int id1, int id2) {
        std::lock_guard<std::mutex> graph_lock(graph_mutex);
        
        auto it1 = nodes.find(id1);
        auto it2 = nodes.find(id2);
        
        if (it1 == nodes.end() || it2 == nodes.end()) {
            std::cout << "Node not found\n";
            return;
        }
        
        // Lock both nodes in a deadlock-free manner
        std::scoped_lock lock(it1->second.mutex, it2->second.mutex);
        std::swap(it1->second.data, it2->second.data);
        
        std::cout << "Swapped data between nodes " << id1 << " and " << id2 << "\n";
    }
    
    void print_node(int id) const {
        std::lock_guard<std::mutex> graph_lock(graph_mutex);
        
        auto it = nodes.find(id);
        if (it == nodes.end()) {
            std::cout << "Node not found\n";
            return;
        }
        
        std::lock_guard<std::mutex> node_lock(it->second.mutex);
        std::cout << "Node " << id << ": " << it->second.data << " -> ";
        for (int edge : it->second.edges) {
            std::cout << edge << " ";
        }
        std::cout << "\n";
    }
};

int main() {
    ThreadSafeGraph graph;
    
    // Setup nodes
    graph.add_node(1, "Node A");
    graph.add_node(2, "Node B");
    graph.add_node(3, "Node C");
    graph.add_node(4, "Node D");
    
    // Concurrent operations
    std::vector<std::thread> threads;
    
    threads.emplace_back([&]() { graph.add_edge(1, 2); });
    threads.emplace_back([&]() { graph.add_edge(2, 3); });
    threads.emplace_back([&]() { graph.swap_node_data(1, 3); });
    threads.emplace_back([&]() { graph.add_edge(3, 4); });
    threads.emplace_back([&]() { graph.swap_node_data(2, 4); });
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "\nFinal graph state:\n";
    graph.print_node(1);
    graph.print_node(2);
    graph.print_node(3);
    graph.print_node(4);
    
    return 0;
}
```

## Comparison with std::lock

Before C++17, the recommended approach was using `std::lock` with `std::lock_guard`:

```cpp
#include <mutex>

std::mutex m1, m2;

// Pre-C++17 approach
void old_way() {
    std::lock(m1, m2);  // Lock both atomically
    std::lock_guard<std::mutex> lock1(m1, std::adopt_lock);
    std::lock_guard<std::mutex> lock2(m2, std::adopt_lock);
    
    // Protected code
}

// C++17 approach - simpler and cleaner
void new_way() {
    std::scoped_lock lock(m1, m2);
    
    // Protected code
}
```

The `std::scoped_lock` approach is more concise and less error-prone.

## Key Advantages

**Deadlock Prevention**: Uses a deadlock avoidance algorithm internally (typically `std::lock`), ensuring mutexes are acquired in a consistent order.

**RAII Compliance**: Automatically releases all locks when the `std::scoped_lock` object goes out of scope, even if exceptions are thrown.

**Simplicity**: Single-line locking of multiple mutexes without manual `std::adopt_lock` management.

**Performance**: No overhead compared to manual locking strategies; the compiler can optimize it effectively.

**Type Safety**: Template-based, works with any lockable type that meets the BasicLockable requirements.

## Summary

`std::scoped_lock` is the modern C++17 solution for safely locking multiple mutexes simultaneously without deadlock risk. It combines the RAII benefits of `std::lock_guard` with the deadlock avoidance of `std::lock`, providing a clean, safe, and efficient interface. When you need to protect multiple resources accessed by concurrent threads, `std::scoped_lock` should be your default choice. It eliminates entire classes of threading bugs while keeping code readable and maintainable. For single mutex scenarios, `std::lock_guard` or `std::unique_lock` remain appropriate, but when two or more mutexes are involved, `std::scoped_lock` is the superior option.