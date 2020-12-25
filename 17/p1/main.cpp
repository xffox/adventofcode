#include <iostream>
#include <cstddef>
#include <iterator>
#include <cassert>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    constexpr size_t CYCLES = 6;
    constexpr size_t N = 3;
    using Eval = CubeEval<N>;
    string inp(istreambuf_iterator<char>(cin), {});
    const auto boardRes = parseBoard<Eval::N>(inp);
    assert(boardRes);
    const auto &board = *boardRes;
    CubeEval eval(board);
    for(size_t i = 0; i < CYCLES; ++i)
    {
        eval.step();
    }
    cout<<eval.board().size()<<endl;
    return 0;
}
