#include <iostream>
#include <vector>
#include <list>
#include <forward_list>
#include <iterator>
#include <concepts>
#include <type_traits>
#include <sstream>

template<typename Iterator>
void print_iterator_category() {
    if constexpr (std::contiguous_iterator<Iterator>) {
        std::cout << "Contiguous Iterator\n";
    } else if constexpr (std::random_access_iterator<Iterator>) {
        std::cout << "Random Access Iterator\n";
    } else if constexpr (std::bidirectional_iterator<Iterator>) {
        std::cout << "Bidirectional Iterator\n";
    } else if constexpr (std::forward_iterator<Iterator>) {
        std::cout << "Forward Iterator\n";
    } else if constexpr (std::input_iterator<Iterator>) {
        std::cout << "Input Iterator\n";
    } else if constexpr (std::output_iterator<Iterator, int>) {
        // Use a dummy type (int) to detect output iterators like ostream_iterator
        std::cout << "Output Iterator\n";
    } else {
        std::cout << "Unknown Iterator Type\n";
    }
}

int main() {
    std::vector<int> vec;
    std::list<int> lst;
    std::forward_list<int> fl;
    std::istringstream iss("1 2 3");
    std::istream_iterator<int> iit(iss);
    std::ostream_iterator<int> oit(std::cout, " ");

    std::cout << "vector: ";
    print_iterator_category<decltype(vec.begin())>();

    std::cout << "list: ";
    print_iterator_category<decltype(lst.begin())>();

    std::cout << "forward_list: ";
    print_iterator_category<decltype(fl.begin())>();

    std::cout << "istream_iterator: ";
    print_iterator_category<decltype(iit)>();

    std::cout << "ostream_iterator: ";
    print_iterator_category<decltype(oit)>();
}
