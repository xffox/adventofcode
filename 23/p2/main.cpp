#include <iostream>
#include <algorithm>
#include <cstddef>
#include <string>
#include <cassert>
#include <vector>
#include <iterator>
#include <ranges>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    using ulint = unsigned long long int;
}

int main()
{
    constexpr size_t STEPS = 10000000;
    constexpr size_t CUPS = 1000000;
    string inp;
    getline(cin, inp);
    const auto cupsRes = parseCups(inp);
    assert(cupsRes);
    const auto &inpCups = *cupsRes;
    const auto maxCup = (!inpCups.empty() ? ranges::max(inpCups) : 0);
    vector<uint> cups;
    cups.reserve(CUPS);
    ranges::copy(inpCups,
        back_inserter(cups));
    ranges::copy(ranges::views::iota(maxCup+1, CUPS+1),
        back_inserter(cups));
    const auto res = exec(cups, STEPS);
    cout<<static_cast<ulint>(res.at(1))*res.at(2)<<endl;
    return 0;
}
