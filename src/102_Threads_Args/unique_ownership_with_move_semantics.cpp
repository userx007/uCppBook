#include <thread>
#include <vector>
#include <iostream>

class ThreadPool {
private:
    std::vector<std::jthread> threads_;
    
public:
    void add_task(auto&& task) {
        threads_.emplace_back(std::forward<decltype(task)>(task));
    }
    
    size_t size() const { return threads_.size(); }
    
    // Move-only semantics
    ThreadPool() = default;
    ThreadPool(const ThreadPool&) = delete;
    ThreadPool& operator=(const ThreadPool&) = delete;
    ThreadPool(ThreadPool&&) = default;
    ThreadPool& operator=(ThreadPool&&) = default;
    
    ~ThreadPool() {
        std::cout << "ThreadPool destructor: all threads will auto-join\n";
    }
};

int main() {
    ThreadPool pool;
    
    for (int i = 0; i < 5; ++i) {
        pool.add_task([i]() {
            std::cout << "Task " << i << " executing\n";
        });
    }
    
    std::cout << "Pool has " << pool.size() << " threads\n";
    
    // All threads automatically joined when pool goes out of scope
    return 0;
}