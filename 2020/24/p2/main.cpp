#include <iostream>
#include <numeric>
#include <cstddef>
#include <string>
#include <iterator>
#include <cassert>
#include <set>
#include <utility>
#include <map>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    class HexEval
    {
    public:
        template<typename T>
        explicit HexEval(T &&tiles)
            :tiles_(forward<T>(tiles))
        {}

        void step()
        {
            constexpr size_t KEEP_COUNT = 1;
            constexpr size_t ACTIVATE_COUNT = 2;
            map<HexMove, size_t> counts;
            for(const auto &t : tiles_)
            {
                forEachNeighbour(t,
                    [&counts](const auto &p){
                        counts[p] += 1;
                    });
            }
            set<HexMove> nextTiles;
            for(const auto &p : counts)
            {
                if((p.second == KEEP_COUNT && tiles_.contains(p.first)) ||
                    (p.second == ACTIVATE_COUNT))
                {
                    nextTiles.insert(p.first);
                }
            }
            tiles_ = move(nextTiles);
        }

        const set<HexMove> &tiles() const
        {
            return tiles_;
        }

    private:
        set<HexMove> tiles_;
    };
}

int main()
{
    constexpr size_t STEPS = 100;
    string inp(istreambuf_iterator<char>(cin), {});
    const auto instructionListRes = parseInstructionList(inp);
    assert(instructionListRes);
    HexEval eval(prepareTiles(*instructionListRes));
    for(size_t i = 0; i < STEPS; ++i)
    {
        eval.step();
    }
    cout<<eval.tiles().size()<<endl;
    return 0;
}
