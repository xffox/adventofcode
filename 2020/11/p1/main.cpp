#include <algorithm>
#include <iostream>
#include <iterator>
#include <string>
#include <cassert>

#include "../task.hpp"

using namespace std;

int main()
{
    constexpr size_t MAX_OCCUPIED = 4;
    string str;
    copy(istreambuf_iterator<char>(cin), {},
        back_inserter(str));
    const auto boardRes = task::parseBoard(str);
    assert(boardRes);
    task::SeatEval eval(*boardRes, MAX_OCCUPIED);
    task::Board prev(0, 0);
    do
    {
        prev = eval.board();
        eval.step();
    }
    while(!task::equalBoards(eval.board(), prev));
    cout<<task::countOccupied(eval.board())<<endl;
    return 0;
}
