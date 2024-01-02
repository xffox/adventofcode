#include <algorithm>
#include <cstddef>
#include <format>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "../task.hpp"

namespace
{
task::uval maxEnergy(const task::Board &board)
{
    constexpr int DIRS = 4;
    task::uval max = 0;
    for(int i = 0; i < DIRS; ++i)
    {
        const auto diff = (i % 2) * 2 - 1;
        const task::Pos stepDir{diff * (i / 2), diff * (1 - i / 2)};
        const auto curDir = rotate(stepDir, task::ROT_RIGHT);
        for(task::Pos curPos{static_cast<int>((curDir.r < 0 || stepDir.r < 0) *
                                              (board.rows() - 1)),
                             static_cast<int>((curDir.c < 0 || stepDir.c < 0) *
                                              (board.columns() - 1))};
            ; curPos = curPos + stepDir)
        {
            if(curPos.r < 0 ||
               static_cast<std::size_t>(curPos.r) >= board.rows() ||
               curPos.c < 0 ||
               static_cast<std::size_t>(curPos.c) >= board.columns())
            {
                break;
            }
            max = std::max(
                max, task::totalEnergy(task::runBeam(board, curPos, curDir)));
        }
    }
    return max;
}
}

int main()
{
    const auto maybeBoard = task::input::parseBoard(std::cin);
    if(!maybeBoard)
    {
        throw std::runtime_error(
            std::format("can't parse input: {}", maybeBoard.error()));
    }
    std::cout << maxEnergy(*maybeBoard) << '\n';
    return 0;
}
