#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <expected>
#include <format>
#include <functional>
#include <istream>
#include <iterator>
#include <map>
#include <optional>
#include <ranges>
#include <regex>
#include <set>
#include <string>
#include <string_view>
#include <tuple>
#include <vector>

#include "../base.hpp"
#include "../geom.hpp"
#include "../geom_plane.hpp"

namespace task
{
using Pos = geom::Pos;
using Size = std::size_t;

namespace input
{
    struct DigStep
    {
        Pos dir{};
        Size size{};
    };
    using DigPlan = std::vector<DigStep>;

    namespace detail
    {
        constexpr std::optional<Pos> parseDirection(std::string_view val)
        {
            if(val.size() != 1)
            {
                return std::nullopt;
            }
            switch(val[0])
            {
            case 'U':
                return Pos{-1, 0};
            case 'D':
                return Pos{1, 0};
            case 'L':
                return Pos{0, -1};
            case 'R':
                return Pos{0, 1};
            default:
                return std::nullopt;
            }
        }

        constexpr std::optional<Pos> parseDirectionSwapped(const char val)
        {
            switch(val)
            {
            case '0':
                return Pos{0, 1};
            case '1':
                return Pos{1, 0};
            case '2':
                return Pos{0, -1};
            case '3':
                return Pos{-1, 0};
            default:
                return std::nullopt;
            }
        }

        template<bool Swapped>
        inline std::expected<DigStep, std::string> parseDigStep(
            std::string_view str)
        {
            const std::regex digStepRegex(R"(([RLUD]) (\d+) \(#(\w+)\))");
            std::match_results<std::string_view::const_iterator> match;
            if(!std::regex_match(std::ranges::begin(str), std::ranges::end(str),
                                 match, digStepRegex))
            {
                return std::unexpected("invalid dig step format");
            }
            if constexpr(!Swapped)
            {
                const auto maybeDir = parseDirection(
                    std::string_view{match[1].first, match[1].second});
                if(!maybeDir)
                {
                    return std::unexpected("invalid dig direction");
                }
                const auto maybeSize = base::parseValue<Size>(
                    std::string_view{match[2].first, match[2].second});
                if(!maybeSize)
                {
                    return std::unexpected("invalid dig size");
                }
                return DigStep{
                    .dir = *maybeDir,
                    .size = *maybeSize,
                };
            }
            else
            {
                constexpr std::size_t sizeLength = 5;
                std::string_view step{match[3].first, match[3].second};
                if(step.size() != sizeLength + 1)
                {
                    return std::unexpected(
                        std::format("invalid dig step: val={}", step));
                }
                const auto maybeSize =
                    base::parseValue<Size>(step.substr(0, sizeLength), 16);
                if(!maybeSize)
                {
                    return std::unexpected("invalid dig size");
                }
                const auto maybeDir = parseDirectionSwapped(step.back());
                if(!maybeDir)
                {
                    return std::unexpected("invalid dig direction");
                }
                return DigStep{
                    .dir = *maybeDir,
                    .size = *maybeSize,
                };
            }
        }
    }

    template<bool Swapped = false>
    inline std::expected<DigPlan, std::string> parseDigPlan(
        std::istream &stream)
    {
        DigPlan plan;
        for(const auto lineView :
            std::ranges::subrange(std::istreambuf_iterator<char>{stream},
                                  std::istreambuf_iterator<char>{}) |
                std::views::lazy_split('\n'))
        {
            std::string line;
            std::ranges::copy(lineView, std::back_inserter(line));
            if(line.empty())
            {
                continue;
            }
            const auto maybeDigStep = detail::parseDigStep<Swapped>(line);
            if(!maybeDigStep)
            {
                return std::unexpected(std::format(
                    "dig step parsing failed: {}", maybeDigStep.error()));
            }
            plan.push_back(*maybeDigStep);
        }
        return plan;
    }
}

namespace detail
{
    inline std::vector<Pos> traverse(const input::DigPlan &digPlan)
    {
        std::vector<Pos> result;
        Pos cur{0, 0};
        Pos prevDir{};
        for(const auto &[dir, size] : digPlan)
        {
            assert(dir != prevDir);
            assert(dir != (prevDir * Pos{-1, -1}));
            result.push_back(cur);
            cur =
                cur + dir * Pos{static_cast<int>(size), static_cast<int>(size)};
        }
        assert((cur == Pos{0, 0}));
        return result;
    }

    inline auto traversePath(const input::DigPlan &digPlan)
    {
        std::set<Pos> seen;
        std::vector<std::tuple<Pos, Pos>> path;
        Pos cur{0, 0};
        for(const auto &[dir, size] : digPlan)
        {
            for(Size i = 0; i < size; ++i)
            {
                path.push_back({cur, dir});
                cur = cur + dir;
                const auto insertRes = seen.insert(cur).second;
                assert(insertRes);
            }
        }
        return path;
    }
}

inline Size calculateArea(const input::DigPlan &plan)
{
    const auto path = detail::traverse(plan);
    if(path.empty())
    {
        return 0;
    }
    std::map<int, std::set<int>> rowColumns;
    for(const auto &pos : path)
    {
        const auto inserted = rowColumns[pos.r].insert(pos.c).second;
        assert(inserted);
    }
    constexpr auto calcLineArea = [](const auto &line) {
        Size lineArea = 0;
        Size extraArea = 0;
        std::optional<int> prevBegin;
        std::optional<std::tuple<int, int>> prevDivider;
        const auto handleDivider = [&](const auto start, const auto end) {
            if(prevBegin)
            {
                assert(end >= *prevBegin);
                lineArea += end - *prevBegin + 1;
                prevBegin.reset();
            }
            else
            {
                prevBegin = start;
            }
        };
        for(const auto &[col, diff] : line)
        {
            if(prevDivider)
            {
                assert(diff != 0);
                const auto &[prevPos, prevDiff] = *prevDivider;
                if(prevDiff == diff)
                {
                    assert(col >= prevPos);
                    if(prevBegin)
                    {
                        extraArea += col - prevPos + 1;
                    }
                    lineArea += col - prevPos + 1;
                }
                else
                {
                    handleDivider(prevPos, col);
                }
                prevDivider.reset();
            }
            else
            {
                if(diff == 0)
                {
                    handleDivider(col, col);
                }
                else
                {
                    prevDivider = {col, diff};
                }
            }
        }
        assert(!prevBegin);
        assert(!prevDivider);
        return lineArea - extraArea;
    };
    Size area = 0;
    Size prevSize = 0;
    std::set<int> line;
    auto prevRow = begin(rowColumns)->first;
    for(const auto &[row, columns] : rowColumns)
    {
        assert(columns.size() % 2 == 0);
        const Size diff = row - prevRow;
        area += prevSize * diff;
        std::map<int, int> curCols;
        std::ranges::copy(line | std::views::transform([](const auto col) {
                              return std::pair{col, 0};
                          }),
                          std::inserter(curCols, end(curCols)));
        for(const auto col : columns)
        {
            const auto [iter, inserted] = line.insert(col);
            if(!inserted)
            {
                line.erase(iter);
            }
            curCols.insert_or_assign(col, inserted * 2 - 1);
        }
        const auto lineArea = calcLineArea(curCols);
        area += lineArea;
        prevSize =
            calcLineArea(line | std::views::transform([](const auto col) {
                             return std::pair{col, 0};
                         }));
        prevRow = row + 1;
    }
    assert(line.empty());
    assert(prevSize == 0);
    return area;
}
}

#endif
