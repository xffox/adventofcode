#include <algorithm>
#include <cstddef>
#include <string>
#include <memory>
#include <cassert>
#include <utility>
#include <iterator>
#include <vector>
#include <ranges>
#include <functional>
#include <optional>

#include "../base.hpp"

namespace task
{
    using namespace std;

    using uint = unsigned int;

    namespace inner
    {
        struct Node
        {
            uint val;
            Node *nxt;
        };

        struct Ring
        {
            vector<Node> nodes;
            vector<Node*> values;
        };

        inline auto makeRing(const vector<uint> &values)
        {
            const auto offset = ranges::min(values);
            Ring ring{vector<Node>(values.size()),
                vector<Node*>{values.size()}};
            Node *prev = nullptr;
            Node *fst = nullptr;
            for(size_t i = 0; i < values.size(); ++i)
            {
                const auto val = values[i];
                auto &node = ring.nodes[i];
                node.val = val;
                auto *cur = &node;
                ring.values[val-offset] = cur;
                if(prev != nullptr)
                {
                    prev->nxt = cur;
                }
                else
                {
                    fst = cur;
                }
                prev = cur;
            }
            prev->nxt = fst;
            return ring;
        }
    }

    inline vector<uint> exec(const vector<uint> &cups, size_t steps)
    {
        constexpr size_t TAKE = 3;
        constexpr uint RESULT_START_CUP = 1;
        if(cups.empty())
        {
            return {};
        }
        const auto [offset, maxCup] = ranges::minmax(cups);
        const auto mod = maxCup-offset+1;
        assert(RESULT_START_CUP >= offset &&
            RESULT_START_CUP <= maxCup);
        auto ring = inner::makeRing(cups);
        auto *curNode = ring.values[cups.front()-offset];
        vector<int> seenCands(TAKE, true);
        for(size_t i = 0; i < steps; ++i)
        {
            ranges::fill(seenCands, true);
            const auto curVal = curNode->val;
            auto *takeNode = curNode;
            for(size_t j = 0; j < TAKE; ++j)
            {
                const auto cand =
                    ((curVal-offset) + mod -
                     (takeNode->nxt->val-offset))%mod;
                if(cand >= 1 && cand <= seenCands.size())
                {
                    seenCands[cand-1] = false;
                }
                takeNode = takeNode->nxt;
            }
            const size_t availCand =
                ranges::find(seenCands, true) - begin(seenCands) + 1;
            auto *insertNode = ring.values[offset +
                (curVal-offset + mod - availCand)%mod - offset];
            assert(insertNode != nullptr);
            auto *tmpNode = curNode->nxt;
            curNode->nxt = takeNode->nxt;
            takeNode->nxt = insertNode->nxt;
            insertNode->nxt = tmpNode;
            curNode = curNode->nxt;
        }
        vector<uint> result;
        result.reserve(cups.size());
        const auto *cur = ring.values[RESULT_START_CUP - offset];
        const auto *fst = cur;
        while(true)
        {
            result.push_back(cur->val);
            cur = cur->nxt;
            if(cur == fst)
            {
                break;
            }
        }
        return result;
    }

    inline optional<vector<uint>> parseCups(const string &cups)
    {
        vector<uint> result;
        result.reserve(cups.size());
        for(auto c : cups)
        {
            const auto cupRes = base::parseValue<uint>(
                string(1, c));
            if(!cupRes)
            {
                return {};
            }
            result.push_back(*cupRes);
        }
        return result;
    }
}
