#include <iostream>
#include <algorithm>
#include <string>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    constexpr Pos SLOPE{
        1,
        3,
    };
    vector<string> lines;
    size_t cols = 0;
    while(cin)
    {
        lines.emplace_back();
        getline(cin, lines.back());
        cols = max(cols, lines.back().size());
    }
    Board board(lines.size(), cols);
    for(size_t i = 0; i < lines.size(); ++i)
    {
        for(size_t j = 0; j < lines[i].size(); ++j)
        {
            Cell cell = Cell::EMPTY;
            if(lines[i][j] == '#')
            {
                cell = Cell::TREE;
            }
            board.ix(i, j) = cell;
        }
    }
    cout<<countTrees(board, SLOPE)<<endl;
    return 0;
}
