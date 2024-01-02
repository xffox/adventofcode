#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <concepts>
#include <cstddef>
#include <expected>
#include <format>
#include <map>
#include <ostream>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "../base.hpp"
#include "../matrix.hpp"

namespace task
{
using val = int;
using uval = unsigned int;

struct Pos
{
    val x{};
    val y{};
    val z{};
};

struct Range
{
    Pos begin{};
    Pos end{};
};

using ID = uval;

using UsageMap = std::unordered_map<ID, std::unordered_set<ID>>;

inline std::ostream &operator<<(std::ostream &stream, const Pos &pos)
{
    return stream << pos.x << ',' << pos.y << ',' << pos.z;
}

inline std::ostream &operator<<(std::ostream &stream, const Range &range)
{
    return stream << range.begin << '~' << range.end;
}

namespace input
{
    inline std::expected<Range, std::string> parseBrick(std::string_view str)
    {
        constexpr std::size_t VALUE_COUNT = 6;
        std::regex brickRegex(R"((\d+),(\d+),(\d+)~(\d+),(\d+),(\d+))");
        std::match_results<std::string_view::iterator> match;
        if(!std::regex_match(std::ranges::begin(str), std::ranges::end(str),
                             match, brickRegex))
        {
            return std::unexpected("invalid brick format");
        }
        std::array<val, VALUE_COUNT> values{};
        for(std::size_t i = 1; i < match.size(); ++i)
        {
            const auto maybeVal = base::parseValue<val>(
                std::string_view{match[i].first, match[i].second});
            assert(maybeVal);
            values[i - 1] = *maybeVal;
        }
        return Range{{values[0], values[1], values[2]},
                     {values[3], values[4], values[5]}};
    }
}

namespace detail
{
    struct RangeHeightLess
    {
        constexpr bool operator()(const Range &left, const Range &right) const
        {
            constexpr auto key = [](const Range &range) {
                return std::min(range.begin.z, range.end.z);
            };
            return key(left) < key(right);
        }
    };

    constexpr ID generateID(const uval idx)
    {
        return idx;
    }
}

template<std::ranges::forward_range R>
requires std::same_as<std::ranges::range_value_t<R>, Range>
inline UsageMap run(const R &bricks)
{
    struct HeightID
    {
        val height{};
        ID id{};
    };
    UsageMap usage;
    if(bricks.empty())
    {
        return usage;
    }
    const auto maxX =
        std::ranges::max(bricks | std::views::transform([](const auto &brick) {
                             return std::max(brick.begin.x, brick.end.x);
                         }));
    const auto maxY =
        std::ranges::max(bricks | std::views::transform([](const auto &brick) {
                             return std::max(brick.begin.y, brick.end.y);
                         }));
    const auto namedBricks = std::views::enumerate(bricks) |
                             std::views::transform([](const auto &p) {
                                 const auto &[idx, pos] = p;
                                 return std::pair{pos, detail::generateID(idx)};
                             });
    std::multimap<Range, ID, detail::RangeHeightLess> sortedBricks{
        std::ranges::begin(namedBricks), std::ranges::end(namedBricks)};
    numutil::Matrix<HeightID> heights(maxY + 1, maxX + 1);
    for(const auto &[brick, id] : sortedBricks)
    {
        Pos maxHeightPos{};
        std::unordered_set<ID> belowIDs{};
        for(val brickY = brick.begin.y; brickY <= brick.end.y; ++brickY)
        {
            for(val brickX = brick.begin.x; brickX <= brick.end.x; ++brickX)
            {
                const auto &cur = heights.ix(brickY, brickX);
                if(cur.height > 0)
                {
                    if(cur.height == maxHeightPos.z)
                    {
                        belowIDs.insert(cur.id);
                    }
                    else if(cur.height > maxHeightPos.z)
                    {
                        maxHeightPos = {brickX, brickY, cur.height};
                        belowIDs = {cur.id};
                    }
                }
            }
        }
        val curHeight = 0;
        if(maxHeightPos.z > 0)
        {
            const auto &cur = heights.ix(maxHeightPos.y, maxHeightPos.x);
            curHeight = cur.height;
            for(const auto &belowID : belowIDs)
            {
                usage[belowID].insert(id);
            }
        }
        const auto finalHeight = curHeight + (brick.end.z - brick.begin.z + 1);
        for(val brickY = brick.begin.y; brickY <= brick.end.y; ++brickY)
        {
            for(val brickX = brick.begin.x; brickX <= brick.end.x; ++brickX)
            {
                heights.ix(brickY, brickX) = {
                    .height = finalHeight,
                    .id = id,
                };
            }
        }
    }
    return usage;
}
}

#endif
