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
        constexpr task::uval repeats = 5;
        const auto &[row, counts] = *maybeInputRow;
        task::Row actualRow;
        task::CountCol actualCounts;
        bool first = true;
        for(task::uval i = 0; i < repeats; ++i)
        {
            if(!first)
            {
                actualRow.push_back(task::Cell::UNKNOWN);
            }
            else
            {
                first = false;
            }
            actualRow.insert(end(actualRow), begin(row), end(row));
            actualCounts.insert(end(actualCounts), begin(counts), end(counts));
        }
        const auto res = task::solve(actualRow, actualCounts);
        sum += res;
    }
    std::cout << sum << '\n';
    return 0;
}
