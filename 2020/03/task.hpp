#ifndef TASK_H
#define TASK_H

#include <cstddef>

#include "../matrix.h"

namespace task
{
    enum class Cell
    {
        EMPTY,
        TREE
    };

    struct Pos
    {
        size_t row;
        size_t col;
    };

    using Board = numutil::Matrix<Cell>;

    size_t countTrees(const Board &board, const Pos &slope)
    {
        if(board.columns() == 0)
        {
            return 0;
        }
        size_t trees = 0;
        for(size_t row = 0, col = 0; row < board.rows();
            row += slope.row, col = (col+slope.col)%board.columns())
        {
            trees += static_cast<size_t>(board.ix(row, col) == Cell::TREE);
        }
        return trees;
    }
}

#endif
