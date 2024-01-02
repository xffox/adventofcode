#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cassert>
#include <expected>
#include <functional>
#include <iterator>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <unordered_set>
#include <vector>

#include "../base.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;

struct Card
{
    std::unordered_set<uval> wins;
    std::vector<uval> numbers;
};

inline std::expected<Card, std::string> parseCard(const std::string &cardStr)
{
    const std::regex cardRegex{R"(Card +\d+: *((\d+ *)+)\| *((\d+ *)+))"};
    constexpr auto extractNumbers = [](auto &inp) {
        return inp | std::views::split(' ') |
               std::views::filter(std::not_fn(std::ranges::empty)) |
               std::views::transform([](auto numberView) {
                   const std::string_view numberStr(
                       std::ranges::begin(numberView),
                       std::ranges::end(numberView));
                   return base::parseValue<uval>(numberStr);
               });
    };
    constexpr auto expectValue = [](const auto &maybeValue) {
        assert(maybeValue);
        return *maybeValue;
    };
    if(std::smatch match; std::regex_match(cardStr, match, cardRegex))
    {
        Card card{};
        {
            const auto &winsMatch = match[1];
            std::ranges::subrange winsRange(winsMatch.first, winsMatch.second);
            std::ranges::copy(extractNumbers(winsRange) |
                                  std::views::transform(expectValue),
                              std::inserter(card.wins, end(card.wins)));
        }
        {
            const auto &numbersMatch = match[3];
            std::ranges::subrange numbersRange(numbersMatch.first,
                                               numbersMatch.second);
            std::ranges::copy(extractNumbers(numbersRange) |
                                  std::views::transform(expectValue),
                              std::back_inserter(card.numbers));
        }
        return card;
    }
    return std::unexpected("invalid card");
}
}

#endif
