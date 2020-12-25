#ifndef TASK_H
#define TASK_H

#include <algorithm>
#include <optional>
#include <tuple>
#include <string>
#include <vector>
#include <ranges>
#include <utility>
#include <iterator>
#include <deque>
#include <array>
#include <unordered_set>
#include <sstream>
#include <type_traits>

#include "../base.hpp"

namespace task
{
    using namespace std;

    using uint = unsigned int;
    using CardCol = vector<uint>;

    namespace inner
    {
        enum class Win
        {
            PLAYER1 = 0,
            PLAYER2
        };

        using HandCol = deque<uint>;

        template<ranges::forward_range R>
        R slice(const R &values, size_t sz)
        {
            R result;
            ranges::copy(values | ranges::views::take(sz),
                back_inserter(result));
            return result;
        }

        template<typename T>
            requires base::MatchRefTuple<T, HandCol>::value &&
                (tuple_size_v<T> == 2)
        string describe(const T &hands)
        {
            stringstream result;
            auto append = [&result](const HandCol &hand){
                for(auto v : hand)
                {
                    result<<v<<',';
                }
            };
            append(get<0>(hands));
            result<<'|';
            append(get<1>(hands));
            return move(result).str();
        }

        template<bool recurse, typename T>
            requires base::MatchRefTuple<T, HandCol>::value &&
                (tuple_size_v<T> == 2)
        Win play(const T &hands)
        {
            const auto &[hand1, hand2] = hands;
            unordered_set<string> seen;
            while(!hand1.empty() && !hand2.empty())
            {
                const auto c1 = hand1.front();
                const auto c2 = hand2.front();
                hand1.pop_front();
                hand2.pop_front();
                if(!seen.insert(describe(hands)).second)
                {
                    return Win::PLAYER1;
                }
                bool fstWin = true;
                if(recurse && c1 <= hand1.size() && c2 <= hand2.size())
                {
                    fstWin = (play<recurse>(
                            tuple<HandCol&&, HandCol&&>(
                                slice(hand1, c1),
                                slice(hand2, c2))) == Win::PLAYER1);
                }
                else
                {
                    fstWin = (c1 >= c2);
                }
                if(fstWin)
                {
                    ranges::copy(to_array({c1, c2}), back_inserter(hand1));
                }
                else
                {
                    ranges::copy(to_array({c2, c1}), back_inserter(hand2));
                }
            }
            return !hand1.empty()?Win::PLAYER1:Win::PLAYER2;
        }

        template<ranges::bidirectional_range R>
        uint score(const R &cards)
        {
            uint mult = 1;
            uint result = 0;
            for(auto v : cards | ranges::views::reverse)
            {
                result += v*mult;
                ++mult;
            }
            return result;
        }
    }

    inline optional<tuple<CardCol, CardCol>> parsePlayers(
        const string &str)
    {
        vector<CardCol> players;
        CardCol cards;
        auto appendCards = [&players, &cards](){
            players.push_back(move(cards));
            cards.clear();
        };
        bool inCards = false;
        for(auto &&line : str | ranges::views::split('\n'))
        {
            string lineStr;
            ranges::copy(line, back_inserter(lineStr));
            if(inCards)
            {
                if(!lineStr.empty())
                {
                    const auto cardRes = base::parseValue<uint>(lineStr);
                    if(!cardRes)
                    {
                        return {};
                    }
                    cards.push_back(*cardRes);
                }
                else
                {
                    appendCards();
                    inCards = false;
                }
            }
            else
            {
                if(!lineStr.empty())
                {
                    inCards = true;
                }
            }
        }
        if(inCards)
        {
            appendCards();
        }
        if(players.size() != 2)
        {
            return {};
        }
        return make_tuple(move(players[0]), move(players[1]));
    }

    template<bool recurse>
    uint play(const tuple<CardCol, CardCol> &players)
    {
        tuple<inner::HandCol, inner::HandCol> hands(
            {begin(get<0>(players)), end(get<0>(players))},
            {begin(get<1>(players)), end(get<1>(players))});
        const auto win = inner::play<recurse>(
            tuple<inner::HandCol&, inner::HandCol&>(
                get<0>(hands), get<1>(hands)));
        return inner::score(
            win==inner::Win::PLAYER1?get<0>(hands):get<1>(hands));
    }
}

#endif
