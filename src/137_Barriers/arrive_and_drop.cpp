/*
g++ -pthread --std=c++20 arrive_and_drop.cpp -o app
*/

#include <iostream>
#include <thread>
#include <barrier>
#include <vector>
#include <random>

std::mutex cout_mutex;

void worker_that_may_exit(int id, std::barrier<>& bar, int max_iterations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> exit_chance(1, 10);
    
    for (int iter = 0; iter < max_iterations; ++iter) {
        // safe std::cout access
        {
            std::lock_guard<std::mutex> lock(cout_mutex);
            std::cout << "Thread " << id << " at iteration " << iter << "\n";
        }
        
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Random chance to exit early
        if (exit_chance(gen) > 8 && iter > 2) {
            // safe std::cout access
            {
                std::lock_guard<std::mutex> lock(cout_mutex);
                std::cout << "Thread " << id << " exiting early at iteration " << iter << "\n";
            }

            // Arrive and permanently drop from barrier
            bar.arrive_and_drop();
            return;
        }
        
        // Normal synchronization
        bar.arrive_and_wait();
    }
    
    // safe std::cout access
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << id << " completed all iterations\n";
    }
}

int main() {
    const int num_threads = 5;
    const int max_iterations = 10;
    
    std::barrier bar(num_threads);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_that_may_exit, i, std::ref(bar), max_iterations);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "All threads completed\n";
    return 0;
}