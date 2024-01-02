#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <stdexcept>
#include <string>
#include <tuple>
#include <unordered_map>

#include "../task.hpp"

namespace
{
auto minCubes(const task::Game &game)
{
    task::CubeCounts cubeCounts;
    for(const auto &round : game)
    {
        for(const auto &[color, count] : round)
        {
            auto &cur = cubeCounts[color];
            cur = std::max(cur, count);
        }
    }
    return cubeCounts;
}

auto cubesPower(const task::CubeCounts &cubeCounts)
{
    return std::ranges::fold_left(
        cubeCounts |
            std::views::transform([](const auto &cnt) { return cnt.second; }),
        static_cast<task::ulval>(1), std::multiplies<>{});
}
}

int main()
{
    task::ulval result = 0;
    for(const auto &line :
        std::ranges::subrange(std::istreambuf_iterator<char>{std::cin},
                              std::istreambuf_iterator<char>{}) |
            std::views::lazy_split('\n') |
            std::views::transform([](auto lineView) {
                std::string line;
                std::ranges::copy(lineView, std::back_inserter(line));
                return line;
            }))
    {
        if(line.empty())
        {
            continue;
        }
        const auto maybeGame = task::parseGame(line);
        if(!maybeGame)
        {
            throw std::invalid_argument(
                std::format("invalid input: {}", maybeGame.error()));
        }
        result += cubesPower(minCubes(std::get<1>(*maybeGame)));
    }
    std::cout << result << '\n';
    return 0;
}
