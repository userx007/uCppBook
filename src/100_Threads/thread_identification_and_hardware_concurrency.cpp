/*

g++ -pthread thread_identification_and_hardware_concurrency.cpp -o app

*/

#include <iostream>
#include <thread>

void printThreadInfo() {
    std::cout << "Thread ID: " << std::this_thread::get_id() << std::endl;
}

int main() {
    // Get number of hardware threads available
    unsigned int numThreads = std::thread::hardware_concurrency();
    std::cout << "Hardware supports " << numThreads << " concurrent threads" << std::endl;
    
    // Main thread ID
    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
    
    // Create and identify another thread
    std::thread t(printThreadInfo);
    std::cout << "Created thread ID (from main): " << t.get_id() << std::endl;
    
    t.join();
    
    return 0;
}