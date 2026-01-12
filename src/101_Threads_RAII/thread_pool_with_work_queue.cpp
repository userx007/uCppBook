/*
g++ -pthread --std=c++20 thread_pool_with_work_queue.cpp -o app
*/
#include <thread>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <iostream>
#include <vector>

class ThreadPoolRAII {
private:
    std::vector<std::jthread> workers_;
    std::queue<std::function<void()>> tasks_;
    std::mutex queue_mutex_;
    std::mutex cout_mutex_;  // Separate mutex for console output
    std::condition_variable cv_;
    bool shutdown_ = false;  // Manual shutdown flag
    
    void worker_thread(std::stop_token stop_token) {
        while (true) {
            std::function<void()> task;
            
            {
                std::unique_lock<std::mutex> lock(queue_mutex_);
                cv_.wait(lock, [this, &stop_token] {
                    return !tasks_.empty() || shutdown_ || stop_token.stop_requested();
                });
                
                // Process remaining tasks even after shutdown or stop request
                // Note: jthread's destructor automatically calls request_stop(), which triggers the immediate exit 
                // before all tasks complete. 
                // We need to check the stop_token only after ensuring the queue is empty:
                if (tasks_.empty()) {
                    // Only exit if queue is truly empty
                    if (shutdown_ || stop_token.stop_requested()) {
                        return;
                    }
                }
                
                if (!tasks_.empty()) {
                    task = std::move(tasks_.front());
                    tasks_.pop();
                }
            }
            
            if (task) {
                task();
            }
        }
    }
    
public:
    explicit ThreadPoolRAII(size_t num_threads) {
        workers_.reserve(num_threads);
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this](std::stop_token st) {
                worker_thread(st);
            });
        }
    }
    
    void enqueue(std::function<void()> task) {
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            if (shutdown_) {
                throw std::runtime_error("Cannot enqueue on shutdown pool");
            }
            tasks_.push(std::move(task));
        }
        cv_.notify_one();
    }
    
    // Provide thread-safe console output
    template<typename... Args>
    void safe_print(Args&&... args) {
        std::lock_guard<std::mutex> lock(cout_mutex_);
        (std::cout << ... << args);
    }
    
    ~ThreadPoolRAII() {
        // Signal shutdown and wake all threads
        {
            std::lock_guard<std::mutex> lock(queue_mutex_);
            shutdown_ = true;
        }
        cv_.notify_all();
        
        // jthreads automatically join here (blocking until all workers finish)
        workers_.clear();  // Explicit join by clearing the vector
        
        {
            std::lock_guard<std::mutex> lock(cout_mutex_);
            std::cout << "ThreadPool shutting down...\n";
        }
    }
};

int main() {
    {
        ThreadPoolRAII pool(3);
        
        for (int i = 0; i < 10; ++i) {
            pool.enqueue([i, &pool]() {
                pool.safe_print("Task ", i, " executing on thread ", 
                               std::this_thread::get_id(), "\n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
            });
        }
        
        std::cout << "All tasks enqueued\n";
        
    } // Pool destructor waits for all tasks to complete, then joins threads
    
    std::cout << "All work completed\n";
    return 0;
}