# Threads

## Passing Arguments to Threads in C++

**Basic Argument Passing**

```cpp
#include <iostream>
#include <thread>
#include <string>

void print_message(int id, std::string message) {
    std::cout << "Thread " << id << ": " << message << std::endl;
}

int main() {
    std::string msg = "Hello from thread";
    std::thread t1(print_message, 1, msg);
    std::thread t2(print_message, 2, "Direct string");
    
    t1.join();
    t2.join();
    
    return 0;
}
```
---

**Using std::ref and std::cref**

```cpp
#include <iostream>
#include <thread>
#include <functional>
#include <vector>

void increment(int& value) {
    for (int i = 0; i < 1000; ++i) {
        ++value;
    }
}

void print_info(const std::string& name, const std::vector<int>& data) {
    std::cout << name << " has " << data.size() << " elements" << std::endl;
}

int main() {
    int counter = 0;
    
    // CORRECT: Use std::ref to pass by reference
    std::thread t1(increment, std::ref(counter));
    t1.join();
    
    std::cout << "Counter after thread: " << counter << std::endl;
    
    // Using std::cref for const references
    std::string name = "MyVector";
    std::vector<int> vec = {1, 2, 3, 4, 5};
    
    std::thread t2(print_info, std::cref(name), std::cref(vec));
    t2.join();
    
    return 0;
}
```

---

**Move Semantics with Threads**

```cpp
#include <iostream>
#include <thread>
#include <memory>
#include <vector>
#include <string>

class LargeData {
    std::vector<int> data;
public:
    LargeData(size_t size) : data(size, 42) {
        std::cout << "LargeData created with " << size << " elements" << std::endl;
    }
    
    // Move constructor
    LargeData(LargeData&& other) noexcept : data(std::move(other.data)) {
        std::cout << "LargeData moved" << std::endl;
    }
    
    size_t size() const { return data.size(); }
};

void process_data(LargeData data) {
    std::cout << "Processing data with " << data.size() << " elements" << std::endl;
}

void process_unique(std::unique_ptr<std::string> ptr) {
    std::cout << "Processing: " << *ptr << std::endl;
}

int main() {
    // Moving a large object
    LargeData large(1000000);
    std::thread t1(process_data, std::move(large));
    // 'large' is now in a moved-from state and should not be used
    t1.join();
    
    // Moving a unique_ptr (move-only type)
    auto ptr = std::make_unique<std::string>("Important data");
    std::thread t2(process_unique, std::move(ptr));
    // 'ptr' is now nullptr
    t2.join();
    
    return 0;
}
```

---

**Pointer Arguments and Implicit Conversions**

```cpp
#include <iostream>
#include <thread>
#include <string>
#include <chrono>

void process_string(const std::string& s) {
    std::cout << "Processing: " << s << std::endl;
}

int main() {
    char buffer[100] = "Hello Thread";
    
    // DANGEROUS: The pointer might dangle!
    // std::thread t(process_string, buffer);
    
    // The char* is stored as-is, and conversion to std::string
    // happens in the thread context. If buffer goes out of scope
    // before the conversion, we have undefined behavior.
    
    // SAFE: Explicitly convert to std::string in the calling thread
    std::thread t(process_string, std::string(buffer));
    
    t.join();
    
    return 0;
}
```

---

**Member Function Threads**

```cpp
#include <iostream>
#include <thread>

class Worker {
    int id;
public:
    Worker(int i) : id(i) {}
    
    void do_work(int iterations) {
        for (int i = 0; i < iterations; ++i) {
            std::cout << "Worker " << id << " iteration " << i << std::endl;
        }
    }
};

int main() {
    Worker w1(1);
    Worker w2(2);
    
    // Pass member function with object reference
    std::thread t1(&Worker::do_work, &w1, 3);
    
    // Or use std::ref
    std::thread t2(&Worker::do_work, std::ref(w2), 3);
    
    t1.join();
    t2.join();
    
    return 0;
}
```

---

**Move Semantics with Threads**


***Option A — Zero moves (pass by reference)***

```cpp
// Option A — Zero moves (pass by reference)

// If the data already exists and you just want the thread to use it, don’t pass by value.
//Change the function signature
void process_data(const LargeData& data) {
    std::cout << "Processing data with " << data.size() << " elements\n";
}

//Launch the thread with std::cref
LargeData large(1000000);

std::thread t1(process_data, std::cref(large));
t1.join();
```


***Option B — Exactly one move (move once, then pass by reference)***

```cpp
// 1. Move into a local object
LargeData large(1000000);
LargeData owned = std::move(large);  // ONE move here


// 2. Pass by reference to the thread
void process_data(LargeData& data) {
    std::cout << "Processing data with " << data.size() << " elements\n";
}

std::thread t1(process_data, std::ref(owned));
t1.join();

```

***Option C — Heap ownership (common & safe)***

```cpp
void process_data(std::unique_ptr<LargeData> data) {
    std::cout << "Processing data with " << data->size() << " elements\n";
}

std::thread t1(
    process_data,
    std::make_unique<LargeData>(1000000)
);
t1.join();
```

---

## Thread identification in C++

**std::thread::id | std::this_thread::get_id()**

```cpp
#include <iostream>
#include <thread>
#include <map>
#include <mutex>

std::mutex cout_mutex;

void worker_function(int task_id) {
    std::thread::id this_id = std::this_thread::get_id();
    
    {
        std::lock_guard<std::mutex> lock(cout_mutex);
        std::cout << "Task " << task_id 
                  << " running on thread: " << this_id << std::endl;
    }
    
    // Simulate work
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
}

int main() {
    std::cout << "Main thread ID: " << std::this_thread::get_id() << std::endl;
    
    std::thread t1(worker_function, 1);
    std::thread t2(worker_function, 2);
    std::thread t3(worker_function, 3);
    
    // Store thread IDs for later reference
    std::thread::id t1_id = t1.get_id();
    std::thread::id t2_id = t2.get_id();
    
    std::cout << "Launched thread 1 with ID: " << t1_id << std::endl;
    std::cout << "Launched thread 2 with ID: " << t2_id << std::endl;
    
    t1.join();
    t2.join();
    t3.join();
    
    return 0;
}
```

---

**Using Thread IDs as Map Keys**

```cpp
    std::map<std::thread::id, std::string> thread_names;
    std::map<std::thread::id, int> operation_counts;
```

---

**std::thread::hardware_concurrency()**

```cpp
#include <iostream>
#include <thread>
#include <vector>
#include <queue>
#include <functional>
#include <mutex>
#include <condition_variable>

class SimpleThreadPool {
private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;
    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;

public:
    SimpleThreadPool(size_t num_threads) : stop(false) {
        for (size_t i = 0; i < num_threads; ++i) {
            workers.emplace_back([this] {
                while (true) {
                    std::function<void()> task;
                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] { 
                            return stop || !tasks.empty(); 
                        });
                        
                        if (stop && tasks.empty()) return;
                        
                        task = std::move(tasks.front());
                        tasks.pop();
                    }
                    task();
                }
            });
        }
    }
    
    template<class F>
    void enqueue(F&& f) {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.emplace(std::forward<F>(f));
        }
        condition.notify_one();
    }
    
    ~SimpleThreadPool() {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (std::thread& worker : workers) {
            worker.join();
        }
    }
};

int main() {
    unsigned int num_cores = std::thread::hardware_concurrency();
    
    std::cout << "Hardware concurrency: " << num_cores << " cores\n";
    
    // Create thread pool with optimal size
    size_t pool_size = (num_cores > 0) ? num_cores : 2;
    std::cout << "Creating thread pool with " << pool_size << " threads\n";
    
    SimpleThreadPool pool(pool_size);
    
    // Enqueue tasks
    for (int i = 0; i < 20; ++i) {
        pool.enqueue([i] {
            std::cout << "Task " << i << " executed on thread " 
                      << std::this_thread::get_id() << std::endl;
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
        });
    }
    
    // Pool destructor will wait for all tasks to complete
    std::this_thread::sleep_for(std::chrono::seconds(3));
    
    return 0;
}
```

