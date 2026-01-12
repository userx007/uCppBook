/*

g++ -pthread thread_lifecycle_management.cpp -o app

*/

#include <iostream>
#include <thread>
#include <chrono>
#include <stdexcept>

class ThreadGuard {
    std::thread& t;
public:
    explicit ThreadGuard(std::thread& thread) : t(thread) {}
    
    ~ThreadGuard() {
        if (t.joinable()) {
            t.join();
            std::cout << "Thread joined ok" << std::endl;            
        }
    }
    
    // Prevent copying
    ThreadGuard(const ThreadGuard&) = delete;
    ThreadGuard& operator=(const ThreadGuard&) = delete;
};

void riskyOperation() {
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    std::cout << "Risky operation completed" << std::endl;
}

void demonstrateThreadGuard() {
    std::thread t(riskyOperation);
    ThreadGuard guard(t);
    
    // Even if an exception is thrown, ThreadGuard destructor
    // will ensure the thread is joined
    std::cout << "Doing other work..." << std::endl;
    
    // Simulate potential exception
    throw std::runtime_error("Something went wrong!");
}

int main() {
    try {
        demonstrateThreadGuard();
        std::cout << "Thread guard demonstration completed successfully" << std::endl;
    } catch (const std::exception& e) {
        std::cout << "Caught exception: " << e.what() << std::endl;
    }
    
    return 0;
}