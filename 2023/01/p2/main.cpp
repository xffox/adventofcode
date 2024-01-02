#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>

#include "../task.hpp"

namespace
{
const std::unordered_map<std::string, char> NAMED_DIGITS{
    {"one", '1'}, {"two", '2'},   {"three", '3'}, {"four", '4'}, {"five", '5'},
    {"six", '6'}, {"seven", '7'}, {"eight", '8'}, {"nine", '9'},
};

task::uval extractAmendedCalibrationValue(std::string_view line)
{
    std::string processedLine;
    for(std::size_t i = 0; i < line.size(); ++i)
    {
        const auto iter =
            std::ranges::find_if(NAMED_DIGITS, [&](const auto &nameDigit) {
                return std::ranges::equal(
                    std::views::drop(line, i) |
                        std::views::take(nameDigit.first.size()),
                    nameDigit.first);
            });
        if(iter != std::ranges::end(NAMED_DIGITS))
        {
            processedLine.push_back(iter->second);
        }
        else
        {
            processedLine.push_back(line[i]);
        }
    }
    return task::extractCalibrationValue(processedLine);
}
}

int main()
{
    std::cout << std::ranges::fold_left(
                     std::ranges::istream_view<std::string>(std::cin) |
                         std::views::transform(extractAmendedCalibrationValue),
                     0LL, std::plus<>{})
              << '\n';
    return 0;
}
