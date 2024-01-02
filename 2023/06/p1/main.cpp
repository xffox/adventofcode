#include <algorithm>
#include <functional>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeRaces = task::parseRaces<false>(std::cin);
    if(!maybeRaces)
    {
        throw std::runtime_error(maybeRaces.error());
    }
    std::cout << std::ranges::fold_left(
                     *maybeRaces | std::views::transform(task::winningWays),
                     static_cast<task::uval>(1), std::multiplies<>{})
              << '\n';
    return 0;
}
