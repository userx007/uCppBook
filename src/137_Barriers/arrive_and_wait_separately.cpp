/*
g++ -pthread --std=c++20 arrive_and_wait_separately.cpp -o app
*/

#include <iostream>
#include <thread>
#include <barrier>
#include <vector>
#include <chrono>
#include <mutex>

std::mutex cout_mutex;

void worker_with_async_arrival(int id, std::barrier<>& bar) {
    std::cout << "Thread " << id << " starting work\n";
    
    // Do some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    
    // Arrive at barrier but don't wait yet (get arrival token)
    auto arrival_token = bar.arrive();

    // safe std::cout access
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << id << " arrived at barrier\n";
    }
    
    // safe std::cout access
    {    
        std::lock_guard<std::mutex> lock(cout_mutex);        
        std::cout << "Thread " << id << " doing post-arrival work\n";
    }
    // Do additional work while others are arriving
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // safe std::cout access
    {    
        std::lock_guard<std::mutex> lock(cout_mutex);                
        std::cout << "Thread " << id << " waiting at barrier\n";
    }
    // Now wait for all threads using the token
    bar.wait(std::move(arrival_token));
    
    // safe std::cout access
    {    
        std::lock_guard<std::mutex> lock(cout_mutex);                
        std::cout << "Thread " << id << " passed barrier\n";
    }    
}

int main() {
    const int num_threads = 3;
    std::barrier bar(num_threads);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_with_async_arrival, i, std::ref(bar));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}