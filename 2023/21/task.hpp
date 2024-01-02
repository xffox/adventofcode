#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <cmath>
#include <cstddef>
#include <execution>
#include <expected>
#include <format>
#include <istream>
#include <iterator>
#include <map>
#include <optional>
#include <queue>
#include <set>
#include <string>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../base.hpp"
#include "../geom.hpp"
#include "../geom_plane.hpp"
#include "../geom_transform.hpp"
#include "../matrix.hpp"

namespace task
{
enum class Cell
{
    EMPTY,
    WALL,
};

using val = int;
using uval = unsigned int;
using ulval = unsigned long long int;
using Board = numutil::Matrix<Cell>;
using Pos = geom::Pos;
using PosSet = std::unordered_set<Pos>;

namespace input
{
    namespace detail
    {
        inline std::expected<Cell, std::string> parseCell(const char val)
        {
            switch(val)
            {
            case '.':
                return Cell::EMPTY;
            case 'S':
                return Cell::EMPTY;
            case '#':
                return Cell::WALL;
            default:
                return std::unexpected(std::format("invalid cell: {}", val));
            }
        }
    }

    inline std::expected<std::tuple<Board, Pos, uval>, std::string> parseInput(
        std::istream &stream)
    {
        uval steps = 0;
        {
            std::string line;
            std::getline(stream, line);
            const auto maybeSteps = base::parseValue<uval>(line);
            if(!maybeSteps)
            {
                return std::unexpected("can't parse steps");
            }
            steps = *maybeSteps;
        }
        std::vector<std::string> rows;
        std::optional<Pos> startPos;
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
                if(value == 'S')
                {
                    startPos = Pos{
                        static_cast<int>(rowIdx),
                        static_cast<int>(colIdx),
                    };
                }
            }
        }
        if(!startPos)
        {
            return std::unexpected("not starting position");
        }
        return std::tuple{std::move(board), *startPos, steps};
    }
}

class WrapBoard
{
public:
    WrapBoard(Board board) : board_(std::move(board))
    {
    }

    auto ix(val row, val col) const
    {
        constexpr auto wrap = [](int val, int mod) {
            const auto mult = val / mod;
            return (val - mult * mod + mod) % mod;
        };
        return board_.ix(wrap(row, board_.rows()), wrap(col, board_.columns()));
    }

    val wrapRows() const
    {
        return board_.rows();
    }
    val wrapColumns() const
    {
        return board_.columns();
    }

    const Board &board() const
    {
        return board_;
    }

private:
    Board board_;
};

class PosEval
{
public:
    explicit PosEval(Board board, const Pos &begin, const Pos &size,
                     const Pos &startPos)
        : board(std::move(board)), begin(begin), points(size.r, size.c),
          generationOddity(positionOddity(startPos - begin))
    {
        set(startPos, true);
    }

    std::optional<bool> get(const Pos &pos) const
    {
        if(const auto maybeOffset = calcOffset(pos))
        {
            return points.ix(maybeOffset->r, maybeOffset->c);
        }
        return std::nullopt;
    }

    void eval(uval steps = 1)
    {
        using geom::forEachCrossNeighbour;
        for(; steps > 0; --steps)
        {
            auto rows =
                std::views::iota(static_cast<std::size_t>(0), points.rows());
            std::for_each(
                std::ranges::begin(rows), std::ranges::end(rows),
                [&](const auto r) {
                    for(std::size_t c = !((r % 2) ^ generationOddity);
                        c < points.columns(); c += 2)
                    {
                        const auto boardPos =
                            Pos{static_cast<int>(r), static_cast<int>(c)} +
                            begin;
                        if(board.ix(boardPos.r, boardPos.c) != Cell::EMPTY)
                        {
                            continue;
                        }
                        bool neighbour = false;
                        forEachCrossNeighbour(
                            Pos{static_cast<int>(r), static_cast<int>(c)},
                            [&](const auto &np) {
                                if(np.r >= 0 &&
                                   static_cast<std::size_t>(np.r) <
                                       points.rows() &&
                                   np.c >= 0 &&
                                   static_cast<std::size_t>(np.c) <
                                       points.columns() &&
                                   points.ix(np.r, np.c))
                                {
                                    neighbour = true;
                                }
                            });
                        points.ix(r, c) = neighbour;
                    }
                });
            generationOddity = generationOddity ^ true;
        }
    }

    void shift(const Pos &diff)
    {
        decltype(points) nextPoints(points.rows(), points.columns());
        for(std::size_t r = 0; r < nextPoints.rows(); ++r)
        {
            for(std::size_t c = 0; c < nextPoints.columns(); ++c)
            {
                const auto src =
                    Pos{static_cast<int>(r), static_cast<int>(c)} - diff;
                if(src.r >= 0 &&
                   static_cast<std::size_t>(src.r) < points.rows() &&
                   src.c >= 0 &&
                   static_cast<std::size_t>(src.c) < points.columns())
                {
                    nextPoints.ix(r, c) = points.ix(src.r, src.c);
                }
            }
        }
        points = std::move(nextPoints);
        generationOddity = generationOddity ^ positionOddity(diff);
    }

    uval count(const Pos &start, const Pos &size) const
    {
        const auto actualBegin = start - begin;
        const auto actualEnd = actualBegin + size;
        const Pos step{1, 1};
        const Pos clampStart{
            std::clamp<int>(actualBegin.r, 0, points.rows() - 1),
            std::clamp<int>(actualBegin.c, 0, points.columns() - 1)};
        const Pos clampEnd{std::clamp<int>(actualEnd.r, -1, points.rows()),
                           std::clamp<int>(actualEnd.c, -1, points.columns())};
        uval cnt = 0;
        const auto totalSteps = (clampEnd - clampStart) / step;
        const bool curGenerationOddity =
            generationOddity ^ positionOddity(actualBegin);
        for(int rowIdx = 0; rowIdx < totalSteps.r; ++rowIdx)
        {
            for(int colIdx = (rowIdx % 2) ^ curGenerationOddity;
                colIdx < totalSteps.c; colIdx += 2)
            {
                const auto [row, col] = clampStart + Pos{rowIdx, colIdx} * step;
                cnt += points.ix(row, col);
            }
        }
        return cnt;
    }

    PosEval rotate(const Pos &direction) const
    {
        return PosEval(geom::rotate(board.board(), direction), begin,
                       geom::rotate(points, direction), generationOddity);
    }

    PosEval cut(const Pos &start, const Pos &size) const
    {
        const auto actualBegin = start - begin;
        const Pos step = {1, 1};
        const auto actualEnd = actualBegin + size;
        const Pos clampStart{
            std::clamp<int>(actualBegin.r, 0, points.rows() - 1),
            std::clamp<int>(actualBegin.c, 0, points.columns() - 1)};
        const Pos clampEnd{std::clamp<int>(actualEnd.r, -1, points.rows()),
                           std::clamp<int>(actualEnd.c, -1, points.columns())};
        const auto totalSteps = (clampEnd - clampStart) / step;
        numutil::Matrix<int> resultPoints(totalSteps.r, totalSteps.c);
        for(int rowIdx = 0; rowIdx < totalSteps.r; ++rowIdx)
        {
            for(int colIdx = 0; colIdx < totalSteps.c; ++colIdx)
            {
                const auto [row, col] = clampStart + Pos{rowIdx, colIdx} * step;
                resultPoints.ix(rowIdx, colIdx) = points.ix(row, col);
            }
        }
        return PosEval(board.board(), start, std::move(resultPoints),
                       generationOddity ^ positionOddity(begin - start));
    }

    const auto &b() const
    {
        return begin;
    }
    auto s() const
    {
        return Pos{
            static_cast<int>(points.rows()),
            static_cast<int>(points.columns()),
        };
    }

    const auto &brd() const
    {
        return board;
    }

private:
    PosEval(Board board, const Pos &begin, numutil::Matrix<int> points,
            bool generationOddity)
        : board(std::move(board)), begin(begin), points(std::move(points)),
          generationOddity(generationOddity)
    {
    }

    std::optional<Pos> calcOffset(const Pos &pos) const
    {
        const auto end = begin + s();
        if(pos.r < begin.r || pos.r >= end.r || pos.c < begin.c ||
           pos.c >= end.c)
        {
            return std::nullopt;
        }
        return pos - begin;
    }

    bool set(const Pos &pos, bool val)
    {
        if(const auto maybeOffset = calcOffset(pos))
        {
            points.ix(maybeOffset->r, maybeOffset->c) = val;
            return true;
        }
        return false;
    }

    static constexpr bool positionOddity(const Pos &pos)
    {
        return (pos.r % 2 != 0) ^ (pos.c % 2 != 0);
    }

    WrapBoard board;
    Pos begin;
    numutil::Matrix<int> points;
    // starting from a single seed points are always on nonintersecting
    // positions
    bool generationOddity;
};

inline ulval reachable(const Board &board, const Pos &startPos, uval steps)
{
    constexpr Pos ROT_RIGHT{0, -1};
    const val size = board.rows();
    const val offset = board.rows() / 2;
    const Pos posSize{size, size};
    if(steps <= board.rows())
    {
        const val s = steps;
        PosEval points(board, {-s, -s}, {posSize.r + s * 2, posSize.c + s * 2},
                       startPos);
        points.eval(steps);
        return points.count(points.b(), points.s());
    }
    const auto restStep = steps % size;
    const auto targetStep = steps / size;
    PosEval initPoints(board, {-size, -size}, {size * 3, size * 3}, startPos);
    initPoints.eval(restStep);
    std::array<uval, 2> majorCounts{};
    {
        PosEval cur = initPoints.cut(startPos - Pos{offset, offset}, posSize);
        uval prevCount = 0;
        while(true)
        {
            cur.eval(1);
            majorCounts[1 - restStep % 2] = cur.count({0, 0}, posSize);
            cur.eval(1);
            majorCounts[restStep % 2] = cur.count({0, 0}, posSize);
            if(majorCounts[restStep % 2] == prevCount)
            {
                break;
            }
            prevCount = majorCounts[restStep % 2];
        }
    }
    const ulval height = targetStep - 1;
    const auto crossHeight = height - 1;
    ulval resultCount =
        majorCounts[1 - restStep % 2] * (crossHeight * crossHeight) +
        majorCounts[restStep % 2] * (crossHeight + 1) * (crossHeight + 1);
    uval prevCount = 0;
    forEachCrossNeighbour(Pos{0, 0}, [&](const auto &diffDir) {
        const Pos dirUp{-1, 0};
        const auto topLeft = startPos - Pos{offset, offset};
        auto cur = initPoints.rotate(diffDir);
        cur = cur.cut(topLeft + dirUp * posSize,
                      Pos{posSize.r * 3, posSize.c * 2});
        while(true)
        {
            cur.eval(size);
            const auto count =
                cur.count(topLeft + dirUp * posSize, Pos{size * 2, size * 2});
            if(count && count == prevCount)
            {
                const auto cnt =
                    cur.count(topLeft + dirUp * posSize, Pos{size * 2, size});
                resultCount += cnt;
                resultCount +=
                    cur.count(topLeft + posSize * rotate(dirUp, ROT_RIGHT),
                              posSize) *
                    height;
                resultCount += cur.count(topLeft + posSize * dirUp +
                                             posSize * rotate(dirUp, ROT_RIGHT),
                                         posSize) *
                               (height + 1);
                resultCount += cur.count(topLeft - posSize * dirUp +
                                             posSize * rotate(dirUp, ROT_RIGHT),
                                         posSize) *
                               (height - 1);
                break;
            }
            prevCount = count;
            cur.shift((-1) * posSize * dirUp);
        }
    });
    return resultCount;
}
}

#endif
