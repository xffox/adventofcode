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
        void addRule(const string &name, const vector<tuple<string, size_t>> &neighbours)
        {
            for(const auto &[dst, count] : neighbours)
            {
                connections[name][dst] = count;
            }
        }

        size_t countContained(const string &src) const
        {
            unordered_set<string> seen;
            seen.insert(src);
            return countContained(seen, src, 1)-1;
        }

    private:
        size_t countContained(unordered_set<string> &seen, const string &src,
            size_t mult) const
        {
            const auto iter = connections.find(src);
            size_t count = mult;
            if(iter != end(connections))
            {
                for(const auto &p : iter->second)
                {
                    if(seen.insert(p.first).second)
                    {
                        count +=
                            countContained(seen, p.first, mult*p.second);
                        seen.erase(p.first);
                    }
                }
            }
            return count;
        }

    private:
        unordered_map<string, unordered_map<string, size_t>> connections{};
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
    cout<<proc.countContained("shiny gold")<<endl;
    return 0;
}
