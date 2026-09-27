# C++ Concurrency Primitives: Full Timeline (C++11 → C++26)

A single consolidated reference showing every concurrency-related element added in each standard, which header it lives in, what category it belongs to, and what it's for. Sorted by the edition that introduced it — nothing is repeated across editions even if it's still used later (e.g. `std::mutex` from C++11 is still used everywhere but only listed once, under C++11).

---

## C++11 — The Foundation

| Primitive | Header | Category | Purpose |
| --- | --- | --- | --- |
| `std::thread` | `<thread>` | Threading | Create and manage an OS thread |
| `std::this_thread::*` (`get_id`, `sleep_for`, `sleep_until`, `yield`) | `<thread>` | Threading | Operate on the calling thread |
| `thread::swap`, `hash<thread::id>` | `<thread>` | Threading | Minor thread utilities |
| `thread_local` | language keyword | Storage duration | Per-thread variable storage |
| `std::mutex` | `<mutex>` | Locking | Basic mutual exclusion |
| `std::recursive_mutex` | `<mutex>` | Locking | Mutex a thread can lock multiple times |
| `std::timed_mutex` / `recursive_timed_mutex` | `<mutex>` | Locking | Mutex with timed lock attempts |
| `.try_lock()` | `<mutex>` | Locking | Non-blocking lock attempt |
| `std::lock_guard` | `<mutex>` | RAII locking | Simple scope-based exclusive lock |
| `std::unique_lock` | `<mutex>` | RAII locking | Flexible exclusive lock (deferred/manual/timed) |
| `std::lock` | `<mutex>` | Locking | Deadlock-free multi-mutex locking |
| `std::call_once` / `std::once_flag` | `<mutex>` | Initialization | Thread-safe run-exactly-once |
| `std::condition_variable` | `<condition_variable>` | Signaling | Block until notified (works with `unique_lock<mutex>`) |
| `std::condition_variable_any` | `<condition_variable>` | Signaling | Same, but works with any lock type |
| `std::cv_status` | `<condition_variable>` | Signaling | Result enum of a timed condition-variable wait |
| `std::atomic<T>` | `<atomic>` | Lock-free data | Atomic read/write/RMW operations on a type |
| `std::atomic_flag` | `<atomic>` | Lock-free data | Guaranteed-lock-free boolean flag (e.g. spinlocks) |
| Memory orders (`relaxed`, `acquire`, `release`, `acq_rel`, `seq_cst`) | `<atomic>` | Memory model | Control instruction reordering guarantees |
| `std::promise` / `std::future` | `<future>` | Async result | One-shot value handoff between threads |
| `std::async` | `<future>` | Async execution | Run a task asynchronously, get a future |
| `std::packaged_task` | `<future>` | Async execution | Wrap a callable to produce a future |
| `std::shared_future` | `<future>` | Async result | Multi-consumer future |

---

## C++14 — Small Refinement

| Primitive | Header | Category | Purpose |
| --- | --- | --- | --- |
| `std::shared_timed_mutex` | `<shared_mutex>` | Locking | Reader/writer lock with timed-lock support |
| `std::shared_lock` | `<shared_mutex>` | RAII locking | Scope-based shared (read) ownership lock |
| `std::chrono` duration literals (`ms`, `s`, `us`, ...) | `<chrono>` | Convenience | Concise duration syntax for timed waits/locks |

---

## C++17 — Parallelism Enters the Standard

| Primitive | Header | Category | Purpose |
| --- | --- | --- | --- |
| `std::shared_mutex` | `<shared_mutex>` | Locking | Reader/writer lock, no timed-lock overhead |
| `std::scoped_lock` | `<mutex>` | RAII locking | Variadic, deadlock-free lock for 0+ mutexes |
| Execution policies (`seq`, `par`, `par_unseq`) | `<execution>` | Parallelism | Parallelize/vectorize standard algorithms |
| `atomic<T>::is_always_lock_free` | `<atomic>` | Lock-free data | Compile-time lock-free guarantee check |
| `hardware_destructive_interference_size` / `hardware_constructive_interference_size` | `<new>` | Performance | Cache-line sizing to avoid/encourage false sharing |

---

## C++20 — The Big Expansion

| Primitive | Header | Category | Purpose |
| --- | --- | --- | --- |
| `std::jthread` | `<thread>` | Threading | Self-joining thread with cooperative cancellation |
| `std::stop_token` / `stop_source` / `stop_callback` | `<stop_token>` | Cancellation | Cooperative cancellation mechanism |
| `std::latch` | `<latch>` | Signaling | Single-use countdown synchronization point |
| `std::barrier` | `<barrier>` | Signaling | Reusable, multi-phase synchronization point |
| `std::counting_semaphore` / `binary_semaphore` | `<semaphore>` | Signaling | Resource-count / handoff signaling primitive |
| `atomic<T>::wait/notify_one/notify_all` | `<atomic>` | Signaling | Block/wake on atomic value changes, no CV needed |
| `atomic_flag::test()` + wait/notify | `<atomic>` | Lock-free data | Non-destructive read + blocking on a flag |
| `std::atomic_ref` | `<atomic>` | Lock-free data | Atomic ops on an existing non-atomic object |
| `atomic<shared_ptr<T>>` / `atomic<weak_ptr<T>>` | `<memory>` | Lock-free data | Type-safe atomic smart-pointer operations |
| `std::osyncstream` | `<syncstream>` | Utility | Interleave-free concurrent stream output |
| Coroutines (`co_await` / `co_yield` / `co_return`) | `<coroutine>` | Async foundation | Language-level suspend/resume for async code |

---

## C++23 — Quiet Release

| Primitive | Header | Category | Purpose |
| --- | --- | --- | --- |
| `std::generator<T>` | `<generator>` | Async foundation | Ready-made coroutine-based lazy generator type |
| `<stdatomic.h>` (`atomic_int`, etc.) | `<stdatomic.h>` | Interop | C11-atomics-compatible aliases for `std::atomic<T>` |
| `std::move_only_function` | `<functional>` | Utility (adjacent) | Callable wrapper supporting move-only captures (task queues) |

---

## C++26 — Structured Concurrency

| Primitive | Header | Category | Purpose |
| --- | --- | --- | --- |
| `std::execution` (schedulers, senders, receivers) | `<execution>` | Async foundation | Structured, composable async/parallel execution model |
| `then`, `when_all`, `let_value`, `sync_wait`, etc. | `<execution>` | Async foundation | Core sender algorithms for composing async work |
| `async_scope` | `<execution>` | Structured concurrency | Lifetime-scoped management of groups of senders |
| System execution context / parallel scheduler | `<execution>` | Async foundation | Ready-to-use standard schedulers (general + CPU-parallel) |
| `std::hazard_pointer` | `<hazard_pointer>` | Lock-free reclamation | Per-pointer safe memory reclamation |
| RCU (`std::rcu_domain`, `rcu_retire`, ...) | `<rcu>` | Lock-free reclamation | Read-side-cheap safe memory reclamation |

*(C++26's final publication is expected later in 2026; compiler/library support, especially for `std::execution`, is still landing as of this writing.)*

---

## Cumulative View by Category

A different cut of the same data — everything available *as of* a given standard, grouped by what problem it solves:

| Category | C++11 | +C++14 | +C++17 | +C++20 | +C++23 | +C++26 |
| --- | --- | --- | --- | --- | --- | --- |
| **Thread management** | `thread`, `this_thread::*` | — | — | `jthread`, `stop_token` family | — | — |
| **Exclusive locking** | `mutex`, `recursive_mutex`, `timed_mutex`, `lock_guard`, `unique_lock`, `lock`, `call_once` | — | `scoped_lock` | — | — | — |
| **Shared (reader/writer) locking** | — | `shared_timed_mutex`, `shared_lock` | `shared_mutex` | — | — | — |
| **Condition/signaling** | `condition_variable`, `condition_variable_any` | — | — | `latch`, `barrier`, `counting_semaphore`, `binary_semaphore`, atomic `wait`/`notify` | — | — |
| **Atomics & memory model** | `atomic<T>`, `atomic_flag`, memory orders | — | `is_always_lock_free` | `atomic_ref`, `atomic<shared_ptr>`, `atomic_flag::test()` | `<stdatomic.h>` interop | — |
| **Async results / tasks** | `promise`, `future`, `async`, `packaged_task`, `shared_future` | — | — | coroutines | `generator` | `std::execution` (senders/receivers, `async_scope`) |
| **Parallel algorithms** | — | — | execution policies (`seq`/`par`/`par_unseq`) | — | — | parallel scheduler (via `std::execution`) |
| **Lock-free memory reclamation** | — | — | — | — | — | `hazard_pointer`, RCU |
| **Performance/interop helpers** | — | `chrono` literals | interference-size constants | `osyncstream` | `move_only_function` | — |

---

**Reading this table:** if you're targeting a specific `-std=c++NN` flag, everything in that edition's row *and every row above it* is available to you — the standard library only ever adds concurrency facilities, it doesn't remove them.