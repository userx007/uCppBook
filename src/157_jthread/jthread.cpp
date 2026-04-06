/*
g++ -pthread --std=c++20 jthread.cpp -o app
*/


#include <thread>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <iostream>

// Protects all accesses to 'items'. Any thread that reads or writes
// the queue must hold this mutex to prevent data races.
std::mutex mtx;

// std::condition_variable_any is used instead of std::condition_variable
// because its wait() overloads accept a std::stop_token as a parameter.
// std::condition_variable only works with std::unique_lock<std::mutex>,
// while _any works with any BasicLockable type and adds stop_token support.
std::condition_variable_any cv;

// Shared queue between the producer (main) and the consumer thread.
// Items are pushed from main and popped inside the consumer.
std::queue<int> items;

// The consumer thread function.
// Declaring std::stop_token as the first parameter is the jthread protocol:
// std::jthread detects this signature at construction time and automatically
// passes its internal stop_token — no manual wiring needed.
void consumer(std::stop_token st)
{
    // Acquire the mutex for the lifetime of the loop.
    // std::unique_lock (vs std::lock_guard) is required here because
    // cv.wait() must temporarily release the lock while waiting, then
    // reacquire it before returning — lock_guard cannot do this.
    std::unique_lock lock(mtx);

    while (!st.stop_requested())
    {
        // Three-argument wait: atomically releases 'lock', suspends the thread,
        // and re-acquires 'lock' before returning. It wakes up when ANY of these
        // three conditions become true (whichever comes first):
        //   1. cv.notify_one() / notify_all() is called by another thread,
        //      AND the predicate [ !items.empty() ] evaluates to true.
        //   2. A stop is requested via st (t.request_stop() in main).
        //      Internally this works via a hidden std::stop_callback that calls
        //      cv.notify_all(), so no manual callback registration is needed.
        //   3. A spurious wakeup — the predicate re-check guards against this.
        //
        // After returning, the mutex is held again, so items can be safely read.
        cv.wait(lock, st, []{ return !items.empty(); });

        // cv.wait() may have returned because stop was requested rather than
        // because an item arrived. Check before touching the queue to avoid
        // popping from an empty container.
        if (st.stop_requested())
        {
            break;
        }

        // Safe to access items: mutex is held and we confirmed the queue is non-empty.
        std::cout << "consumed: " << items.front() << std::endl;
        items.pop();
    }

    // Reached when stop_requested() is true after a wait() return.
    // At this point 'lock' is still held; it is released when 'lock'
    // goes out of scope at the closing brace below.
    std::cout << "consumer: done" << std::endl;
}
// 'lock' destructor fires here — mutex released.

int main()
{
    using namespace std::chrono_literals;   // enables the 80ms / 50ms literals

    // Construct the jthread. Because consumer()'s first parameter is
    // std::stop_token, jthread injects its internal token automatically.
    // The thread starts executing consumer() immediately after this line.
    std::jthread t(consumer);

    // Simulate a producer that pushes three items at 80 ms intervals.
    for (int i : {10, 20, 30})
    {
        // Wait before each push so the consumer has time to process
        // the previous item, demonstrating interleaved producer/consumer timing.
        std::this_thread::sleep_for(80ms);

        {
            // Inner scope: lock_guard releases the mutex the moment the brace
            // closes, before notify_one() is called. This is intentional:
            // notifying while holding the lock can cause the consumer to wake
            // up, immediately block again waiting for the mutex we still hold,
            // and then go back to sleep — a needless context switch.
            std::lock_guard g(mtx);
            items.push(i);
        } // mutex released here

        // Wake the consumer. Because the predicate (!items.empty()) is now
        // true, cv.wait() will not go back to sleep and will proceed to pop.
        cv.notify_one();
    }

    // Give the consumer a moment to drain the last item before we stop it.
    // Without this pause, request_stop() could arrive while the queue still
    // has the final item, causing it to be silently discarded.
    std::this_thread::sleep_for(50ms);

    // Signal the consumer to exit its loop. This does two things atomically
    // inside the stop_token machinery:
    //   1. Sets the stop_requested() flag to true.
    //   2. Fires the hidden stop_callback registered by cv.wait(), which
    //      calls cv.notify_all() — waking the consumer even if the queue
    //      is empty and it would otherwise wait indefinitely.
    t.request_stop();

    // jthread destructor is called here. It calls join(), blocking main
    // until the consumer thread has returned from consumer(). This guarantees
    // no thread is still running when the program exits. With std::thread
    // this would require an explicit t.join(); forgetting it causes
    // std::terminate(). jthread makes it automatic (RAII).
}