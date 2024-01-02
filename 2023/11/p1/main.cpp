#include <algorithm>
#include <format>
#include <functional>
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
    constexpr task::uval expansion = 2;
    const auto distances = task::galaxyDistances(*maybeBoard, expansion);
    std::cout << std::ranges::fold_left(distances, static_cast<task::uval>(0),
                                        std::plus<>{})
              << '\n';
    return 0;
}
