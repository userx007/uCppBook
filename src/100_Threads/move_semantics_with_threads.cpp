/*

g++ -pthread move_semantics_with_threads.cpp -o app

*/

#include <iostream>
#include <thread>
#include <vector>

void task(int id) {
    std::cout << "Task " << id << " executing" << std::endl;
}

std::thread createThread(int id) {
    return std::thread(task, id);
}

int main() {
    // Move construction
    std::thread t1(task, 1);
    std::thread t2 = std::move(t1);  // t1 is now empty, t2 owns the thread
    
    // Move from function return
    std::thread t3 = createThread(3);
    
    // Store threads in a vector
    std::vector<std::thread> threads;
    for (int i = 0; i < 3; ++i) {
        threads.push_back(std::thread(task, i + 10));
    }
    
    // Join all threads
    t2.join();
    t3.join();
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}