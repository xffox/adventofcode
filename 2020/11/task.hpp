#ifndef TASK_H
#define TASK_H

#include <algorithm>
#include <cstddef>
#include <string>
#include <optional>
#include <ranges>
#include <iterator>
#include <vector>
#include <tuple>
#include <utility>
#include <cassert>

#include "../matrix.h"

namespace task
{
    using namespace std;

    enum class Cell
    {
        FLOOR,
        EMPTY,
        OCCUPIED
    };

    using Board = numutil::Matrix<Cell>;

    namespace
    {
        template<ranges::view R>
        size_t size(R view)
        {
            size_t cols = 0;
            ranges::for_each(view, [&cols](auto&&){++cols;});
            return cols;
        }
    }

    inline optional<Board> parseBoard(const string &str)
    {
        auto lines = str | ranges::views::split('\n');
        if(begin(lines) == end(lines))
        {
            Board b(0, 0);
            return b;
        }
        const auto rows = size(lines);
        const auto cols = size(*begin(lines));
        Board b(rows, cols);
        size_t r = 0;
        for(auto &&line : lines)
        {
            size_t c = 0;
            for(auto v : line)
            {
                Cell cell = Cell::FLOOR;
                switch(v)
                {
                case '.':
                    cell = Cell::FLOOR;
                    break;
                case 'L':
                    cell = Cell::EMPTY;
                    break;
                case '#':
                    cell = Cell::OCCUPIED;
                    break;
                default:
                    return {};
                }
                b.ix(r, c) = cell;
                ++c;
            }
            ++r;
        }
        return b;
    }

    inline bool equalBoards(
        const Board &left, const Board &right)
    {
        if(left.rows() != right.rows() ||
            left.columns() != right.columns())
        {
            return false;
        }
        for(size_t i = 0; i < left.rows(); ++i)
        {
            for(size_t j = 0; j < left.columns(); ++j)
            {
                if(left.ix(i, j) != right.ix(i, j))
                {
                    return false;
                }
            }
        }
        return true;
    }

    inline string format(const Board &board)
    {
        string str;
        for(size_t i = 0; i < board.rows(); ++i)
        {
            for(size_t j = 0; j < board.columns(); ++j)
            {
                auto c = '\0';
                switch(board.ix(i, j))
                {
                case Cell::FLOOR:
                    c = '.';
                    break;
                case Cell::EMPTY:
                    c = 'L';
                    break;
                case Cell::OCCUPIED:
                    c = '#';
                    break;
                default:
                    assert(false);
                    break;
                }
                str.push_back(c);
            }
            str.push_back('\n');
        }
        return str;
    }

    auto countOccupied(const task::Board &board)
    {
        size_t cnt = 0;
        for(size_t r = 0; r < board.rows(); ++r)
        {
            for(size_t c = 0; c < board.columns(); ++c)
            {
                if(board.ix(r, c) == task::Cell::OCCUPIED)
                {
                    ++cnt;
                }
            }
        }
        return cnt;
    }

    class SeatEval
    {
    public:
        explicit SeatEval(const Board &board,
            size_t maxOccupied, bool trace = false)
            :board_(board), maxOccupied(maxOccupied), trace(trace), seats()
        {
            for(size_t r = 0; r < board_.rows(); ++r)
            {
                for(size_t c = 0; c < board_.columns(); ++c)
                {
                    if(board_.ix(r, c) != Cell::FLOOR)
                    {
                        seats.emplace_back(r, c);
                    }
                }
            }
        }

        void step()
        {
            Board next(board_.rows(), board_.columns(), Cell::FLOOR);
            for(const auto &[row, col] : seats)
            {
                constexpr int DIRS = 4;
                size_t occupied = 0;
                auto count = [&occupied, this, row=row, col=col](int dr, int dc){
                    int r = row;
                    int c = col;
                    r += dr;
                    c += dc;
                    for(;r >= 0 && static_cast<size_t>(r) < board_.rows() &&
                        c >= 0 && static_cast<size_t>(c) < board_.columns();
                        r += dr, c += dc)
                    {
                        const auto v = board_.ix(r, c);
                        if(v == Cell::OCCUPIED)
                        {
                            ++occupied;
                            break;
                        }
                        if(v == Cell::EMPTY || !trace)
                        {
                            break;
                        }
                    }
                };
                for(int i = 0; i < DIRS; ++i)
                {
                    const auto dr = (i/2)*(i%2*2-1);
                    const auto dc = (1-i/2)*(i%2*2-1);
                    count(dr, dc);
                }
                for(int i = 0; i < DIRS; ++i)
                {
                    const auto dr = (i%2*2-1);
                    const auto dc = (i/2*2-1);
                    count(dr, dc);
                }
                if(occupied == 0)
                {
                    next.ix(row, col) = Cell::OCCUPIED;
                }
                else if(occupied >= maxOccupied)
                {
                    next.ix(row, col) = Cell::EMPTY;
                }
                else
                {
                    next.ix(row, col) = board_.ix(row, col);
                }
            }
            board_ = move(next);
        }

        const Board &board() const
        {
            return board_;
        }

    private:
        using Pos = tuple<size_t, size_t>;

    private:
        Board board_;
        size_t maxOccupied;
        bool trace;
        vector<Pos> seats;
    };
}

#endif
