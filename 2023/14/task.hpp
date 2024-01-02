#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <expected>
#include <format>
#include <functional>
#include <istream>
#include <ranges>
#include <string>
#include <utility>
#include <vector>

#include "../geom.hpp"
#include "../geom_transform.hpp"
#include "../matrix.hpp"

namespace task
{
enum class Cell
{
    EMPTY,
    ROUND,
    CUBE,
};

using Board = numutil::Matrix<Cell>;

using uval = unsigned int;
using ulval = unsigned long long int;

namespace input
{
    namespace detail
    {
        constexpr std::expected<Cell, std::string> parseCell(char val)
        {
            switch(val)
            {
            case '.':
                return Cell::EMPTY;
            case '#':
                return Cell::CUBE;
            case 'O':
                return Cell::ROUND;
            default:
                return std::unexpected(
                    std::format("invalid cell value: {}", val));
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
    using Pos = geom::Pos;
}

inline uval totalLoad(const Board &board)
{
    const auto rows = board.columns();
    uval load = 0;
    for(std::size_t col = 0; col < board.columns(); ++col)
    {
        for(std::size_t row = 0; row < rows; ++row)
        {
            load += (board.ix(row, col) == Cell::ROUND) * (rows - row);
        }
    }
    return load;
}

inline Board &tiltNorth(Board &board)
{
    const auto columns = board.columns();
    for(std::size_t col = 0; col < columns; ++col)
    {
        uval height = 0;
        const auto handleColumn = [&](const std::size_t row) {
            for(uval i = 0; i < height; ++i)
            {
                board.ix(row + i, col) = Cell::ROUND;
            }
        };
        for(auto row = board.rows(); row > 0; --row)
        {
            auto &val = board.ix(row - 1, col);
            if(val == Cell::ROUND)
            {
                ++height;
                val = Cell::EMPTY;
            }
            else if(val == Cell::CUBE)
            {
                handleColumn(row);
                height = 0;
            }
        }
        handleColumn(0);
    }
    return board;
}

inline Board cyclePlatform(const Board &board, const ulval cycles)
{
    constexpr detail::Pos ROT_LEFT{0, 1};
    constexpr detail::Pos ROT_RIGHT{0, -1};
    std::vector<Board> prevs{board};
    for(ulval i = 0; i < cycles; ++i)
    {
        auto cur = prevs.back();
        for(const auto &rotation :
            {ROT_RIGHT, rotate(rotate(ROT_LEFT, ROT_LEFT), ROT_LEFT), ROT_RIGHT,
             ROT_RIGHT})
        {
            tiltNorth(cur);
            cur = geom::rotate(cur, rotation);
        }
        const auto iter = std::ranges::find(prevs, cur);
        if(iter != std::ranges::end(prevs))
        {
            const auto loopInit = iter - std::ranges::begin(prevs);
            const auto loopSize = prevs.size() - loopInit;
            return prevs[loopInit + ((cycles - loopInit) % loopSize)];
        }
        prevs.push_back(std::move(cur));
    }
    return {};
}
}

#endif
