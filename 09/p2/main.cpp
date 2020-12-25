#include <iostream>
#include <algorithm>
#include <cstddef>
#include <vector>
#include <optional>
#include <iterator>
#include <unordered_set>
#include <cassert>

#include "../task.hpp"

using namespace std;

namespace
{
    vector<task::ulint> continuosSum(const vector<task::ulint> &vs,
        task::ulint sum)
    {
        task::ulint cur = 0;
        size_t last = 0;
        for(size_t i = 0; i < vs.size(); ++i)
        {
            const auto v = vs[i];
            cur += v;
            for(; last <= i && cur > sum; ++last)
            {
                cur -= vs[last];
            }
            if(cur == sum && i > last)
            {
                return {begin(vs)+last, begin(vs)+i+1};
            }
        }
        return {};
    }
}

int main()
{
    size_t preamble = 0;
    cin>>preamble;
    cin.ignore();
    vector<task::ulint> vs;
    while(cin)
    {
        string str;
        getline(cin, str);
        if(!str.empty())
        {
            const auto v = stoll(str);
            vs.push_back(v);
        }
    }
    const auto sumRes = task::findInvalid(vs, preamble);
    assert(sumRes);
    auto res = continuosSum(vs, *sumRes);
    assert(res.size() >= 2);
    ranges::sort(res);
    cout<<res.front()+res.back()<<endl;
    return 0;
}
