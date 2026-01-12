/*
g++ -pthread --std=c++20 unique_ownership_with_move_semantics.cpp -o app
*/
#include <thread>
#include <vector>
#include <iostream>
#include <mutex>

class ThreadPool {
private:
    std::vector<std::jthread> threads_;
    std::mutex mutex_;
    
public:
    void add_task(auto&& task) {
        std::lock_guard<std::mutex> lock(mutex_);
        threads_.emplace_back(std::forward<decltype(task)>(task));
    }
    
    size_t size() const { return threads_.size(); }
    
    std::mutex& get_mutex() { return mutex_; }
    
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
        pool.add_task([i, &pool]() {
            std::lock_guard<std::mutex> lock(pool.get_mutex());
            std::cout << "Task " << i << " executing\n";
        });
    }
    
    // Give threads a moment to start before checking size
    std::this_thread::sleep_for(std::chrono::milliseconds(10));
    
    {
        std::lock_guard<std::mutex> lock(pool.get_mutex());
        std::cout << "Pool has " << pool.size() << " threads\n";
    }
    
    // All threads automatically joined when pool goes out of scope
    return 0;
}