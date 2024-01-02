#include <algorithm>
#include <format>
#include <iostream>
#include <numeric>
#include <ranges>
#include <stdexcept>

#include "../task.hpp"

namespace
{
constexpr task::Size commonCycle(const task::Size left, const task::Size right)
{
    const auto g = std::gcd(left, right);
    return (left / g) * (right / g) * g;
}
}

int main()
{
    const auto maybeInput = task::input::parseInput(std::cin);
    if(!maybeInput)
    {
        throw std::runtime_error(
            std::format("input parsing error: {}", maybeInput.error()));
    }
    const auto cycle = std::ranges::fold_left(
        maybeInput->map | std::views::filter([](const auto &connection) {
            return connection.first.ends_with('A');
        }) | std::views::transform([&](const auto &connection) {
            const auto maybePathCharacteristic = task::pathCharacteristic(
                maybeInput->map, maybeInput->path, connection.first,
                [](const auto &node) { return node.ends_with('Z'); });
            if(maybePathCharacteristic)
            {
                return maybePathCharacteristic->cycle.length;
            }
            return static_cast<task::Size>(1);
        }),
        static_cast<task::Size>(1), commonCycle);
    std::cout << cycle << '\n';
    return 0;
}
