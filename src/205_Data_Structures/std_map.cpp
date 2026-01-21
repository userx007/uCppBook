/*
g++ -pthread --std=c++20 std_map.cpp -o app
*/

#include <map>
#include <string>
#include <iostream>

int main() {
    // Creation (automatically sorted by key using std::less<std::string> by default)
    // Alternative: std::unordered_map for O(1) average lookup without sorting
    std::map<std::string, int> scores;
    
    // Insertion methods
    scores["Alice"] = 95;      // operator[] - creates if not exists, overwrites if exists
    scores["Charlie"] = 88;
    scores["Bob"] = 92;
    
    // Alternative insertion methods:
    // scores.insert({"David", 90});           // insert() - doesn't overwrite
    // scores.insert_or_assign("Eve", 87);     // C++17 - overwrites if exists
    // scores.emplace("Frank", 85);            // constructs in-place
    // scores.try_emplace("Grace", 93);        // C++17 - only inserts if key doesn't exist
    
    // Iteration (sorted order by key - alphabetically for strings)
    // Time complexity: O(n), where n is the number of elements
    for (const auto& [name, score] : scores) {  // C++17 structured binding
        std::cout << name << ": " << score << std::endl;
    }
    // Output: Alice: 95, Bob: 92, Charlie: 88
    
    // Alternative iteration methods:
    // for (auto it = scores.begin(); it != scores.end(); ++it)
    //     std::cout << it->first << ": " << it->second << std::endl;
    
    // Range queries - one of std::map's key advantages over std::unordered_map
    auto it = scores.lower_bound("Bob");  // Returns iterator to first element >= "Bob"
    std::cout << "First >= Bob: " << it->first << std::endl;
    
    // Other useful range query methods:
    // auto it2 = scores.upper_bound("Bob");   // First element > "Bob"
    // auto [low, high] = scores.equal_range("Bob"); // Range of elements == "Bob"
    
    // Lookup methods:
    // if (scores.count("Alice")) { }          // Returns 0 or 1
    // if (scores.find("Alice") != scores.end()) { } // Returns iterator
    // if (scores.contains("Alice")) { }       // C++20 - most readable
    
    // Access methods:
    // int val = scores["Alice"];              // Creates entry if not exists (use with caution)
    // int val = scores.at("Alice");           // Throws std::out_of_range if not exists (safer)
    
    // Performance characteristics:
    // - Insertion: O(log n)
    // - Lookup: O(log n)
    // - Deletion: O(log n)
    // - Maintains sorted order
    // Compare with std::unordered_map: O(1) average for insert/lookup/delete, no ordering
    
    return 0;
}