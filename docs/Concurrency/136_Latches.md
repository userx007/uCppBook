# C++ Concurrency: Latches (std::latch)

## Overview

`std::latch` is a synchronization primitive introduced in C++20 that enables one-time coordination between multiple threads. It acts as a countdown mechanism where threads can wait until a specified count reaches zero. Once the latch count hits zero, all waiting threads are released simultaneously, and the latch cannot be reused.

Think of a latch like a starting gate at a race: once all participants are ready (count reaches zero), the gate opens and everyone proceeds. Unlike barriers, latches are single-use and don't reset.

## Key Characteristics

- **Single-use**: Once the count reaches zero, the latch cannot be reset or reused
- **Thread-safe**: Multiple threads can safely decrement the counter
- **Non-blocking options**: Threads can wait or check status without blocking
- **Efficient**: Designed for minimal overhead in synchronization scenarios

## Basic Usage

### Constructor and Core Methods

```cpp
#include <latch>
#include <thread>
#include <iostream>
#include <vector>

// Create a latch with initial count
std::latch my_latch(3);  // Count of 3

// Core operations:
my_latch.count_down();      // Decrement counter by 1
my_latch.wait();            // Block until count reaches 0
my_latch.arrive_and_wait(); // Decrement and wait
bool ready = my_latch.try_wait(); // Non-blocking check
```

## Practical Examples

### Example 1: Worker Thread Coordination

This example demonstrates using a latch to ensure all worker threads complete initialization before proceeding:

```cpp
#include <latch>
#include <thread>
#include <iostream>
#include <vector>
#include <chrono>

void worker_task(int id, std::latch& ready_latch) {
    // Simulate initialization work
    std::cout << "Worker " << id << " initializing...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    
    std::cout << "Worker " << id << " ready!\n";
    
    // Signal that this worker is ready
    ready_latch.count_down();
    
    // Wait for all workers to be ready
    ready_latch.wait();
    
    // Now all workers proceed together
    std::cout << "Worker " << id << " starting main work!\n";
}

int main() {
    const int num_workers = 4;
    std::latch ready_latch(num_workers);
    
    std::vector<std::thread> workers;
    
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back(worker_task, i, std::ref(ready_latch));
    }
    
    for (auto& t : workers) {
        t.join();
    }
    
    return 0;
}
```

### Example 2: Main Thread Waiting for Workers

A common pattern where the main thread spawns workers and waits for all to complete:

```cpp
#include <latch>
#include <thread>
#include <iostream>
#include <vector>

void process_data(int id, std::latch& completion_latch) {
    std::cout << "Thread " << id << " processing data...\n";
    std::this_thread::sleep_for(std::chrono::seconds(1));
    std::cout << "Thread " << id << " finished!\n";
    
    // Signal completion
    completion_latch.count_down();
}

int main() {
    const int num_threads = 5;
    std::latch completion_latch(num_threads);
    
    std::vector<std::thread> threads;
    
    // Spawn worker threads
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(process_data, i, std::ref(completion_latch));
    }
    
    // Main thread can do other work here
    std::cout << "Main thread waiting for workers...\n";
    
    // Wait for all workers to complete
    completion_latch.wait();
    
    std::cout << "All workers completed! Main thread proceeding.\n";
    
    // Clean up
    for (auto& t : threads) {
        t.join();
    }
    
    return 0;
}
```

### Example 3: arrive_and_wait() Usage

Demonstrating the combined decrement-and-wait operation:

```cpp
#include <latch>
#include <thread>
#include <iostream>
#include <vector>

void stage_worker(int id, std::latch& stage_latch) {
    std::cout << "Worker " << id << " preparing...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100 * id));
    
    // Decrement and wait in one atomic operation
    std::cout << "Worker " << id << " ready and waiting...\n";
    stage_latch.arrive_and_wait();
    
    // All workers reach here simultaneously
    std::cout << "Worker " << id << " executing synchronized phase!\n";
}

int main() {
    const int num_workers = 3;
    std::latch stage_latch(num_workers);
    
    std::vector<std::thread> workers;
    
    for (int i = 0; i < num_workers; ++i) {
        workers.emplace_back(stage_worker, i, std::ref(stage_latch));
    }
    
    for (auto& t : workers) {
        t.join();
    }
    
    return 0;
}
```

### Example 4: Multi-Stage Pipeline with Multiple Latches

Using multiple latches for different synchronization points:

```cpp
#include <latch>
#include <thread>
#include <iostream>
#include <vector>

void pipeline_stage(int id, std::latch& init_latch, 
                   std::latch& compute_latch, std::latch& finalize_latch) {
    // Stage 1: Initialization
    std::cout << "Thread " << id << " initializing...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    init_latch.arrive_and_wait();
    
    // Stage 2: Computation (all threads start together)
    std::cout << "Thread " << id << " computing...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    compute_latch.arrive_and_wait();
    
    // Stage 3: Finalization (all threads start together)
    std::cout << "Thread " << id << " finalizing...\n";
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
    finalize_latch.arrive_and_wait();
    
    std::cout << "Thread " << id << " complete!\n";
}

int main() {
    const int num_threads = 4;
    
    std::latch init_latch(num_threads);
    std::latch compute_latch(num_threads);
    std::latch finalize_latch(num_threads);
    
    std::vector<std::thread> threads;
    
    for (int i = 0; i < num_threads; ++i) {
        threads.emplace_back(pipeline_stage, i, 
                           std::ref(init_latch), 
                           std::ref(compute_latch), 
                           std::ref(finalize_latch));
    }
    
    for (auto& t : threads) {
        t.join();
    }
    
    std::cout << "All pipeline stages completed!\n";
    
    return 0;
}
```

### Example 5: Non-blocking Status Check

Using `try_wait()` for non-blocking latch checks:

```cpp
#include <latch>
#include <thread>
#include <iostream>
#include <chrono>

void background_worker(std::latch& work_done) {
    std::cout << "Background task starting...\n";
    std::this_thread::sleep_for(std::chrono::seconds(2));
    std::cout << "Background task completed!\n";
    work_done.count_down();
}

int main() {
    std::latch work_done(1);
    
    std::thread worker(background_worker, std::ref(work_done));
    
    // Main thread can check status without blocking
    std::cout << "Main thread checking status periodically...\n";
    
    while (!work_done.try_wait()) {
        std::cout << "Work still in progress... doing other tasks\n";
        std::this_thread::sleep_for(std::chrono::milliseconds(500));
    }
    
    std::cout << "Work completed! Main thread proceeding.\n";
    
    worker.join();
    
    return 0;
}
```

## When to Use std::latch

**Use latches when:**
- You need one-time synchronization of multiple threads
- Threads need to wait for a specific number of events to complete
- You want to coordinate a "start together" scenario after initialization
- You need a simple countdown mechanism with wait functionality

**Don't use latches when:**
- You need reusable synchronization (use `std::barrier` instead)
- You need mutual exclusion for shared data (use `std::mutex`)
- You need ongoing producer-consumer coordination (use `std::condition_variable`)
- Only single-thread notification is needed (use `std::future`/`std::promise`)

## Summary

`std::latch` is a powerful C++20 synchronization primitive that provides efficient one-time coordination between multiple threads through a countdown mechanism. Its key operations include `count_down()` for decrementing the counter, `wait()` for blocking until the count reaches zero, and `arrive_and_wait()` for combining both actions atomically. Latches are ideal for scenarios like ensuring all threads complete initialization before proceeding, coordinating simultaneous starts, or waiting for parallel task completion. Unlike barriers, latches are single-use and cannot be reset, making them perfect for one-shot synchronization points in concurrent applications. With minimal overhead and a simple API, `std::latch` simplifies complex thread coordination patterns while maintaining type safety and efficiency.