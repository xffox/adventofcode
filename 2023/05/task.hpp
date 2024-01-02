#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <format>
#include <istream>
#include <iterator>
#include <map>
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <tuple>
#include <utility>
#include <vector>

#include "../base.hpp"

namespace task
{
using uval = unsigned long long int;

namespace input
{
    struct NumberMapRange
    {
        uval destination{};
        uval source{};
        uval length{};
    };

    struct Map
    {
        std::vector<NumberMapRange> ranges;
    };

    struct Almanac
    {
        std::vector<uval> seeds;
        std::vector<Map> maps;
    };
}

namespace inner
{
    inline std::expected<std::vector<uval>, std::string> parseNumbers(
        std::string_view str)
    {
        std::vector<uval> numbers;
        for(auto numberView : str | std::views::split(' '))
        {
            if(std::ranges::empty(numberView))
            {
                continue;
            }
            const auto maybeNumber = base::parseValue<uval>(std::string_view(
                std::ranges::begin(numberView), std::ranges::end(numberView)));
            if(!maybeNumber)
            {
                return std::unexpected("invalid number");
            }
            numbers.push_back(*maybeNumber);
        }
        return numbers;
    }
}

struct Range
{
    uval start{};
    uval length{};
};

class Map
{
public:
    Map() = default;
    explicit Map(const input::Map &map)
        : ranges([&] {
              std::map<uval, Range> ranges;
              std::ranges::copy(
                  map.ranges | std::views::transform([](const auto &rng) {
                      return std::make_pair(rng.source,
                                            Range{rng.destination, rng.length});
                  }),
                  std::inserter(ranges, end(ranges)));
              return ranges;
          }())
    {
    }

    [[nodiscard]] uval operator()(const uval val) const
    {
        auto iter = ranges.upper_bound(val);
        if(iter == begin(ranges))
        {
            return val;
        }
        --iter;
        if(const auto maybeDiff =
               intersection(val, iter->first, iter->second.length))
        {
            return iter->second.start + *maybeDiff;
        }
        return val;
    }

    [[nodiscard]] Map join(const Map &other) const
    {
        std::map<uval, Range> result;
        for(const auto &[source, rng] : other.ranges)
        {
            const auto &destRanges = lookup({source, rng.length});
            uval offset = 0;
            for(const auto &dstRng : destRanges)
            {
                if(dstRng.start == source + offset)
                {
                    result.insert_or_assign(
                        source + offset,
                        Range{rng.start + offset, dstRng.length});
                }
                offset += dstRng.length;
            }
        }
        for(const auto &[source, rng] : ranges)
        {
            const auto &destRanges = other.lookup(rng);
            auto cur = source;
            for(const auto &dstRng : destRanges)
            {
                result.insert_or_assign(cur, dstRng);
                cur += dstRng.length;
            }
        }
        return Map(std::move(result));
    }

    [[nodiscard]] std::vector<Range> lookup(const Range &rng) const
    {
        std::vector<Range> result;
        auto cur = rng.start;
        auto iter = ranges.upper_bound(cur);
        if(iter != begin(ranges))
        {
            auto prev = iter;
            --prev;
            if(const auto maybeDiff =
                   intersection(rng.start, prev->first, prev->second.length))
            {
                result.push_back(Range{
                    prev->second.start + *maybeDiff,
                    std::min(rng.length, prev->second.length - *maybeDiff)});
                cur += result.back().length;
            }
        }
        for(; iter != end(ranges); ++iter)
        {
            if(const auto maybeDiff =
                   intersection(iter->first, rng.start, rng.length))
            {
                if(cur < iter->first)
                {
                    result.push_back(Range{cur, iter->first - cur});
                    cur += result.back().length;
                }
                result.push_back(Range{
                    iter->second.start,
                    std::min(iter->second.length, rng.length - *maybeDiff)});
                cur += result.back().length;
            }
            else
            {
                break;
            }
        }
        if(const auto maybeDiff = intersection(cur, rng.start, rng.length))
        {
            result.push_back(Range{cur, rng.length - *maybeDiff});
        }
        return result;
    }

private:
    explicit Map(std::map<uval, Range> ranges) : ranges(std::move(ranges))
    {
    }

    static constexpr std::optional<uval> intersection(const uval value,
                                                      const uval source,
                                                      const uval length)
    {
        assert(value >= source);
        if(const auto diff = value - source; diff < length)
        {
            return diff;
        }
        return std::nullopt;
    }

    std::map<uval, Range> ranges;
};

inline std::expected<input::Almanac, std::string> parseAlmanac(
    std::istream &stream)
{
    const std::regex seedsRegex(R"(seeds: ((\d+ ?)+))");

    std::string line;
    std::getline(stream, line);
    if(std::smatch match; std::regex_match(line, match, seedsRegex))
    {
        auto maybeSeeds = inner::parseNumbers(
            std::string_view(match[1].first, match[1].second));
        if(!maybeSeeds)
        {
            return std::unexpected(
                std::format("invalid seeds: {}", maybeSeeds.error()));
        }
        std::getline(stream, line);
        std::vector<input::Map> maps;
        while(stream)
        {
            std::getline(stream, line);
            std::vector<input::NumberMapRange> ranges;
            while(true)
            {
                std::getline(stream, line);
                if(line.empty())
                {
                    break;
                }
                auto maybeRanges = inner::parseNumbers(line);
                if(!maybeRanges)
                {
                    return std::unexpected(
                        std::format("invalid ranges: {}", maybeRanges.error()));
                }
                constexpr std::size_t RANGE_NUMS = 3;
                if(maybeRanges->size() != RANGE_NUMS)
                {
                    return std::unexpected("invalid range number count");
                }
                ranges.push_back({
                    (*maybeRanges)[0],
                    (*maybeRanges)[1],
                    (*maybeRanges)[2],
                });
            }
            maps.push_back({std::move(ranges)});
        }
        return input::Almanac{
            std::move(*maybeSeeds),
            std::move(maps),
        };
    }
    else
    {
        return std::unexpected("invalid seeds");
    }
}
}

#endif
