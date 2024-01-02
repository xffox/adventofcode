#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cmath>
#include <expected>
#include <format>
#include <istream>
#include <iterator>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <vector>

#include "../base.hpp"

namespace task
{
using uval = unsigned long long int;
using val = long long int;

namespace input
{
    struct Race
    {
        uval time{};
        uval distance{};
    };
}

namespace detail
{
    inline std::expected<std::vector<uval>, std::string> parseValues(
        std::string_view str)
    {
        const std::regex REGEX(R"(\w+: *((\d+ *)*))");
        std::match_results<std::ranges::const_iterator_t<std::string_view>>
            match;
        if(!std::regex_match(std::ranges::begin(str), std::ranges::end(str),
                             match, REGEX))
        {
            return std::unexpected("invalid values");
        }
        std::vector<uval> result;
        for(const auto &valuesMatch = match[1];
            const auto valueView :
            std::ranges::subrange(valuesMatch.first, valuesMatch.second) |
                std::views::split(' '))
        {
            if(std::ranges::empty(valueView))
            {
                continue;
            }
            const auto maybeValue = base::parseValue<uval>(std::string_view(
                std::ranges::begin(valueView), std::ranges::end(valueView)));
            assert(maybeValue);
            result.push_back(*maybeValue);
        }
        return result;
    }

    inline std::expected<uval, std::string> parseJoinedValue(
        std::string_view str)
    {
        std::string valueStr;
        std::ranges::copy(str | std::views::filter([](const auto ch) {
                              return std::isdigit(ch);
                          }),
                          std::back_inserter(valueStr));
        const auto maybeValue = base::parseValue<uval>(valueStr);
        assert(maybeValue);
        return *maybeValue;
    }
}

task::uval winningWays(const task::input::Race &race)
{
    const auto d = std::sqrt(race.time * race.time - 4 * race.distance);
    const auto v1 = (race.time - d) / 2;
    const auto v2 = (race.time + d) / 2;
    const auto low = std::ceil(v1) + (std::ceil(v1) == v1);
    const auto high = std::floor(v2) - (std::floor(v2) == v2);
    const auto res = high - low + 1;
    return res;
}

template<bool Joined = false>
inline std::expected<std::vector<input::Race>, std::string> parseRaces(
    std::istream &stream)
{
    std::string line;
    std::getline(stream, line);
    constexpr auto parseValues = [](const auto &line) {
        if constexpr(!Joined)
        {
            return detail::parseValues(line);
        }
        else
        {
            return detail::parseJoinedValue(line).transform(
                [](const auto val) { return std::vector{val}; });
        }
    };
    const auto maybeTimes = parseValues(line);
    if(!maybeTimes)
    {
        return std::unexpected(
            std::format("invalid times: {}", maybeTimes.error()));
    }
    std::getline(stream, line);
    const auto maybeDistances = parseValues(line);
    if(!maybeDistances)
    {
        return std::unexpected(
            std::format("invalid distances: {}", maybeDistances.error()));
    }
    if(maybeTimes->size() != maybeDistances->size())
    {
        return std::unexpected("values lengths mismatch");
    }
    std::vector<input::Race> result;
    std::ranges::copy(std::views::zip_transform(
                          [](const auto time, const auto distance) {
                              return input::Race{time, distance};
                          },
                          *maybeTimes, *maybeDistances),
                      std::back_inserter(result));
    return result;
}
}

#endif
