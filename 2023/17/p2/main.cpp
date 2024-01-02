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
    if(maybeBoard->rows() == 0 || maybeBoard->columns() == 0)
    {
        throw std::runtime_error("invalid board: empty");
    }
    constexpr task::uval MAX_DIRECT_STEPS = 10;
    constexpr task::uval MIN_DIRECT_STEPS = 4;
    const auto maybeCost =
        task::minPathCost(*maybeBoard, {0, 0},
                          {static_cast<int>(maybeBoard->rows()) - 1,
                           static_cast<int>(maybeBoard->columns()) - 1},
                          [&](const auto &, const auto &state) {
                              return ((state.steps >= MIN_DIRECT_STEPS) *
                                      task::CrucibleActionMask::ROTATE) |
                                     ((state.steps < MAX_DIRECT_STEPS) *
                                      task::CrucibleActionMask::DIRECT);
                          });
    if(!maybeCost)
    {
        throw std::invalid_argument("no path found");
    }
    std::cout << *maybeCost << '\n';
    return 0;
}
