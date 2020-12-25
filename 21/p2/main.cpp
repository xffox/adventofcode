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
    vector<tuple<string, string>> fullDangerousList;
    ranges::copy(result
        | ranges::views::filter([](const auto &p){
                return static_cast<bool>(p.second);
            })
        | ranges::views::transform([](const auto &p){
                return make_tuple(*p.second, p.first);
            }),
        back_inserter(fullDangerousList));
    ranges::sort(fullDangerousList);
    vector<string> dangerousList;
    ranges::copy(fullDangerousList
        | ranges::views::transform([](const auto &p){
                return get<1>(p);
            }),
        back_inserter(dangerousList));
    for(size_t i = 0; i < dangerousList.size(); ++i)
    {
        cout<<dangerousList[i];
        if(i+1 < dangerousList.size())
        {
            cout<<',';
        }
    }
    cout<<endl;
    return 0;
}
