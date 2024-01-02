#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>

#include "../task.hpp"

int main()
{
    task::ulval sum = 0;
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
        const auto maybeInputRow = task::input::parseInputRow(line);
        if(!maybeInputRow)
        {
            throw std::runtime_error(std::format("input row parsing failed: {}",
                                                 maybeInputRow.error()));
        }
        const auto &[row, counts] = *maybeInputRow;
        const auto res = task::solve(row, counts);
        sum += res;
    }
    std::cout << sum << '\n';
    return 0;
}
