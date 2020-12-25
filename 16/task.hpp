#ifndef TASK_H
#define TASK_H

#include <algorithm>
#include <string>
#include <optional>
#include <string_view>
#include <regex>
#include <array>
#include <vector>
#include <ranges>
#include <iterator>
#include <utility>

#include "../base.hpp"

namespace task
{
    using namespace std;

    using uint = unsigned int;
    using ulint = unsigned long long int;
    using ValCol = vector<uint>;

    struct Range
    {
        uint start;
        uint len;
    };

    struct Rule
    {
        string name;
        array<Range, 2> ranges;
    };

    struct Notes
    {
        vector<Rule> rules;
        vector<ValCol> tickets;
    };

    namespace
    {
        inline optional<Range> parseRange(
            const string &start,
            const string &end)
        {
            const auto startRes = base::parseValue<uint>(start);
            const auto endRes = base::parseValue<uint>(end);
            if(!startRes || !endRes || *startRes > *endRes)
            {
                return {};
            }
            return Range{*startRes, *endRes-*startRes+1};
        }

        inline optional<Rule> parseRule(const string &rule)
        {
            static regex RULE_REGEX(
                "([a-zA-Z ]+): ([0-9]+)-([0-9]+) or ([0-9]+)-([0-9]+)");
            smatch matches;
            if(!regex_match(rule, matches, RULE_REGEX))
            {
                return {};
            }
            const auto fstRangeRes = parseRange(
                matches.str(2), matches.str(3));
            const auto sndRangeRes = parseRange(
                matches.str(4), matches.str(5));
            if(!fstRangeRes || !sndRangeRes)
            {
                return {};
            }
            return Rule{matches.str(1), {*fstRangeRes, *sndRangeRes}};
        }

        optional<ValCol> parseValues(string_view values)
        {
            ValCol result;
            for(auto &&value : values | ranges::views::split(','))
            {
                string valueStr;
                ranges::copy(value, back_inserter(valueStr));
                const auto valueRes = base::parseValue<uint>(valueStr);
                if(!valueRes)
                {
                    return {};
                }
                result.push_back(*valueRes);
            }
            return result;
        }

        constexpr bool inrange(const Range &range, uint val)
        {
            return val >= range.start && val < range.start+range.len;
        }
    }

    inline optional<Notes> parseNotes(
        const string &notes)
    {
        auto lines = notes | ranges::views::split('\n');
        bool insideRules = true;
        vector<Rule> rules;
        vector<ValCol> tickets;
        for(auto &&line : lines)
        {
            string lineStr;
            ranges::copy(line, back_inserter(lineStr));
            if(lineStr.empty())
            {
                continue;
            }
            if(lineStr == "your ticket:" ||
                lineStr == "nearby tickets:")
            {
                insideRules = false;
                continue;
            }
            if(insideRules)
            {
                const auto ruleRes = parseRule(lineStr);
                if(!ruleRes)
                {
                    return {};
                }
                rules.push_back(*ruleRes);
            }
            else
            {
                const auto ticketRes = parseValues(lineStr);
                if(!ticketRes)
                {
                    return {};
                }
                tickets.push_back(*ticketRes);
            }
        }
        return Notes{move(rules), move(tickets)};
    }

    constexpr bool matchesRule(const Rule &rule, uint val)
    {
        return inrange(rule.ranges[0], val) || inrange(rule.ranges[1], val);
    }
}

#endif
