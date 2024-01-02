#include <format>
#include <iostream>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeSteps = task::input::readSteps(std::cin);
    if(!maybeSteps)
    {
        throw std::runtime_error(
            std::format("input parsing failed: {}", maybeSteps.error()));
    }
    std::cout << task::executeSteps(*maybeSteps) << '\n';
    return 0;
}
