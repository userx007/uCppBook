/*
 * C++20 Ranges - Comprehensive Examples
 * Compile with: g++ -std=c++20 ranges_examples.cpp -o ranges_examples
 * or: clang++ -std=c++20 ranges_examples.cpp -o ranges_examples
 */

#include <ranges>
#include <vector>
#include <string>
#include <map>
#include <set>
#include <iostream>
#include <algorithm>
#include <numeric>
#include <cctype>

// Namespace alias for convenience
namespace views = std::ranges::views;
namespace ranges = std::ranges;

// ============================================================================
// SECTION 1: BASIC VIEWS AND ADAPTORS
// ============================================================================

void section1_basic_views() {
    std::cout << "\n=== SECTION 1: BASIC VIEWS ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    
    // 1.1 Filter View
    std::cout << "\n1.1 Filter (even numbers): ";
    auto evens = numbers | views::filter([](int x) { return x % 2 == 0; });
    for (int n : evens) std::cout << n << " ";
    
    // 1.2 Transform View
    std::cout << "\n1.2 Transform (square): ";
    auto squares = numbers | views::transform([](int x) { return x * x; });
    for (int n : squares) std::cout << n << " ";
    
    // 1.3 Take View
    std::cout << "\n1.3 Take (first 4): ";
    auto first_four = numbers | views::take(4);
    for (int n : first_four) std::cout << n << " ";
    
    // 1.4 Drop View
    std::cout << "\n1.4 Drop (skip first 7): ";
    auto skip_seven = numbers | views::drop(7);
    for (int n : skip_seven) std::cout << n << " ";
    
    // 1.5 Reverse View
    std::cout << "\n1.5 Reverse: ";
    auto reversed = numbers | views::reverse;
    for (int n : reversed) std::cout << n << " ";
    
    // 1.6 Take While
    std::cout << "\n1.6 Take While (< 6): ";
    auto take_while_less_6 = numbers | views::take_while([](int x) { return x < 6; });
    for (int n : take_while_less_6) std::cout << n << " ";
    
    // 1.7 Drop While
    std::cout << "\n1.7 Drop While (< 6): ";
    auto drop_while_less_6 = numbers | views::drop_while([](int x) { return x < 6; });
    for (int n : drop_while_less_6) std::cout << n << " ";
}

// ============================================================================
// SECTION 2: PIPELINE COMPOSITION
// ============================================================================

void section2_pipelines() {
    std::cout << "\n\n=== SECTION 2: PIPELINE COMPOSITION ===\n";
    
    std::vector<int> data = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15};
    
    // 2.1 Multiple operations chained
    std::cout << "\n2.1 Complex pipeline (filter->transform->take): ";
    auto result = data
        | views::filter([](int x) { return x % 2 == 0; })     // Keep evens
        | views::transform([](int x) { return x * x; })        // Square them
        | views::take(3);                                      // Take first 3
    
    for (int n : result) std::cout << n << " ";
    
    // 2.2 Deep pipeline
    std::cout << "\n2.2 Deep pipeline: ";
    auto deep_result = data
        | views::filter([](int x) { return x > 5; })
        | views::reverse
        | views::transform([](int x) { return x + 10; })
        | views::take(4)
        | views::drop(1);
    
    for (int n : deep_result) std::cout << n << " ";
}

// ============================================================================
// SECTION 3: GENERATING VIEWS
// ============================================================================

void section3_generating_views() {
    std::cout << "\n\n=== SECTION 3: GENERATING VIEWS ===\n";
    
    // 3.1 Iota - Infinite sequence
    std::cout << "\n3.1 Iota (1 to 10): ";
    auto seq = views::iota(1, 11);  // [1, 11) - half-open interval
    for (int n : seq) std::cout << n << " ";
    
    // 3.2 Iota - Infinite with take
    std::cout << "\n3.2 Infinite iota with take: ";
    auto infinite = views::iota(1) | views::take(5);
    for (int n : infinite) std::cout << n << " ";
    
    // 3.3 Single element
    std::cout << "\n3.3 Single element: ";
    auto single = views::single(42);
    for (int n : single) std::cout << n << " ";
    
    // 3.4 Empty range
    std::cout << "\n3.4 Empty range: ";
    auto empty = views::empty<int>;
    for (int n : empty) std::cout << n << " ";
    std::cout << "(nothing printed - empty range)";
    
    // 3.5 Repeat (C++23 feature, may not be available)
    // auto repeated = views::repeat(7) | views::take(5);
}

// ============================================================================
// SECTION 4: WORKING WITH PAIRS AND MAPS
// ============================================================================

void section4_pairs_and_maps() {
    std::cout << "\n\n=== SECTION 4: PAIRS, MAPS, AND STRUCTURED BINDINGS ===\n";
    
    std::map<int, std::string> scores = {
        {95, "Alice"},
        {87, "Bob"},
        {92, "Charlie"},
        {88, "Diana"},
        {96, "Eve"}
    };
    
    // 4.1 Keys view
    std::cout << "\n4.1 Keys only: ";
    auto keys = scores | views::keys;
    for (const auto& key : keys) std::cout << key << " ";
    
    // 4.2 Values view
    std::cout << "\n4.2 Values only: ";
    auto values = scores | views::values;
    for (const auto& value : values) std::cout << value << " ";
    
    // 4.3 Elements view (access tuple elements by index)
    std::cout << "\n4.3 First elements (keys): ";
    auto first_elements = scores | views::elements<0>;
    for (const auto& elem : first_elements) std::cout << elem << " ";
    
    // 4.4 Filter on keys
    std::cout << "\n4.4 High scores (>= 90): ";
    auto high_scores = scores
        | views::filter([](const auto& pair) { return pair.first >= 90; })
        | views::values;
    for (const auto& name : high_scores) std::cout << name << " ";
}

// ============================================================================
// SECTION 5: STRING OPERATIONS
// ============================================================================

void section5_string_operations() {
    std::cout << "\n\n=== SECTION 5: STRING OPERATIONS ===\n";
    
    // 5.1 Split view
    std::cout << "\n5.1 Split string: ";
    std::string text = "Hello,World,C++,Ranges";
    auto words = text | views::split(',');
    for (const auto& word : words) {
        for (char c : word) std::cout << c;
        std::cout << " | ";
    }
    
    // 5.2 Character filtering
    std::cout << "\n5.2 Filter vowels: ";
    std::string sentence = "The quick brown fox";
    auto vowels = sentence | views::filter([](char c) {
        c = std::tolower(c);
        return c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u';
    });
    for (char c : vowels) std::cout << c;
    
    // 5.3 Transform to uppercase
    std::cout << "\n5.3 To uppercase: ";
    auto upper = sentence | views::transform([](char c) {
        return static_cast<char>(std::toupper(c));
    });
    for (char c : upper) std::cout << c;
    
    // 5.4 Join view (flatten)
    std::cout << "\n5.4 Join nested ranges: ";
    std::vector<std::string> words_vec = {"C++", "20", "is", "cool"};
    auto joined = words_vec | views::join;
    for (char c : joined) std::cout << c;
}

// ============================================================================
// SECTION 6: ADVANCED VIEWS
// ============================================================================

void section6_advanced_views() {
    std::cout << "\n\n=== SECTION 6: ADVANCED VIEWS ===\n";
    
    std::vector<int> numbers = {1, 2, 3, 4, 5};
    
    // 6.1 Zip view (C++23) - simulate with transform
    std::cout << "\n6.1 Cartesian product (nested loops): ";
    auto cartesian = views::iota(1, 4)
        | views::transform([](int x) {
            return views::iota(1, 4)
                | views::transform([x](int y) {
                    return std::make_pair(x, y);
                });
        })
        | views::join;
    
    for (const auto& [x, y] : cartesian) {
        std::cout << "(" << x << "," << y << ") ";
    }
    
    // 6.2 Common view (ensures iterator/sentinel are same type)
    std::cout << "\n6.2 Common view: ";
    auto common = numbers | views::take(3) | views::common;
    std::vector<int> vec(common.begin(), common.end());
    for (int n : vec) std::cout << n << " ";
    
    // 6.3 All view (identity)
    std::cout << "\n6.3 All view (identity): ";
    auto all = numbers | views::all;
    for (int n : all) std::cout << n << " ";
}

// ============================================================================
// SECTION 7: RANGE ALGORITHMS
// ============================================================================

void section7_range_algorithms() {
    std::cout << "\n\n=== SECTION 7: RANGE ALGORITHMS ===\n";
    
    std::vector<int> data = {5, 2, 8, 1, 9, 3, 7, 4, 6};
    
    // 7.1 Sort
    std::cout << "\n7.1 Sorted: ";
    std::vector<int> sorted_data = data;
    ranges::sort(sorted_data);
    for (int n : sorted_data) std::cout << n << " ";
    
    // 7.2 Find
    std::cout << "\n7.2 Find value 7: ";
    auto it = ranges::find(data, 7);
    if (it != data.end()) std::cout << "Found at position " << (it - data.begin());
    
    // 7.3 Count
    std::cout << "\n7.3 Count values > 5: ";
    auto count = ranges::count_if(data, [](int x) { return x > 5; });
    std::cout << count;
    
    // 7.4 Accumulate
    std::cout << "\n7.4 Sum of elements: ";
    auto sum = std::accumulate(data.begin(), data.end(), 0);
    std::cout << sum;
    
    // 7.5 Min/Max
    std::cout << "\n7.5 Min element: " << *ranges::min_element(data);
    std::cout << ", Max element: " << *ranges::max_element(data);
    
    // 7.6 Any/All/None
    std::cout << "\n7.6 Any > 8? " << ranges::any_of(data, [](int x) { return x > 8; });
    std::cout << ", All > 0? " << ranges::all_of(data, [](int x) { return x > 0; });
    std::cout << ", None negative? " << ranges::none_of(data, [](int x) { return x < 0; });
}

// ============================================================================
// SECTION 8: PRACTICAL REAL-WORLD EXAMPLES
// ============================================================================

void section8_practical_examples() {
    std::cout << "\n\n=== SECTION 8: PRACTICAL EXAMPLES ===\n";
    
    // 8.1 FizzBuzz
    std::cout << "\n8.1 FizzBuzz (1-20):\n";
    auto fizzbuzz = views::iota(1, 21)
        | views::transform([](int n) -> std::string {
            if (n % 15 == 0) return "FizzBuzz";
            if (n % 3 == 0) return "Fizz";
            if (n % 5 == 0) return "Buzz";
            return std::to_string(n);
        });
    
    for (const auto& val : fizzbuzz) std::cout << val << " ";
    
    // 8.2 Processing log entries
    std::cout << "\n\n8.2 Log processing:\n";
    std::vector<std::string> logs = {
        "ERROR: Database connection failed",
        "INFO: Server started",
        "ERROR: Null pointer exception",
        "WARNING: High memory usage",
        "INFO: Request processed",
        "ERROR: Timeout occurred"
    };
    
    std::cout << "Error logs only:\n";
    auto errors = logs
        | views::filter([](const std::string& log) {
            return log.find("ERROR") != std::string::npos;
        });
    
    for (const auto& log : errors) std::cout << "  " << log << "\n";
    
    // 8.3 Data analysis
    std::cout << "\n8.3 Sales data analysis:\n";
    std::vector<double> sales = {1250.50, 890.25, 2150.00, 675.80, 3200.00, 
                                  1800.50, 950.00, 4100.00, 1500.25, 2800.00};
    
    auto high_sales = sales
        | views::filter([](double x) { return x >= 2000.0; })
        | views::transform([](double x) { return x * 1.1; });  // 10% bonus
    
    std::cout << "High-value sales with bonus: ";
    for (double s : high_sales) std::cout << "$" << s << " ";
    
    // 8.4 Finding prime numbers
    std::cout << "\n\n8.4 Prime numbers (2-50):\n";
    auto is_prime = [](int n) {
        if (n < 2) return false;
        for (int i = 2; i * i <= n; ++i) {
            if (n % i == 0) return false;
        }
        return true;
    };
    
    auto primes = views::iota(2, 51) | views::filter(is_prime);
    for (int p : primes) std::cout << p << " ";
    
    // 8.5 Flatten nested structure
    std::cout << "\n\n8.5 Flatten nested vectors:\n";
    std::vector<std::vector<int>> nested = {{1, 2, 3}, {4, 5}, {6, 7, 8, 9}};
    auto flattened = nested | views::join;
    for (int n : flattened) std::cout << n << " ";
}

// ============================================================================
// SECTION 9: PERFORMANCE AND LAZY EVALUATION
// ============================================================================

void section9_lazy_evaluation() {
    std::cout << "\n\n=== SECTION 9: LAZY EVALUATION DEMONSTRATION ===\n";
    
    std::cout << "\n9.1 Expensive operation (only computed for needed elements):\n";
    
    int computation_count = 0;
    auto expensive_transform = [&computation_count](int x) {
        computation_count++;
        std::cout << "  Computing for: " << x << "\n";
        return x * x;
    };
    
    auto lazy_view = views::iota(1, 1000000)  // Million elements!
        | views::transform(expensive_transform)
        | views::take(3);  // But we only take 3
    
    std::cout << "Created view (no computation yet)\n";
    std::cout << "Now iterating:\n";
    
    for (int n : lazy_view) {
        std::cout << "  Result: " << n << "\n";
    }
    
    std::cout << "Total computations: " << computation_count << " (not 1 million!)\n";
}

// ============================================================================
// SECTION 10: COMMON PATTERNS AND IDIOMS
// ============================================================================

void section10_patterns() {
    std::cout << "\n\n=== SECTION 10: COMMON PATTERNS ===\n";
    
    // 10.1 Sliding window
    std::cout << "\n10.1 Sliding window (size 3):\n";
    std::vector<int> data = {1, 2, 3, 4, 5, 6};
    
    for (size_t i : views::iota(0u, data.size() - 2)) {
        auto window = data | views::drop(i) | views::take(3);
        std::cout << "  Window: ";
        for (int n : window) std::cout << n << " ";
        std::cout << "\n";
    }
    
    // 10.2 Pairwise differences
    std::cout << "\n10.2 Pairwise differences:\n";
    std::vector<int> sequence = {10, 15, 13, 20, 25};
    
    for (size_t i : views::iota(1u, sequence.size())) {
        std::cout << "  " << sequence[i] << " - " << sequence[i-1] 
                  << " = " << (sequence[i] - sequence[i-1]) << "\n";
    }
    
    // 10.3 Enumerate pattern (with index)
    std::cout << "\n10.3 Enumerate with index:\n";
    std::vector<std::string> names = {"Alice", "Bob", "Charlie"};
    
    auto indexed = views::iota(0)
        | views::transform([&names](size_t i) {
            return std::make_pair(i, names[i]);
        })
        | views::take(names.size());
    
    for (const auto& [idx, name] : indexed) {
        std::cout << "  [" << idx << "]: " << name << "\n";
    }
}

// ============================================================================
// SECTION 11: CONVERTING VIEWS TO CONTAINERS
// ============================================================================

void section11_conversion() {
    std::cout << "\n\n=== SECTION 11: CONVERTING VIEWS TO CONTAINERS ===\n";
    
    auto view = views::iota(1, 11)
        | views::filter([](int x) { return x % 2 == 0; })
        | views::transform([](int x) { return x * x; });
    
    // 11.1 Manual conversion to vector
    std::cout << "\n11.1 Convert to std::vector: ";
    std::vector<int> vec(view.begin(), view.end());
    for (int n : vec) std::cout << n << " ";
    
    // 11.2 Convert to set
    std::cout << "\n11.2 Convert to std::set: ";
    std::set<int> s(view.begin(), view.end());
    for (int n : s) std::cout << n << " ";
    
    // Note: C++23 introduces ranges::to<Container>() for direct conversion
    // auto vec2 = view | ranges::to<std::vector>();
}

// ============================================================================
// SECTION 12: PITFALLS AND BEST PRACTICES
// ============================================================================

void section12_pitfalls() {
    std::cout << "\n\n=== SECTION 12: PITFALLS AND BEST PRACTICES ===\n";
    
    // 12.1 Dangling references - DANGER!
    std::cout << "\n12.1 AVOIDING DANGLING REFERENCES:\n";
    std::cout << "  ❌ BAD: Don't return views of local variables\n";
    std::cout << "  ✅ GOOD: Ensure source outlives the view\n";
    
    // 12.2 Multiple iterations
    std::cout << "\n12.2 Multiple iterations of input ranges:\n";
    auto input_range = views::iota(1, 6) 
        | views::filter([](int x) { return x % 2 == 0; });
    
    std::cout << "  First iteration: ";
    for (int n : input_range) std::cout << n << " ";
    
    std::cout << "\n  Second iteration: ";
    for (int n : input_range) std::cout << n << " ";
    std::cout << " ✅ Works fine\n";
    
    // 12.3 View materialization
    std::cout << "\n12.3 When to materialize (convert to container):\n";
    std::cout << "  - Need random access\n";
    std::cout << "  - Multiple passes with modifications\n";
    std::cout << "  - Need to store results permanently\n";
}

// ============================================================================
// MAIN FUNCTION
// ============================================================================

int main() {
    std::cout << "╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║   C++20 RANGES - COMPREHENSIVE EXAMPLES             ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    
    section1_basic_views();
    section2_pipelines();
    section3_generating_views();
    section4_pairs_and_maps();
    section5_string_operations();
    section6_advanced_views();
    section7_range_algorithms();
    section8_practical_examples();
    section9_lazy_evaluation();
    section10_patterns();
    section11_conversion();
    section12_pitfalls();
    
    std::cout << "\n\n╔══════════════════════════════════════════════════════╗\n";
    std::cout << "║   END OF EXAMPLES                                    ║\n";
    std::cout << "╚══════════════════════════════════════════════════════╝\n";
    
    return 0;
}