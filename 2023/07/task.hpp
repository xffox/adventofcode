#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <array>
#include <compare>
#include <concepts>
#include <cstddef>
#include <expected>
#include <format>
#include <functional>
#include <iterator>
#include <ranges>
#include <span>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "../base.hpp"

namespace task
{
constexpr std::size_t HAND_CARDS = 5;
enum class Card
{
    N2 = 0,
    N3,
    N4,
    N5,
    N6,
    N7,
    N8,
    N9,
    T,
    J,
    Q,
    K,
    A,
};
using uval = unsigned int;
using ulval = unsigned long long int;
using Hand = std::array<Card, HAND_CARDS>;

template<bool Joker = false>
class HandOrder
{
    static constexpr auto cardRank(const Card card)
    {
        const auto val = std::to_underlying(card);
        if constexpr(Joker)
        {
            return card != Card::J ? val + 1 : 0;
        }
        else
        {
            return val;
        }
    }

    static constexpr auto prepareHandTypeRank(const Hand &hand)
    {
        std::array<uval, std::to_underlying(Card::A) + 1> cardCounts{};
        for(const auto c : hand)
        {
            cardCounts[std::to_underlying(c)] += 1;
        }
        uval jokers = 0;
        if constexpr(Joker)
        {
            auto &jokerCnt = cardCounts[std::to_underlying(Card::J)];
            jokers = jokerCnt;
            jokerCnt = 0;
        }
        std::ranges::partial_sort(
            cardCounts, std::ranges::begin(cardCounts) + HAND_TYPE_RANK_CARDS,
            std::greater<>{});
        cardCounts[0] += jokers;
        HandTypeRank result{};
        std::ranges::copy(cardCounts | std::views::take(HAND_TYPE_RANK_CARDS),
                          std::ranges::begin(result));
        return result;
    }

public:
    explicit constexpr HandOrder(const Hand &hand)
        : handTypeRank(prepareHandTypeRank(hand)), handSpan(hand)
    {
    }

    friend constexpr std::strong_ordering operator<=>(const HandOrder &left,
                                                      const HandOrder &right)
    {
        constexpr auto cmpCol = [](const auto &left, const auto &right) {
            const auto [leftIter, rightIter] =
                std::ranges::mismatch(left, right);
            if(leftIter != std::ranges::end(left))
            {
                return *leftIter <=> *rightIter;
            }
            return std::strong_ordering::equal;
        };
        constexpr auto prepareCardRanks = [](auto &cards) {
            return cards | std::views::transform(cardRank);
        };
        if(const auto handTypeCmp =
               cmpCol(left.handTypeRank, right.handTypeRank);
           handTypeCmp != 0)
        {
            return handTypeCmp;
        }
        return cmpCol(prepareCardRanks(left.handSpan),
                      prepareCardRanks(right.handSpan));
    }

    friend constexpr bool operator==(const HandOrder &left,
                                     const HandOrder &right)
    {
        return (left <=> right) == 0;
    }

private:
    static constexpr std::size_t HAND_TYPE_RANK_CARDS = 2;

    using HandTypeRank = std::array<uval, HAND_TYPE_RANK_CARDS>;

    HandTypeRank handTypeRank;
    std::span<const Card, HAND_CARDS> handSpan;
};

namespace input
{
    struct Play
    {
        Hand hand{};
        uval bid{};
    };
}

template<std::constructible_from<Hand> Order,
         std::ranges::random_access_range R>
requires std::totally_ordered<Order> &&
         std::same_as<std::ranges::range_value_t<R>, input::Play>
ulval totalWinnings(const R &plays)
{
    std::vector<std::tuple<Order, uval>> sortedBids;
    sortedBids.reserve(std::ranges::size(plays));
    std::ranges::copy(plays | std::views::transform([](const auto &play) {
                          return std::tuple{Order(play.hand), play.bid};
                      }),
                      std::back_inserter(sortedBids));
    std::ranges::sort(sortedBids);
    return std::ranges::fold_left(std::views::enumerate(sortedBids) |
                                      std::views::transform([](const auto &p) {
                                          const auto &[idx, bid] = p;
                                          return (static_cast<ulval>(idx) + 1) *
                                                 std::get<1>(bid);
                                      }),
                                  static_cast<ulval>(0), std::plus<>{});
}

namespace detail
{
    constexpr std::expected<Card, std::string> parseCard(char c)
    {
        switch(c)
        {
        case '2':
            return Card::N2;
        case '3':
            return Card::N3;
        case '4':
            return Card::N4;
        case '5':
            return Card::N5;
        case '6':
            return Card::N6;
        case '7':
            return Card::N7;
        case '8':
            return Card::N8;
        case '9':
            return Card::N9;
        case 'T':
            return Card::T;
        case 'J':
            return Card::J;
        case 'Q':
            return Card::Q;
        case 'K':
            return Card::K;
        case 'A':
            return Card::A;
        default:
            return std::unexpected("invalid card");
        }
    }

    constexpr std::expected<Hand, std::string> parseHand(std::string_view str)
    {
        Hand hand{};
        if(str.size() != hand.size())
        {
            return std::unexpected("invalid length for hand");
        }
        for(const auto [i, c] : std::views::enumerate(str))
        {
            const auto maybeCard = parseCard(c);
            if(!maybeCard)
            {
                return std::unexpected(
                    std::format("invalid card: {}", maybeCard.error()));
            }
            hand[i] = *maybeCard;
        }
        return hand;
    }
}

inline std::expected<input::Play, std::string> parsePlay(std::string_view str)
{
    const auto sepPos = str.find(' ');
    if(sepPos == std::string_view::npos)
    {
        return std::unexpected("no play separator");
    }
    const auto maybeCard = detail::parseHand(str.substr(0, sepPos));
    if(!maybeCard)
    {
        return std::unexpected(
            std::format("invalid hand: {}", maybeCard.error()));
    }
    const auto maybeBid = base::parseValue<uval>(str.substr(sepPos + 1));
    if(!maybeBid)
    {
        return std::unexpected("invalid bid");
    }
    return input::Play{*maybeCard, *maybeBid};
}
}

#endif
