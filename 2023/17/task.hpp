#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <concepts>
#include <cstddef>
#include <expected>
#include <format>
#include <istream>
#include <iterator>
#include <map>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "../base.hpp"
#include "../geom.hpp"
#include "../matrix.hpp"

namespace task
{
using uval = unsigned int;
using Board = numutil::Matrix<uval>;
using Pos = geom::Pos;

struct CrucibleState
{
    Pos dir{};
    uval steps{};
};

enum CrucibleActionMask : unsigned int
{
    DIRECT = 1,
    ROTATE = 2,
};

namespace input
{
    namespace detail
    {
        inline std::expected<uval, std::string> parseCell(const char val)
        {
            std::array<char, 2> buf{};
            buf[0] = val;
            const auto maybeCell = base::parseValue<uval>(std::string_view(
                std::ranges::begin(buf), std::ranges::begin(buf) + 1));
            if(!maybeCell)
            {
                return std::unexpected("invalid cell value");
            }
            return *maybeCell;
        }
    }

    inline std::expected<Board, std::string> parseBoard(std::istream &stream)
    {
        std::vector<std::string> rows;
        while(stream)
        {
            std::string line;
            std::getline(stream, line);
            if(line.empty())
            {
                continue;
            }
            if(!rows.empty() && rows.back().size() != line.size())
            {
                return std::unexpected("row lengths mismatch");
            }
            rows.push_back(std::move(line));
        }
        if(rows.empty())
        {
            return std::unexpected("empty board");
        }
        Board board(rows.size(), rows.front().size());
        for(std::size_t rowIdx = 0; rowIdx < rows.size(); ++rowIdx)
        {
            const auto &row = rows[rowIdx];
            for(std::size_t colIdx = 0; colIdx < row.size(); ++colIdx)
            {
                const auto value = row[colIdx];
                const auto maybeCell = detail::parseCell(value);
                if(!maybeCell)
                {
                    return std::unexpected(std::format(
                        "board parsing failed: {}", maybeCell.error()));
                }
                board.ix(rowIdx, colIdx) = *maybeCell;
            }
        }
        return board;
    }
}

namespace detail
{
    struct Cand
    {
        Pos pos{};
        CrucibleState state{};
        uval cost{};
    };
    class CandMore
    {
    public:
        explicit CandMore(const Pos &dst) : dst(dst)
        {
        }

        bool operator()(const Cand &left, const Cand &right) const
        {
            const auto orderKey = [this](const auto &cand) {
                // dist estimate for the actual cost
                return dist(dst, cand.pos) + cand.cost;
            };
            return orderKey(right) < orderKey(left);
        }

    private:
        Pos dst;
    };

    struct CrucibleStateLess
    {
        constexpr bool operator()(const CrucibleState &left,
                                  const CrucibleState &right) const
        {
            return std::forward_as_tuple(left.dir, left.steps) <
                   std::forward_as_tuple(right.dir, right.steps);
        }
    };
}

constexpr bool isInside(const Pos &pos, const Pos &size)
{
    return pos.r >= 0 && pos.r < size.r && pos.c >= 0 && pos.c < size.c;
}

template<typename CrucibleFunc>
requires std::convertible_to<
    std::invoke_result_t<CrucibleFunc, Pos, CrucibleState>,
    std::underlying_type_t<CrucibleActionMask>>
std::optional<uval> minPathCost(const Board &board, const Pos &src,
                                const Pos &dst, CrucibleFunc &&steerCrucible)
{
    constexpr task::Pos ROT_LEFT{0, 1};
    constexpr task::Pos ROT_RIGHT{0, -1};
    numutil::Matrix<std::map<CrucibleState, uval, detail::CrucibleStateLess>>
        seen(board.rows(), board.columns());
    std::priority_queue<detail::Cand, std::vector<detail::Cand>,
                        detail::CandMore>
        front(detail::CandMore{dst});
    front.push(detail::Cand{
        .pos = src,
        .state = {.dir = {0, 1}, .steps = 0},
        .cost = 0,
    });
    const Pos size{
        static_cast<int>(board.rows()),
        static_cast<int>(board.columns()),
    };
    const auto appendMove = [&](const Pos &pos, uval cost,
                                const CrucibleState &state) {
        if(isInside(pos, size))
        {
            const auto nxtCost = cost + board.ix(pos.r, pos.c);
            const auto [iter, inserted] =
                seen.ix(pos.r, pos.c).emplace(state, nxtCost);
            if(inserted || nxtCost < iter->second)
            {
                iter->second = nxtCost;
                front.push(detail::Cand{
                    .pos = pos,
                    .state = state,
                    .cost = nxtCost,
                });
            }
        }
    };
    std::optional<uval> minCost;
    while(!front.empty())
    {
        const auto cur = front.top();
        front.pop();
        const auto actions = steerCrucible(cur.pos, cur.state);
        if(cur.pos == dst && (actions & CrucibleActionMask::ROTATE))
        {
            minCost = std::min(cur.cost, minCost.value_or(cur.cost));
            continue;
        }
        if(minCost && cur.cost >= *minCost)
        {
            continue;
        }
        if(actions & CrucibleActionMask::DIRECT)
        {
            appendMove(cur.pos + cur.state.dir, cur.cost,
                       {
                           .dir = cur.state.dir,
                           .steps = cur.state.steps + 1,
                       });
        }
        if(actions & CrucibleActionMask::ROTATE)
        {
            for(const auto &rotation : {ROT_LEFT, ROT_RIGHT})
            {
                const auto nxtDir = rotate(cur.state.dir, rotation);
                appendMove(cur.pos + nxtDir, cur.cost,
                           {
                               .dir = nxtDir,
                               .steps = 1,
                           });
            }
        }
    }
    return minCost;
}
}

#endif
