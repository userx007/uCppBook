/*
g++ -pthread --std=c++14 thread_creation.cpp -o app
*/

#include <iostream>
#include <thread>
#include <vector>
#include <string>
#include <chrono>
#include <future>
#include <mutex>

std::mutex mtx;

// Simple worker function that takes parameters
void worker(int id, const std::string& message) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::lock_guard<std::mutex> lock(mtx);
    std::cout << "Thread " << id << ": " << message << std::endl;
}

// Worker function that returns a value (simulated with shared state)
int calculate_sum(int start, int end) {
    int sum = 0;
    for (int i = start; i <= end; ++i) {
        sum += i;
    }
    return sum;
}

int main() {
    std::cout << "=== Basic Pattern with emplace_back ===" << std::endl;
    
    // Pattern 1: Simple thread creation and joining
    {
        std::vector<std::thread> threads;
        const int num_threads = 5;
        
        // Launch threads using emplace_back
        for (int i = 0; i < num_threads; ++i) {
            threads.emplace_back(worker, i, "Hello from thread");
        }
        
        // Join all threads
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "\n=== Pattern with Return Values (using promises) ===" << std::endl;
    
    // Pattern 2: Getting return values using promise/future
    {
        std::vector<std::thread> threads;
        std::vector<std::promise<int>> promises(3);
        std::vector<std::future<int>> futures;
        
        // Create futures from promises
        for (auto& p : promises) {
            futures.push_back(p.get_future());
        }
        
        // Launch threads with lambda to capture promise
        for (int i = 0; i < 3; ++i) {
            threads.emplace_back([](std::promise<int> p, int start, int end) {
                p.set_value(calculate_sum(start, end));
            }, std::move(promises[i]), i * 10, (i + 1) * 10);
        }
        
        // Get results
        for (int i = 0; i < 3; ++i) {
            std::cout << "Sum " << i << ": " << futures[i].get() << std::endl;
        }
        
        // Join threads
        for (auto& t : threads) {
            t.join();
        }
    }
    
    std::cout << "\n=== Pattern with Lambda Capture ===" << std::endl;
    
    // Pattern 3: Using lambdas with capture
    {
        std::vector<std::thread> threads;
        std::vector<int> data = {10, 20, 30, 40, 50};
        
        for (size_t i = 0; i < data.size(); ++i) {
            threads.emplace_back([i, val = data[i]]() {
                std::cout << "Processing index " << i 
                          << " with value " << val * 2 << std::endl;
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
    
    return 0;
}