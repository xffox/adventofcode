#include <iostream>
#include <cstddef>
#include <string>
#include <iterator>
#include <regex>
#include <unordered_map>
#include <unordered_set>
#include <vector>
#include <string_view>
#include <limits>
#include <optional>
#include <utility>
#include <queue>
#include <cassert>

#include "../task.hpp"

using namespace std;

namespace
{
    class Processor
    {
    public:
        void addRule(const string &name,
            const vector<tuple<string, size_t>> &neighbours)
        {
            for(const auto &[dst, count] : neighbours)
            {
                connections[dst].insert(name);
            }
        }

        size_t countContainers(const string &dst) const
        {
            unordered_set<string> seen;
            seen.insert(dst);
            queue<string> front;
            front.push(dst);
            while(!front.empty())
            {
                const auto cur = front.front();
                front.pop();
                auto iter = connections.find(cur);
                if(iter == end(connections))
                {
                    continue;
                }
                for(const auto &nxt : iter->second)
                {
                    if(seen.insert(nxt).second)
                    {
                        front.push(nxt);
                    }
                }
            }
            return seen.size()-1;
        }

    private:
        unordered_map<string, unordered_set<string>> connections{};
    };
}

int main()
{
    Processor proc;
    while(cin)
    {
        string rule;
        getline(cin, rule);
        if(rule.empty())
        {
            continue;
        }
        const auto r = task::parseRule(rule);
        assert(r);
        proc.addRule(get<0>(*r), get<1>(*r));
    }
    cout<<proc.countContainers("shiny gold")<<endl;
    return 0;
}
