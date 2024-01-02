#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <iterator>
#include <ranges>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

#include "../task.hpp"

namespace
{
task::uval cardMatches(const task::Card &card)
{
    return std::ranges::fold_left(
        card.numbers | std::views::transform([&](const auto val) {
            return static_cast<task::uval>(card.wins.contains(val));
        }),
        static_cast<task::uval>(0), std::plus<>{});
}

template<std::ranges::random_access_range R>
task::ulval totalCards(const R &cards)
{
    const auto size = std::ranges::size(cards);
    if(size == 0)
    {
        return 0;
    }
    std::vector<task::ulval> cardWins(size, 0);
    for(auto i = size; i > 0; --i)
    {
        const auto idx = i - 1;
        const auto matches = cardMatches(cards[idx]);
        auto &curWins = cardWins[idx];
        curWins = 1;
        for(std::size_t j = 0; j < matches && idx + 1 + j < cardWins.size();
            ++j)
        {
            curWins += cardWins[idx + 1 + j];
        }
    }
    return std::ranges::fold_left(cardWins, static_cast<task::ulval>(0),
                                  std::plus<>{});
}
}

int main()
{
    std::vector<task::Card> cards;
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
        auto maybeCard = task::parseCard(line);
        if(!maybeCard)
        {
            throw std::runtime_error("invalid input");
        }
        cards.push_back(std::move(*maybeCard));
    }
    std::cout << totalCards(cards) << '\n';
    return 0;
}
