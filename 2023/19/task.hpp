#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cassert>
#include <cctype>
#include <cstddef>
#include <expected>
#include <format>
#include <istream>
#include <iterator>
#include <ranges>
#include <string>
#include <string_view>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "../base.hpp"
#include "../parse_stream.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;

namespace rule
{
    using WorkflowName = std::string;
    using RatingName = std::string;

    enum class Action
    {
        REJECT,
        ACCEPT,
    };

    struct DestinationWorkflow
    {
        WorkflowName name{};
    };
    struct DestinationAction
    {
        Action action{};
    };
    using Destination = std::variant<DestinationWorkflow, DestinationAction>;

    enum Operation
    {
        LESS,
        GREATER,
    };

    struct Condition
    {
        Operation operation{};
        RatingName name{};
        uval value{};
    };

    struct Rule
    {
        Condition condition;
        Destination destination;
    };

    struct Workflow
    {
        std::vector<Rule> rules;
        Destination defaultDestination;
    };

    using Workflows = std::unordered_map<WorkflowName, Workflow>;
    using PartRatings = std::unordered_map<RatingName, uval>;
}

namespace input
{
    struct Input
    {
        rule::Workflows workflows;
        std::vector<rule::PartRatings> partRatings;
    };

    namespace detail
    {
        template<typename T>
        using ParseResult = std::expected<T, std::string>;

        inline ParseResult<uval> parseValue(parser::ParseStream &stream)
        {
            std::string valStr;
            while(const auto maybeVal = stream.peek())
            {
                if(!std::isdigit(*maybeVal))
                {
                    break;
                }
                valStr.push_back(*maybeVal);
                stream.read();
            }
            const auto maybeValue = base::parseValue<uval>(valStr);
            if(!maybeValue)
            {
                return std::unexpected(
                    std::format("value not parsed: {}", valStr));
            }
            return *maybeValue;
        }

        inline ParseResult<std::string> parseName(parser::ParseStream &stream)
        {
            std::string result;
            while(const auto maybeVal = stream.peek())
            {
                if(!std::isalpha(*maybeVal))
                {
                    break;
                }
                result.push_back(*maybeVal);
                stream.read();
            }
            if(result.empty())
            {
                return std::unexpected("empty name");
            }
            return result;
        }

        inline ParseResult<rule::Operation> parseOperation(
            parser::ParseStream &stream)
        {
            if(const auto maybeOp = stream.read())
            {
                switch(*maybeOp)
                {
                case '<':
                    return rule::Operation::LESS;
                case '>':
                    return rule::Operation::GREATER;
                default:
                    return std::unexpected(
                        std::format("invalid operation: {}", *maybeOp));
                }
            }
            return std::unexpected("condition operation missing");
        }

        inline ParseResult<rule::Condition> parseCondition(
            parser::ParseStream &stream)
        {
            auto maybeName = parseName(stream);
            if(!maybeName)
            {
                return std::unexpected(std::format(
                    "condition name parse failed: {}", maybeName.error()));
            }
            const auto maybeOperation = parseOperation(stream);
            if(!maybeOperation)
            {
                return std::unexpected(std::format("operation parse failed: {}",
                                                   maybeOperation.error()));
            }
            const auto maybeValue = parseValue(stream);
            if(!maybeValue)
            {
                return std::unexpected(
                    std::format("value parse failed: {}", maybeValue.error()));
            }
            return rule::Condition{
                .operation = *maybeOperation,
                .name = std::move(*maybeName),
                .value = *maybeValue,
            };
        }

        inline ParseResult<rule::Destination> parseDestination(
            parser::ParseStream &stream)
        {
            auto maybeName = parseName(stream);
            if(!maybeName)
            {
                return std::unexpected(
                    std::format("name parsing failed: {}", maybeName.error()));
            }
            if(*maybeName == "R")
            {
                return rule::DestinationAction{rule::Action::REJECT};
            }
            if(*maybeName == "A")
            {
                return rule::DestinationAction{rule::Action::ACCEPT};
            }
            return rule::DestinationWorkflow{std::move(*maybeName)};
        }

        inline ParseResult<rule::Rule> parseRule(parser::ParseStream &stream)
        {
            auto maybeCondition = parseCondition(stream);
            if(!maybeCondition)
            {
                return std::unexpected(std::format("condition parse failed: {}",
                                                   maybeCondition.error()));
            }
            if(const auto maybeSep = stream.read();
               !maybeSep || *maybeSep != ':')
            {
                return std::unexpected("invalid condition separator");
            }
            auto maybeDestination = parseDestination(stream);
            if(!maybeDestination)
            {
                return std::unexpected(std::format(
                    "destination parse failed: {}", maybeDestination.error()));
            }
            return rule::Rule{
                .condition = std::move(*maybeCondition),
                .destination = std::move(*maybeDestination),
            };
        }

        inline ParseResult<rule::Workflow> parseWorkflow(
            parser::ParseStream &stream)
        {
            rule::Workflow workflow{};
            while(true)
            {
                auto ruleStream = stream;
                if(auto maybeRule = parseRule(ruleStream))
                {
                    workflow.rules.push_back(std::move(*maybeRule));
                    stream = ruleStream;
                    if(const auto maybeSep = stream.read();
                       !maybeSep || *maybeSep != ',')
                    {
                        return std::unexpected("invalid rule separator");
                    }
                }
                else
                {
                    auto maybeDestination = parseDestination(stream);
                    if(!maybeDestination)
                    {
                        return std::unexpected(
                            std::format("workflow parsing failed: {}",
                                        maybeDestination.error()));
                    }
                    workflow.defaultDestination = std::move(*maybeDestination);
                    break;
                }
            }
            return workflow;
        }

        inline ParseResult<std::tuple<rule::WorkflowName, rule::Workflow>>
        parseNamedWorkflow(parser::ParseStream &stream)
        {
            auto maybeName = parseName(stream);
            if(!maybeName)
            {
                return std::unexpected(std::format(
                    "workflow name parsing failed: {}", maybeName.error()));
            }
            if(const auto maybeWorkflowStart = stream.read();
               !maybeWorkflowStart || *maybeWorkflowStart != '{')
            {
                return std::unexpected(std::format(
                    "no workflow start character: {}",
                    maybeWorkflowStart ? std::string(1, *maybeWorkflowStart)
                                       : "<null>"));
            }
            const auto maybeWorkflow = parseWorkflow(stream);
            if(!maybeWorkflow)
            {
                return std::unexpected(std::format(
                    "workflow parsing failed: {}", maybeWorkflow.error()));
            }
            if(const auto maybeWorkflowEnd = stream.read();
               !maybeWorkflowEnd || *maybeWorkflowEnd != '}')
            {
                return std::unexpected("no workflow end character");
            }
            if(const auto maybeEos = stream.peek(); maybeEos)
            {
                return std::unexpected("unexpected workflow end");
            }
            return std::tuple{std::move(*maybeName), std::move(*maybeWorkflow)};
        }

        inline ParseResult<std::tuple<rule::RatingName, uval>> parseRating(
            parser::ParseStream &stream)
        {
            auto maybeName = parseName(stream);
            if(!maybeName)
            {
                return std::unexpected(std::format(
                    "rating name parse failed: {}", maybeName.error()));
            }
            if(const auto maybeSep = stream.read();
               !maybeSep || *maybeSep != '=')
            {
                return std::unexpected("invalid rating separator");
            }
            const auto maybeValue = parseValue(stream);
            if(!maybeValue)
            {
                return std::unexpected(std::format("invalid rating value: {}",
                                                   maybeValue.error()));
            }
            return std::tuple{std::move(*maybeName), *maybeValue};
        }

        inline ParseResult<rule::PartRatings> parsePartRating(
            parser::ParseStream &stream)
        {
            if(const auto maybeBegin = stream.read();
               !maybeBegin || *maybeBegin != '{')
            {
                return std::unexpected("invalid ratings begin");
            }
            rule::PartRatings ratings;
            bool first = true;
            while(true)
            {
                if(!first)
                {
                    if(const auto maybeSep = stream.peek();
                       !maybeSep || *maybeSep != ',')
                    {
                        break;
                    }
                    else
                    {
                        stream.read();
                    }
                }
                if(auto ratingStream = stream;
                   auto maybeRating = parseRating(ratingStream))
                {
                    stream = ratingStream;
                    auto &[name, value] = *maybeRating;
                    ratings.emplace(std::move(name), value);
                }
                else
                {
                    if(first)
                    {
                        break;
                    }
                    stream = ratingStream;
                    return std::unexpected(std::format(
                        "rating parsing failed: {}", maybeRating.error()));
                }
                first = false;
            }
            if(const auto maybeEnd = stream.read();
               !maybeEnd || *maybeEnd != '}')
            {
                return std::unexpected("invalid ratings end");
            }
            if(const auto maybeEos = stream.peek(); maybeEos)
            {
                return std::unexpected("ratings extra characters");
            }
            return ratings;
        }

        inline std::expected<std::tuple<rule::WorkflowName, rule::Workflow>,
                             std::string>
        parseWorkflow(std::string_view str)
        {
            parser::ParseStream stream(str);
            return parseNamedWorkflow(stream);
        }

        inline std::expected<rule::PartRatings, std::string> parsePartRatings(
            std::string_view str)
        {
            parser::ParseStream stream(str);
            return parsePartRating(stream);
        }
    }

    inline std::expected<Input, std::string> parseInput(std::istream &stream)
    {
        rule::Workflows workflows;
        while(stream)
        {
            std::string line;
            std::getline(stream, line);
            if(line.empty())
            {
                break;
            }
            auto maybeWorkflow = detail::parseWorkflow(line);
            if(!maybeWorkflow)
            {
                return std::unexpected(
                    std::format("invalid workflow: {}", maybeWorkflow.error()));
            }
            workflows.emplace(std::move(std::get<0>(*maybeWorkflow)),
                              std::move(std::get<1>(*maybeWorkflow)));
        }
        std::vector<rule::PartRatings> partRatings;
        while(stream)
        {
            std::string line;
            std::getline(stream, line);
            if(line.empty())
            {
                continue;
            }
            auto maybePartRatings = detail::parsePartRatings(line);
            if(!maybePartRatings)
            {
                return std::unexpected(std::format("invalid part ratings: {}",
                                                   maybePartRatings.error()));
            }
            partRatings.push_back(std::move(*maybePartRatings));
        }
        return Input{std::move(workflows), std::move(partRatings)};
    }
}

class Range
{
public:
    struct JoinResult
    {
        std::vector<Range> left;
        std::vector<Range> right;
        std::vector<Range> common;
    };

    constexpr Range(uval begin, uval end) : begin(begin), end(end)
    {
    }

    constexpr uval b() const
    {
        return begin;
    }
    constexpr uval e() const
    {
        return end;
    }

    friend auto operator<=>(const Range &, const Range &) = default;

    constexpr friend JoinResult join(const Range &left, const Range &right)
    {
        if(left > right)
        {
            auto res = join(right, left);
            return {
                .left = std::move(res.right),
                .right = std::move(res.left),
                .common = std::move(res.common),
            };
        }
        if(left.end < right.begin)
        {
            return {
                .left = {left},
                .right = {right},
                .common = {},
            };
        }
        if(left == right)
        {
            return {
                .left = {},
                .right = {},
                .common = {left},
            };
        }
        if(left.begin == right.begin)
        {
            return {
                .left = {},
                .right = {Range(left.end + 1, right.end)},
                .common = {Range(left.begin, left.end)},
            };
        }
        if(left.end == right.end)
        {
            return {
                .left = {Range(left.begin, right.begin - 1)},
                .right = {},
                .common = {Range(right.begin, right.end)},
            };
        }
        if(left.end > right.end)
        {
            return {
                .left = {Range(left.begin, right.begin - 1),
                         Range(right.end + 1, left.end)},
                .right = {},
                .common = {Range(right.begin, right.end)},
            };
        }
        return {
            .left = {Range(left.begin, right.begin - 1)},
            .right = {Range(left.end + 1, right.end)},
            .common = {Range(right.begin, left.end)},
        };
    }

    friend constexpr std::vector<Range> common(const Range &left,
                                               const Range &right)
    {
        if(left > right)
        {
            return common(right, left);
        }
        if(left.end < right.begin)
        {
            return {left, right};
        }
        if(left.end >= right.end)
        {
            return {left};
        }
        return {Range(left.begin, right.end)};
    }

    friend constexpr std::optional<Range> intersect(const Range &left,
                                                    const Range &right)
    {
        if(left > right)
        {
            return intersect(right, left);
        }
        if(left.end < right.begin)
        {
            return std::nullopt;
        }
        return Range(right.begin, std::min(left.end, right.end));
    }

private:
    uval begin;
    uval end;
};
using RangeDict = std::unordered_map<std::string, Range>;

struct SolveParam
{
    std::vector<rule::RatingName> names;
    Range fullRange;
};

namespace detail
{
    inline std::vector<RangeDict> solveWorkflow(
        const rule::WorkflowName &name, const rule::Workflows &workflows,
        const SolveParam &params);

    inline std::vector<RangeDict> solveDestination(
        const rule::Destination &dst, const rule::Workflows &workflows,
        const SolveParam &param)
    {
        if(const auto *const actionDst =
               std::get_if<rule::DestinationAction>(&dst))
        {
            switch(actionDst->action)
            {
            case rule::Action::REJECT:
                return {};
            case rule::Action::ACCEPT: {
                auto resView =
                    param.names | std::views::transform([&](const auto &name) {
                        return std::pair{name, param.fullRange};
                    });
                return {RangeDict{std::ranges::begin(resView),
                                  std::ranges::end(resView)}};
            }
            }
            std::unreachable();
        }
        if(const auto *const workflowDst =
               std::get_if<rule::DestinationWorkflow>(&dst))
        {
            return solveWorkflow(workflowDst->name, workflows, param);
        }
        std::unreachable();
    }

    inline std::vector<RangeDict> solveWorkflow(
        const rule::WorkflowName &name, const rule::Workflows &workflows,
        const SolveParam &params)
    {
        const auto condRange = [&](const auto &cond) {
            switch(cond.operation)
            {
            case rule::Operation::LESS:
                return Range(params.fullRange.b(), cond.value - 1);
            case rule::Operation::GREATER:
                return Range(cond.value + 1, params.fullRange.e());
            }
            std::unreachable();
        };
        const auto workflowIter = workflows.find(name);
        if(workflowIter == end(workflows))
        {
            return {};
        }
        const auto &workflow = workflowIter->second;
        struct ConditionRange
        {
            rule::RatingName name;
            Range range;
        };
        std::vector<ConditionRange> targetRanges;
        std::vector<RangeDict> result;
        const auto solveDstRules = [&](const auto &dst) {
            for(auto &cand : solveDestination(dst, workflows, params))
            {
                bool valid = true;
                for(const auto &targetRange : targetRanges)
                {
                    auto &candRng = cand.at(targetRange.name);
                    if(const auto maybeIntersectRng =
                           intersect(targetRange.range, candRng))
                    {
                        candRng = *maybeIntersectRng;
                    }
                    else
                    {
                        valid = false;
                        break;
                    }
                }
                if(valid)
                {
                    result.push_back(std::move(cand));
                }
            }
        };
        for(const auto &curRule : workflow.rules)
        {
            targetRanges.push_back({
                .name = curRule.condition.name,
                .range = condRange(curRule.condition),
            });
            solveDstRules(curRule.destination);
            const auto [leftInv, rightInv, commonInv] =
                join(targetRanges.back().range, params.fullRange);
            targetRanges.pop_back();
            for(const auto &inv : rightInv)
            {
                targetRanges.push_back({
                    .name = curRule.condition.name,
                    .range = inv,
                });
            }
        }
        solveDstRules(workflow.defaultDestination);
        return result;
    }
}

inline std::vector<RangeDict> solve(const rule::Workflows &workflows,
                                    const SolveParam &params,
                                    const rule::WorkflowName &startWorkflow)
{
    return detail::solveWorkflow(startWorkflow, workflows, params);
}

inline std::expected<rule::Action, std::string> execute(
    const rule::PartRatings &ratings, const rule::Workflows &workflows,
    const rule::WorkflowName &startWorkflow)
{
    auto workflow = startWorkflow;
    const auto execCond = [](const auto val, const auto &condition) {
        switch(condition.operation)
        {
        case rule::Operation::LESS:
            return val < condition.value;
        case rule::Operation::GREATER:
            return val > condition.value;
        }
        assert(false);
        return false;
    };
    const auto matchRule = [&](const auto &ratings, const auto &rule) {
        const auto &cond = rule.condition;
        const auto valIter = ratings.find(cond.name);
        if(valIter == end(ratings))
        {
            return false;
        }
        return execCond(valIter->second, cond);
    };
    while(true)
    {
        const auto iter = workflows.find(workflow);
        if(iter == end(workflows))
        {
            return std::unexpected(
                std::format("workflow not found: {}", workflow));
        }
        const auto matchIter =
            std::ranges::find_if(iter->second.rules, [&](const auto &rule) {
                return matchRule(ratings, rule);
            });
        const auto dst = [&] {
            if(matchIter != std::ranges::end(iter->second.rules))
            {
                return matchIter->destination;
            }
            return iter->second.defaultDestination;
        }();
        if(const auto *workflowDst =
               std::get_if<rule::DestinationWorkflow>(&dst))
        {
            workflow = workflowDst->name;
        }
        else if(const auto *actionDst =
                    std::get_if<rule::DestinationAction>(&dst))
        {
            return actionDst->action;
        }
        else
        {
            assert(false);
        }
    }
}
}

#endif
