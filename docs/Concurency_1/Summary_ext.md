# C++ Concurrency Primitives: Timeline with Linked Examples (C++11 → C++26)

Click a primitive in any table to jump to its example; click "↩ back to table" under an example to return. Links use GitHub's actual heading-anchor algorithm (every jump target is a real heading), so this works both here and on GitHub.

---

## C++11 — The Foundation

| Primitive | Header | Purpose |
| --- | --- | --- |
| [`std::thread`](#stdthread) | `<thread>` | Create/manage an OS thread |
| [`std::this_thread::*`](#stdthis_thread) | `<thread>` | Sleep/yield/get id of current thread |
| [`thread_local`](#thread_local) | `keyword` | Per-thread variable storage |
| [`std::mutex`](#stdmutex) | `<mutex>` | Basic mutual exclusion |
| [`std::recursive_mutex`](#stdrecursive_mutex) | `<mutex>` | Same-thread re-lockable mutex |
| [`std::timed_mutex`](#stdtimed_mutex) | `<mutex>` | Mutex with timed lock attempts |
| [`.try_lock()`](#try_lock) | `<mutex>` | Non-blocking lock attempt |
| [`std::lock_guard`](#stdlock_guard) | `<mutex>` | Scope-based exclusive lock |
| [`std::unique_lock`](#stdunique_lock) | `<mutex>` | Flexible exclusive lock |
| [`std::lock`](#stdlock) | `<mutex>` | Deadlock-free multi-lock |
| [`std::call_once`](#stdcall_once) | `<mutex>` | Thread-safe run-exactly-once |
| [`std::condition_variable`](#stdcondition_variable) | `<condition_variable>` | Block until notified |
| [`std::condition_variable_any`](#stdcondition_variable_any) | `<condition_variable>` | Same, any lock type |
| [`std::cv_status`](#stdcv_status) | `<condition_variable>` | Timed-wait result |
| [`std::atomic<T>`](#stdatomict) | `<atomic>` | Atomic ops on a type |
| [`std::atomic_flag`](#stdatomic_flag) | `<atomic>` | Lock-free boolean flag |
| [`Memory orders`](#memory-orders) | `<atomic>` | Ordering guarantees per-op |
| [`std::promise/std::future`](#stdpromisestdfuture) | `<future>` | One-shot value handoff |
| [`std::async`](#stdasync) | `<future>` | Run task async, get future |
| [`std::packaged_task`](#stdpackaged_task) | `<future>` | Wrap callable → future |
| [`std::shared_future`](#stdshared_future) | `<future>` | Multi-consumer future |

#### `std::thread`

```cpp
#include <thread>
#include <iostream>
void hello() { std::cout << "hi from thread\n"; }
int main() {
    std::thread t(hello);
    t.join();
}
```

[↩ back to table](#c11--the-foundation)

#### `std::this_thread::*`

```cpp
#include <thread>
#include <chrono>
int main() {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    std::this_thread::yield();
}
```

[↩ back to table](#c11--the-foundation)

#### `thread_local`

```cpp
#include <thread>
#include <iostream>
thread_local int counter = 0;
void bump() { std::cout << ++counter << "\n"; } // always prints 1
int main() {
    std::thread t1(bump), t2(bump);
    t1.join(); t2.join();
}
```

[↩ back to table](#c11--the-foundation)

#### `std::mutex`

```cpp
#include <mutex>
#include <thread>
std::mutex m;
int shared_val = 0;
void inc() { m.lock(); ++shared_val; m.unlock(); }
int main() {
    std::thread t1(inc), t2(inc);
    t1.join(); t2.join();
}
```

[↩ back to table](#c11--the-foundation)

#### `std::recursive_mutex`

```cpp
#include <mutex>
std::recursive_mutex rm;
void nested() { rm.lock(); rm.lock(); /* same thread OK */ rm.unlock(); rm.unlock(); }
int main() { nested(); }
```

[↩ back to table](#c11--the-foundation)

#### `std::timed_mutex`

```cpp
#include <mutex>
#include <chrono>
#include <iostream>
std::timed_mutex tm;
int main() {
    if (tm.try_lock_for(std::chrono::milliseconds(50))) {
        std::cout << "got it\n";
        tm.unlock();
    }
}
```

[↩ back to table](#c11--the-foundation)

#### `.try_lock()`

```cpp
#include <mutex>
#include <iostream>
std::mutex m;
int main() {
    if (m.try_lock()) {
        std::cout << "acquired without blocking\n";
        m.unlock();
    }
}
```

[↩ back to table](#c11--the-foundation)

#### `std::lock_guard`

```cpp
#include <mutex>
std::mutex m;
int shared_val = 0;
void inc() {
    std::lock_guard<std::mutex> lk(m); // locked here
    ++shared_val;
} // auto-unlocked here
int main() { inc(); }
```

[↩ back to table](#c11--the-foundation)

#### `std::unique_lock`

```cpp
#include <mutex>
std::mutex m;
int main() {
    std::unique_lock<std::mutex> lk(m, std::defer_lock); // not locked yet
    lk.lock();
    // ... critical section ...
    lk.unlock();
}
```

[↩ back to table](#c11--the-foundation)

#### `std::lock`

```cpp
#include <mutex>
std::mutex m1, m2;
int main() {
    std::lock(m1, m2); // locks both, deadlock-free
    std::lock_guard<std::mutex> l1(m1, std::adopt_lock);
    std::lock_guard<std::mutex> l2(m2, std::adopt_lock);
}
```

[↩ back to table](#c11--the-foundation)

#### `std::call_once`

```cpp
#include <mutex>
#include <iostream>
std::once_flag flag;
void init() { std::cout << "initialized\n"; }
int main() {
    std::call_once(flag, init);
    std::call_once(flag, init); // won't run init() again
}
```

[↩ back to table](#c11--the-foundation)

#### `std::condition_variable`

```cpp
#include <condition_variable>
#include <mutex>
#include <thread>
std::mutex m;
std::condition_variable cv;
bool ready = false;
int main() {
    std::thread t([]{
        std::unique_lock<std::mutex> lk(m);
        cv.wait(lk, [] { return ready; }); // sleeps until notified
    });
    { std::lock_guard<std::mutex> lk(m); ready = true; }
    cv.notify_one();
    t.join();
}
```

[↩ back to table](#c11--the-foundation)

#### `std::condition_variable_any`

```cpp
#include <condition_variable>
#include <mutex>
std::recursive_mutex rm;
std::condition_variable_any cv_any;
int main() {
    std::unique_lock<std::recursive_mutex> lk(rm);
    // cv_any.wait(lk, [] { return true; }); // works with non-std::mutex locks too
}
```

[↩ back to table](#c11--the-foundation)

#### `std::cv_status`

```cpp
#include <condition_variable>
#include <mutex>
#include <chrono>
#include <iostream>
std::mutex m;
std::condition_variable cv;
int main() {
    std::unique_lock<std::mutex> lk(m);
    if (cv.wait_for(lk, std::chrono::milliseconds(50)) == std::cv_status::timeout)
        std::cout << "timed out\n";
}
```

[↩ back to table](#c11--the-foundation)

#### `std::atomic<T>`

```cpp
#include <atomic>
#include <thread>
#include <iostream>
std::atomic<int> counter{0};
void inc() { counter.fetch_add(1); }
int main() {
    std::thread t1(inc), t2(inc);
    t1.join(); t2.join();
    std::cout << counter.load() << "\n"; // 2
}
```

[↩ back to table](#c11--the-foundation)

#### `std::atomic_flag`

```cpp
#include <atomic>
std::atomic_flag flag = ATOMIC_FLAG_INIT;
void spin_acquire() { while (flag.test_and_set()) {} }
void release() { flag.clear(); }
int main() { spin_acquire(); release(); }
```

[↩ back to table](#c11--the-foundation)

#### `Memory orders`

```cpp
#include <atomic>
std::atomic<int> value{0};
std::atomic<bool> ready{false};
void writer() {
    value.store(42, std::memory_order_relaxed);
    ready.store(true, std::memory_order_release);
}
int main() { writer(); }
```

[↩ back to table](#c11--the-foundation)

#### `std::promise/std::future`

```cpp
#include <future>
#include <thread>
#include <iostream>
void compute(std::promise<int> p) { p.set_value(6 * 7); }
int main() {
    std::promise<int> prom;
    std::future<int> fut = prom.get_future();
    std::thread t(compute, std::move(prom));
    std::cout << fut.get() << "\n"; // 42
    t.join();
}
```

[↩ back to table](#c11--the-foundation)

#### `std::async`

```cpp
#include <future>
#include <iostream>
int square(int x) { return x * x; }
int main() {
    std::future<int> fut = std::async(std::launch::async, square, 12);
    std::cout << fut.get() << "\n"; // 144
}
```

[↩ back to table](#c11--the-foundation)

#### `std::packaged_task`

```cpp
#include <future>
#include <thread>
#include <iostream>
int add(int a, int b) { return a + b; }
int main() {
    std::packaged_task<int(int, int)> task(add);
    std::future<int> fut = task.get_future();
    std::thread t(std::move(task), 3, 4);
    t.join();
    std::cout << fut.get() << "\n"; // 7
}
```

[↩ back to table](#c11--the-foundation)

#### `std::shared_future`

```cpp
#include <future>
#include <thread>
#include <iostream>
int main() {
    std::promise<int> prom;
    std::shared_future<int> sf = prom.get_future().share();
    std::thread t1([sf]{ std::cout << sf.get() << "\n"; });
    std::thread t2([sf]{ std::cout << sf.get() << "\n"; });
    prom.set_value(99);
    t1.join(); t2.join();
}
```

[↩ back to table](#c11--the-foundation)

---

## C++14 — Small Refinement

| Primitive | Header | Purpose |
| --- | --- | --- |
| [`std::shared_timed_mutex`](#stdshared_timed_mutex) | `<shared_mutex>` | Reader/writer lock, timed |
| [`std::shared_lock`](#stdshared_lock) | `<shared_mutex>` | RAII shared-ownership lock |
| [`chrono literals`](#chrono-literals) | `<chrono>` | Concise durations |

#### `std::shared_timed_mutex`

```cpp
#include <shared_mutex>
std::shared_timed_mutex stm;
int main() {
    stm.lock_shared();   // many readers allowed
    stm.unlock_shared();
    stm.lock();           // one writer at a time
    stm.unlock();
}
```

[↩ back to table](#c14--small-refinement)

#### `std::shared_lock`

```cpp
#include <shared_mutex>
std::shared_timed_mutex stm;
void read() {
    std::shared_lock<std::shared_timed_mutex> lk(stm); // RAII shared lock
} // auto-released
int main() { read(); }
```

[↩ back to table](#c14--small-refinement)

#### `chrono literals`

```cpp
#include <chrono>
#include <thread>
using namespace std::chrono_literals;
int main() {
    std::this_thread::sleep_for(100ms); // instead of milliseconds(100)
}
```

[↩ back to table](#c14--small-refinement)

---

## C++17 — Parallelism Enters the Standard

| Primitive | Header | Purpose |
| --- | --- | --- |
| [`std::shared_mutex`](#stdshared_mutex) | `<shared_mutex>` | Reader/writer lock, no timing |
| [`std::scoped_lock`](#stdscoped_lock) | `<mutex>` | Variadic deadlock-free lock |
| [`Execution policies`](#execution-policies) | `<execution>` | Parallelize algorithms |
| [`atomic::is_always_lock_free`](#atomicis_always_lock_free) | `<atomic>` | Compile-time lock-free check |
| [`hardware_destructive_interference_size`](#hardware_destructive_interference_size) | `<new>` | Avoid false sharing |

#### `std::shared_mutex`

```cpp
#include <shared_mutex>
std::shared_mutex sm;
int main() {
    std::shared_lock<std::shared_mutex> lk(sm); // read lock, no timing support needed
}
```

[↩ back to table](#c17--parallelism-enters-the-standard)

#### `std::scoped_lock`

```cpp
#include <mutex>
std::mutex m1, m2;
int main() {
    std::scoped_lock lock(m1, m2); // locks both, deadlock-free, one line
}
```

[↩ back to table](#c17--parallelism-enters-the-standard)

#### `Execution policies`

```cpp
#include <algorithm>
#include <execution>
#include <vector>
int main() {
    std::vector<int> v{5, 3, 1, 4, 2};
    std::sort(std::execution::par, v.begin(), v.end()); // parallel sort
}
```

[↩ back to table](#c17--parallelism-enters-the-standard)

#### `atomic::is_always_lock_free`

```cpp
#include <atomic>
int main() {
    static_assert(std::atomic<int>::is_always_lock_free,
                  "int atomics must be lock-free here");
}
```

[↩ back to table](#c17--parallelism-enters-the-standard)

#### `hardware_destructive_interference_size`

```cpp
#include <new>
#include <atomic>
struct alignas(std::hardware_destructive_interference_size) Padded {
    std::atomic<int> value{0}; // avoids false sharing with neighbors
};
int main() { Padded p; }
```

[↩ back to table](#c17--parallelism-enters-the-standard)

---

## C++20 — The Big Expansion

| Primitive | Header | Purpose |
| --- | --- | --- |
| [`std::jthread`](#stdjthread) | `<thread>` | Self-joining, cancellable thread |
| [`stop_token/stop_source`](#stop_tokenstop_source) | `<stop_token>` | Cooperative cancellation |
| [`std::latch`](#stdlatch) | `<latch>` | One-shot countdown |
| [`std::barrier`](#stdbarrier) | `<barrier>` | Reusable phase sync |
| [`counting_semaphore`](#counting_semaphore) | `<semaphore>` | Resource/signal count |
| [`atomic::wait/notify`](#atomicwaitnotify) | `<atomic>` | Block/wake on atomic change |
| [`atomic_flag::test()`](#atomic_flagtest) | `<atomic>` | Non-destructive flag read |
| [`std::atomic_ref`](#stdatomic_ref) | `<atomic>` | Atomic ops on existing object |
| [`atomic<shared_ptr<T>>`](#atomicshared_ptrt) | `<memory>` | Atomic smart-pointer ops |
| [`std::osyncstream`](#stdosyncstream) | `<syncstream>` | Interleave-free output |
| [`Coroutines`](#coroutines) | `<coroutine>` | Suspend/resume functions |

#### `std::jthread`

```cpp
#include <thread>
#include <chrono>
void work(std::stop_token st) {
    while (!st.stop_requested())
        std::this_thread::sleep_for(std::chrono::milliseconds(10));
}
int main() {
    std::jthread t(work); // auto request_stop()+join() on destruction
    std::this_thread::sleep_for(std::chrono::milliseconds(50));
}
```

[↩ back to table](#c20--the-big-expansion)

#### `stop_token/stop_source`

```cpp
#include <stop_token>
#include <thread>
std::stop_source src;
void work(std::stop_token tok) { while (!tok.stop_requested()) {} }
int main() {
    std::thread t(work, src.get_token());
    src.request_stop();
    t.join();
}
```

[↩ back to table](#c20--the-big-expansion)

#### `std::latch`

```cpp
#include <latch>
#include <thread>
std::latch done(2);
void worker() { done.count_down(); }
int main() {
    std::thread t1(worker), t2(worker);
    done.wait(); // blocks until both count down
    t1.join(); t2.join();
}
```

[↩ back to table](#c20--the-big-expansion)

#### `std::barrier`

```cpp
#include <barrier>
#include <thread>
std::barrier sync_point(2);
void work() {
    // phase 1 work
    sync_point.arrive_and_wait(); // waits for the other thread
    // phase 2 work
}
int main() {
    std::thread t1(work), t2(work);
    t1.join(); t2.join();
}
```

[↩ back to table](#c20--the-big-expansion)

#### `counting_semaphore`

```cpp
#include <semaphore>
#include <thread>
std::counting_semaphore<2> slots(2); // max 2 concurrent
void use() { slots.acquire(); /* work */ slots.release(); }
int main() {
    std::thread t1(use), t2(use), t3(use);
    t1.join(); t2.join(); t3.join();
}
```

[↩ back to table](#c20--the-big-expansion)

#### `atomic::wait/notify`

```cpp
#include <atomic>
#include <thread>
std::atomic<bool> ready{false};
void worker() { ready.wait(false); /* proceeds once true */ }
int main() {
    std::thread t(worker);
    ready.store(true);
    ready.notify_one();
    t.join();
}
```

[↩ back to table](#c20--the-big-expansion)

#### `atomic_flag::test()`

```cpp
#include <atomic>
std::atomic_flag flag{};
int main() {
    bool currently_set = flag.test(); // read without modifying (C++20)
}
```

[↩ back to table](#c20--the-big-expansion)

#### `std::atomic_ref`

```cpp
#include <atomic>
int plain_int = 0;
int main() {
    std::atomic_ref<int> ref(plain_int); // atomic ops on an ordinary int
    ref.fetch_add(1);
}
```

[↩ back to table](#c20--the-big-expansion)

#### `atomic<shared_ptr<T>>`

```cpp
#include <memory>
#include <atomic>
std::atomic<std::shared_ptr<int>> state = std::make_shared<int>(0);
int main() {
    state.store(std::make_shared<int>(42));
    std::shared_ptr<int> local = state.load();
}
```

[↩ back to table](#c20--the-big-expansion)

#### `std::osyncstream`

```cpp
#include <syncstream>
#include <iostream>
#include <thread>
void print(int id) {
    std::osyncstream(std::cout) << "thread " << id << " done\n"; // atomic block output
}
int main() {
    std::thread t1(print, 1), t2(print, 2);
    t1.join(); t2.join();
}
```

[↩ back to table](#c20--the-big-expansion)

#### `Coroutines`

```cpp
#include <coroutine>
// A full example needs a hand-written promise_type — see the C++20 guide.
// Minimal shape:
// ReturnType coro() { co_yield value; co_return; }
int main() {}
```

[↩ back to table](#c20--the-big-expansion)

---

## C++23 — Quiet Release

| Primitive | Header | Purpose |
| --- | --- | --- |
| [`std::generator<T>`](#stdgeneratort) | `<generator>` | Ready-made lazy generator |
| [`stdatomic.h`](#stdatomich) | `<stdatomic.h>` | C11-atomics-compatible aliases |
| [`std::move_only_function`](#stdmove_only_function) | `<functional>` | Move-only callable wrapper |

#### `std::generator<T>`

```cpp
#include <generator>
#include <iostream>
std::generator<int> count_to(int n) {
    for (int i = 1; i <= n; ++i) co_yield i;
}
int main() {
    for (int v : count_to(3)) std::cout << v << " "; // 1 2 3
}
```

[↩ back to table](#c23--quiet-release)

#### `stdatomic.h`

```cpp
#include <stdatomic.h>
atomic_int counter = 0; // C-style alias for std::atomic<int>
int main() {
    atomic_fetch_add(&counter, 1);
}
```

[↩ back to table](#c23--quiet-release)

#### `std::move_only_function`

```cpp
#include <functional>
#include <memory>
int main() {
    auto ptr = std::make_unique<int>(5);
    std::move_only_function<void()> f = [p = std::move(ptr)] { /* use *p */ };
    f();
}
```

[↩ back to table](#c23--quiet-release)

---

## C++26 — Structured Concurrency

| Primitive | Header | Purpose |
| --- | --- | --- |
| [`std::execution`](#stdexecution) | `<execution>` | Structured async model |
| [`then/when_all/sync_wait`](#thenwhen_allsync_wait) | `<execution>` | Compose async work |
| [`async_scope`](#async_scope) | `<execution>` | Scoped sender lifetime mgmt |
| [`std::hazard_pointer`](#stdhazard_pointer) | `<hazard_pointer>` | Per-pointer safe reclamation |
| [`RCU`](#rcu) | `<rcu>` | Read-cheap safe reclamation |

#### `std::execution`

```cpp
#include <execution>
#include <iostream>
int main() {
    using namespace std::execution;
    auto sch = get_system_scheduler(); // some scheduler, e.g. from a thread pool
    sender auto s = then(schedule(sch), [] { return 42; });
    auto [result] = this_thread::sync_wait(s).value();
    std::cout << result << "\n";
}
```

[↩ back to table](#c26--structured-concurrency)

#### `then/when_all/sync_wait`

```cpp
#include <execution>
int main() {
    using namespace std::execution;
    auto sch = get_system_scheduler();
    auto combined = when_all(
        then(schedule(sch), [] { return 1; }),
        then(schedule(sch), [] { return 2; })
    );
    auto [a, b] = this_thread::sync_wait(combined).value();
}
```

[↩ back to table](#c26--structured-concurrency)

#### `async_scope`

```cpp
#include <execution>
int main() {
    using namespace std::execution;
    async_scope scope;
    auto sch = get_system_scheduler();
    scope.spawn(schedule(sch) | then([]{ /* fire-and-forget work */ }));
} // scope destructor waits for spawned work to finish
```

[↩ back to table](#c26--structured-concurrency)

#### `std::hazard_pointer`

```cpp
#include <hazard_pointer>
#include <atomic>
struct Node : std::hazard_pointer_obj_base<Node> { int value; };
std::atomic<Node*> head;
int main() {
    std::hazard_pointer hp = std::make_hazard_pointer();
    Node* p = hp.protect(head); // safe to dereference p here
}
```

[↩ back to table](#c26--structured-concurrency)

#### `RCU`

```cpp
#include <rcu>
#include <atomic>
std::rcu_domain domain;
std::atomic<int*> config = new int(1);
int main() {
    int* old_cfg = config.exchange(new int(2));
    domain.rcu_retire(old_cfg); // freed once no reader is mid-critical-section
}
```

[↩ back to table](#c26--structured-concurrency)

---

**A note on coroutines and `std::execution`:** these two need more surrounding machinery than fits comfortably in a "minimal main" (a real promise type, or a real scheduler from a thread pool). The snippets above show the correct API shape with a comment marking what's assumed to exist; see the full per-edition guides for fully compilable versions of just those two.