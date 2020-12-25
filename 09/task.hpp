#include <cstddef>
#include <vector>
#include <optional>
#include <iterator>
#include <unordered_set>

namespace task
{
    using ulint = unsigned long long int;

    using namespace std;

    optional<ulint> findInvalid(const vector<ulint> &vs, size_t preamble)
    {
        if(vs.size() < preamble)
        {
            return {};
        }
        unordered_multiset<ulint> seen(begin(vs), begin(vs)+preamble);
        for(size_t i = preamble; i < vs.size(); ++i)
        {
            const ulint tgt = vs[i];
            bool found = false;
            for(auto v : seen)
            {
                if(tgt < v)
                {
                    continue;
                }
                const auto other = tgt - v;
                auto range = seen.equal_range(other);
                if(other == v)
                {
                    ++range.first;
                }
                if(range.first != range.second)
                {
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                return tgt;
            }
            seen.erase(seen.find(vs[i-preamble]));
            seen.insert(tgt);
        }
        return {};
    }
}
