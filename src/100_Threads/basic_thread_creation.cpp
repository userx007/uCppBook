/*

g++ -pthread basic_thread_creation.cpp -o app

*/

#include <iostream>
#include <thread>
#include <chrono>

// Simple function to run in a thread
void printMessage(const std::string& message, int count) {
    for (int i = 0; i < count; ++i) {
        std::cout << message << " (" << i + 1 << ")" << std::endl;
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
}

int main() {
    // Create a thread that executes printMessage
    std::thread t1(printMessage, "Hello from thread", 3);
    
    // Main thread continues execution
    std::cout << "Main thread running" << std::endl;
    
    // Wait for the thread to complete
    t1.join();
    
    std::cout << "Thread completed" << std::endl;
    return 0;
}