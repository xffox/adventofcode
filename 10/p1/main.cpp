#include <iostream>
#include <cstddef>
#include <algorithm>
#include <vector>
#include <string>
#include <ranges>
#include <iterator>
#include <unordered_map>

#include "../task.hpp"

using namespace std;

namespace
{
    task::uint multDiffs(const vector<task::uint> &vs)
    {
        vector<task::uint> svs(vs);
        ranges::sort(svs);
        svs.push_back(svs.back()+task::GAP);
        unordered_map<task::uint, size_t> counts;
        task::uint prev = 0;
        for(auto v : svs)
        {
            const auto diff = v - prev;
            counts[diff] += 1;
            prev = v;
        }
        return task::lookupWithDefault(
            counts, 1, 0)*task::lookupWithDefault(counts, 3, 0);
    }
}

int main()
{
    vector<uint> vs;
    while(cin)
    {
        string str;
        getline(cin, str);
        if(!str.empty())
        {
            vs.push_back(stoul(str));
        }
    }
    cout<<multDiffs(vs)<<endl;
    return 0;
}
