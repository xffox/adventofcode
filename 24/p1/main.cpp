#include <iostream>
#include <numeric>
#include <cstddef>
#include <string>
#include <iterator>
#include <cassert>
#include <set>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    size_t countUpTiles(const vector<vector<HexMove>> &instructionList)
    {
        return prepareTiles(instructionList).size();
    }
}

int main()
{
    string inp(istreambuf_iterator<char>(cin), {});
    const auto instructionListRes = parseInstructionList(inp);
    assert(instructionListRes);
    cout<<countUpTiles(*instructionListRes)<<endl;
    return 0;
}
