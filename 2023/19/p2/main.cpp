#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <vector>

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
    constexpr task::Range fullRange(1, 4000);
    const std::vector<std::string> names{"x", "m", "a", "s"};
    const auto res = std::ranges::fold_left(
        task::solve(maybeInput->workflows,
                    task::SolveParam{
                        .names = names,
                        .fullRange = fullRange,
                    },
                    startWorkflow) |
            std::views::transform([](const auto &cand) {
                return std::ranges::fold_left(
                    cand | std::views::transform([](const auto &p) {
                        return p.second.e() - p.second.b() + 1;
                    }),
                    static_cast<task::ulval>(1), std::multiplies<>{});
            }),
        static_cast<task::uval>(0), std::plus<>{});
    std::cout << res << '\n';
    return 0;
}
