#include <iostream>
#include <algorithm>
#include <string>
#include <iterator>
#include <cassert>

#include "../task.hpp"

using namespace std;

int main()
{
    string str;
    copy(istreambuf_iterator<char>(cin), {},
        back_inserter(str));
    const auto programRes = task::parseProgram(str);
    assert(programRes);
    task::Proc proc(*programRes);
    const auto mem = proc.exec();
    task::ulint res = 0;
    for(const auto &p : mem)
    {
        res += get<1>(p);
    }
    cout<<res<<endl;
    return 0;
}
