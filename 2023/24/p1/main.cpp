#include <algorithm>
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

#include "../task.hpp"

namespace
{
// z coordinates are ignored
constexpr bool withinArea(const task::Pos &pos, const task::Pos &areaBegin,
                          const task::Pos &areaEnd)
{
    return pos.x >= areaBegin.x && pos.y >= areaBegin.y && pos.x <= areaEnd.x &&
           pos.y <= areaEnd.y;
}

// z coordinates are ignored
constexpr std::optional<task::Pos> pathsIntersection(
    const task::Hailstone &left, const task::Hailstone &right)
{
    constexpr auto calcTime =
        [](const auto &left,
           const auto &right) -> std::optional<task::floatval> {
        const auto denom = left.velocity.x * right.velocity.y -
                           right.velocity.x * left.velocity.y;
        if(denom == 0)
        {
            return std::nullopt;
        }
        const auto time = (right.velocity.x * left.position.y -
                           right.velocity.x * right.position.y -
                           right.velocity.y * left.position.x +
                           right.velocity.y * right.position.x) /
                          denom;
        return time;
    };
    const auto maybeLeftTime = calcTime(left, right);
    if(!maybeLeftTime || *maybeLeftTime < 0)
    {
        return std::nullopt;
    }
    const auto maybeRightTime = calcTime(right, left);
    if(!maybeRightTime || *maybeRightTime < 0)
    {
        return std::nullopt;
    }
    return task::Pos{left.position.x + left.velocity.x * (*maybeLeftTime),
                     left.position.y + left.velocity.y * (*maybeLeftTime), 0};
}
}

int main()
{
    task::Pos areaBegin{};
    task::Pos areaEnd{};
    std::cin >> areaBegin.x >> areaBegin.y;
    std::cin >> areaEnd.x >> areaEnd.y;
    std::cin.ignore();
    std::vector<task::Hailstone> ps;
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
        const auto maybeHailstone = task::input::parseHailstone(line);
        if(!maybeHailstone)
        {
            throw std::runtime_error(std::format("input parsing failed: {}",
                                                 maybeHailstone.error()));
        }
        ps.push_back(*maybeHailstone);
    }
    task::uval count = 0;
    for(std::size_t i = 0; i < ps.size(); ++i)
    {
        for(std::size_t j = i + 1; j < ps.size(); ++j)
        {
            if(const auto maybePos = pathsIntersection(ps[i], ps[j]))
            {
                count += withinArea(*maybePos, areaBegin, areaEnd);
            }
        }
    }
    std::cout << count << '\n';
    return 0;
}
