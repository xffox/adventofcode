#include <iostream>
#include <string>
#include <iterator>
#include <cassert>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    string inp(istreambuf_iterator<char>(cin), {});
    const auto playersRes = parsePlayers(inp);
    assert(playersRes);
    cout<<play<false>(*playersRes)<<endl;
    return 0;
}
