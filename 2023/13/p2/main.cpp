#include <algorithm>
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <sstream>
#include <stdexcept>
#include <string>

#include "../task.hpp"

int main()
{
    std::string boardStr;
    task::uval result = 0;
    std::size_t idx = 0;
    for(const auto lineView :
        std::ranges::subrange(std::istreambuf_iterator<char>{std::cin},
                              std::istreambuf_iterator<char>{}) |
            std::views::lazy_split('\n'))
    {
        const auto oldSize = boardStr.size();
        std::ranges::copy(lineView, std::back_inserter(boardStr));
        if(boardStr.size() != oldSize)
        {
            boardStr.push_back('\n');
        }
        else
        {
            const auto maybeBoard = task::input::parseBoard(boardStr);
            if(!maybeBoard)
            {
                throw std::runtime_error(std::format(
                    "input board parsing failed: {}", maybeBoard.error()));
            }
            const auto maybeReflection =
                task::findSmudgedReflection(*maybeBoard);
            if(!maybeReflection)
            {
                throw std::runtime_error(
                    std::format("reflection not found: idx={}", idx));
            }
            result +=
                maybeReflection->pos *
                (maybeReflection->direction == task::Dir::HORIZONTAL ? 100 : 1);
            boardStr.clear();
            ++idx;
        }
    }
    std::cout << result << '\n';
    return 0;
}
