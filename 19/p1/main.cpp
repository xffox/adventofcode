#include <iostream>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <cassert>

#include "../task.hpp"

using namespace std;
using namespace task;


int main()
{
    string inp(istreambuf_iterator<char>(cin), {});
    auto lines = inp | ranges::views::split('\n');
    auto notEmptyLine = [](auto line){
            return ranges::count_if(line, [](auto){return true;}) != 0;
        };
    auto rules = lines | ranges::views::take_while(notEmptyLine);
    string rulesStr;
    ranges::for_each(rules, [&rulesStr](auto line){
            ranges::copy(line, back_inserter(rulesStr));
            rulesStr.push_back('\n');
        });
    auto matcherRes = parseRules(rulesStr);
    const auto &matcher = *matcherRes;
    assert(matcherRes);
    const auto cnt = ranges::count_if(lines
        | ranges::views::drop_while(notEmptyLine)
        | ranges::views::drop(1),
        [&matcher](auto &&line){
            string str;
            ranges::copy(line, back_inserter(str));
            return matcher(str);
        });
    cout<<cnt<<endl;
    return 0;
}
