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
    constexpr task::ulval CYCLES = 1000000000;
    const auto res = task::cyclePlatform(*maybeBoard, CYCLES);
    std::cout << task::totalLoad(res) << '\n';
    return 0;
}
