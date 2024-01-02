#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <cstddef>
#include <expected>
#include <istream>
#include <optional>
#include <ranges>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "../geom.hpp"
#include "../geom_plane.hpp"
#include "../matrix.hpp"

namespace task
{
enum class Cell
{
    EMPTY = 0,
    HORIZONTAL,
    VERTICAL,
    ROT_UP_RIGHT,
    ROT_UP_LEFT,
    ROT_DOWN_LEFT,
    ROT_DOWN_RIGHT,
};

using Pos = geom::Pos;
using Board = numutil::Matrix<Cell>;
using Size = std::size_t;

struct PathEntry
{
    Pos pos{};
    Pos dir{};
};

using Path = std::vector<PathEntry>;

namespace input
{
    struct AnimalBoard
    {
        Board board{};
        Pos start{};
    };

    namespace detail
    {
        constexpr std::optional<Cell> parseCell(char c)
        {
            switch(c)
            {
            case '.':
                return Cell::EMPTY;
            case '-':
                return Cell::HORIZONTAL;
            case '|':
                return Cell::VERTICAL;
            case 'L':
                return Cell::ROT_UP_RIGHT;
            case 'J':
                return Cell::ROT_UP_LEFT;
            case '7':
                return Cell::ROT_DOWN_LEFT;
            case 'F':
                return Cell::ROT_DOWN_RIGHT;
            default:
                return std::nullopt;
            }
        }
    }

    inline std::expected<AnimalBoard, std::string> parseAnimalBoard(
        std::istream &stream)
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
            return std::unexpected("no start position");
        }
        std::optional<Pos> start;
        Board board(rows.size(), rows.front().size());
        for(std::size_t rowIdx = 0; rowIdx < rows.size(); ++rowIdx)
        {
            const auto &row = rows[rowIdx];
            for(std::size_t colIdx = 0; colIdx < row.size(); ++colIdx)
            {
                const auto value = row[colIdx];
                Cell cell{};
                if(value != 'S')
                {
                    const auto maybeCell = detail::parseCell(value);
                    if(!maybeCell)
                    {
                        return std::unexpected("invalid board cell");
                    }
                    cell = *maybeCell;
                }
                else
                {
                    start =
                        Pos{static_cast<int>(rowIdx), static_cast<int>(colIdx)};
                    cell = Cell::EMPTY;
                }
                board.ix(rowIdx, colIdx) = cell;
            }
        }
        if(!start)
        {
            return std::unexpected("no start position");
        }
        return AnimalBoard{std::move(board), *start};
    }
}

namespace detail
{
    using TransformMatrix = numutil::Matrix<int>;
    constexpr std::size_t TRANSFORM_SIZE = 2;

    inline TransformMatrix prepareTransform(
        const std::array<std::array<int, TRANSFORM_SIZE>, TRANSFORM_SIZE>
            &values)
    {
        TransformMatrix result(values.size(), values.front().size());
        for(std::size_t r = 0; r < values.size(); ++r)
        {
            for(std::size_t c = 0; c < values.size(); ++c)
            {
                result.ix(r, c) = values[r][c];
            }
        }
        return result;
    }

    inline bool check(const Pos &pos, const TransformMatrix &transform)
    {
        return (pos.r == transform.ix(0, 0) || pos.c == transform.ix(0, 1)) ||
               (pos.r == transform.ix(1, 0) || pos.c == transform.ix(1, 1));
    }

    inline Pos transform(const Pos &pos, const TransformMatrix &transform)
    {
        return Pos{
            pos.r * transform.ix(0, 0) + pos.c * transform.ix(0, 1),
            pos.r * transform.ix(1, 0) + pos.c * transform.ix(1, 1),
        };
    }
}

inline std::optional<Path> findLoop(const Board &board, const Pos &pos)
{
    using Operation =
        std::tuple<detail::TransformMatrix, detail::TransformMatrix>;

    constexpr int DIRS = 4;

    const auto directTransform = detail::prepareTransform({{{1, 0}, {0, 1}}});
    const auto rotateTransform = detail::prepareTransform({{{0, 1}, {1, 0}}});
    const auto invRotateTransform =
        detail::prepareTransform({{{0, -1}, {-1, 0}}});
    std::array<Operation, std::to_underlying(Cell::ROT_DOWN_RIGHT) + 1>
        operations{
            Operation{detail::prepareTransform({0, 0, 0, 0}),
                      detail::prepareTransform({0, 0, 0, 0})},
            Operation{detail::prepareTransform({0, 1, 0, -1}), directTransform},
            Operation{detail::prepareTransform({1, 0, -1, 0}), directTransform},
            Operation{detail::prepareTransform({1, -1, 0, 0}), rotateTransform},
            Operation{detail::prepareTransform({1, 1, 0, 0}),
                      invRotateTransform},
            Operation{detail::prepareTransform({-1, 1, 0, 0}), rotateTransform},
            Operation{detail::prepareTransform({1, -1, 0, 0}),
                      invRotateTransform},
        };
    const auto checkStep = [&](const auto &pos, const auto &dir) {
        const auto next = pos + dir;
        if(next.r >= 0 && static_cast<std::size_t>(next.r) < board.rows() &&
           next.c >= 0 && static_cast<std::size_t>(next.c) < board.columns())
        {
            return detail::check(
                dir,
                std::get<0>(
                    operations[std::to_underlying(board.ix(next.r, next.c))]));
        }
        return false;
    };
    const auto follow = [&](const auto &pos, auto dir) -> std::optional<Path> {
        constexpr Pos INVALID_DIR{0, 0};
        auto cur = pos;
        Path steps;
        while(dir != INVALID_DIR)
        {
            if(checkStep(cur, dir))
            {
                steps.emplace_back(cur, dir);
                cur = cur + dir;
                if(cur == pos)
                {
                    break;
                }
                dir = detail::transform(
                    dir, std::get<1>(operations[std::to_underlying(
                             board.ix(cur.r, cur.c))]));
            }
            else
            {
                return std::nullopt;
            }
        }
        return dir != INVALID_DIR ? std::make_optional(std::move(steps))
                                  : std::nullopt;
    };
    for(int i = 0; i < DIRS; ++i)
    {
        const auto coef = ((i / 2) * 2 - 1);
        const auto dirRow = coef * (i % 2);
        const auto dirCol = coef * (1 - i % 2);
        const Pos dir{dirRow, dirCol};
        if(const auto maybeSteps = follow(pos, dir))
        {
            return *maybeSteps;
        }
    }
    return std::nullopt;
}

using geom::calculateInside;
}

#endif
