#include <algorithm>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "../task.hpp"

namespace
{
task::uval lowestDestination(const task::input::Almanac &almanac)
{
    if(almanac.seeds.empty())
    {
        return 0;
    }
    const auto finalMap = std::ranges::fold_left(
        almanac.maps, task::Map{}, [](const auto &result, const auto &map) {
            return result.join(task::Map(map));
        });
    return std::ranges::min(
        almanac.seeds |
        std::views::transform([&](const auto seed) { return finalMap(seed); }));
}
}

int main()
{
    const auto maybeAlmanac = task::parseAlmanac(std::cin);
    if(!maybeAlmanac)
    {
        throw std::runtime_error(maybeAlmanac.error());
    }
    std::cout << lowestDestination(*maybeAlmanac) << '\n';
    return 0;
}
