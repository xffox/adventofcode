#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>

#include "../task.hpp"

int main()
{
    std::cout << std::ranges::fold_left(
                     std::ranges::istream_view<std::string>(std::cin) |
                         std::views::transform(task::extractCalibrationValue),
                     0LL, std::plus<>{})
              << '\n';
    return 0;
}
