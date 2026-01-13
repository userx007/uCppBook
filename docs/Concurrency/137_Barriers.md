# Barriers in C++ Concurrency

## Overview

A **barrier** is a synchronization primitive introduced in C++20 that allows multiple threads to wait until all participating threads have reached a common synchronization point before any of them can proceed. It's particularly useful for coordinating phases of work across threads, where each phase must be completed by all threads before moving to the next phase.

The `std::barrier` class template provides a reusable barrier that can be used multiple times throughout program execution, making it ideal for iterative parallel algorithms where threads need to synchronize at regular intervals.

## Key Concepts

### Basic Functionality
- **Arrival and Wait**: Threads arrive at the barrier and wait until all threads have arrived
- **Phase Completion**: Once all threads arrive, they all proceed to the next phase
- **Reusability**: The same barrier can be used for multiple synchronization points
- **Completion Function**: An optional function that runs once when all threads arrive

### Comparison with Other Primitives
- Unlike `std::latch` (single-use countdown), barriers are reusable across multiple phases
- Unlike mutexes, barriers coordinate multiple threads simultaneously rather than serializing access
- Barriers ensure all threads progress together through distinct phases of computation

## Core API

```cpp
#include <barrier>

// Constructor
std::barrier bar(thread_count);                    // Simple barrier
std::barrier bar(thread_count, completion_function); // With completion callback

// Member functions
bar.arrive_and_wait();      // Arrive and block until all threads arrive
bar.arrive_and_drop();      // Arrive and permanently reduce participant count
auto token = bar.arrive();  // Arrive without waiting (returns token)
bar.wait(std::move(token)); // Wait using previously obtained token
```

## Detailed Code Examples

### Example 1: Basic Barrier Usage - Parallel Matrix Computation

```cpp
#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <cmath>

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
    std::cout << "Thread " << thread_id << " completed initialization\n";
    
    // Wait for all threads to complete initialization
    sync_point.arrive_and_wait();
    
    // Phase 2: Process data (can safely read all elements now)
    double sum = 0.0;
    for (int i = start; i < end; ++i) {
        sum += std::sqrt(data[i]);
    }
    std::cout << "Thread " << thread_id << " computed sum: " << sum << "\n";
    
    // Wait for all threads to complete processing
    sync_point.arrive_and_wait();
    
    // Phase 3: Finalize (modify based on global state)
    for (int i = start; i < end; ++i) {
        data[i] = data[i] / (sum + 1.0);
    }
    std::cout << "Thread " << thread_id << " completed finalization\n";
    
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
```

### Example 2: Barrier with Completion Function

```cpp
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
```

### Example 3: arrive() and wait() Separately

```cpp
#include <iostream>
#include <thread>
#include <barrier>
#include <vector>
#include <chrono>

void worker_with_async_arrival(int id, std::barrier<>& bar) {
    std::cout << "Thread " << id << " starting work\n";
    
    // Do some work
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    
    // Arrive at barrier but don't wait yet (get arrival token)
    auto arrival_token = bar.arrive();
    std::cout << "Thread " << id << " arrived at barrier\n";
    
    // Do additional work while others are arriving
    std::cout << "Thread " << id << " doing post-arrival work\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    
    // Now wait for all threads using the token
    std::cout << "Thread " << id << " waiting at barrier\n";
    bar.wait(std::move(arrival_token));
    
    std::cout << "Thread " << id << " passed barrier\n";
}

int main() {
    const int num_threads = 3;
    std::barrier bar(num_threads);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_with_async_arrival, i, std::ref(bar));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 4: arrive_and_drop() for Dynamic Thread Management

```cpp
#include <iostream>
#include <thread>
#include <barrier>
#include <vector>
#include <random>

void worker_that_may_exit(int id, std::barrier<>& bar, int max_iterations) {
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> exit_chance(1, 10);
    
    for (int iter = 0; iter < max_iterations; ++iter) {
        std::cout << "Thread " << id << " at iteration " << iter << "\n";
        
        // Simulate work
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        
        // Random chance to exit early
        if (exit_chance(gen) > 8 && iter > 2) {
            std::cout << "Thread " << id << " exiting early at iteration " 
                     << iter << "\n";
            // Arrive and permanently drop from barrier
            bar.arrive_and_drop();
            return;
        }
        
        // Normal synchronization
        bar.arrive_and_wait();
    }
    
    std::cout << "Thread " << id << " completed all iterations\n";
}

int main() {
    const int num_threads = 5;
    const int max_iterations = 10;
    
    std::barrier bar(num_threads);
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(worker_that_may_exit, i, std::ref(bar), max_iterations);
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "All threads completed\n";
    return 0;
}
```

### Example 5: Multi-Stage Pipeline Processing

```cpp
#include <iostream>
#include <vector>
#include <thread>
#include <barrier>
#include <array>

struct DataBatch {
    std::vector<int> values;
    bool processed = false;
};

void pipeline_worker(int stage, int thread_id, 
                    std::array<std::barrier<>, 3>& stage_barriers,
                    std::vector<DataBatch>& batches) {
    const int num_batches = batches.size();
    const int chunk_size = num_batches / 4; // 4 threads per stage
    const int start = thread_id * chunk_size;
    const int end = (thread_id == 3) ? num_batches : start + chunk_size;
    
    std::cout << "Stage " << stage << ", Thread " << thread_id 
             << " processing batches " << start << "-" << end << "\n";
    
    // Stage 0: Data generation
    if (stage == 0) {
        for (int i = start; i < end; ++i) {
            batches[i].values.resize(100);
            for (auto& v : batches[i].values) {
                v = i * 100 + thread_id;
            }
        }
        stage_barriers[0].arrive_and_wait();
    }
    
    // Stage 1: Data transformation
    if (stage <= 1) {
        stage_barriers[0].arrive_and_wait(); // Wait for stage 0
        
        for (int i = start; i < end; ++i) {
            for (auto& v : batches[i].values) {
                v = v * 2 + 1;
            }
        }
        stage_barriers[1].arrive_and_wait();
    }
    
    // Stage 2: Data validation
    if (stage <= 2) {
        stage_barriers[1].arrive_and_wait(); // Wait for stage 1
        
        for (int i = start; i < end; ++i) {
            bool valid = true;
            for (const auto& v : batches[i].values) {
                if (v < 0) valid = false;
            }
            batches[i].processed = valid;
        }
        stage_barriers[2].arrive_and_wait();
    }
}

int main() {
    const int num_threads = 4;
    const int num_stages = 3;
    const int num_batches = 16;
    
    std::vector<DataBatch> batches(num_batches);
    std::array<std::barrier<>, 3> stage_barriers = {
        std::barrier<>(num_threads),
        std::barrier<>(num_threads),
        std::barrier<>(num_threads)
    };
    
    std::vector<std::thread> threads;
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(pipeline_worker, 0, i, 
                            std::ref(stage_barriers), std::ref(batches));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "Pipeline processing complete\n";
    int processed_count = 0;
    for (const auto& batch : batches) {
        if (batch.processed) processed_count++;
    }
    std::cout << "Successfully processed: " << processed_count 
             << "/" << num_batches << " batches\n";
    
    return 0;
}
```

## Best Practices

1. **Choose the Right Participant Count**: Set the barrier count to match the number of threads that will actually use it
2. **Use Completion Functions Wisely**: Leverage completion callbacks for phase transitions, buffer swaps, or aggregate computations
3. **Handle Dynamic Thread Counts**: Use `arrive_and_drop()` when threads need to exit early
4. **Minimize Work in Completion Functions**: Keep completion callbacks short and non-blocking
5. **Consider Exception Safety**: Ensure all threads can reach the barrier even in error conditions
6. **Avoid Deadlocks**: Never hold locks while waiting at a barrier
7. **Profile Performance**: Barriers introduce synchronization overhead; measure impact on parallel efficiency

## Common Pitfalls

- **Mismatched Participant Count**: If the barrier count doesn't match active threads, some will wait forever
- **Exceptions Across Barriers**: If a thread throws before reaching the barrier, other threads will deadlock
- **Reusing Without Proper Synchronization**: Ensure all threads complete one phase before any start the next
- **Heavy Completion Functions**: Long-running completion callbacks block all threads

## Summary

`std::barrier` is a powerful C++20 synchronization primitive designed for coordinating multiple threads through distinct phases of parallel computation. Unlike single-use latches, barriers are reusable and ideal for iterative algorithms where threads must repeatedly synchronize. The completion function mechanism provides an elegant way to perform phase-transition logic exactly once when all threads arrive. Key operations include `arrive_and_wait()` for standard synchronization, separate `arrive()` and `wait()` for flexibility, and `arrive_and_drop()` for dynamic thread management. Barriers shine in applications like parallel simulations, multi-stage pipelines, and matrix computations where lock-step execution across threads is required. Proper use of barriers can significantly simplify the coordination of phased parallel work while maintaining clean, maintainable code.