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
    const auto res = task::reachable(board, pos, steps);
    std::cout << res << '\n';
    return 0;
}
