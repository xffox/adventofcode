#include <format>
#include <iostream>
#include <stdexcept>
#include <variant>

#include "../task.hpp"

namespace
{
task::Board &removeSlopes(task::Board &board)
{
    for(task::uval row = 0; row < board.rows(); ++row)
    {
        for(task::uval col = 0; col < board.columns(); ++col)
        {
            auto &cell = board.ix(row, col);
            if(std::holds_alternative<task::CellSlope>(cell))
            {
                cell = task::CellPath{};
            }
        }
    }
    return board;
}
}

int main()
{
    auto maybeBoard = task::input::parseBoard(std::cin);
    if(!maybeBoard)
    {
        throw std::runtime_error(
            std::format("input parsing failed: {}", maybeBoard.error()));
    }
    auto maybeRes = task::longestHike(removeSlopes(*maybeBoard));
    if(!maybeRes)
    {
        throw std::runtime_error("no hike found");
    }
    std::cout << *maybeRes << '\n';
    return 0;
}
