#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <stdexcept>

#include "../task.hpp"

int main()
{
    const auto maybeInput = task::input::parseInput(std::cin);
    if(!maybeInput)
    {
        throw std::runtime_error(
            std::format("parsing failed: {}", maybeInput.error()));
    }
    const std::string startWorkflow = "in";
    const auto res = std::ranges::fold_left(
        maybeInput->partRatings | std::views::filter([&](const auto &rating) {
            return task::execute(rating, maybeInput->workflows,
                                 startWorkflow) == task::rule::Action::ACCEPT;
        }) | std::views::transform([](const auto &rating) {
            const auto val = std::ranges::fold_left(
                rating | std::views::transform(
                             [](const auto &p) { return p.second; }),
                static_cast<task::uval>(0), std::plus<>{});
            return val;
        }),
        static_cast<task::uval>(0), std::plus<>{});
    std::cout << res << '\n';
    return 0;
}
