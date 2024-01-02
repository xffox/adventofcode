#ifndef TASK_HPP
#define TASK_HPP

#include <cassert>
#include <expected>
#include <ranges>
#include <regex>
#include <string>
#include <tuple>
#include <unordered_map>
#include <utility>
#include <vector>

#include "../base.hpp"

namespace task
{
using ulval = unsigned long long int;
using GameID = unsigned int;
using Count = unsigned int;
using Cube = std::string;
using CubeCounts = std::unordered_map<Cube, Count>;
using Game = std::vector<CubeCounts>;

std::expected<std::tuple<GameID, Game>, std::string> parseGame(const auto &game)
{
    const std::regex lineRegex(R"(Game (\d+):(.*))");
    const std::regex cubeRegex(R"( *(\d+) (\w+),?)");
    if(std::smatch lineMatch; std::regex_match(game, lineMatch, lineRegex))
    {
        assert(lineMatch.size() == 3);
        const auto maybeID = base::parseValue<GameID>(lineMatch[1].str());
        if(!maybeID)
        {
            return std::unexpected("invalid game ID");
        }
        Game rounds;
        for(const auto &roundsStr = lineMatch[2].str();
            const auto &roundStr : roundsStr | std::views::split(';'))
        {
            auto &round = rounds.emplace_back();
            for(auto iter =
                    std::sregex_iterator(std::ranges::begin(roundStr),
                                         std::ranges::end(roundStr), cubeRegex);
                iter != std::sregex_iterator{}; ++iter)
            {
                const auto &match = *iter;
                assert(match.size() == 3);
                const auto maybeCount = base::parseValue<Count>(match[1].str());
                if(!maybeCount)
                {
                    return std::unexpected("invalid color count");
                }
                round[match[2].str()] += *maybeCount;
            }
        }
        return std::tuple{*maybeID, std::move(rounds)};
    }
    return std::unexpected("invalid game");
}
}

#endif
