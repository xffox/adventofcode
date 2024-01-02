#include <iostream>
#include <cstddef>
#include <string>
#include <ranges>
#include <cassert>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    constexpr size_t STEPS = 100;
    string inp;
    getline(cin, inp);
    const auto cupsRes = parseCups(inp);
    assert(cupsRes);
    const auto res = exec(*cupsRes, STEPS);
    for(auto v : res | ranges::views::drop(1))
    {
        cout<<v;
    }
    cout<<endl;
    return 0;
}
