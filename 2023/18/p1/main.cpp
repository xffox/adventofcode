#include <format>
#include <iostream>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeDigPlan = task::input::parseDigPlan<>(std::cin);
    if(!maybeDigPlan)
    {
        throw std::runtime_error(
            std::format("input parsing failed: {}", maybeDigPlan.error()));
    }
    std::cout << task::calculateArea(*maybeDigPlan) << '\n';
    return 0;
}
