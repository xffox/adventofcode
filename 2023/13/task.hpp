#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <concepts>
#include <cstddef>
#include <expected>
#include <functional>
#include <optional>
#include <ranges>
#include <span>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "../matrix.hpp"

namespace task
{
using Board = numutil::Matrix<int>;
using uval = std::size_t;

enum class Dir
{
    HORIZONTAL,
    VERTICAL,
};

struct Reflection
{
    uval pos{};
    Dir direction{};
};

namespace input
{
    inline std::expected<Board, std::string> parseBoard(std::string_view str)
    {
        std::vector<std::string> rows;
        for(const auto rowView : str | std::views::split('\n'))
        {
            const auto size = std::ranges::size(rowView);
            if(size == 0)
            {
                continue;
            }
            if(!rows.empty() && rows.back().size() != size)
            {
                return std::unexpected("row lengths mismatch");
            }
            rows.emplace_back(std::ranges::begin(rowView),
                              std::ranges::end(rowView));
        }
        Board board(rows.size(), rows.front().size());
        for(std::size_t rowIdx = 0; rowIdx < rows.size(); ++rowIdx)
        {
            const auto &row = rows[rowIdx];
            for(std::size_t colIdx = 0; colIdx < row.size(); ++colIdx)
            {
                const auto value = row[colIdx];
                board.ix(rowIdx, colIdx) = (value == '#');
            }
        }
        return board;
    }
}

namespace detail
{
    struct Cmp
    {
        constexpr bool operator()(uval diff)
        {
            return diff == 0;
        }
        explicit constexpr operator bool() const
        {
            return true;
        }
    };

    class SmudgeCmp
    {
    public:
        constexpr bool operator()(uval diff)
        {
            if(diff > 1)
            {
                return false;
            }
            if(diff == 1)
            {
                if(smudge)
                {
                    return false;
                }
                smudge = true;
            }
            return true;
        }
        explicit constexpr operator bool() const
        {
            return smudge;
        }

    private:
        bool smudge = false;
    };

    template<typename Cmp>
    requires std::is_invocable_r_v<bool, Cmp &, uval> &&
             std::constructible_from<bool, Cmp>
    std::optional<Reflection> findReflection(const Board &board)
    {
        using namespace std::placeholders;
        using Symbol = std::vector<bool>;
        const auto countDifferences = [](const auto &left, const auto &right) {
            return std::ranges::fold_left(
                std::views::zip_transform(std::not_equal_to<>{}, left, right),
                static_cast<uval>(0), std::plus<>{});
        };
        const auto findPos = [&](const auto &board) -> std::optional<uval> {
            std::vector<Symbol> symbols;
            for(std::size_t row = 0; row < board.rows(); ++row)
            {
                Symbol symb;
                for(std::size_t col = 0; col < board.columns(); ++col)
                {
                    symb.push_back(static_cast<bool>(board.ix(row, col)));
                }
                symbols.push_back(std::move(symb));
            }
            const std::span<const Symbol> symbolsSpan{symbols};
            for(std::size_t i = 1; i < symbols.size(); ++i)
            {
                const auto size = std::min(i, symbols.size() - i);
                const auto left = symbolsSpan.subspan(i - size, size);
                const auto right = symbolsSpan.subspan(i, size);
                if([&] {
                       Cmp cmp;
                       for(const auto &[leftSymb, rightSymb] :
                           std::views::zip(std::views::reverse(left), right))
                       {
                           const auto diff =
                               countDifferences(leftSymb, rightSymb);
                           if(!cmp(diff))
                           {
                               return false;
                           }
                       }
                       return static_cast<bool>(cmp);
                   }())
                {
                    return i;
                }
            }
            return std::nullopt;
        };
        if(const auto rowPos = findPos(board))
        {
            return Reflection(*rowPos, Dir::HORIZONTAL);
        }
        if(const auto colPos = findPos(board.transpose()))
        {
            return Reflection(*colPos, Dir::VERTICAL);
        }
        return std::nullopt;
    }
}

inline std::optional<Reflection> findReflection(const Board &board)
{
    return detail::findReflection<detail::Cmp>(board);
}

inline std::optional<Reflection> findSmudgedReflection(const Board &board)
{
    return detail::findReflection<detail::SmudgeCmp>(board);
}
}

#endif
