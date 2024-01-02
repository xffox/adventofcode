#include <iostream>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <cassert>
#include <optional>
#include <vector>
#include <array>
#include <cmath>
#include <utility>
#include <unordered_map>
#include <unordered_set>
#include <set>
#include <tuple>
#include <ranges>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    string inp(istreambuf_iterator<char>(cin), {});
    const auto tilesRes = parseTiles(inp);
    assert(tilesRes);
    const auto &tiles = *tilesRes;
    const auto solRes = matchTiles(tiles);
    assert(solRes);
    const auto &sol = *solRes;
    assert(sol.rows() > 0 && sol.columns() > 0);
    ulint res = 1;
    for(size_t i = 0; i < BORDERS; ++i)
    {
        size_t row = i%2*(sol.rows()-1);
        size_t col = i/2*(sol.columns()-1);
        res *= get<0>(sol.ix(row, col));
    }
    cout<<res<<endl;
    return 0;
}
