#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <expected>
#include <format>
#include <functional>
#include <istream>
#include <iterator>
#include <optional>
#include <ranges>
#include <regex>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>
#include <vector>

namespace task
{
enum class Dir
{
    LEFT = 0,
    RIGHT,
};

constexpr std::size_t NEIGHBOURS = 2;

using Size = std::size_t;

using Node = std::string;
using Path = std::vector<Dir>;
using Neighbours = std::array<Node, NEIGHBOURS>;
using Map = std::unordered_map<Node, Neighbours>;

struct PathCycle
{
    Size init{};
    Size length{};

    friend auto operator<=>(const PathCycle &, const PathCycle &) = default;
};

struct PathCharacteristic
{
    PathCycle cycle{};
    std::vector<Size> zeros{};
};

namespace input
{
    struct Input
    {
        Path path{};
        Map map{};
    };

    namespace detail
    {
        inline std::expected<Path, std::string> parsePath(std::string_view str)
        {
            Path result;
            result.reserve(str.size());
            for(const auto v : str)
            {
                Dir dir{};
                if(v == 'L')
                {
                    dir = Dir::LEFT;
                }
                else if(v == 'R')
                {
                    dir = Dir::RIGHT;
                }
                else
                {
                    return std::unexpected(
                        std::format("invalid path value: {}", v));
                }
                result.push_back(dir);
            }
            return result;
        }

        inline std::expected<std::pair<Node, Neighbours>, std::string>
        parseMapConnection(std::string_view str)
        {
            const std::regex REGEX(R"((\w+) = \((\w+), (\w+)\))");
            std::match_results<std::ranges::const_iterator_t<std::string_view>>
                match;
            if(std::regex_match(std::ranges::begin(str), std::ranges::end(str),
                                match, REGEX))
            {
                return std::pair{
                    std::string(match[1].first, match[1].second),
                    Neighbours{
                        std::string(match[2].first, match[2].second),
                        std::string(match[3].first, match[3].second),
                    }};
            }
            return std::unexpected("error parsing map connection");
        }
    }

    inline std::expected<Input, std::string> parseInput(std::istream &stream)
    {
        std::string line;
        std::getline(stream, line);
        auto maybePath = detail::parsePath(line);
        if(!maybePath)
        {
            return std::unexpected(
                std::format("invalid path: {}", maybePath.error()));
        }
        std::getline(stream, line);
        Map map;
        while(stream)
        {
            std::getline(stream, line);
            if(line.empty())
            {
                continue;
            }
            auto maybeMapConnection = detail::parseMapConnection(line);
            if(!maybeMapConnection)
            {
                return std::unexpected(std::format("invalid map connection: {}",
                                                   maybeMapConnection.error()));
            }
            map.insert(std::move(*maybeMapConnection));
        }
        return Input{
            std::move(*maybePath),
            std::move(map),
        };
    }
}

namespace detail
{
    struct PathState
    {
        Node node{};
        Size pathIdx{};

        friend auto operator<=>(const PathState &, const PathState &) = default;
    };
}
}

template<>
struct std::hash<task::detail::PathState>
{
    std::size_t operator()(const task::detail::PathState &state) const
    {
        const auto h1 = std::hash<decltype(state.node)>{}(state.node);
        const auto h2 = std::hash<decltype(state.pathIdx)>{}(state.pathIdx);
        return h1 ^ (h2 << 1);
    }
};

namespace task
{
template<typename Visitor>
requires std::is_invocable_r_v<bool, Visitor, Node, Size>
void traverse(const Map &map, const Path &path, const Node &source,
              Visitor &&visitor)
{
    if(path.empty())
    {
        return;
    }
    const auto pathSize = path.size();
    auto *cur = &source;
    Size idx = 0;
    while(true)
    {
        if(!visitor(*cur, idx))
        {
            break;
        }
        const auto iter = map.find(*cur);
        if(iter == end(map))
        {
            break;
        }
        cur = &iter->second[std::to_underlying(path[idx % pathSize])];
        ++idx;
    }
}

template<typename Func>
requires std::is_invocable_r_v<bool, Func, Node>
std::optional<PathCharacteristic> pathCharacteristic(const Map &map,
                                                     const Path &path,
                                                     const Node &source,
                                                     Func &&checkZero)
{
    std::unordered_map<detail::PathState, Size> seenStates;
    std::vector<Size> zeros;
    std::optional<detail::PathState> cycleState;
    task::Size cycleIdx{};
    traverse(map, path, source, [&](const auto &node, const auto idx) {
        const auto pathIdx = idx % path.size();
        if(detail::PathState state{node, pathIdx};
           !seenStates.emplace(state, idx).second)
        {
            cycleState = std::move(state);
            cycleIdx = idx;
            return false;
        }
        if(checkZero(node))
        {
            zeros.push_back(idx);
        }
        return true;
    });
    if(!cycleState)
    {
        return std::nullopt;
    }
    const auto initIter = seenStates.find(*cycleState);
    assert(initIter != end(seenStates));
    return PathCharacteristic{
        {initIter->second, cycleIdx - initIter->second},
        std::move(zeros),
    };
}
}

#endif
