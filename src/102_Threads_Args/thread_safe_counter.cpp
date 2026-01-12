/*

g++ -pthread thread_safe_counter.cpp -o app

*/

#include <iostream>
#include <thread>
#include <vector>
#include <mutex>
#include <memory>

class ThreadSafeCounter {
    int value;
    std::mutex mtx;
public:
    ThreadSafeCounter(int initial = 0) : value(initial) {}
    
    void increment(int amount) {
        std::lock_guard<std::mutex> lock(mtx);
        value += amount;
    }
    
    int get() {
        std::lock_guard<std::mutex> lock(mtx);
        return value;
    }
};

// Pass by value - receives a copy
void worker_copy(int id, std::string name) {
    std::cout << "Worker " << id << " (" << name << ") starting" << std::endl;
}

// Pass by reference - modifies the original
void worker_ref(int id, ThreadSafeCounter& counter) {
    for (int i = 0; i < 100; ++i) {
        counter.increment(1);
    }
    std::cout << "Worker " << id << " completed" << std::endl;
}

// Pass by move - takes ownership
void worker_move(int id, std::unique_ptr<std::vector<int>> data) {
    std::cout << "Worker " << id << " received vector with " 
              << data->size() << " elements" << std::endl;
}

int main() {
    // Example 1: Pass by value
    std::string name = "Alpha";
    std::thread t1(worker_copy, 1, name);
    t1.join();
    
    // Example 2: Pass by reference with std::ref
    ThreadSafeCounter counter(0);
    std::vector<std::thread> threads;
    
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker_ref, i, std::ref(counter));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Final counter value: " << counter.get() << std::endl;
    
    // Example 3: Pass by move
    auto vec = std::make_unique<std::vector<int>>(100, 42);
    std::thread t2(worker_move, 2, std::move(vec));
    t2.join();
    
    return 0;
}