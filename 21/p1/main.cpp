#include <iostream>
#include <algorithm>
#include <vector>
#include <cstddef>
#include <string>
#include <iterator>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <tuple>
#include <utility>
#include <ranges>
#include <cassert>
#include <stdexcept>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    string inp(istreambuf_iterator<char>(cin), {});
    const auto foodListRes = parseFoodList(inp);
    assert(foodListRes);
    const auto &foodList = *foodListRes;
    const auto result = findIngridients(foodList);
    unordered_set<string> safeIngridients;
    for(const auto &p : result)
    {
        if(!p.second)
        {
            safeIngridients.insert(p.first);
        }
    }
    size_t cnt = 0;
    for(const auto &food : foodList)
    {
        for(const auto &ingridient : get<0>(food))
        {
            if(safeIngridients.contains(ingridient))
            {
                ++cnt;
            }
        }
    }
    cout<<cnt<<endl;
    return 0;
}
