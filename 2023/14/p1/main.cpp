#include <format>
#include <iostream>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    auto maybeBoard = task::input::parseBoard(std::cin);
    if(!maybeBoard)
    {
        throw std::runtime_error(
            std::format("input parsing failed: {}", maybeBoard.error()));
    }
    std::cout << task::totalLoad(task::tiltNorth(*maybeBoard)) << '\n';
    return 0;
}
