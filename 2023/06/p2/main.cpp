#include <algorithm>
#include <cassert>
#include <functional>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeRaces = task::parseRaces<true>(std::cin);
    if(!maybeRaces)
    {
        throw std::runtime_error(maybeRaces.error());
    }
    assert(std::ranges::size(*maybeRaces) == 1);
    std::cout << task::winningWays((*maybeRaces)[0]) << '\n';
    return 0;
}
