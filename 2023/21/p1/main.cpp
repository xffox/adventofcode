#include <format>
#include <iostream>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeInput = task::input::parseInput(std::cin);
    if(!maybeInput)
    {
        throw std::runtime_error(
            std::format("input parsing failed: {}", maybeInput.error()));
    }
    const auto &[board, pos, steps] = *maybeInput;
    std::cout << task::reachable(board, pos, steps) << '\n';
    return 0;
}
