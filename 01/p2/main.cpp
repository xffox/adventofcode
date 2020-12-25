#include <iostream>
#include <iterator>
#include <tuple>
#include <unordered_set>
#include <unordered_map>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    using lint = long long int;

    lint multElementsWithSum(const ValCol &vs, int targetSum)
    {
        unordered_set<int> seen;
        unordered_map<int, int> sumMults;
        for(auto v : vs)
        {
            const auto opposite = targetSum - v;
            auto iter = sumMults.find(opposite);
            if(iter != end(sumMults))
            {
                return static_cast<lint>(v)*iter->second;
            }
            for(auto prev : seen)
            {
                sumMults[prev+v] = prev*v;
            }
            seen.insert(v);
        }
        return 0;
    }
}

int main()
{
    ValCol vs;
    while(cin)
    {
        int v = 0;
        cin>>v;
        vs.push_back(v);
    }
    constexpr int TARGET_SUM = 2020;
    cout<<multElementsWithSum(vs, TARGET_SUM)<<endl;
    return 0;
}
