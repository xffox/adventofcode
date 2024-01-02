#include <iostream>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <ranges>
#include <cassert>
#include <unordered_map>
#include <utility>

#include "../task.hpp"
#include "../../base.hpp"

using namespace std;
using namespace task;


int main()
{
    const unordered_map<size_t, string> REPLACEMENTS{
        make_pair(8, "8: 42 | 42 8"),
        make_pair(11, "11: 42 31 | 42 11 31")
    };
    string inp(istreambuf_iterator<char>(cin), {});
    auto lines = inp | ranges::views::split('\n');
    auto notEmptyLine = [](auto line){
            return ranges::count_if(line, [](auto){return true;}) != 0;
        };
    auto rules = lines | ranges::views::take_while(notEmptyLine);
    string rulesStr;
    ranges::for_each(rules, [&rulesStr, &REPLACEMENTS](auto line){
            string lineStr;
            ranges::copy(line, back_inserter(lineStr));
            const auto sepPos = lineStr.find(':');
            if(sepPos != string::npos)
            {
                const auto idRes = base::parseValue<size_t>(
                    lineStr.substr(0, sepPos));
                if(idRes)
                {
                    auto iter = REPLACEMENTS.find(*idRes);
                    if(iter != end(REPLACEMENTS))
                    {
                        lineStr = iter->second;
                    }
                }
            }
            ranges::copy(lineStr, back_inserter(rulesStr));
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
