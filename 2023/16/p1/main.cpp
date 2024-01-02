#include <cstddef>
#include <format>
#include <iostream>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeBoard = task::input::parseBoard(std::cin);
    if(!maybeBoard)
    {
        throw std::runtime_error(
            std::format("can't parse input: {}", maybeBoard.error()));
    }
    const auto result = task::runBeam(*maybeBoard, {0, 0}, {0, 1});
    std::cout << task::totalEnergy(result) << '\n';
    return 0;
}
