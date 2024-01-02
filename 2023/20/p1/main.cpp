#include <algorithm>
#include <format>
#include <functional>
#include <iostream>
#include <ranges>
#include <stdexcept>
#include <string>
#include <unordered_map>

#include "../task.hpp"

int main()
{
    const auto maybeConfiguration = task::input::parseConfiguration(std::cin);
    if(!maybeConfiguration)
    {
        throw std::runtime_error(std::format("input parsing failed: {}",
                                             maybeConfiguration.error()));
    }
    const std::string rootName("broadcaster");
    task::network::Network network(*maybeConfiguration, rootName);
    constexpr task::uval presses = 1000;
    std::unordered_map<task::network::Signal, task::ulval> counts;
    for(task::uval i = 0; i < presses; ++i)
    {
        network.exec(task::network::Signal::LOW,
                     [&counts](const auto &, const auto signal) {
                         counts[signal] += 1;
                         return true;
                     });
    }
    std::cout << std::ranges::fold_left(
                     counts | std::views::transform(
                                  [](const auto &p) { return p.second; }),
                     static_cast<task::ulval>(1), std::multiplies<>{})
              << '\n';
    return 0;
}
