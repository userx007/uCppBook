/*

g++ -pthread detaching_threads.cpp -o app

*/

#include <iostream>
#include <thread>
#include <chrono>

void backgroundTask(int id) {
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Background task " << id << " completed" << std::endl;
}

int main() {
    std::thread t1(backgroundTask, 1);
    
    // Detach the thread - it will run independently
    t1.detach();
    
    std::cout << "Thread detached, is joinable: " << std::boolalpha 
              << t1.joinable() << std::endl;
    
    // Main thread continues without waiting
    std::cout << "Main thread continuing..." << std::endl;
    
    // Give detached thread time to complete before program exits
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    return 0;
}