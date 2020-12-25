#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <algorithm>
#include <optional>
#include <unordered_map>
#include <unordered_set>
#include <string>
#include <ranges>
#include <regex>
#include <vector>
#include <iterator>
#include <utility>
#include <cassert>
#include <array>
#include <cmath>
#include <set>
#include <tuple>

#include "../matrix.h"
#include "../base.hpp"

namespace task
{
    using namespace std;

    using Tile = numutil::Matrix<char>;
    using TileID = unsigned int;
    using TileMap = unordered_map<TileID, Tile>;

    inline optional<TileMap> parseTiles(const string &tiles)
    {
        static const regex TILE_REGEX("Tile ([0-9]+):");
        TileMap result;
        vector<string> rowStrs;
        bool inTile = false;
        TileID tileID = 0;
        auto appendTile = [&result, &rowStrs, &tileID](){
            const auto rows = rowStrs.size();
            size_t cols = 0;
            if(!rowStrs.empty())
            {
                cols = rowStrs.front().size();
            }
            Tile tile(rows, cols);
            for(size_t i = 0; i < rows; ++i)
            {
                const auto &rowStr = rowStrs[i];
                if(rowStr.size() != cols)
                {
                    return false;
                }
                for(size_t j = 0; j < cols; ++j)
                {
                    tile.ix(i, j) = rowStr[j];
                }
            }
            result.insert(
                make_pair(tileID, move(tile)));
            rowStrs.clear();
            return true;
        };
        for(auto &&line : tiles | ranges::views::split('\n'))
        {
            string lineStr;
            ranges::copy(line, back_inserter(lineStr));
            if(lineStr.empty())
            {
                if(inTile)
                {
                    if(!appendTile())
                    {
                        return {};
                    }
                    inTile = false;
                    tileID = 0;
                }
            }
            else
            {
                if(inTile)
                {
                    rowStrs.push_back(move(lineStr));
                }
                else
                {
                    smatch matches;
                    if(!regex_match(lineStr, matches, TILE_REGEX))
                    {
                        return {};
                    }
                    const auto idRes =
                        base::parseValue<TileID>(matches.str(1));
                    if(!idRes)
                    {
                        return {};
                    }
                    tileID = *idRes;
                    inTile = true;
                }
            }
        }
        if(inTile)
        {
            if(!appendTile())
            {
                return {};
            }
        }
        return result;
    }

    using ulint = unsigned long long int;

    constexpr size_t BORDERS = 4;
    constexpr size_t TOP_BORDER = 0;
    constexpr size_t RIGHT_BORDER = 1;
    constexpr size_t BOTTOM_BORDER = 2;
    constexpr size_t LEFT_BORDER = 3;
    struct Border
    {
        array<string, BORDERS> borders{};
    };

    inline string flipBorder(const string &border)
    {
        return string(rbegin(border), rend(border));
    }

    inline Border extractBorders(const Tile &tile)
    {
        Border borders;
        if(tile.rows() == 0 || tile.columns() == 0)
        {
            return borders;
        }
        for(int i = 0; i < static_cast<int>(BORDERS); ++i)
        {
            const int dr = (i%2)*((1-i/2)*2-1);
            const int dc = (1-i%2)*((1-i/2)*2-1);
            size_t curRow = (i/2)*(tile.rows()-1);
            size_t curCol = ((1+i)%BORDERS/2)*(tile.columns()-1);
            const size_t steps = max(
                dr!=0 ? tile.rows()/abs(dr) : 0,
                dc!=0 ? tile.columns()/abs(dc) : 0);
            string border;
            border.reserve(steps);
            for(size_t j = 0; j < steps; ++j, curRow += dr, curCol += dc)
            {
                border.push_back(tile.ix(curRow, curCol));
            }
            borders.borders[i] = move(border);
            if(i >= 2)
            {
                borders.borders[i] = flipBorder(borders.borders[i]);
            }
        }
        return borders;
    }

    inline Border rotate(const Border &border, size_t cnt = 1)
    {
        Border result;
        for(size_t i = 0; i < result.borders.size(); ++i)
        {
            const auto idx = (i+cnt)%BORDERS;
            result.borders[i] =
                (i/2 == idx/2
                 ?border.borders[idx]
                 :flipBorder(border.borders[idx]));
        }
        return result;
    }

    inline Border flipHor(const Border &border)
    {
        return {{
            flipBorder(border.borders[TOP_BORDER]),
            border.borders[LEFT_BORDER],
            flipBorder(border.borders[BOTTOM_BORDER]),
            border.borders[RIGHT_BORDER]
        }};
    }

    struct Pos
    {
        size_t row;
        size_t col;
    };

    constexpr Pos posFromIdx(size_t idx, size_t len)
    {
        return {idx/len, idx%len};
    }

    struct Operation
    {
        size_t rotate;
        bool flip;
    };

    inline optional<numutil::Matrix<tuple<TileID, Operation>>> matchTiles(
        const TileMap &tiles)
    {
        vector<tuple<TileID, Border, Operation>> candidates;
        for(const auto &p : tiles)
        {
            const auto border = extractBorders(p.second);
            for(size_t i = 0; i < BORDERS; ++i)
            {
                const auto rot = rotate(border, i);
                const auto flip = flipHor(rot);
                candidates.push_back(make_tuple(p.first,
                        rot, Operation{i, false}));
                candidates.push_back(make_tuple(p.first,
                        flip, Operation{i, true}));
            }
        }
        unordered_map<string, set<size_t>> rightCands;
        unordered_map<string, set<size_t>> bottomCands;
        for(size_t i = 0; i < candidates.size(); ++i)
        {
            const auto &[tileID, border, operation] = candidates[i];
            rightCands[border.borders[LEFT_BORDER]].insert(i);
            bottomCands[border.borders[TOP_BORDER]].insert(i);
        }
        vector<vector<size_t>> backtrack;
        backtrack.emplace_back();
        ranges::copy(ranges::views::iota(static_cast<size_t>(0), candidates.size()),
            back_inserter(backtrack.back()));
        set<size_t> allCandidates;
        ranges::copy(ranges::views::iota(static_cast<size_t>(0), candidates.size()),
            inserter(allCandidates, end(allCandidates)));
        unordered_set<size_t> unusedTiles;
        for(const auto &p : tiles)
        {
            unusedTiles.insert(p.first);
        }
        const size_t len = round(sqrt(tiles.size()));
        assert(len*len == tiles.size());
        numutil::Matrix<size_t> solution(len, len);
        size_t filled = 0;
        while(filled < tiles.size() && !backtrack.empty())
        {
            const auto curPos = posFromIdx(filled, len);
            if(backtrack.back().empty())
            {
                if(filled > 0)
                {
                    --filled;
                    const auto prevPos = posFromIdx(filled, len);
                    unusedTiles.insert(
                        get<0>(candidates.at(solution.ix(prevPos.row, prevPos.col))));
                }
                backtrack.pop_back();
                continue;
            }
            const auto curID = backtrack.back().back();
            solution.ix(curPos.row, curPos.col) = curID;
            unusedTiles.erase(get<0>(candidates.at(curID)));
            ++filled;
            backtrack.back().pop_back();
            if(filled < tiles.size())
            {
                const auto nextPos = posFromIdx(filled, len);
                set<size_t> newCandidates = allCandidates;
                if(nextPos.row > 0)
                {
                    const auto topSol =
                        solution.ix(nextPos.row-1, nextPos.col);
                    const auto &cand =
                        candidates.at(topSol);
                    const auto &border = get<1>(cand).borders[BOTTOM_BORDER];
                    auto iter = bottomCands.find(border);
                    if(iter != end(bottomCands))
                    {
                        set<size_t> candIntersection;
                        ranges::set_intersection(newCandidates, iter->second,
                            inserter(candIntersection, end(candIntersection)));
                        newCandidates = move(candIntersection);
                    }
                    else
                    {
                        newCandidates.clear();
                    }
                }
                if(nextPos.col > 0)
                {
                    const auto leftSol =
                        solution.ix(nextPos.row, nextPos.col-1);
                    const auto &cand =
                        candidates.at(leftSol);
                    const auto &border = get<1>(cand).borders[RIGHT_BORDER];
                    auto iter = rightCands.find(border);
                    if(iter != end(rightCands))
                    {
                        set<size_t> candIntersection;
                        ranges::set_intersection(newCandidates, iter->second,
                            inserter(candIntersection, end(candIntersection)));
                        newCandidates = move(candIntersection);
                    }
                    else
                    {
                        newCandidates.clear();
                    }
                }
                for(auto iter = begin(newCandidates); iter != end(newCandidates);)
                {
                    if(unusedTiles.contains(get<0>(candidates.at(*iter))))
                    {
                        ++iter;
                    }
                    else
                    {
                        newCandidates.erase(iter++);
                    }
                }
                backtrack.emplace_back(
                    begin(newCandidates), end(newCandidates));
            }
        }
        if(filled != tiles.size())
        {
            return {};
        }
        numutil::Matrix<tuple<TileID, Operation>> result(
            solution.rows(), solution.columns());
        for(size_t r = 0; r < result.rows(); ++r)
        {
            for(size_t c = 0; c < result.columns(); ++c)
            {
                const auto &cand = candidates[solution.ix(r, c)];
                result.ix(r, c) =
                    make_tuple(get<0>(cand), get<2>(cand));
            }
        }
        return result;
    }
}

#endif
