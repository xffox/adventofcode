#ifndef TASK_HPP
#define TASK_HPP

#include <cctype>
#include <optional>
#include <string>
#include <string_view>

#include "../base.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;

inline uval extractCalibrationValue(std::string_view line)
{
    std::optional<char> firstDigit;
    char lastDigit = '0';
    for(const auto v : line)
    {
        if(std::isdigit(v))
        {
            if(!firstDigit)
            {
                firstDigit = v;
            }
            lastDigit = v;
        }
    }
    std::string digits;
    if(firstDigit)
    {
        digits.push_back(*firstDigit);
    }
    digits.push_back(lastDigit);
    return *base::parseValue<uval>(digits);
}
}

#endif
