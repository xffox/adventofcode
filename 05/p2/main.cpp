#include <iostream>
#include <iterator>
#include <algorithm>
#include <cstddef>
#include <string>
#include <vector>
#include <ranges>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    template<ranges::view V>
    uint findSeat(V tickets)
    {
        vector<uint> vs;
        ranges::copy(tickets, back_inserter(vs));
        ranges::sort(vs);
        for(size_t i = 1; i < vs.size(); ++i)
        {
            const auto exp = vs[i-1]+1;
            if(exp != vs[i])
            {
                return exp;
            }
        }
        return 0;
    }
}

int main()
{
    cout<<findSeat(ranges::istream_view<string>(cin)
        | ranges::views::transform(parseID))<<endl;
    return 0;
}
