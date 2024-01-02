#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>

#include "../task.hpp"

namespace
{
task::ulval cardPoints(const task::Card &card)
{
    return std::ranges::fold_left(card.numbers, static_cast<task::ulval>(0),
                                  [&](const auto cur, const auto val) {
                                      const task::ulval found =
                                          card.wins.contains(val);
                                      return std::max(found, cur * (found + 1));
                                  });
}
}

int main()
{
    task::ulval totalPoints = 0;
    for(const auto &line :
        std::ranges::subrange(std::istreambuf_iterator<char>{std::cin},
                              std::istreambuf_iterator<char>{}) |
            std::views::lazy_split('\n') |
            std::views::transform([](auto lineView) {
                std::string line;
                std::ranges::copy(lineView, std::back_inserter(line));
                return line;
            }))
    {
        if(line.empty())
        {
            continue;
        }
        const auto maybeCard = task::parseCard(line);
        if(!maybeCard)
        {
            throw std::runtime_error("invalid input");
        }
        totalPoints += cardPoints(*maybeCard);
    }
    std::cout << totalPoints << '\n';
    return 0;
}
