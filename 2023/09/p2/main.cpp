#include <algorithm>
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>

#include "../task.hpp"

int main()
{
    task::lval sum = 0;
    for(const auto lineView :
        std::ranges::subrange(std::istreambuf_iterator<char>{std::cin},
                              std::istreambuf_iterator<char>{}) |
            std::views::lazy_split('\n'))
    {
        std::string line;
        std::ranges::copy(lineView, std::back_inserter(line));
        if(line.empty())
        {
            continue;
        }
        const auto maybeValues = task::input::parseValues(line);
        if(!maybeValues)
        {
            throw std::runtime_error(
                std::format("input parse failed: {}", maybeValues.error()));
        }
        const auto res = task::predictPrevious(*maybeValues);
        sum += res;
    }
    std::cout << sum << '\n';
    return 0;
}
