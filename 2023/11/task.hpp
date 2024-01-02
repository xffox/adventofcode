#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cstddef>
#include <expected>
#include <istream>
#include <ranges>
#include <string>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../geom.hpp"
#include "../matrix.hpp"
#include "../range_tree.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;

using Board = numutil::Matrix<int>;
using Pos = geom::Pos;

namespace input
{
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
                board.ix(rowIdx, colIdx) = (value == '#');
            }
        }
        return board;
    }
}

auto galaxyDistances(const Board &board, uval expansion = 2)
{
    std::vector<Pos> galaxies;
    std::unordered_set<uval> emptyRows;
    std::ranges::copy(std::views::iota(0) | std::views::take(board.rows()),
                      std::inserter(emptyRows, std::ranges::end(emptyRows)));
    std::unordered_set<uval> emptyCols;
    std::ranges::copy(std::views::iota(0) | std::views::take(board.columns()),
                      std::inserter(emptyCols, std::ranges::end(emptyCols)));
    for(uval row = 0; row < board.rows(); ++row)
    {
        for(uval col = 0; col < board.columns(); ++col)
        {
            if(board.ix(row, col) != 0)
            {
                galaxies.push_back(
                    {static_cast<int>(row), static_cast<int>(col)});
                emptyRows.erase(row);
                emptyCols.erase(col);
            }
        }
    }
    xutil::RangeTree<uval, uval> expandingRows;
    xutil::RangeTree<uval, uval> expandingCols;
    for(const auto row : emptyRows)
    {
        expandingRows.insert(row, 1);
    }
    for(const auto col : emptyCols)
    {
        expandingCols.insert(col, 1);
    }
    std::vector<ulval> distances;
    for(std::size_t i = 0; i < galaxies.size(); ++i)
    {
        for(std::size_t j = i + 1; j < galaxies.size(); ++j)
        {
            const auto &[rowLeft, colLeft] = galaxies[i];
            const auto &[rowRight, colRight] = galaxies[j];
            const auto [minRow, maxRow] = std::minmax(rowLeft, rowRight);
            const auto [minCol, maxCol] = std::minmax(colLeft, colRight);
            distances.push_back(
                static_cast<ulval>(maxRow - minRow) +
                static_cast<ulval>(expandingRows.querySum(minRow, maxRow)) *
                    (expansion - 1) +
                static_cast<ulval>(maxCol - minCol) +
                static_cast<ulval>(expandingCols.querySum(minCol, maxCol)) *
                    (expansion - 1));
        }
    }
    return distances;
}
}

#endif
