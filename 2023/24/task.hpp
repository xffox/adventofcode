#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <expected>
#include <iterator>
#include <optional>
#include <ostream>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>

#include "../base.hpp"

namespace task
{
using lval = long long int;
using uval = unsigned int;
using ulval = unsigned long long int;
using floatval = long double;

struct Pos
{
    floatval x{};
    floatval y{};
    floatval z{};
};

struct Hailstone
{
    Pos position{};
    Pos velocity{};
};

inline std::ostream &operator<<(std::ostream &stream, const Pos &pos)
{
    return stream << pos.x << ',' << pos.y << ',' << pos.z;
}

inline std::ostream &operator<<(std::ostream &stream,
                                const Hailstone &hailstone)
{
    return stream << hailstone.position << " @ " << hailstone.velocity;
}

namespace input
{
    inline std::expected<Hailstone, std::string> parseHailstone(
        std::string_view str)
    {
        const std::regex hailstoneRegex(
            R"((-?\d+), *(-?\d+), *(-?\d+) *@ *(-?\d+), *(-?\d+), *(-?\d+))");
        std::match_results<std::string_view::iterator> match;
        if(!std::regex_match(begin(str), end(str), match, hailstoneRegex))
        {
            return std::unexpected("invalid hailstone format");
        }
        constexpr std::size_t VAL_COUNT = 6;
        std::array<floatval, VAL_COUNT> values;
        for(const auto &[idx, valueMatch] :
            std::views::enumerate(match | std::views::drop(1)))
        {
            const auto maybeVal = base::parseValue<lval>(
                std::string_view{valueMatch.first, valueMatch.second});
            assert(maybeVal);
            values[idx] = *maybeVal;
        }
        return Hailstone{
            {values[0], values[1], values[2]},
            {values[3], values[4], values[5]},
        };
    }
}
}

#endif
