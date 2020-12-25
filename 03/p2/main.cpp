#include <iostream>
#include <algorithm>
#include <string>
#include <tuple>
#include <ranges>
#include <array>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    using luint = unsigned long long int;

    template<ranges::range PosCol>
    auto multTreesOnSlopes(const Board &board, const PosCol &slopes)
    {
        luint result = 1;
        ranges::for_each(
            slopes | ranges::views::transform([&board](const auto &slope){
                    return countTrees(board, slope);
                }),
            [&result](auto v){
                result *= v;
            });
        return result;
    }
}

int main()
{
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
    cout<<multTreesOnSlopes(board,
        to_array<Pos>({
                {1, 1},
                {1, 3},
                {1, 5},
                {1, 7},
                {2, 1}
            }))<<endl;
    return 0;
}
