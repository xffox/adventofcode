#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include <string>
#include <ranges>

#include "../task.hpp"

using namespace std;

int main()
{
    constexpr size_t N = 2020;
    string inp;
    ranges::copy(ranges::istream_view<char>(cin),
        back_inserter(inp));
    const auto vs = task::parseValues(ranges::views::all(inp));
    cout<<task::nthRecitation(vs, N)<<endl;
    return 0;
}
