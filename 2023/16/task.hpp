#ifndef TASK_HPP
#define TASK_HPP

#include <cstddef>
#include <expected>
#include <format>
#include <istream>
#include <queue>
#include <set>
#include <string>
#include <utility>
#include <variant>
#include <vector>

#include "../geom.hpp"
#include "../geom_transform.hpp"
#include "../matrix.hpp"

namespace task
{
namespace detail
{
    struct CellEmpty
    {
    };
    struct CellSplitter
    {
        bool vertical{};
    };
    struct CellMirror
    {
        geom::TransformMatrix transform{};
    };
}
using Cell =
    std::variant<detail::CellEmpty, detail::CellSplitter, detail::CellMirror>;

using uval = unsigned int;
using Board = numutil::Matrix<Cell>;
using EnergizedBoard = numutil::Matrix<int>;
using Pos = geom::Pos;

constexpr Pos ROT_LEFT{0, 1};
constexpr Pos ROT_RIGHT{0, -1};

namespace input
{
    namespace detail
    {
        inline std::expected<Cell, std::string> parseCell(char val)
        {
            switch(val)
            {
            case '.':
                return Cell{task::detail::CellEmpty{}};
            case '\\':
                return Cell{task::detail::CellMirror{
                    geom::prepareTransform({{{0, 1}, {1, 0}}})}};
            case '/':
                return Cell{task::detail::CellMirror{
                    geom::prepareTransform({{{0, -1}, {-1, 0}}})}};
            case '-':
                return Cell{task::detail::CellSplitter{false}};
            case '|':
                return Cell{task::detail::CellSplitter{true}};
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

inline EnergizedBoard runBeam(const Board &board, const Pos &pos,
                              const Pos &dir)
{
    struct Cand
    {
        Pos pos{};
        Pos dir{};
    };
    using CandQueue = std::queue<Cand>;
    struct CellVisitor
    {
        void operator()(const detail::CellEmpty &) const
        {
        }
        void operator()(const detail::CellSplitter &cell) const
        {
            const Pos dir{!cell.vertical, cell.vertical};
            if(cand.dir.r * dir.r != 0 || cand.dir.c * dir.c != 0)
            {
                {
                    auto other = cand;
                    other.dir = rotate(other.dir, ROT_LEFT);
                    cands.push(other);
                }
                cand.dir = rotate(cand.dir, ROT_RIGHT);
            }
        }
        void operator()(const detail::CellMirror &cell) const
        {
            cand.dir = geom::transform(cand.dir, cell.transform);
        }

        Cand &cand;
        CandQueue &cands;
    };
    numutil::Matrix<std::set<Pos>> seen(board.rows(), board.columns());
    CandQueue cands;
    cands.push({pos, dir});
    while(!cands.empty())
    {
        auto &cur = cands.front();
        auto &[curPos, curDir] = cur;
        if(curPos.r < 0 || static_cast<std::size_t>(curPos.r) >= board.rows() ||
           curPos.c < 0 ||
           static_cast<std::size_t>(curPos.c) >= board.columns() ||
           !seen.ix(curPos.r, curPos.c).insert(curDir).second)
        {
            cands.pop();
            continue;
        }
        std::visit(CellVisitor{cur, cands}, board.ix(curPos.r, curPos.c));
        curPos = curPos + curDir;
    }
    EnergizedBoard result(seen.rows(), seen.columns());
    for(std::size_t row = 0; row < result.rows(); ++row)
    {
        for(std::size_t col = 0; col < result.columns(); ++col)
        {
            result.ix(row, col) = !seen.ix(row, col).empty();
        }
    }
    return result;
}

inline uval totalEnergy(const EnergizedBoard &board)
{
    task::uval cnt = 0;
    for(std::size_t row = 0; row < board.rows(); ++row)
    {
        for(std::size_t col = 0; col < board.columns(); ++col)
        {
            cnt += board.ix(row, col);
        }
    }
    return cnt;
}
}

#endif
