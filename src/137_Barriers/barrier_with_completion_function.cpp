/*
g++ -pthread --std=c++20 barrier_with_completion_function.cpp -o app
*/

#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <atomic>
#include <chrono>

class IterativeSimulation {
private:
    std::atomic<int> iteration{0};
    std::vector<double> current_state;
    std::vector<double> next_state;
    
    // Completion function runs once when all threads arrive
    auto make_completion_function() {
        return [this]() noexcept {
            // Swap buffers
            current_state.swap(next_state);
            iteration++;
            std::cout << "=== Iteration " << iteration << " completed ===\n";
        };
    }
    
public:
    IterativeSimulation(size_t size) 
        : current_state(size, 0.0), next_state(size, 0.0) {}
    
    void run_simulation(int num_threads, int max_iterations) {
        // Barrier with completion function
        std::barrier sync_point(num_threads, make_completion_function());
        
        std::vector<std::thread> threads;
        const size_t chunk_size = current_state.size() / num_threads;
        
        for (int tid = 0; tid < num_threads; ++tid) {
            threads.emplace_back([this, tid, num_threads, chunk_size, 
                                 max_iterations, &sync_point]() {
                const size_t start = tid * chunk_size;
                const size_t end = (tid == num_threads - 1) 
                    ? current_state.size() 
                    : start + chunk_size;
                
                for (int iter = 0; iter < max_iterations; ++iter) {
                    // Compute next state based on current state
                    for (size_t i = start; i < end; ++i) {
                        next_state[i] = current_state[i] * 0.9 + 
                                       (i > 0 ? current_state[i-1] * 0.05 : 0) +
                                       (i < current_state.size()-1 ? current_state[i+1] * 0.05 : 0);
                    }
                    
                    // Synchronize - completion function swaps buffers
                    sync_point.arrive_and_wait();
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
    }
};

int main() {
    IterativeSimulation sim(1000);
    sim.run_simulation(4, 5);
    std::cout << "Simulation complete\n";
    return 0;
}