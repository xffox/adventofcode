#include <format>
#include <iostream>
#include <optional>
#include <stdexcept>

#include "../task.hpp"

namespace
{
std::optional<task::Size> stepsToReach(const task::Map &map,
                                       const task::Path &path,
                                       const task::Node &source,
                                       const task::Node &target)
{
    std::optional<task::Size> steps;
    task::traverse(map, path, source, [&](const auto &node, const auto idx) {
        if(node == target)
        {
            steps = idx;
            return false;
        }
        return true;
    });
    return steps;
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
    const task::Node source("AAA");
    const task::Node target("ZZZ");
    const auto maybeSteps =
        stepsToReach(maybeInput->map, maybeInput->path, source, target);
    if(!maybeSteps)
    {
        throw std::runtime_error("target unreachable");
    }
    std::cout << *maybeSteps << '\n';
    return 0;
}
