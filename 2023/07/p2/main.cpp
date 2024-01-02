#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "../task.hpp"

int main()
{
    std::vector<task::input::Play> plays;
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
        const auto maybePlay = task::parsePlay(line);
        if(!maybePlay)
        {
            throw std::runtime_error(
                std::format("input parse error: {}", maybePlay.error()));
        }
        plays.push_back(*maybePlay);
    }
    std::cout << task::totalWinnings<task::HandOrder<true>>(plays) << '\n';
    return 0;
}
