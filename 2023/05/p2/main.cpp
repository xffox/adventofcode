#include <algorithm>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "../task.hpp"

namespace
{
task::uval lowestDestination(const task::input::Almanac &almanac)
{
    auto startsView = almanac.seeds | std::views::stride(2);
    auto lengthsView =
        almanac.seeds | std::views::drop(1) | std::views::stride(2);
    auto ranges = std::views::zip_transform(
        [](const auto source, const auto length) {
            return task::Range{source, length};
        },
        startsView, lengthsView);
    auto nonemptyRanges = ranges | std::views::filter([](const auto &rng) {
                              return rng.length > 0;
                          });
    if(std::ranges::empty(ranges))
    {
        return 0;
    }
    const auto finalMap = std::ranges::fold_left(
        almanac.maps, task::Map{}, [](const auto &result, const auto &map) {
            return result.join(task::Map(map));
        });
    return std::ranges::min(
        nonemptyRanges | std::views::transform([&](const auto &range) {
            return std::ranges::min(finalMap.lookup(range) |
                                    std::views::transform([](const auto &rng) {
                                        return rng.start;
                                    }));
        }));
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
