#include "../task.hpp"

#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <queue>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <vector>

namespace
{
auto solve(const task::UsageMap &usage)
{
    std::unordered_map<task::ID, std::unordered_set<task::ID>> holders;
    for(const auto &[id, bricks] : usage)
    {
        for(const auto &brick : bricks)
        {
            holders[brick].insert(id);
        }
    }
    const auto lookup = [&](const auto &id) {
        auto currentHolders = holders;
        std::queue<task::ID> failing;
        failing.push(id);
        task::uval cnt = 0;
        while(!failing.empty())
        {
            auto cur = failing.front();
            failing.pop();
            if(const auto useIter = usage.find(cur); useIter != end(usage))
            {
                for(const auto &nxt : useIter->second)
                {
                    auto &curHolder = currentHolders[nxt];
                    if(curHolder.erase(cur))
                    {
                        if(curHolder.empty())
                        {
                            cnt += 1;
                            failing.push(nxt);
                        }
                    }
                }
            }
        }
        return cnt;
    };
    task::uval cnt = 0;
    for(const auto &[brick, dst] : usage)
    {
        cnt += lookup(brick);
    }
    return cnt;
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
    const auto cnt = solve(usage);
    std::cout << cnt << '\n';
    return 0;
}
