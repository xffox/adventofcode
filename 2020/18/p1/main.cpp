#include <iostream>
#include <algorithm>
#include <string>
#include <ranges>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    lint s = 0;
    while(cin)
    {
        string str;
        getline(cin, str);
        if(!str.empty())
        {
            auto exprRes = parseExpression(str);
            assert(*exprRes);
            s += eval(**exprRes);
        }
    }
    cout<<s<<endl;
    return 0;
}
