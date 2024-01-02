#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <expected>
#include <format>
#include <functional>
#include <ranges>
#include <string>
#include <string_view>
#include <vector>

#include "../base.hpp"

namespace task
{
using val = int;
using lval = long long int;

namespace input
{
    using ValCol = std::vector<val>;

    inline std::expected<ValCol, std::string> parseValues(std::string_view str)
    {
        ValCol result;
        for(const auto valueView : str | std::views::split(' '))
        {
            if(std::ranges::empty(valueView))
            {
                continue;
            }
            std::string_view valueStr(std::ranges::begin(valueView),
                                      std::ranges::end(valueView));
            const auto maybeValue = base::parseValue<val>(valueStr);
            if(!maybeValue)
            {
                return std::unexpected(
                    std::format("invalid value: '{}'", valueStr));
            }
            result.push_back(*maybeValue);
        }
        return result;
    }
}

template<std::ranges::random_access_range R>
task::val predictNext(const R &values)
{
    std::vector<task::val> diffs(std::ranges::begin(values),
                                 std::ranges::end(values));
    std::vector<task::val> hist;
    while(!diffs.empty())
    {
        hist.push_back(diffs.back());
        bool zeros = true;
        for(std::size_t i = 0; i + 1 < diffs.size(); ++i)
        {
            diffs[i] = diffs[i + 1] - diffs[i];
            zeros = (zeros && diffs[i] == 0);
        }
        if(zeros)
        {
            break;
        }
        diffs.pop_back();
    }
    return std::ranges::fold_left(hist, static_cast<task::val>(0),
                                  std::plus<>{});
}

template<std::ranges::random_access_range R>
task::val predictPrevious(const R &values)
{
    return predictNext(std::ranges::views::reverse(values));
}
}

#endif
