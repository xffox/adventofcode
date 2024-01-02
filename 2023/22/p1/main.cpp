#include "../task.hpp"

#include <algorithm>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
task::uval countUsed(const task::UsageMap &usage)
{
    std::unordered_map<task::ID, std::unordered_set<task::ID>> rev;
    for(const auto &[belowID, aboves] : usage)
    {
        for(const auto &aboveID : aboves)
        {
            rev[aboveID].insert(belowID);
        }
    }
    std::unordered_set<task::ID> used;
    std::ranges::copy(rev | std::views::filter([](const auto &p) {
                          return p.second.size() == 1;
                      }) | std::views::transform([](const auto &p) {
                          return *std::ranges::begin(p.second);
                      }),
                      std::inserter(used, end(used)));
    return used.size();
}
}

int main()
{
    std::vector<task::Range> bricks;
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
        const auto maybeBrick = task::input::parseBrick(line);
        if(!maybeBrick)
        {
            throw std::runtime_error(
                std::format("brick parsing failed: {}", maybeBrick.error()));
        }
        bricks.push_back(*maybeBrick);
    }
    const auto usage = task::run(bricks);
    std::cout << bricks.size() - countUsed(usage) << '\n';
    return 0;
}
