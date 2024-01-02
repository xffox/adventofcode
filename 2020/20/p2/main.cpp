#include <iostream>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <cassert>
#include <optional>
#include <vector>
#include <array>
#include <cmath>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>
#include <ranges>
#include <complex>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    numutil::Matrix<char> makePattern()
    {
        const vector<string> PAT = {
            "                  # ",
            "#    ##    ##    ###",
            " #  #  #  #  #  #   "
        };
        assert(!PAT.empty() && !PAT.front().empty());
        numutil::Matrix<char> result(PAT.size(), PAT.front().size());
        for(size_t i = 0; i < PAT.size(); ++i)
        {
            const auto cols = PAT[i].size();
            assert(cols == result.columns());
            for(size_t j = 0; j < PAT[i].size(); ++j)
            {
                result.ix(i, j) = PAT[i][j];
            }
        }
        return result;
    }

    const numutil::Matrix<char> PATTERN = makePattern();

    Pos tileSize(const TileMap &tiles)
    {
        if(tiles.empty())
        {
            return {0, 0};
        }
        const auto &fst = *begin(tiles);
        Pos sz{fst.second.rows(), fst.second.columns()};
        assert(sz.row >= 2 && sz.col >= 2);
        sz.row -= 2;
        sz.col -= 2;
        return sz;
    }

    numutil::Matrix<char> applyOperation(const numutil::Matrix<char> &tile,
        const Operation &operation)
    {
        using CPos = complex<double>;
        auto resultRows = tile.rows();
        auto resultCols = tile.columns();
        const auto dimChange = (operation.rotate%2 != 0);
        if(dimChange)
        {
            const auto t = resultCols;
            resultCols = resultRows;
            resultRows = t;
        }
        numutil::Matrix<char> result(resultRows, resultCols);
        const CPos middle(
            static_cast<double>(tile.columns()-1)/2,
            static_cast<double>(tile.rows()-1)/2
            );
        const CPos rotateMult(0, -1);
        for(size_t r = 0; r < tile.rows(); ++r)
        {
            for(size_t c = 0; c < tile.columns(); ++c)
            {
                CPos p(c, r);
                auto dst = (p-middle);
                const auto m = pow(rotateMult, operation.rotate);
                dst *= m;
                if(operation.flip)
                {
                    dst = CPos(dst.real()*(-1), dst.imag());
                }
                auto actualMiddle = middle;
                if(dimChange)
                {
                    actualMiddle = CPos(middle.imag(), middle.real());
                }
                dst += actualMiddle;
                const Pos tgtPos{
                    static_cast<size_t>(round(dst.imag())),
                    static_cast<size_t>(round(dst.real()))
                };
                result.ix(tgtPos.row, tgtPos.col) = tile.ix(r, c);
            }
        }
        return result;
    }

    numutil::Matrix<char> prepareTile(const numutil::Matrix<char> &tile,
        const Operation &operation)
    {
        assert(tile.rows() >= 2 && tile.columns() >= 2);
        numutil::Matrix<char> result(tile.rows()-2, tile.columns()-2);
        for(size_t r = 1; r+1 < tile.rows(); ++r)
        {
            for(size_t c = 1; c+1 < tile.columns(); ++c)
            {
                result.ix(r-1, c-1) = tile.ix(r, c);
            }
        }
        return applyOperation(result, operation);
    }

    size_t countSafe(const TileMap &tiles,
        const numutil::Matrix<tuple<TileID, Operation>> &operations)
    {
        if(tiles.empty())
        {
            return 0;
        }
        const auto sz = tileSize(tiles);
        numutil::Matrix<char> image(
            sz.row*operations.rows(),
            sz.col*operations.columns());
        for(size_t r = 0; r < operations.rows(); ++r)
        {
            for(size_t c = 0; c < operations.columns(); ++c)
            {
                const auto &[tileID, op] = operations.ix(r, c);
                const auto &before = tiles.at(tileID);
                const auto tile = prepareTile(before, op);
                for(size_t i = 0; i < tile.rows(); ++i)
                {
                    for(size_t j = 0; j < tile.columns(); ++j)
                    {
                        image.ix(r*sz.row + i, c*sz.col + j) =
                            tile.ix(i, j);
                    }
                }
            }
        }
        vector<numutil::Matrix<char>> patterns;
        for(size_t i = 0; i < BORDERS; ++i)
        {
            patterns.push_back(applyOperation(PATTERN,
                    Operation{i, false}));
            patterns.push_back(applyOperation(PATTERN,
                    Operation{i, true}));
        }
        size_t fulls = 0;
        set<tuple<size_t, size_t>> monsters;
        for(size_t r = 0; r < image.rows(); ++r)
        {
            for(size_t c = 0; c < image.columns(); ++c)
            {
                if(image.ix(r, c) == '#')
                {
                    ++fulls;
                }
                for(const auto &pat : patterns)
                {
                    if(r+pat.rows() <= image.rows() &&
                        c+pat.columns() <= image.columns())
                    {
                        set<tuple<size_t, size_t>> curMonsters;
                        bool matches = true;
                        for(size_t i = 0; matches && i < pat.rows(); ++i)
                        {
                            for(size_t j = 0; matches && j < pat.columns(); ++j)
                            {
                                const auto curr = r+i;
                                const auto curc = c+j;
                                if(pat.ix(i, j) == '#')
                                {
                                    if(image.ix(curr, curc) != '#')
                                    {
                                        matches = false;
                                        break;
                                    }
                                    curMonsters.insert(
                                        make_tuple(curr, curc));
                                }
                            }
                        }
                        if(matches)
                        {
                            ranges::copy(curMonsters,
                                inserter(monsters, end(monsters)));
                        }
                    }
                }
            }
        }
        return fulls - monsters.size();
    }
}

int main()
{
    string inp(istreambuf_iterator<char>(cin), {});
    const auto tilesRes = parseTiles(inp);
    assert(tilesRes);
    const auto &tiles = *tilesRes;
    const auto solRes = matchTiles(tiles);
    assert(solRes);
    const auto &sol = *solRes;
    assert(sol.rows() > 0 && sol.columns() > 0);
    cout<<countSafe(tiles, sol)<<endl;
    return 0;
}
