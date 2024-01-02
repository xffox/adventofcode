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
            std::format("input parsing failed: {}", maybeBoard.error()));
    }
    const auto maybeRes = task::longestHike(*maybeBoard);
    if(!maybeRes)
    {
        throw std::runtime_error("no hike found");
    }
    std::cout << *maybeRes << '\n';
    return 0;
}
