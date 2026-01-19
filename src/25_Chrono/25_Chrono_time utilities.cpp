#include <chrono>
#include <iostream>
#include <thread>
#include <iomanip>
#include <ctime>
#include <vector>
#include <algorithm>

// ============================================================================
// 1. BASIC DURATION OPERATIONS
// ============================================================================
void demonstrateDurations() {
    std::cout << "\n=== DURATION OPERATIONS ===\n";
    
    // Creating durations
    std::chrono::seconds sec(60);
    std::chrono::milliseconds ms(5000);
    std::chrono::minutes min(2);
    
    std::cout << "60 seconds = " << sec.count() << " sec\n";
    std::cout << "5000 milliseconds = " << ms.count() << " ms\n";
    std::cout << "2 minutes = " << min.count() << " min\n";
    
    // Duration arithmetic
    auto total = sec + ms;  // Automatically converts to common type
    std::cout << "60s + 5000ms = " << total.count() << " ms\n";
    
    // Duration comparison
    if (sec > ms) {
        std::cout << "60 seconds is greater than 5000 milliseconds\n";
    }
    
    // Converting between duration types
    auto sec_to_ms = std::chrono::duration_cast<std::chrono::milliseconds>(sec);
    std::cout << "60 seconds = " << sec_to_ms.count() << " milliseconds\n";
    
    // Converting to coarser units (requires duration_cast)
    auto ms_to_sec = std::chrono::duration_cast<std::chrono::seconds>(ms);
    std::cout << "5000 milliseconds = " << ms_to_sec.count() << " seconds\n";
    
    // Custom duration type
    using deciseconds = std::chrono::duration<long long, std::ratio<1, 10>>;
    deciseconds ds(25);  // 2.5 seconds
    auto ds_to_ms = std::chrono::duration_cast<std::chrono::milliseconds>(ds);
    std::cout << "25 deciseconds = " << ds_to_ms.count() << " milliseconds\n";
}

// ============================================================================
// 2. TIME POINT OPERATIONS
// ============================================================================
void demonstrateTimePoints() {
    std::cout << "\n=== TIME POINT OPERATIONS ===\n";
    
    // Get current time
    auto now = std::chrono::system_clock::now();
    
    // Time point arithmetic
    auto future = now + std::chrono::hours(2);
    auto past = now - std::chrono::minutes(30);
    
    // Calculate duration between time points
    auto diff = future - now;
    auto hours = std::chrono::duration_cast<std::chrono::hours>(diff);
    std::cout << "Difference between now and future: " << hours.count() << " hours\n";
    
    // Convert to time_t for display
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    std::cout << "Current time: " << std::ctime(&now_time_t);
    
    auto future_time_t = std::chrono::system_clock::to_time_t(future);
    std::cout << "Future time (+2h): " << std::ctime(&future_time_t);
}

// ============================================================================
// 3. PERFORMANCE MEASUREMENT
// ============================================================================
void expensiveOperation() {
    // Simulate some work
    std::vector<int> vec(1000000);
    std::generate(vec.begin(), vec.end(), []() { return rand() % 1000; });
    std::sort(vec.begin(), vec.end());
}

void measurePerformance() {
    std::cout << "\n=== PERFORMANCE MEASUREMENT ===\n";
    
    // Using high_resolution_clock for precise measurements
    auto start = std::chrono::high_resolution_clock::now();
    
    expensiveOperation();
    
    auto end = std::chrono::high_resolution_clock::now();
    
    // Calculate elapsed time in different units
    auto duration = end - start;
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(duration);
    auto us = std::chrono::duration_cast<std::chrono::microseconds>(duration);
    auto ns = std::chrono::duration_cast<std::chrono::nanoseconds>(duration);
    
    std::cout << "Operation took:\n";
    std::cout << "  " << ms.count() << " milliseconds\n";
    std::cout << "  " << us.count() << " microseconds\n";
    std::cout << "  " << ns.count() << " nanoseconds\n";
}

// ============================================================================
// 4. TIMER CLASS EXAMPLE
// ============================================================================
class Timer {
private:
    std::chrono::high_resolution_clock::time_point start_time;
    bool running;
    
public:
    Timer() : running(false) {}
    
    void start() {
        start_time = std::chrono::high_resolution_clock::now();
        running = true;
    }
    
    void stop() {
        running = false;
    }
    
    template<typename Duration = std::chrono::milliseconds>
    long long elapsed() const {
        auto end_time = std::chrono::high_resolution_clock::now();
        auto duration = end_time - start_time;
        return std::chrono::duration_cast<Duration>(duration).count();
    }
    
    void printElapsed(const std::string& label = "Elapsed time") const {
        std::cout << label << ": " << elapsed() << " ms\n";
    }
};

void demonstrateTimer() {
    std::cout << "\n=== TIMER CLASS USAGE ===\n";
    
    Timer timer;
    timer.start();
    
    // Simulate some work
    std::this_thread::sleep_for(std::chrono::milliseconds(250));
    
    timer.printElapsed("Operation 1");
    
    std::this_thread::sleep_for(std::chrono::milliseconds(150));
    
    timer.printElapsed("Total time");
    
    // Get elapsed time in different units
    std::cout << "Time in microseconds: " << timer.elapsed<std::chrono::microseconds>() << " μs\n";
    std::cout << "Time in seconds: " << timer.elapsed<std::chrono::seconds>() << " s\n";
}

// ============================================================================
// 5. TIMEOUT AND DEADLINE IMPLEMENTATION
// ============================================================================
bool tryOperation(int& attempt) {
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    attempt++;
    // Simulate success on 5th attempt
    return attempt >= 5;
}

void demonstrateTimeout() {
    std::cout << "\n=== TIMEOUT IMPLEMENTATION ===\n";
    
    auto timeout = std::chrono::seconds(2);
    auto deadline = std::chrono::steady_clock::now() + timeout;
    
    int attempt = 0;
    bool success = false;
    
    std::cout << "Attempting operation with 2-second timeout...\n";
    
    while (std::chrono::steady_clock::now() < deadline) {
        if (tryOperation(attempt)) {
            success = true;
            std::cout << "✓ Operation succeeded on attempt " << attempt << "\n";
            break;
        }
        std::cout << "  Attempt " << attempt << " failed, retrying...\n";
    }
    
    if (!success) {
        std::cout << "✗ Operation timed out after " << attempt << " attempts\n";
    }
}

// ============================================================================
// 6. STEADY CLOCK VS SYSTEM CLOCK
// ============================================================================
void demonstrateClockDifferences() {
    std::cout << "\n=== CLOCK DIFFERENCES ===\n";
    
    // steady_clock - monotonic, never adjusted
    auto steady_start = std::chrono::steady_clock::now();
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
    auto steady_end = std::chrono::steady_clock::now();
    
    auto steady_duration = std::chrono::duration_cast<std::chrono::milliseconds>(
        steady_end - steady_start
    );
    
    std::cout << "steady_clock measured: " << steady_duration.count() << " ms\n";
    std::cout << "  (monotonic, good for intervals)\n";
    
    // system_clock - wall clock time
    auto system_now = std::chrono::system_clock::now();
    auto system_time_t = std::chrono::system_clock::to_time_t(system_now);
    std::cout << "system_clock time: " << std::ctime(&system_time_t);
    std::cout << "  (wall clock, good for timestamps)\n";
    
    // Clock properties
    std::cout << "\nClock properties:\n";
    std::cout << "  steady_clock is steady: " 
              << std::chrono::steady_clock::is_steady << "\n";
    std::cout << "  system_clock is steady: " 
              << std::chrono::system_clock::is_steady << "\n";
}

// ============================================================================
// 7. SLEEP AND WAIT OPERATIONS
// ============================================================================
void demonstrateSleepOperations() {
    std::cout << "\n=== SLEEP OPERATIONS ===\n";
    
    std::cout << "Sleeping for 500ms...\n";
    auto start = std::chrono::high_resolution_clock::now();
    
    std::this_thread::sleep_for(std::chrono::milliseconds(500));
    
    auto end = std::chrono::high_resolution_clock::now();
    auto actual = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    std::cout << "Actually slept for: " << actual.count() << " ms\n";
    
    // Sleep until specific time point
    std::cout << "Sleeping until 300ms from now...\n";
    auto wake_time = std::chrono::steady_clock::now() + std::chrono::milliseconds(300);
    std::this_thread::sleep_until(wake_time);
    std::cout << "Woke up!\n";
}

// ============================================================================
// 8. RATE LIMITING / THROTTLING
// ============================================================================
class RateLimiter {
private:
    std::chrono::milliseconds min_interval;
    std::chrono::steady_clock::time_point last_call;
    
public:
    RateLimiter(std::chrono::milliseconds interval) 
        : min_interval(interval), last_call(std::chrono::steady_clock::now()) {}
    
    bool canProceed() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - last_call;
        
        if (elapsed >= min_interval) {
            last_call = now;
            return true;
        }
        return false;
    }
    
    void waitUntilReady() {
        auto now = std::chrono::steady_clock::now();
        auto elapsed = now - last_call;
        
        if (elapsed < min_interval) {
            auto wait_time = min_interval - elapsed;
            std::this_thread::sleep_for(wait_time);
            last_call = std::chrono::steady_clock::now();
        }
    }
};

void demonstrateRateLimiting() {
    std::cout << "\n=== RATE LIMITING ===\n";
    
    RateLimiter limiter(std::chrono::milliseconds(200));
    
    for (int i = 0; i < 5; ++i) {
        limiter.waitUntilReady();
        std::cout << "Action " << (i + 1) << " executed at "
                  << std::chrono::duration_cast<std::chrono::milliseconds>(
                         std::chrono::steady_clock::now().time_since_epoch()
                     ).count() % 100000 << " ms\n";
    }
}

// ============================================================================
// 9. DURATION LITERALS (C++14)
// ============================================================================
void demonstrateLiterals() {
    std::cout << "\n=== DURATION LITERALS ===\n";
    
    using namespace std::chrono_literals;
    
    auto one_hour = 1h;
    auto thirty_mins = 30min;
    auto five_secs = 5s;
    auto hundred_ms = 100ms;
    auto fifty_us = 50us;
    auto ten_ns = 10ns;
    
    std::cout << "1h = " << one_hour.count() << " hours\n";
    std::cout << "30min = " << thirty_mins.count() << " minutes\n";
    std::cout << "5s = " << five_secs.count() << " seconds\n";
    std::cout << "100ms = " << hundred_ms.count() << " milliseconds\n";
    
    // Arithmetic with literals
    auto total = 1h + 30min + 45s;
    auto in_seconds = std::chrono::duration_cast<std::chrono::seconds>(total);
    std::cout << "1h + 30min + 45s = " << in_seconds.count() << " seconds\n";
}

// ============================================================================
// 10. FORMATTING TIME OUTPUT
// ============================================================================
void demonstrateTimeFormatting() {
    std::cout << "\n=== TIME FORMATTING ===\n";
    
    auto now = std::chrono::system_clock::now();
    auto time_t_now = std::chrono::system_clock::to_time_t(now);
    
    // Different formatting options
    std::cout << "Default format: " << std::ctime(&time_t_now);
    
    std::tm* tm_now = std::localtime(&time_t_now);
    
    std::cout << "Custom format: ";
    std::cout << std::put_time(tm_now, "%Y-%m-%d %H:%M:%S") << "\n";
    
    std::cout << "ISO 8601 format: ";
    std::cout << std::put_time(tm_now, "%FT%T") << "\n";
    
    // Time since epoch
    auto since_epoch = now.time_since_epoch();
    auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(since_epoch);
    std::cout << "Milliseconds since epoch: " << millis.count() << "\n";
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================
int main() {
    std::cout << "C++ CHRONO LIBRARY - COMPREHENSIVE EXAMPLES\n";
    std::cout << "==========================================\n";
    
    demonstrateDurations();
    demonstrateTimePoints();
    measurePerformance();
    demonstrateTimer();
    demonstrateTimeout();
    demonstrateClockDifferences();
    demonstrateSleepOperations();
    demonstrateRateLimiting();
    demonstrateLiterals();
    demonstrateTimeFormatting();
    
    std::cout << "\n=== ALL DEMONSTRATIONS COMPLETE ===\n";
    
    return 0;
}