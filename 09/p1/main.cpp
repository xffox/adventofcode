#include <iostream>
#include <cstddef>
#include <vector>
#include <optional>
#include <iterator>
#include <unordered_set>
#include <cassert>

#include "../task.hpp"

using namespace std;

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
            const auto v = stoull(str);
            vs.push_back(v);
        }
    }
    const auto res = task::findInvalid(vs, preamble);
    assert(res);
    cout<<*res<<endl;
    return 0;
}
