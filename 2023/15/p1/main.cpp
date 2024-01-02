#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>

#include "../task.hpp"

int main()
{
    const auto sum =
        std::ranges::fold_left(task::input::readInput(std::cin) |
                                   std::views::transform([](const auto &str) {
                                       return task::hash(str);
                                   }),
                               static_cast<task::uval>(0), std::plus<>{});
    std::cout << sum << '\n';
    return 0;
}
