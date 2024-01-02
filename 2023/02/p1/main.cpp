#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <tuple>

#include "../task.hpp"

namespace
{
const task::CubeCounts CUBE_LIMITS{
    {"red", 12},
    {"green", 13},
    {"blue", 14},
};

bool checkGame(const task::Game &game)
{
    return std::ranges::all_of(game, [](const auto &counts) {
        return std::ranges::all_of(counts, [](const auto &cubeCount) {
            if(const auto iter = CUBE_LIMITS.find(std::get<0>(cubeCount));
               iter != end(CUBE_LIMITS))
            {
                return std::get<1>(cubeCount) <= iter->second;
            }
            return false;
        });
    });
}
}

int main()
{
    task::ulval result = 0;
    while(std::cin)
    {
        std::string str;
        std::getline(std::cin, str);
        if(str.empty())
        {
            continue;
        }
        const auto maybeGame = task::parseGame(str);
        if(!maybeGame)
        {
            throw std::invalid_argument(
                std::format("invalid input: {}", maybeGame.error()));
        }
        if(checkGame(std::get<1>(*maybeGame)))
        {
            result += std::get<0>(*maybeGame);
        }
    }
    std::cout << result << '\n';
    return 0;
}
