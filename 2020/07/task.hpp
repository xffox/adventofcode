#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <string>
#include <iterator>
#include <regex>
#include <vector>
#include <string_view>
#include <limits>
#include <optional>
#include <utility>
#include <queue>
#include <cassert>

namespace task
{
    using namespace std;

    template<typename T>
    inline optional<T> parseNumber(string_view s)
    {
        size_t sz = 0;
        const auto res = stoull(string(s), &sz);
        if(sz != s.size() || res > numeric_limits<T>::max())
        {
            return {};
        }
        return res;
    }

    inline optional<tuple<string, vector<tuple<string, size_t>>>> parseRule(
        const string &rule)
    {
        static const regex RULE_REGEX("([a-z]+ [a-z]+) bags contain ((([0-9]+ [a-z]+ [a-z]+) bags?(, )?)+|(no other bags))\\.");
        static const regex SUBRULE_REGEX("[0-9]+ [a-z]+ [a-z]+");
        smatch matches;
        if(!regex_match(rule, matches, RULE_REGEX))
        {
            return {};
        }
        const auto &name = matches.str(1);
        vector<tuple<string, size_t>> connections;
        const auto &neighbours = matches[2].str();
        if(neighbours != "no other bags")
        {
            auto regexIter = sregex_iterator(
                begin(neighbours), end(neighbours), SUBRULE_REGEX);
            const auto endRegexIter = sregex_iterator();
            for(; regexIter != endRegexIter; ++regexIter)
            {
                const auto &neighbour = regexIter->str();
                const auto sepPos = neighbour.find(' ');
                assert(sepPos != string::npos);
                const auto numRes = parseNumber<size_t>(
                    neighbour.substr(0, sepPos));
                assert(numRes);
                connections.emplace_back(
                    neighbour.substr(sepPos+1), *numRes);
            }
        }
        return {make_tuple(name, move(connections))};
    }
}

#endif
