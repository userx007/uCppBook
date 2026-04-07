# `std::unique_lock<T>` in C++

`std::unique_lock<T>` is a flexible, general-purpose mutex ownership wrapper defined in `<mutex>`. It offers significantly more control than `std::lock_guard` — supporting deferred locking, timed locking, manual lock/unlock, and transfer of ownership.

---

## Core Concept

```cpp
template <class Mutex>
class unique_lock;
```

It owns a mutex **exclusively** (like `lock_guard`), but unlike `lock_guard`, the lock state is **not tied to object lifetime** — you can lock and unlock it freely during its lifetime.

---

## Construction Policies

The second constructor argument controls the initial lock state:

| Tag | Behavior |
|---|---|
| *(none)* | Locks immediately on construction |
| `std::defer_lock` | Does **not** lock; you lock manually later |
| `std::try_to_lock` | Attempts a non-blocking `try_lock()` |
| `std::adopt_lock` | Assumes the mutex is **already locked** by this thread |

---

## Key Member Functions

| Method | Description |
|---|---|
| `.lock()` | Blocks until the mutex is acquired |
| `.unlock()` | Releases the mutex |
| `.try_lock()` | Non-blocking attempt; returns `bool` |
| `.try_lock_for(duration)` | Timed attempt (requires `TimedMutex`) |
| `.try_lock_until(time_point)` | Deadline-based attempt |
| `.owns_lock()` | Returns `true` if currently holding the lock |
| `.release()` | Detaches from the mutex **without unlocking** |
| `.mutex()` | Returns a pointer to the managed mutex |

---

## Example 1 — Basic Usage (vs `lock_guard`)

```cpp
#include <mutex>
#include <iostream>

std::mutex mtx;
int shared_counter = 0;

void increment() {
    std::unique_lock<std::mutex> lock(mtx);  // locks immediately
    ++shared_counter;
    // unlocks automatically when 'lock' goes out of scope
}
```

So far this is identical to `lock_guard`. The power comes next.

---

## Example 2 — Deferred Locking & Manual Control

You can defer locking to lock later, or unlock mid-scope to allow other threads in during a long operation.

```cpp
#include <mutex>
#include <vector>
#include <iostream>

std::mutex mtx;
std::vector<int> data;

void process(int value) {
    std::unique_lock<std::mutex> lock(mtx, std::defer_lock);

    // Do expensive preparation WITHOUT holding the lock
    int result = value * value;

    lock.lock();  // Acquire only when needed
    data.push_back(result);
    lock.unlock();  // Release early — no need to hold during cleanup

    // More work here without holding the lock
    std::cout << "Processed: " << result << "\n";
}   // lock is already unlocked; destructor is a no-op
```

---

## Example 3 — Condition Variables (the primary use case)

`std::condition_variable` **requires** `unique_lock` — `lock_guard` cannot be used here because `cv.wait()` must temporarily release the mutex.

```cpp
#include <mutex>               // std::mutex, std::unique_lock
#include <condition_variable>  // std::condition_variable
#include <thread>              // std::thread
#include <queue>               // std::queue
#include <iostream>

// ─── Shared State ─────────────────────────────────────────────────────────────
// All three of these are accessed by both threads, so every read/write
// must be done while holding 'mtx'.

std::mutex             mtx;        // Guards all access to job_queue and done
std::condition_variable cv;        // Signals the consumer when something changed
std::queue<int>        job_queue;  // The shared work queue
bool                   done = false; // Flag: producer has finished all work

// ─── Producer ─────────────────────────────────────────────────────────────────
void producer() {
    for (int i = 0; i < 5; ++i) {

        {   // Inner scope — lock lives only as long as the push needs it
            std::unique_lock<std::mutex> lock(mtx); // Acquire mutex before
                                                    // touching shared state

            job_queue.push(i);  // Safe: we hold the lock

            std::cout << "Produced: " << i << "\n";

        }   // lock goes out of scope → mutex released HERE, before notify

        // Notify AFTER releasing the lock (best practice):
        // If we notified while still holding the lock, the consumer would
        // wake up, immediately block trying to re-acquire the mutex,
        // then go back to sleep — a wasteful spurious wake/sleep cycle.
        cv.notify_one(); // Wake exactly one waiting thread (our consumer)
    }

    {
        std::unique_lock<std::mutex> lock(mtx); // Must hold lock to write 'done'
                                                // safely — consumer reads it
        done = true; // Signal that no more jobs will ever be pushed
    }

    // Wake ALL waiting threads so they can observe done == true and exit.
    // notify_one() would work here too since there's only one consumer,
    // but notify_all() is safer if you later add more consumers.
    cv.notify_all();
}

// ─── Consumer ─────────────────────────────────────────────────────────────────
void consumer() {
    while (true) {

        // Lock must be held BEFORE calling cv.wait() —
        // the condition variable needs to release it atomically while sleeping.
        std::unique_lock<std::mutex> lock(mtx);

        // cv.wait(lock, predicate) does the following in a loop:
        //
        //   1. Evaluate predicate — if TRUE, return immediately (lock still held)
        //   2. If FALSE:
        //        a. Atomically release 'lock' and suspend this thread
        //        b. Wait for a cv.notify_one() or cv.notify_all()
        //        c. Reacquire 'lock' and re-evaluate the predicate
        //        d. If still false, repeat from (a)  ← guards against spurious wakeups
        //
        // The predicate here: wake up only when there's actual work to do
        // OR when the producer has declared it is finished.
        cv.wait(lock, [] { return !job_queue.empty() || done; });

        // At this point:
        //   - 'lock' is held again
        //   - At least one of: queue is non-empty, or done == true

        // Drain the entire queue before looping back to wait.
        // Using a while (not if) handles the case where the producer
        // pushed multiple items between two notify calls.
        while (!job_queue.empty()) {
            std::cout << "Consumed: " << job_queue.front() << "\n";
            job_queue.pop(); // Remove the front element after reading it
        }

        // If the producer is done AND the queue is now empty, exit the loop.
        // Checking done only AFTER draining ensures we never miss a job
        // that was pushed just before done was set to true.
        if (done) break;

        // Otherwise, loop back: re-acquire lock and wait for more work.
        // 'lock' is destroyed at the top of the next iteration anyway,
        // but going out of scope here releases it cleanly before re-entering.
    }
    // 'lock' is destroyed here on break → mutex released automatically
}

// ─── Main ──────────────────────────────────────────────────────────────────────
int main() {
    std::thread t1(producer); // Start producer — begins pushing jobs
    std::thread t2(consumer); // Start consumer — begins waiting for jobs

    // join() blocks main until each thread has finished executing.
    // Without join(), main could return and destroy shared state
    // (mtx, cv, job_queue, done) while threads are still using them → UB.
    t1.join();
    t2.join();
}
```

`cv.wait(lock, predicate)` is equivalent to:
```cpp
while (!predicate()) {
    cv.wait(lock);  // releases lock, suspends, reacquires on notify
}
```

---

## Example 4 — Timed Locking

Requires `std::timed_mutex` or `std::recursive_timed_mutex`.

```cpp
#include <mutex>
#include <chrono>
#include <iostream>

std::timed_mutex resource_mutex;

bool try_access_resource() {
    std::unique_lock<std::timed_mutex> lock(
        resource_mutex,
        std::chrono::milliseconds(100)  // try_lock_for implicitly
    );

    if (!lock.owns_lock()) {
        std::cout << "Could not acquire lock within 100ms\n";
        return false;
    }

    std::cout << "Resource acquired, doing work...\n";
    return true;
}
```

Or explicitly:
```cpp
std::unique_lock<std::timed_mutex> lock(resource_mutex, std::defer_lock);

if (lock.try_lock_for(std::chrono::milliseconds(200))) {
    // Got the lock
} else {
    // Timed out
}
```

---

## Example 5 — Ownership Transfer (`std::move`)

`unique_lock` is **movable but not copyable**, allowing lock ownership to be transferred between scopes or returned from functions.

```cpp
#include <mutex>

std::mutex mtx;

std::unique_lock<std::mutex> acquire_lock() {
    std::unique_lock<std::mutex> lock(mtx);
    return lock;  // ownership transferred via move
}

void do_work() {
    auto lock = acquire_lock();  // lock is now owned here
    // mutex remains locked throughout this scope
}
```

---

## Example 6 — `std::lock()` for Deadlock-Free Multi-Mutex Locking

When locking multiple mutexes simultaneously, use `std::lock()` with `std::adopt_lock` or `std::defer_lock` to avoid deadlocks.

```cpp
#include <mutex>

std::mutex mtx_a, mtx_b;

void transfer(int& from, int& to, int amount) {
    // Lock both atomically — std::lock avoids deadlock
    std::unique_lock<std::mutex> lock_a(mtx_a, std::defer_lock);
    std::unique_lock<std::mutex> lock_b(mtx_b, std::defer_lock);

    std::lock(lock_a, lock_b);  // acquires both, deadlock-safe

    from -= amount;
    to   += amount;
}  // both locks released here
```

> In C++17, prefer `std::scoped_lock<MutexA, MutexB>` for this specific pattern — it's simpler.

---

## `unique_lock` vs `lock_guard` — When to Use Which

| | `lock_guard` | `unique_lock` |
|---|---|---|
| Overhead | Minimal | Slightly higher (stores lock state) |
| Lock on construction | Always | Configurable |
| Manual unlock/relock | ✗ | ✓ |
| Movable | ✗ | ✓ |
| Works with `condition_variable` | ✗ | ✓ |
| Timed locking | ✗ | ✓ |
| **Use when** | Simple RAII guard | Conditions, deferred/timed locking, ownership transfer |

---

## Important Caveats

- Calling `.lock()` on an already-locked `unique_lock` is **undefined behavior**.
- Calling `.unlock()` when not owning the lock **throws** `std::system_error`.
- `.release()` detaches without unlocking — you become responsible for calling `.unlock()` on the raw mutex, or you'll have a **mutex leak**.
- Always check `.owns_lock()` after `try_lock` variants before accessing shared state.

---

## Justification for `std::adopt_lock`

The classic and most justified scenario is **locking multiple mutexes atomically with `std::lock()`**, then handing them off to RAII guards for safe cleanup.

### The Problem — Deadlock with Naive Locking

```cpp
// Thread 1          Thread 2
mtx_a.lock();        mtx_b.lock();
mtx_b.lock(); ←—→   mtx_a.lock();  // DEADLOCK
```

Both threads hold one mutex and wait forever for the other.

---

### The Solution — `std::lock()` + `adopt_lock`

`std::lock()` acquires **multiple mutexes atomically** using a deadlock-avoidance algorithm. But it gives you raw locks — so you hand them to RAII guards using `adopt_lock` to say *"don't lock again, just manage what's already locked."*

```cpp
#include <mutex>
#include <iostream>

struct Account {
    int balance;
    std::mutex mtx;
    Account(int b) : balance(b) {}
};

void transfer(Account& from, Account& to, int amount) {
    // Locks BOTH atomically — no deadlock regardless of call order
    std::lock(from.mtx, to.mtx);

    // Mutexes are now locked — adopt_lock tells the guards:
    // "don't lock again, just take ownership for RAII cleanup"
    std::lock_guard<std::mutex> lock_from(from.mtx, std::adopt_lock);
    std::lock_guard<std::mutex> lock_to  (to.mtx,   std::adopt_lock);

    from.balance -= amount;
    to.balance   += amount;

    std::cout << "Transferred " << amount << "\n";
}   // both guards unlock here safely

int main() {
    Account alice(1000), bob(500);

    // These can run concurrently without deadlock:
    // transfer(alice, bob, 100)  — locks alice then bob
    // transfer(bob, alice, 50)   — might lock bob then alice
    // std::lock() handles the ordering either way
    transfer(alice, bob, 100);
    transfer(bob, alice, 50);

    std::cout << "Alice: " << alice.balance << "\n";  // 950
    std::cout << "Bob:   " << bob.balance   << "\n";  // 550
}
```

---

### Why `adopt_lock` and not just skip the guards?

Without the guards you'd need manual cleanup — and exceptions break that:

```cpp
std::lock(from.mtx, to.mtx);

from.balance -= amount;
to.balance += amount;   // what if this throws?

from.mtx.unlock();      // never reached → mutex leaked forever
to.mtx.unlock();
```

`adopt_lock` gives you the best of both worlds: `std::lock()` handles the deadlock-safe acquisition, and the RAII guards handle the safe release.

---

> **C++17 note:** `std::scoped_lock` can replace this entire pattern more cleanly, since it accepts multiple mutexes directly and handles everything internally:
> ```cpp
> std::scoped_lock locks(from.mtx, to.mtx);  // atomic + RAII in one line
> ```
> `adopt_lock` with `lock_guard` remains relevant in C++11/14 codebases, or when you need the guards to be named and visible separately.

---

## Why the Release + Suspend Must Be Atomic

If those two operations were **separate**, there would be a window for a critical race condition. Let's break it down.

---

### What Would Happen if They Were NOT Atomic

```
CONSUMER THREAD                     PRODUCER THREAD
───────────────────────────────     ───────────────────────────────
predicate is false (queue empty)
lock.unlock()                   →   [producer gets the mutex]
                                    job_queue.push(1)
                                    cv.notify_one()  ← LOST! nobody
                                                       is sleeping yet
thread.suspend() ← sleeps forever,
                   notification
                   already fired
```

This is called a **missed wakeup**. The consumer goes to sleep *after* the only notification was already sent. Since the producer has no reason to notify again, the consumer **sleeps forever** — a deadlock.

---

### What Atomically Guarantees

By making unlock + suspend a **single indivisible operation**, the OS/runtime ensures there is **no gap** between the two:

```
CONSUMER THREAD                     PRODUCER THREAD
───────────────────────────────     ───────────────────────────────
┌─ atomic begin ─────────────┐
│ release lock               │
│ suspend thread             │  ←  producer must wait for this to
└─ atomic end ───────────────┘     complete before grabbing the mutex
                                    job_queue.push(1)
                                    cv.notify_one()  ← consumer is
                                                       guaranteed asleep,
wakes up ◄──────────────────────────────────────────   wakeup received ✓
reacquires lock
```

The producer **cannot** sneak in between the unlock and the suspend. Either:
- It sends the notification **before** the consumer even starts waiting → predicate re-evaluates to true on the next check, wait returns immediately.
- It sends the notification **after** the consumer is fully asleep → wakeup is received correctly.

---

### The Underlying Mechanic

The atomicity is enforced at the OS level. Internally, `cv.wait()` does roughly:

```cpp
// Pseudocode of what the OS/runtime actually does
void condition_variable::wait(unique_lock& lock, Predicate pred) {
    while (!pred()) {
        // The OS registers "this thread is a waiter" and
        // releases the mutex in ONE syscall — no gap possible
        os_atomically_unlock_and_sleep(lock.mutex(), this->waiters_list);

        // When woken, reacquire before returning control here
        lock.lock();
    }
}
```

The key is that **registering as a waiter** and **releasing the mutex** happen inside the same syscall (e.g. `futex(FUTEX_WAIT)` on Linux), making it physically impossible for a notification to slip through undetected.


### One-Line Summary

> The consumer must become *"notify-able"* and *"lock-released"* at the **exact same instant** — any gap between the two creates a window where a notification fires into the void, leaving the consumer sleeping with no one to wake it.

---

## Why the Lock Must Be Released During Wait

Simple answer: **if the consumer keeps the lock while sleeping, the producer can never acquire it — instant deadlock.**

```
CONSUMER THREAD                     PRODUCER THREAD
───────────────────────────────     ───────────────────────────────
acquires lock ✓
predicate false → goes to sleep
(still holding lock)                tries to acquire lock...
                                    blocks forever ← can never push
                                                     can never notify
sleeps forever ← nobody ever
                 wakes it up
```

Both threads wait on each other. Neither can ever make progress.

---

### The Fundamental Conflict

The consumer needs the lock to **safely read shared state** (the predicate check), but it also needs to **sleep and give the lock up** so the producer can change that same shared state:

```cpp
// Consumer needs the lock to safely check this:
cv.wait(lock, [] { return !job_queue.empty() || done; });
//                         ↑                   ↑
//                  shared state — must be read under lock

// Producer needs the lock to safely write this:
{
    std::unique_lock<std::mutex> lock(mtx);
    job_queue.push(i);   // ← same shared state
    done = true;         // ← same shared state
}
cv.notify_one();
```

Both threads need the **same mutex** to safely access the **same data**. The consumer cannot monopolize it while sleeping — that would starve the very thread it's waiting on.

---

### What `cv.wait()` Actually Solves

It elegantly resolves this catch-22:

```
Hold lock → check predicate → predicate false → need to sleep
                                                     ↓
                                     but can't sleep holding the lock
                                                     ↓
                         release lock + sleep (atomically)
                                                     ↓
                         producer can now lock → push → notify
                                                     ↓
                         consumer wakes → reacquires lock → checks predicate again
```

The lock is released **only during the sleep** — the consumer still holds it during the predicate check and during the actual queue processing. It's the minimal window necessary.

### One-Line Summary

> The consumer holds the lock to *check* shared state, but must *release* it while sleeping so the producer can *change* that shared state — otherwise the producer is permanently locked out and can never create the condition the consumer is waiting for.

---

## Why Sleep Instead of Just Looping?

The naive alternative to sleeping is **busy-waiting** — just spinning in a loop checking the predicate:

```cpp
// Why not just do this?
while (job_queue.empty() && !done) {
    // keep checking... forever
}
```

This "works" in terms of correctness, but it's deeply problematic.

---

### Problem 1 — CPU Waste

A spinning thread consumes **100% of a CPU core** doing nothing useful:

```
CONSUMER THREAD                     PRODUCER THREAD
───────────────────────────────     ───────────────────────────────
while (queue.empty()) {}  ← burning CPU cycles
while (queue.empty()) {}
while (queue.empty()) {}
while (queue.empty()) {}  ...       trying to get scheduled by the OS,
                                    but consumer is hogging the core
```

On a single-core machine, the producer might **never get CPU time** to push anything — the consumer starves it by monopolizing the core.

---

### Problem 2 — Power & Heat

A busy-waiting thread prevents the CPU from entering low-power states. On battery-powered devices this drains the battery. On any system it generates unnecessary heat — for literally zero productive work.

---

### Problem 3 — Lock Contention

If the spin loop holds the mutex while checking (which it must, to read shared state safely), you're back to the deadlock from before. If it releases and reacquires rapidly, you get **lock thrashing**:

```cpp
while (true) {
    lock.lock();
    if (!job_queue.empty()) break;  // predicate check
    lock.unlock();                  // release
    // immediately try to grab it again ↑
    // producer and consumer fight over the mutex constantly
}
```

The producer and consumer hammer the mutex back and forth, adding synchronization overhead with every iteration.

---

### What Sleep Actually Buys You

When the consumer sleeps, the OS **deschedules** it entirely — removes it from the run queue:

```
CONSUMER THREAD                     PRODUCER THREAD         OS SCHEDULER
───────────────────────────────     ───────────────         ────────────
sleeps → removed from run queue                             consumer is off
                                                            the run queue,
                                    gets CPU time ✓         producer runs ✓
                                    pushes job
                                    cv.notify_one()  →      puts consumer
                                                            back on run queue
wakes up, reacquires lock ✓
```

The CPU core is now **free** for the producer (or any other thread) to use. The consumer consumes **zero CPU** while waiting. The OS wakes it precisely when there is something to do.


### One-Line Summary

> Sleeping hands the CPU back to the OS so other threads — including the producer the consumer is waiting on — can actually run. Without sleeping, the consumer wastes CPU, starves the producer, and may prevent the very condition it is waiting for from ever occurring.