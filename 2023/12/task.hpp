#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <deque>
#include <expected>
#include <format>
#include <istream>
#include <iterator>
#include <map>
#include <optional>
#include <ostream>
#include <ranges>
#include <span>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "../base.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;

enum class Cell
{
    OPERATIONAL,
    DAMAGED,
    UNKNOWN,
};

using Row = std::vector<Cell>;
using CountCol = std::vector<uval>;

inline std::ostream &operator<<(std::ostream &stream, Cell cell)
{
    char c{};
    switch(cell)
    {
    case Cell::OPERATIONAL:
        c = '.';
        break;
    case Cell::DAMAGED:
        c = '#';
        break;
    case Cell::UNKNOWN:
        c = '?';
        break;
    default:
        assert(false);
        break;
    }
    return stream << c;
}

namespace input
{
    namespace detail
    {
        constexpr std::expected<Cell, std::string> parseCell(char val)
        {
            switch(val)
            {
            case '#':
                return Cell::DAMAGED;
            case '.':
                return Cell::OPERATIONAL;
            case '?':
                return Cell::UNKNOWN;
            default:
                return std::unexpected(
                    std::format("invalid cell value: {}", val));
            }
        }

        inline std::expected<Row, std::string> parseRow(std::string_view str)
        {
            Row result;
            for(const auto val : str)
            {
                const auto maybeCell = parseCell(val);
                if(!maybeCell)
                {
                    return std::unexpected(
                        std::format("invalid cell: {}", maybeCell.error()));
                }
                result.push_back(*maybeCell);
            }
            return result;
        }

        inline std::expected<CountCol, std::string> parseCounts(
            std::string_view str)
        {
            CountCol result;
            for(const auto numberView : str | std::views::split(','))
            {
                const auto maybeVal = base::parseValue<uval>(
                    std::string_view(std::ranges::begin(numberView),
                                     std::ranges::end(numberView)));
                if(!maybeVal)
                {
                    std::unexpected("invalid count number");
                }
                result.push_back(*maybeVal);
            }
            return result;
        }
    }

    inline std::expected<std::tuple<Row, CountCol>, std::string> parseInputRow(
        std::string_view str)
    {
        const auto sepPos = str.find(' ');
        if(sepPos == std::string_view::npos)
        {
            return std::unexpected("missing separator");
        }
        const auto maybeRow = detail::parseRow(str.substr(0, sepPos));
        if(!maybeRow)
        {
            return std::unexpected(
                std::format("invalid row: {}", maybeRow.error()));
        }
        const auto maybeCounts = detail::parseCounts(str.substr(sepPos + 1));
        if(!maybeCounts)
        {
            return std::unexpected(
                std::format("invalid counts: {}", maybeCounts.error()));
        }
        return std::make_tuple(std::move(*maybeRow), std::move(*maybeCounts));
    }
}

namespace detail
{
    struct CandKey
    {
        uval candSize{};
        uval frontCount{};
        uval countSize{};

        friend auto operator<=>(const CandKey &left,
                                const CandKey &right) = default;
    };
}

inline ulval solve(const Row &row, const CountCol &counts)
{
    struct Cand
    {
        Cell cell{};
        uval count{};
    };
    std::deque<Cand> cands;
    {
        Cell prev = Cell::UNKNOWN;
        uval count = 0;
        const auto handleChunk = [&] {
            if(count)
            {
                cands.emplace_back(prev, count);
            }
        };
        for(const auto val : row)
        {
            if(val != prev)
            {
                handleChunk();
                prev = val;
                count = 1;
            }
            else
            {
                ++count;
            }
        }
        handleChunk();
    }

    const auto decrementCand = [](auto &cands, const uval count) {
        assert(!cands.empty());
        auto &cur = cands.front().count;
        assert(cur >= count);
        cur -= count;
        if(cur == 0)
        {
            cands.pop_front();
        }
    };
    std::map<detail::CandKey, ulval> mem;
    const auto traverse = [&](auto traverse, auto &&cands,
                              std::span<const uval> counts) -> ulval {
        if(counts.empty() && cands.empty())
        {
            return 1;
        }
        if(cands.empty())
        {
            return 0;
        }
        const detail::CandKey candKey{
            static_cast<uval>(cands.size()),
            cands.front().count,
            static_cast<uval>(counts.size()),
        };
        if(const auto iter = mem.find(candKey); iter != end(mem))
        {
            return iter->second;
        }
        const auto res = [&] {
            if(cands.front().cell == Cell::OPERATIONAL)
            {
                auto nxt = std::forward<decltype(cands)>(cands);
                nxt.pop_front();
                return traverse(traverse, std::move(nxt), counts);
            }
            if(counts.empty())
            {
                if(cands.front().cell == Cell::UNKNOWN)
                {
                    auto nxt = std::forward<decltype(cands)>(cands);
                    nxt.pop_front();
                    return traverse(traverse, std::move(nxt), counts);
                }
                return static_cast<ulval>(0);
            }
            const auto takeCount = [&](const auto &cands, auto count)
                -> std::optional<
                    std::tuple<std::decay_t<decltype(cands)>, bool>> {
                auto result = cands;
                while(count > 0 && !result.empty())
                {
                    if(result.front().cell == Cell::OPERATIONAL)
                    {
                        return std::nullopt;
                    }
                    if(result.front().cell == Cell::DAMAGED)
                    {
                        if(result.front().count > count)
                        {
                            return std::nullopt;
                        }
                        count -= result.front().count;
                        decrementCand(result, result.front().count);
                    }
                    else
                    {
                        const auto diff = std::min(count, result.front().count);
                        count -= diff;
                        decrementCand(result, diff);
                    }
                }
                if(count == 0)
                {
                    bool add = false;
                    if(!result.empty())
                    {
                        if(result.front().cell == Cell::DAMAGED)
                        {
                            return std::nullopt;
                        }
                        if(result.front().cell == Cell::UNKNOWN)
                        {
                            decrementCand(result, 1);
                            add = true;
                        }
                    }
                    return std::make_tuple(std::move(result), add);
                }
                return std::nullopt;
            };
            ulval result = 0;
            if(cands.front().cell == Cell::UNKNOWN)
            {
                if(auto maybeTake = takeCount(cands, counts.front()))
                {
                    auto &[maybeNxt, add] = *maybeTake;
                    result += traverse(traverse, std::move(maybeNxt),
                                       counts.subspan(1));
                }
                {
                    auto nxt = std::forward<decltype(cands)>(cands);
                    decrementCand(nxt, 1);
                    result += traverse(traverse, std::move(nxt), counts);
                }
            }
            else if(cands.front().cell == Cell::DAMAGED)
            {
                if(auto maybeTake = takeCount(cands, counts.front()))
                {
                    auto &[maybeNxt, add] = *maybeTake;
                    result += traverse(traverse, std::move(maybeNxt),
                                       counts.subspan(1));
                }
            }
            return result;
        }();
        mem.emplace(candKey, res);
        return res;
    };
    return traverse(traverse, cands,
                    std::span<const uval>{std::ranges::begin(counts),
                                          std::ranges::end(counts)});
}
}

#endif
