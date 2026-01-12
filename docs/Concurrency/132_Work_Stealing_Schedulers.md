# Work Stealing Schedulers

## Overview

Work stealing is a sophisticated scheduling algorithm designed to achieve dynamic load balancing in parallel computing systems. The fundamental principle is elegantly simple: when a worker thread runs out of tasks in its own queue, it "steals" work from other busy threads rather than remaining idle. This approach minimizes thread idleness and maximizes CPU utilization, making it particularly effective for irregular parallel workloads where task execution times are unpredictable.

## Core Concepts

**Work stealing** operates on the principle of distributed task queues. Each worker thread maintains its own double-ended queue (deque) of tasks. The thread operates on its own queue using LIFO (Last-In-First-Out) order from one end, while potential thieves access the queue using FIFO (First-In-First-Out) order from the opposite end. This design minimizes contention between the owner and thieves.

The key advantages of work stealing include:

- **Automatic load balancing**: Work naturally migrates from busy threads to idle ones
- **Cache locality**: Threads primarily work on their own tasks, maintaining good cache performance
- **Scalability**: The distributed nature reduces contention on centralized data structures
- **Adaptability**: Handles irregular workloads where task durations vary significantly

## Implementation Components

A work-stealing scheduler typically consists of several core components:

**Work-Stealing Deque**: A lock-free or low-contention double-ended queue that allows the owner to push and pop from one end while thieves steal from the other end.

**Thread Pool**: A collection of worker threads, each with its own deque, that execute tasks and steal work when idle.

**Task Representation**: A unit of work that can be executed independently, often supporting nested parallelism through task spawning.

## Code Examples

### Basic Work-Stealing Deque

```cpp
#include <deque>
#include <mutex>
#include <optional>
#include <functional>
#include <thread>
#include <vector>
#include <atomic>
#include <random>

// Simple task type
using Task = std::function<void()>;

// Work-stealing deque for a single thread
class WorkStealingDeque {
private:
    std::deque<Task> tasks_;
    mutable std::mutex mutex_;

public:
    // Owner pushes to the back (private end)
    void push(Task task) {
        std::lock_guard<std::mutex> lock(mutex_);
        tasks_.push_back(std::move(task));
    }

    // Owner pops from the back (LIFO for better cache locality)
    std::optional<Task> pop() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tasks_.empty()) {
            return std::nullopt;
        }
        Task task = std::move(tasks_.back());
        tasks_.pop_back();
        return task;
    }

    // Thieves steal from the front (FIFO)
    std::optional<Task> steal() {
        std::lock_guard<std::mutex> lock(mutex_);
        if (tasks_.empty()) {
            return std::nullopt;
        }
        Task task = std::move(tasks_.front());
        tasks_.pop_front();
        return task;
    }

    bool empty() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return tasks_.empty();
    }

    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return tasks_.size();
    }
};
```

### Work-Stealing Thread Pool

```cpp
class WorkStealingThreadPool {
private:
    std::vector<std::unique_ptr<WorkStealingDeque>> queues_;
    std::vector<std::thread> workers_;
    std::atomic<bool> done_{false};
    std::atomic<size_t> active_tasks_{0};
    
    // Thread-local index for the current worker
    static thread_local size_t thread_index_;
    static thread_local WorkStealingThreadPool* current_pool_;

    // Worker thread function
    void worker_thread(size_t index) {
        thread_index_ = index;
        current_pool_ = this;
        
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<size_t> dist(0, queues_.size() - 1);

        while (!done_) {
            // Try to get work from own queue first
            if (auto task = queues_[index]->pop()) {
                (*task)();
                active_tasks_.fetch_sub(1, std::memory_order_release);
                continue;
            }

            // Try to steal from other queues
            bool found_work = false;
            for (size_t attempts = 0; attempts < queues_.size(); ++attempts) {
                size_t victim = dist(gen);
                if (victim == index) continue;

                if (auto stolen_task = queues_[victim]->steal()) {
                    (*stolen_task)();
                    active_tasks_.fetch_sub(1, std::memory_order_release);
                    found_work = true;
                    break;
                }
            }

            if (!found_work) {
                // No work available, yield to avoid busy waiting
                std::this_thread::yield();
            }
        }
    }

public:
    explicit WorkStealingThreadPool(size_t num_threads = std::thread::hardware_concurrency()) {
        queues_.reserve(num_threads);
        workers_.reserve(num_threads);

        // Create queues
        for (size_t i = 0; i < num_threads; ++i) {
            queues_.emplace_back(std::make_unique<WorkStealingDeque>());
        }

        // Create worker threads
        for (size_t i = 0; i < num_threads; ++i) {
            workers_.emplace_back([this, i] { worker_thread(i); });
        }
    }

    ~WorkStealingThreadPool() {
        done_ = true;
        for (auto& worker : workers_) {
            if (worker.joinable()) {
                worker.join();
            }
        }
    }

    // Submit a task to the pool
    void submit(Task task) {
        active_tasks_.fetch_add(1, std::memory_order_acquire);
        
        // If called from a worker thread, use its queue
        if (current_pool_ == this && thread_index_ < queues_.size()) {
            queues_[thread_index_]->push(std::move(task));
        } else {
            // Otherwise, use a random queue
            static std::atomic<size_t> counter{0};
            size_t index = counter.fetch_add(1, std::memory_order_relaxed) % queues_.size();
            queues_[index]->push(std::move(task));
        }
    }

    // Wait for all tasks to complete
    void wait() {
        while (active_tasks_.load(std::memory_order_acquire) > 0) {
            std::this_thread::yield();
        }
    }

    size_t num_threads() const {
        return workers_.size();
    }
};

// Initialize thread-local variables
thread_local size_t WorkStealingThreadPool::thread_index_ = 0;
thread_local WorkStealingThreadPool* WorkStealingThreadPool::current_pool_ = nullptr;
```

### Parallel Recursion with Work Stealing

```cpp
#include <iostream>
#include <vector>
#include <numeric>

// Parallel merge sort using work stealing
void parallel_merge(std::vector<int>& result, 
                   const std::vector<int>& left,
                   const std::vector<int>& right) {
    size_t i = 0, j = 0, k = 0;
    
    while (i < left.size() && j < right.size()) {
        if (left[i] <= right[j]) {
            result[k++] = left[i++];
        } else {
            result[k++] = right[j++];
        }
    }
    
    while (i < left.size()) result[k++] = left[i++];
    while (j < right.size()) result[k++] = right[j++];
}

void parallel_merge_sort(WorkStealingThreadPool& pool,
                        std::vector<int>& data,
                        size_t threshold = 1000) {
    if (data.size() <= 1) return;
    
    // Use sequential sort for small arrays
    if (data.size() < threshold) {
        std::sort(data.begin(), data.end());
        return;
    }
    
    size_t mid = data.size() / 2;
    std::vector<int> left(data.begin(), data.begin() + mid);
    std::vector<int> right(data.begin() + mid, data.end());
    
    std::atomic<int> tasks_done{0};
    
    // Spawn parallel tasks for left and right halves
    pool.submit([&pool, &left, threshold, &tasks_done]() {
        parallel_merge_sort(pool, left, threshold);
        tasks_done.fetch_add(1, std::memory_order_release);
    });
    
    pool.submit([&pool, &right, threshold, &tasks_done]() {
        parallel_merge_sort(pool, right, threshold);
        tasks_done.fetch_add(1, std::memory_order_release);
    });
    
    // Wait for both tasks to complete
    while (tasks_done.load(std::memory_order_acquire) < 2) {
        std::this_thread::yield();
    }
    
    // Merge the sorted halves
    parallel_merge(data, left, right);
}
```

### Advanced Example: Parallel Tree Traversal

```cpp
#include <memory>

template<typename T>
struct TreeNode {
    T value;
    std::shared_ptr<TreeNode<T>> left;
    std::shared_ptr<TreeNode<T>> right;
    
    TreeNode(T val) : value(val), left(nullptr), right(nullptr) {}
};

// Parallel tree sum using work stealing
template<typename T>
T parallel_tree_sum(WorkStealingThreadPool& pool,
                    std::shared_ptr<TreeNode<T>> node,
                    size_t depth_threshold = 3) {
    if (!node) return T{};
    
    // Sequential processing for deep subtrees
    if (depth_threshold == 0) {
        T sum = node->value;
        if (node->left) sum += parallel_tree_sum(pool, node->left, 0);
        if (node->right) sum += parallel_tree_sum(pool, node->right, 0);
        return sum;
    }
    
    std::atomic<T> left_sum{0};
    std::atomic<T> right_sum{0};
    std::atomic<int> tasks_done{0};
    
    // Process left subtree in parallel
    if (node->left) {
        pool.submit([&pool, &left_sum, &tasks_done, left_node = node->left, depth_threshold]() {
            T result = parallel_tree_sum(pool, left_node, depth_threshold - 1);
            left_sum.store(result, std::memory_order_release);
            tasks_done.fetch_add(1, std::memory_order_release);
        });
    } else {
        tasks_done.fetch_add(1, std::memory_order_release);
    }
    
    // Process right subtree in parallel
    if (node->right) {
        pool.submit([&pool, &right_sum, &tasks_done, right_node = node->right, depth_threshold]() {
            T result = parallel_tree_sum(pool, right_node, depth_threshold - 1);
            right_sum.store(result, std::memory_order_release);
            tasks_done.fetch_add(1, std::memory_order_release);
        });
    } else {
        tasks_done.fetch_add(1, std::memory_order_release);
    }
    
    // Wait for subtasks
    while (tasks_done.load(std::memory_order_acquire) < 2) {
        std::this_thread::yield();
    }
    
    return node->value + left_sum.load(std::memory_order_acquire) + 
           right_sum.load(std::memory_order_acquire);
}
```

### Practical Usage Example

```cpp
#include <chrono>

int main() {
    // Create a work-stealing thread pool
    WorkStealingThreadPool pool(std::thread::hardware_concurrency());
    
    std::cout << "Work-Stealing Thread Pool with " 
              << pool.num_threads() << " threads\n\n";
    
    // Example 1: Simple parallel tasks
    std::cout << "Example 1: Parallel task execution\n";
    std::atomic<int> counter{0};
    
    for (int i = 0; i < 100; ++i) {
        pool.submit([&counter, i]() {
            // Simulate work
            std::this_thread::sleep_for(std::chrono::milliseconds(10));
            counter.fetch_add(1, std::memory_order_relaxed);
        });
    }
    
    pool.wait();
    std::cout << "Completed " << counter.load() << " tasks\n\n";
    
    // Example 2: Parallel merge sort
    std::cout << "Example 2: Parallel merge sort\n";
    std::vector<int> data(100000);
    std::iota(data.rbegin(), data.rend(), 1); // Reverse sorted
    
    auto start = std::chrono::high_resolution_clock::now();
    parallel_merge_sort(pool, data, 5000);
    auto end = std::chrono::high_resolution_clock::now();
    
    bool sorted = std::is_sorted(data.begin(), data.end());
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    
    std::cout << "Sorted: " << (sorted ? "Yes" : "No") << "\n";
    std::cout << "Time: " << duration.count() << " ms\n\n";
    
    // Example 3: Parallel tree traversal
    std::cout << "Example 3: Parallel tree sum\n";
    auto root = std::make_shared<TreeNode<int>>(1);
    root->left = std::make_shared<TreeNode<int>>(2);
    root->right = std::make_shared<TreeNode<int>>(3);
    root->left->left = std::make_shared<TreeNode<int>>(4);
    root->left->right = std::make_shared<TreeNode<int>>(5);
    root->right->left = std::make_shared<TreeNode<int>>(6);
    root->right->right = std::make_shared<TreeNode<int>>(7);
    
    int sum = parallel_tree_sum(pool, root);
    std::cout << "Tree sum: " << sum << " (expected: 28)\n";
    
    return 0;
}
```

## Performance Considerations

Work stealing schedulers excel in scenarios where task granularity and execution times are irregular. However, several factors affect their performance:

**Contention Management**: The deque implementation must minimize lock contention. Lock-free implementations using atomic operations can significantly improve scalability, though they are more complex to implement correctly.

**Stealing Strategy**: Random victim selection distributes stealing attempts uniformly, but other strategies like stealing from neighbors or based on queue sizes can be more effective in certain scenarios.

**Task Granularity**: Tasks should be neither too fine-grained (causing excessive overhead) nor too coarse-grained (limiting parallelism). Adaptive thresholds that switch between parallel and sequential execution are often beneficial.

**Cache Effects**: The LIFO order for the owner thread promotes cache locality since recently created tasks often access similar data. FIFO order for thieves minimizes interference with the owner's cache.

## Summary

Work stealing schedulers provide an elegant and efficient solution for dynamic load balancing in parallel systems. By allowing idle threads to steal work from busy threads, they achieve excellent CPU utilization while maintaining good cache locality through distributed task queues. The design is particularly effective for divide-and-conquer algorithms and irregular workloads where traditional static scheduling would leave threads idle.

The key to successful work stealing lies in the efficient implementation of the work-stealing deque, minimizing contention between owners and thieves, and choosing appropriate task granularity. Modern parallel frameworks like Intel TBB, Cilk Plus, and the C++ parallel algorithms library often employ work stealing as their underlying scheduling mechanism due to its proven effectiveness and scalability.