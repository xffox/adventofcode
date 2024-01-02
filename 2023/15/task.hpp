#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <cctype>
#include <concepts>
#include <cstddef>
#include <cstdint>
#include <expected>
#include <format>
#include <functional>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <ranges>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <variant>
#include <vector>

#include "../base.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;

using Hash = std::uint8_t;

struct OperationSet
{
    uval value{};
};
struct OperationRemove
{
};
using Operation = std::variant<OperationSet, OperationRemove>;

struct Step
{
    Operation operation{};
    std::string label{};
};

namespace input
{
    inline auto readInput(std::istream &stream)
    {
        return std::ranges::subrange(std::istreambuf_iterator<char>{stream},
                                     std::istreambuf_iterator<char>{}) |
               std::views::filter([](const auto val) { return val != '\n'; }) |
               std::views::lazy_split(',');
    }

    inline std::expected<std::vector<Step>, std::string> readSteps(
        std::istream &stream)
    {
        std::vector<Step> result;
        for(const auto &stepView :
            std::ranges::subrange(std::istreambuf_iterator<char>{stream},
                                  std::istreambuf_iterator<char>{}) |
                std::views::filter([](const auto val) { return val != '\n'; }) |
                std::views::lazy_split(','))
        {
            std::string str;
            std::ranges::copy(stepView, std::back_inserter(str));
            const auto opIter = std::ranges::find_if_not(
                str, [](const auto val) { return std::isalnum(val); });
            if(opIter == std::ranges::end(str))
            {
                return std::unexpected("invalid step: missing operation");
            }
            const auto operation = *opIter;
            Step step{
                .operation = {},
                .label = {std::ranges::begin(str), opIter},
            };
            if(operation == '-')
            {
                const auto endIter = opIter + 1;
                if(endIter != std::ranges::end(str))
                {
                    return std::unexpected("invalid step: extra characters");
                }
                step.operation = OperationRemove{};
            }
            else if(operation == '=')
            {
                const auto nextIter = opIter + 1;
                const auto maybeFocalLength = base::parseValue<uval>(
                    std::string_view(nextIter, std::ranges::end(str)));
                if(!maybeFocalLength)
                {
                    return std::unexpected("invalid focal length");
                }
                step.operation = OperationSet{*maybeFocalLength};
            }
            else
            {
                return std::unexpected(std::format(
                    "invalid step: unknown operation: {}", operation));
            }
            result.push_back(std::move(step));
        }
        return result;
    }
}

template<std::ranges::input_range R>
constexpr Hash hash(const R &str)
{
    using Accum = unsigned int;
    constexpr Accum COEF = 17;
    constexpr auto toASCII = [](const char val) {
        // assuming ASCII system
        return static_cast<Accum>(val);
    };
    return std::ranges::fold_left(
        str, static_cast<Hash>(0), [&](const Hash cur, const char val) {
            return static_cast<Hash>((static_cast<Accum>(cur) + toASCII(val)) *
                                     COEF);
        });
}

namespace detail
{
    class Box
    {
    public:
        void set(const std::string &label, uval value)
        {
            if(auto iter = labelLens.find(label); iter != end(labelLens))
            {
                *(iter->second) = value;
            }
            else
            {
                labelLens.insert_or_assign(label,
                                           lens.insert(end(lens), value));
            }
        }
        void remove(const std::string &label)
        {
            if(auto iter = labelLens.find(label); iter != end(labelLens))
            {
                lens.erase(iter->second);
                labelLens.erase(iter);
            }
        }

        ulval focusingPower() const
        {
            return std::ranges::fold_left(
                std::views::enumerate(lens) |
                    std::views::transform([](const auto &p) {
                        const auto &[idx, val] = p;
                        return (idx + 1) * static_cast<ulval>(val);
                    }),
                static_cast<ulval>(0), std::plus<>{});
        }

    private:
        using LensList = std::list<uval>;

        LensList lens;
        std::unordered_map<std::string, LensList::iterator> labelLens;
    };
}

template<std::ranges::forward_range R>
requires std::same_as<std::ranges::range_value_t<R>, Step>
ulval executeSteps(const R &steps)
{
    struct Visitor
    {
        void operator()(const OperationRemove &) const
        {
            box.remove(label);
        }
        void operator()(const OperationSet &op) const
        {
            box.set(label, op.value);
        }

        const std::string &label;
        detail::Box &box;
    };
    std::array<detail::Box,
               static_cast<std::size_t>(std::numeric_limits<Hash>::max()) + 1>
        boxes{};
    for(const auto &step : steps)
    {
        auto &box = boxes[hash(step.label)];
        std::visit(Visitor{step.label, box}, step.operation);
    }
    return std::ranges::fold_left(
        std::views::enumerate(boxes |
                              std::views::transform([](const auto &box) {
                                  return box.focusingPower();
                              })) |
            std::views::transform([](const auto &p) {
                const auto &[idx, val] = p;
                return (idx + 1) * static_cast<ulval>(val);
            }),
        static_cast<ulval>(0), std::plus<>{});
}
}

#endif
