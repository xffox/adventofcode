#include <iostream>
#include <cstddef>
#include <algorithm>
#include <vector>
#include <string>
#include <ranges>
#include <iterator>
#include <utility>
#include <unordered_map>

#include "../task.hpp"

using namespace std;

namespace
{
    using ulint = unsigned long long int;

    ulint countArrangements(const vector<task::uint> &vs)
    {
        if(vs.empty())
        {
            return 0;
        }
        vector<task::uint> svs(vs);
        ranges::sort(svs);
        svs.push_back(svs.back()+task::GAP);
        unordered_map<task::uint, ulint> counts{make_pair(0, 1)};
        for(auto v : svs)
        {
            ulint s = 0;
            for(task::uint i = v - min(v, task::GAP); i <= v; ++i)
            {
                s += task::lookupWithDefault(counts, i, 0);
            }
            counts[v] += s;
        }
        return task::lookupWithDefault(counts, svs.back(), 0);
    }
}

int main()
{
    vector<task::uint> vs;
    while(cin)
    {
        string str;
        getline(cin, str);
        if(!str.empty())
        {
            vs.push_back(stoul(str));
        }
    }
    cout<<countArrangements(vs)<<endl;
    return 0;
}
