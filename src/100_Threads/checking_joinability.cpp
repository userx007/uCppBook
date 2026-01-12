/*

g++ -pthread checking_joinability.cpp -o app

*/


#include <iostream>
#include <thread>

void simpleTask() {
    std::cout << "Task executing" << std::endl;
}

int main() {
    std::thread t1(simpleTask);
    
    // Check if thread is joinable
    if (t1.joinable()) {
        std::cout << "Thread is joinable" << std::endl;
        t1.join();
    }
    
    // After joining, thread is no longer joinable
    if (!t1.joinable()) {
        std::cout << "Thread is no longer joinable" << std::endl;
    }
    
    // Default-constructed threads are not joinable
    std::thread t2;
    std::cout << "Default thread joinable: " << std::boolalpha 
              << t2.joinable() << std::endl;
    
    return 0;
}