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
#include <queue>
#include <ranges>
#include <set>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "../geom.hpp"
#include "../geom_plane.hpp"
#include "../matrix.hpp"

namespace task
{
using Pos = geom::Pos;
using val = int;
using uval = unsigned int;

struct CellPath
{
};
struct CellForest
{
};
struct CellSlope
{
    Pos dir{};
};
using Cell = std::variant<CellPath, CellForest, CellSlope>;

using Board = numutil::Matrix<Cell>;

namespace input
{
    namespace detail
    {
        inline std::expected<Cell, std::string> parseCell(const char val)
        {
            switch(val)
            {
            case '.':
                return CellPath{};
            case '#':
                return CellForest{};
            case '<':
                return CellSlope{{0, -1}};
            case '>':
                return CellSlope{{0, 1}};
            case '^':
                return CellSlope{{-1, 0}};
            case 'v':
                return CellSlope{{1, 0}};
            default:
                return std::unexpected(std::format("invalid cell: {}", val));
            }
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
    inline std::optional<Pos> emptyPositionForRow(const Board &board,
                                                  const val row)
    {
        const auto columns =
            std::views::iota(static_cast<std::size_t>(0), board.rows());
        const auto iter = std::ranges::find_if(columns, [&](const auto col) {
            return std::holds_alternative<CellPath>(board.ix(row, col));
        });
        return iter != std::ranges::end(columns)
                   ? std::make_optional(Pos{row, static_cast<val>(*iter)})
                   : std::nullopt;
    }

    auto getCell(const Board &board, const Pos &pos)
    {
        if(pos.r >= 0 && static_cast<uval>(pos.r) < board.rows() &&
           pos.c >= 0 && static_cast<uval>(pos.c) < board.columns())
        {
            return board.ix(pos.r, pos.c);
        }
        return Cell{CellForest{}};
    };

    template<typename Handler>
    requires std::is_invocable_r_v<std::optional<Handler>, Handler, Pos>
    void traverse(const Board &board, const Pos &src, Handler &&handler)
    {
        struct CellVisitor
        {
            constexpr bool operator()(const CellForest &) const
            {
                return false;
            }
            constexpr bool operator()(const CellPath &) const
            {
                return true;
            }
            constexpr bool operator()(const CellSlope &cell) const
            {
                return cell.dir == dst - src;
            }
            const Pos &src;
            const Pos &dst;
        };
        struct Cand
        {
            Pos pos{};
            Pos prev{};
        };
        const auto canMove = [board](const Pos &src, const Pos &dst) {
            return std::visit(CellVisitor{src, dst}, getCell(board, dst));
        };
        const auto traverse = [&](auto traverse, const Cand &cand,
                                  const auto &handler) -> void {
            geom::forEachCrossNeighbour(cand.pos, [&](const Pos &nextPos) {
                const auto cell = getCell(board, nextPos);
                if(canMove(cand.pos, nextPos) && nextPos != cand.prev)
                {
                    if(const auto nextHandler = handler(nextPos))
                    {
                        traverse(traverse, Cand{nextPos, cand.pos},
                                 *nextHandler);
                    }
                }
            });
        };
        Cand cand{src, src};
        traverse(traverse, cand, handler);
    }

    struct Visit
    {
        uval visits = 0;
        uval longestDist = 0;
    };
    using VisitBoard = numutil::Matrix<Visit>;

    auto visitPaths(const Board &board, const Pos &src, const Pos &dst)
    {
        using DistMap = std::map<Pos, std::map<Pos, uval>>;
        class VisitCand
        {
        public:
            explicit VisitCand(DistMap &dists, const Board &board,
                               const Pos &dst, const Pos &prevCrossroadPos,
                               uval length)
                : dists(dists), board(board), dst(dst),
                  prevCrossroadPos(prevCrossroadPos), length(length)
            {
            }

            std::optional<VisitCand> operator()(const Pos &pos) const
            {
                const auto nextLength = length + 1;
                const auto updateMax = [&](const auto &leftPos,
                                           const auto &rightPos) {
                    auto &dist = dists[leftPos][rightPos];
                    dist = std::max(dist, nextLength);
                };
                if(pos == dst)
                {
                    updateMax(pos, prevCrossroadPos);
                    updateMax(prevCrossroadPos, pos);
                    return std::nullopt;
                }
                uval neighbours = 0;
                forEachCrossNeighbour(pos,
                                      [&neighbours, this](const Pos &nextPos) {
                                          if(std::holds_alternative<CellPath>(
                                                 getCell(board, nextPos)))
                                          {
                                              ++neighbours;
                                          }
                                      });
                if(neighbours <= 2)
                {
                    return VisitCand(dists, board, dst, prevCrossroadPos,
                                     nextLength);
                }
                else
                {
                    const auto existed = dists.contains(pos);
                    updateMax(pos, prevCrossroadPos);
                    updateMax(prevCrossroadPos, pos);
                    if(!existed)
                    {
                        return VisitCand(dists, board, dst, pos, 0);
                    }
                    return std::nullopt;
                }
            }

        private:
            DistMap &dists;
            const Board &board;
            Pos dst;
            Pos prevCrossroadPos;
            uval length;
        };

        DistMap dists;
        traverse(board, src, VisitCand(dists, board, dst, src, 0));
        return dists;
    }
}

inline std::optional<uval> longestHike(const Board &board)
{
    if(board.rows() == 0)
    {
        return std::nullopt;
    }
    const auto maybeSrc = detail::emptyPositionForRow(board, 0);
    const auto maybeDst = detail::emptyPositionForRow(board, board.rows() - 1);
    if(!maybeSrc || !maybeDst)
    {
        return std::nullopt;
    }
    const auto &src = *maybeSrc;
    const auto &dst = *maybeDst;
    const auto dists = detail::visitPaths(board, src, dst);

    numutil::Matrix<int> seen(board.rows(), board.columns());
    uval length = 0;
    const auto traverse = [&](auto traverse,
                              const Pos &pos) -> std::optional<uval> {
        if(pos == dst)
        {
            return std::make_optional(length);
        }
        if(const auto distIter = dists.find(pos); distIter != end(dists))
        {
            std::optional<uval> maxDist;
            for(const auto &[neighbourPos, dist] : distIter->second)
            {
                auto &curSeen = seen.ix(neighbourPos.r, neighbourPos.c);
                if(!curSeen)
                {
                    curSeen = true;
                    length += dist;
                    maxDist = std::max<decltype(maxDist)>(
                        maxDist, traverse(traverse, neighbourPos));
                    curSeen = false;
                    length -= dist;
                }
            }
            return maxDist;
        }
        return std::nullopt;
    };
    return traverse(traverse, src);
}
}

#endif
