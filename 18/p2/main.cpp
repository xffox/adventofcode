#include <iostream>
#include <algorithm>
#include <string>
#include <ranges>
#include <utility>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    lint s = 0;
    const PriorityMap priorities{
        make_pair(Operator::PLUS, 2),
            make_pair(Operator::MULT, 1)
    };
    while(cin)
    {
        string str;
        getline(cin, str);
        if(!str.empty())
        {
            auto exprRes = parseExpression(str, priorities);
            assert(*exprRes);
            s += eval(**exprRes);
        }
    }
    cout<<s<<endl;
    return 0;
}
