#include <iostream>
#include <unordered_set>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    int multElementsWithSum(const ValCol &vs, int targetSum)
    {
        unordered_set<int> seen;
        for(auto v : vs)
        {
            const auto opposite = targetSum - v;
            if(seen.contains(opposite))
            {
                return v*opposite;
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
