#ifndef GEOMPLANE_HPP
#define GEOMPLANE_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <iterator>
#include <limits>
#include <map>
#include <optional>
#include <ranges>
#include <tuple>

#include "geom.hpp"

namespace geom
{
template<std::ranges::random_access_range R>
requires requires(std::ranges::range_value_t<R> val) {
    {
        [](const auto &val) {
            const auto &[pos, dir] = val;
            return std::tuple{pos, dir};
        }(val)
    } -> std::same_as<std::tuple<geom::Pos, geom::Pos>>;
}
std::size_t calculateInside(const R &loop)
{
    Pos minPos{std::numeric_limits<int>::max(),
               std::numeric_limits<int>::max()};
    Pos maxPos{std::numeric_limits<int>::min(),
               std::numeric_limits<int>::min()};
    std::map<Pos, std::tuple<Pos, Pos>> positions;
    for(std::size_t i = 0; i < loop.size(); ++i)
    {
        const auto &[pos, dir] = loop[i];
        const auto &[prevPos, prevDir] =
            loop[(i + loop.size() - 1) % loop.size()];
        positions.insert_or_assign(pos, std::tuple{prevDir, dir});
        minPos.r = std::min(minPos.r, pos.r);
        minPos.c = std::min(minPos.c, pos.c);
        maxPos.r = std::max(maxPos.r, pos.r);
        maxPos.c = std::max(maxPos.c, pos.c);
    }
    std::size_t sz = 0;
    for(int row = minPos.r; row <= maxPos.r; ++row)
    {
        bool inside = false;
        std::optional<Pos> rotateDir{};
        std::size_t curSz = 0;
        for(int col = minPos.c; col <= maxPos.c; ++col)
        {
            if(const auto iter = positions.find(Pos{row, col});
               iter != end(positions))
            {
                const auto &[prevDir, curDir] = iter->second;
                const auto &vertDir = (prevDir.r != 0 ? prevDir : curDir);
                if(prevDir != curDir)
                {
                    if(rotateDir)
                    {
                        if(*rotateDir == vertDir)
                        {
                            inside = !inside;
                        }
                        rotateDir.reset();
                    }
                    else
                    {
                        rotateDir = vertDir;
                    }
                }
                else if(!rotateDir)
                {
                    inside = !inside;
                }
            }
            else
            {
                curSz += inside;
            }
        }
        sz += curSz;
    }
    return sz;
}

template<std::regular_invocable<geom::Pos> Func>
void forEachCrossNeighbour(const Pos &pos, Func &&func)
{
    constexpr int DIRS = 4;
    for(int i = 0; i < DIRS; ++i)
    {
        const auto diff = (i % 2) * 2 - 1;
        const auto diffRow = diff * (i / 2);
        const auto diffCol = diff * (1 - i / 2);
        const Pos nextPos{
            pos.r + diffRow,
            pos.c + diffCol,
        };
        func(nextPos);
    }
}
}

#endif
