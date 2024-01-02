#ifndef TASK_H
#define TASK_H

#include <algorithm>
#include <numeric>
#include <vector>
#include <string>
#include <optional>
#include <ranges>
#include <iterator>
#include <string_view>
#include <utility>
#include <unordered_map>
#include <array>
#include <tuple>
#include <cassert>
#include <set>

namespace task
{
    using namespace std;

    struct HexMove
    {
        auto operator<=>(const HexMove&) const = default;

        int c;
        int r;
    };

    constexpr HexMove operator+(const HexMove &left, const HexMove &right)
    {
        return {left.c + right.c, left.r + right.r};
    }

    namespace inner
    {
        using HexMoveMap = unordered_map<string, HexMove>;

        const vector<tuple<string, HexMove>> NEIGHBOUR_MOVES{
            make_tuple("e", HexMove{2, 0}),
            make_tuple("ne", HexMove{1, 1}),
            make_tuple("nw", HexMove{-1, 1}),
            make_tuple("w", HexMove{-2, 0}),
            make_tuple("sw", HexMove{-1, -1}),
            make_tuple("se", HexMove{1, -1}),
        };

        inline vector<HexMoveMap> prepareParseRules(
            const vector<tuple<string, HexMove>> &rules)
        {
            if(rules.empty())
            {
                return {};
            }
            const auto sz = get<0>(ranges::max(rules, {},
                [](const auto &r){
                        return get<0>(r).size();
                    })).size();
            vector<HexMoveMap> result(sz);
            for(const auto &[rule, dir] : rules)
            {
                assert(!rule.empty());
                const auto insertRes =
                    result[rule.size()-1].insert(make_pair(rule, dir)).second;
                assert(insertRes);
            }
            return result;
        }

        inline optional<vector<HexMove>> parseInstructions(
            string_view instructions)
        {
            static const auto RULES = prepareParseRules(NEIGHBOUR_MOVES);
            vector<HexMove> result;
            for(size_t i = 0; i < instructions.size();)
            {
                size_t sz = 0;
                for(size_t j = 0; j < RULES.size(); ++j)
                {
                    const auto &r = RULES[j];
                    auto iter = r.find(string(instructions.substr(i, j+1)));
                    if(iter != end(r))
                    {
                        result.push_back(iter->second);
                        sz = j+1;
                        break;
                    }
                }
                if(sz == 0)
                {
                    return {};
                }
                i += sz;
            }
            return result;
        }
    }

    inline optional<vector<vector<HexMove>>> parseInstructionList(
        const string &instructions)
    {
        vector<vector<HexMove>> result;
        for(auto &&line : instructions | ranges::views::split('\n'))
        {
            string lineStr;
            ranges::copy(line, back_inserter(lineStr));
            auto instructionsRes = inner::parseInstructions(lineStr);
            if(!instructionsRes)
            {
                return {};
            }
            result.push_back(move(*instructionsRes));
        }
        return result;
    }

    inline HexMove sumMoves(const vector<HexMove> &moves)
    {
        return accumulate(begin(moves), end(moves), HexMove{});
    }

    inline set<HexMove> prepareTiles(
        const vector<vector<HexMove>> &instructionList)
    {
        set<HexMove> tiles;
        for(const auto &instructions : instructionList)
        {
            const auto p = sumMoves(instructions);
            if(tiles.erase(p) == 0)
            {
                tiles.insert(p);
            }
        }
        return tiles;
    }

    template<typename F>
    inline void forEachNeighbour(const HexMove &pos, F func)
    {
        constexpr size_t NEIGHBOURS = 6;
        HexMove m{2, 0};
        for(size_t i = 0; i < NEIGHBOURS; ++i)
        {
            func(pos+m);
            m = HexMove{
                (m.c - m.r) - (m.c + m.r)/2,
                (m.c + m.r)/2,
            };
        }
    }
}

#endif
