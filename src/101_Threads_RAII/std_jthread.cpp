/*

g++ -pthread --std=c++20 std_jthread.cpp -o app

*/


#include <thread>
#include <iostream>
#include <chrono>

void simple_task() {
    std::cout << "Simple task running\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::cout << "Simple task completed\n";
}

void cancellable_task(std::stop_token stop_token) {
    int count = 0;
    while (!stop_token.stop_requested() && count < 10) {
        std::cout << "Count: " << count++ << "\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
    }
    
    if (stop_token.stop_requested()) {
        std::cout << "Task cancelled!\n";
    } else {
        std::cout << "Task completed normally\n";
    }
}

int main() {
    // Basic usage - automatically joins
    {
        std::jthread t(simple_task);
        std::cout << "Main continues while thread runs\n";
    } // Automatic join happens here
    
    std::cout << "\n--- Cooperative Cancellation ---\n";
    
    // Cooperative cancellation
    {
        std::jthread t(cancellable_task);
        
        std::this_thread::sleep_for(std::chrono::milliseconds(350));
        
        std::cout << "Requesting stop...\n";
        t.request_stop();  // Signal the thread to stop
        
    } // Still joins automatically, but thread will exit early
    
    std::cout << "All operations completed\n";
    return 0;
}