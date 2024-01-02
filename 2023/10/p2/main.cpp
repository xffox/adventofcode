#include <format>
#include <iostream>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeAnimalBoard = task::input::parseAnimalBoard(std::cin);
    if(!maybeAnimalBoard)
    {
        throw std::runtime_error(
            std::format("input parsing failed: {}", maybeAnimalBoard.error()));
    }
    const auto loopSteps =
        task::findLoop(maybeAnimalBoard->board, maybeAnimalBoard->start);
    if(!loopSteps)
    {
        throw std::runtime_error("no animal loop found");
    }
    std::cout << task::calculateInside(*loopSteps) << '\n';
    return 0;
}
