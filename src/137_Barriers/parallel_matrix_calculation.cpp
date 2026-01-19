/*
g++ -pthread --std=c++20 parallel_matrix_calculation.cpp -o app
*/
#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <mutex>
#include <cmath>

std::mutex cout_mutex;

void parallel_matrix_computation(int thread_id, int num_threads, 
                                 std::vector<double>& data,
                                 std::barrier<>& sync_point) {
    const int size = data.size();
    const int chunk_size = size / num_threads;
    const int start = thread_id * chunk_size;
    const int end = (thread_id == num_threads - 1) ? size : start + chunk_size;
    
    // Phase 1: Initialize data
    for (int i = start; i < end; ++i) {
        data[i] = thread_id * 100.0 + i;
    }
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << thread_id << " completed initialization\n";
    }
    
    // Wait for all threads to complete initialization
    sync_point.arrive_and_wait();
    
    // Phase 2: Process data (can safely read all elements now)
    double sum = 0.0;
    for (int i = start; i < end; ++i) {
        sum += std::sqrt(data[i]);
    }
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << thread_id << " computed sum: " << sum << "\n";
    }
    
    // Wait for all threads to complete processing
    sync_point.arrive_and_wait();
    
    // Phase 3: Finalize (modify based on global state)
    for (int i = start; i < end; ++i) {
        data[i] = data[i] / (sum + 1.0);
    }
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Thread " << thread_id << " completed finalization\n";
    }
    
    sync_point.arrive_and_wait();
}

int main() {
    const int num_threads = 4;
    const int data_size = 100;
    
    std::vector<double> data(data_size);
    std::barrier sync_point(num_threads);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(parallel_matrix_computation, 
                            i, num_threads, std::ref(data), std::ref(sync_point));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "All phases completed successfully\n";
    return 0;
}