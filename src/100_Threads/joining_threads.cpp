/*

g++ -pthread joining_threads.cpp -o app

*/

#include <iostream>
#include <thread>
#include <chrono>
#include <vector>

void worker(int id, int duration) {
    std::cout << "Worker " << id << " started" << std::endl;
    std::this_thread::sleep_for(std::chrono::milliseconds(duration));
    std::cout << "Worker " << id << " finished" << std::endl;
}

int main() {
    std::vector<std::thread> threads;
    
    // Create multiple threads
    for (int i = 0; i < 5; ++i) {
        threads.emplace_back(worker, i, (i + 1) * 100);
    }
    
    std::cout << "All threads created, waiting for completion..." << std::endl;
    
    // Join all threads
    for (auto& t : threads) {
        if (t.joinable()) {
            t.join();
        }
    }
    
    std::cout << "All workers completed" << std::endl;
    return 0;
}