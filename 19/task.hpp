#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <algorithm>
#include <string>
#include <string_view>
#include <memory>
#include <unordered_map>
#include <vector>
#include <utility>
#include <optional>
#include <ranges>
#include <tuple>
#include <algorithm>
#include <iterator>
#include <cctype>
#include <stdexcept>
#include <cassert>
#include <type_traits>

#include "../base.hpp"

namespace task
{
    using namespace std;

    namespace inner
    {
        class Rule;

        using RuleID = size_t;
        using RuleMap = unordered_map<RuleID, unique_ptr<Rule>>;
        using PosMap = unordered_map<const void*, size_t>;

        class Rule
        {
        public:
            virtual ~Rule() = default;

            virtual vector<size_t> apply(
                PosMap &prevs,
                size_t pos, string_view line,
                const RuleMap &rules) const = 0;

        };

        vector<size_t> scopePrevPos(PosMap &prevs, size_t pos,
            string_view line, const RuleMap &rules,
            const Rule &rule)
        {
            auto iter = prevs.find(&rule);
            const auto prev =
                (iter != end(prevs)
                 ? make_optional(iter->second) : nullopt);
            if(!prev || *prev != pos)
            {
                prevs[&rule] = pos;
                const auto res = rule.apply(prevs, pos, line, rules);
                if(!prev)
                {
                    prevs.erase(&rule);
                }
                else
                {
                    prevs[&rule] = *prev;
                }
                return res;
            }
            return {};
        }

        class CharRule: public Rule
        {
        public:
            explicit CharRule(char val)
                :val(val)
            {}

            vector<size_t> apply(
                PosMap&,
                size_t pos, string_view line,
                const RuleMap&) const override
            {
                if(pos < line.size() && line[pos] == val)
                {
                    return {pos+1};
                }
                return {};
            }

        private:
            char val;
        };

        class AndRule: public Rule
        {
        public:
            template<typename R>
            explicit AndRule(R &&ids)
                :ids(forward<R>(ids))
            {}

            vector<size_t> apply(
                PosMap &prevs,
                size_t pos, string_view line,
                const RuleMap &rules) const override
            {
                return apply(prevs, pos, line, rules, 0);
            }

        private:
            vector<size_t> apply(PosMap &prevs,
                size_t pos, string_view line,
                const RuleMap &rules,
                size_t idx) const
            {
                if(idx == ids.size())
                {
                    return {pos};
                }
                const auto id = ids[idx];
                auto iter = rules.find(id);
                if(iter == end(rules))
                {
                    throw runtime_error("invalid rule ID");
                }
                const auto &rule = iter->second;
                assert(rule);
                const auto posRes =
                    scopePrevPos(prevs, pos, line, rules, *rule);
                vector<size_t> result;
                for(auto nextPos : posRes)
                {
                    const auto nextRes =
                        apply(prevs, nextPos, line, rules, idx+1);
                    result.insert(end(result), begin(nextRes), end(nextRes));
                }
                return result;
            }

        private:
            vector<RuleID> ids;
        };

        class OrRule: public Rule
        {
        public:
            OrRule(unique_ptr<Rule> left, unique_ptr<Rule> right)
                :left(move(left)), right(move(right))
            {}

            vector<size_t> apply(
                PosMap &prevs,
                size_t pos, string_view line,
                const RuleMap &rules) const override
            {
                vector<size_t> result;
                assert(left);
                const auto leftRes =
                    scopePrevPos(prevs, pos, line, rules, *left);
                result.insert(end(result), begin(leftRes), end(leftRes));
                assert(right);
                const auto rightRes =
                    scopePrevPos(prevs, pos, line, rules, *right);
                result.insert(end(result), begin(rightRes), end(rightRes));
                return result;
            }

        private:
            unique_ptr<Rule> left;
            unique_ptr<Rule> right;
        };

        void parseSpaces(size_t &pos, string_view line)
        {
            while(pos < line.size() && isspace(line[pos]))
            {
                ++pos;
            }
        }

        optional<size_t> parseValue(size_t &pos, string_view line)
        {
            const auto valueStart = pos;
            for(; pos < line.size() && isdigit(line[pos]); ++pos)
            {}
            return base::parseValue<size_t>(
                line.substr(valueStart, pos - valueStart));
        }

        bool parseSymb(size_t &pos, string_view line, char c)
        {
            const auto r = (pos < line.size() && line[pos] == c);
            if(r)
            {
                ++pos;
            }
            return r;
        }

        optional<char> parseChar(size_t &pos, string_view line)
        {
            if(pos >= line.size() || line[pos] != '"')
            {
                return {};
            }
            ++pos;
            if(pos >= line.size())
            {
                return {};
            }
            const auto v = line[pos];
            ++pos;
            if(pos >= line.size() || line[pos] != '"')
            {
                return {};
            }
            ++pos;
            return v;
        }

        optional<unique_ptr<Rule>> parseBody(size_t &pos, string_view line)
        {
            parseSpaces(pos, line);
            auto charRes = parseChar(pos, line);
            if(charRes)
            {
                return make_unique<CharRule>(*charRes);
            }
            vector<RuleID> ids;
            while(true)
            {
                parseSpaces(pos, line);
                const auto idRes = parseValue(pos, line);
                if(!idRes)
                {
                    break;
                }
                ids.push_back(*idRes);
            }
            unique_ptr<Rule> left = make_unique<AndRule>(move(ids));
            if(pos < line.size() && line[pos] == '|')
            {
                ++pos;
                auto rightRes = parseBody(pos, line);
                if(!rightRes)
                {
                    return {};
                }
                left = make_unique<OrRule>(move(left), move(*rightRes));
            }
            return left;
        }

        optional<tuple<RuleID, unique_ptr<Rule>>> parseRule(
            size_t &pos, string_view line)
        {
            parseSpaces(pos, line);
            const auto idRes = parseValue(pos, line);
            if(!idRes)
            {
                return {};
            }
            if(!parseSymb(pos, line, ':'))
            {
                return {};
            }
            parseSpaces(pos, line);
            auto bodyRes = parseBody(pos, line);
            if(!bodyRes)
            {
                return {};
            }
            return make_tuple(*idRes, move(*bodyRes));
        }

        optional<tuple<RuleID, unique_ptr<Rule>>> parseRule(
            string_view line)
        {
            size_t pos = 0;
            return parseRule(pos, line);
        }
    }

    class Matcher
    {
    public:
        template<typename R>
            requires is_same_v<decay_t<R>, inner::RuleMap>
        explicit Matcher(R &&rules)
            :rules(forward<R>(rules))
        {}

        bool operator()(const string &str) const
        {
            constexpr size_t ROOT_ID = 0;
            auto iter = rules.find(ROOT_ID);
            if(iter == end(rules))
            {
                throw runtime_error("invalid rule ID");
            }
            inner::PosMap prevs;
            const auto &rule = iter->second;
            assert(rule);
            const auto res = scopePrevPos(
                prevs, 0, str, rules, *rule);
            return ranges::any_of(res,
                [&str](auto v){return v == str.size();});
        }

    private:
        inner::RuleMap rules;
    };

    inline optional<Matcher> parseRules(string_view rules)
    {
        inner::RuleMap result;
        for(auto &&line : rules | ranges::views::split('\n'))
        {
            string lineStr;
            ranges::copy(line, back_inserter(lineStr));
            auto ruleRes = inner::parseRule(lineStr);
            if(!ruleRes)
            {
                return {};
            }
            result.insert(
                make_pair(get<0>(*ruleRes), move(get<1>(*ruleRes))));
        }
        return Matcher(move(result));
    }
}

#endif
