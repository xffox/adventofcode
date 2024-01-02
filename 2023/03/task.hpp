#ifndef TASK_HPP
#define TASK_HPP

#include <array>
#include <cctype>
#include <concepts>
#include <cstddef>
#include <functional>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <utility>

#include "../base.hpp"

namespace task
{
using ulval = unsigned long long int;

struct Pos
{
    std::size_t row{};
    std::size_t col{};

    friend bool operator==(const Pos &, const Pos &) = default;
};

using PosSymbolMap = std::unordered_map<Pos, char>;

template<typename Handler>
concept PartHandler = std::invocable<Handler, Pos, ulval, PosSymbolMap>;
}

template<>
struct std::hash<task::Pos>
{
    std::size_t operator()(const task::Pos &pos) const
    {
        std::hash<decltype(pos.row)> hash{};
        const auto hashRow = hash(pos.row);
        const auto hashCol = hash(pos.col);
        return hashRow ^ (hashCol << 1);
    }
};

namespace task
{
template<PartHandler PartHandler>
class SchematicProcessor
{
public:
    template<std::constructible_from<PartHandler> Handler>
    explicit SchematicProcessor(Handler &&partHandler)
        : partHandler(std::forward<Handler>(partHandler))
    {
    }

    template<typename Func>
    requires std::is_invocable_r_v<bool, Func, std::string &>
    void produceRow(Func func)
    {
        const auto nextIdx = (rowIdx + 1) % ROW_COUNT;
        rows[nextIdx].clear();
        if(func(rows[nextIdx]))
        {
            processRow();
            ++rowIdx;
        }
    }

    void produceEmptyRow()
    {
        produceRow([](std::string &) { return true; });
    }

private:
    void processRow()
    {
        const auto curRowIdx = rowIdx % ROW_COUNT;
        std::optional<std::size_t> numberStart;
        PosSymbolMap partSymbols;
        std::string_view curRow = rows[curRowIdx];
        const auto handleNumber = [&](const auto col) {
            if(numberStart)
            {
                const auto val = *base::parseValue<ulval>(
                    curRow.substr(*numberStart, col - *numberStart));
                partHandler(Pos{rowIdx, col}, val, partSymbols);
            }
        };
        struct PosDiff
        {
            int row = 0;
            int col = 0;
        };
        const auto checkSymbol = [&](const auto col, const PosDiff &diff) {
            const auto targetRowIdx =
                (curRowIdx + ROW_COUNT + diff.row) % ROW_COUNT;
            const auto targetColIdx = static_cast<int>(col) + diff.col;
            const auto &targetRow = rows[targetRowIdx];
            if(targetColIdx >= 0 &&
               static_cast<std::size_t>(targetColIdx) < targetRow.size() &&
               std::ispunct(targetRow[targetColIdx]) &&
               targetRow[targetColIdx] != '.')
            {
                partSymbols.emplace(Pos{rowIdx + diff.row,
                                        static_cast<std::size_t>(targetColIdx)},
                                    targetRow[targetColIdx]);
            }
        };
        for(std::size_t col = 0; col < curRow.size(); ++col)
        {
            if(std::isdigit(curRow[col]) != 0)
            {
                if(!numberStart)
                {
                    numberStart = col;
                }
                constexpr int DIRECTIONS = 4;
                for(int i = 0; i < DIRECTIONS; ++i)
                {
                    checkSymbol(col, {(i % 2 * 2 - 1) * (i / 2),
                                      (i % 2 * 2 - 1) * (1 - i / 2)});
                }
                for(int i = 0; i < DIRECTIONS; ++i)
                {
                    checkSymbol(col, {(i % 2 * 2 - 1), (i / 2 * 2 - 1)});
                }
            }
            else
            {
                handleNumber(col);
                partSymbols.clear();
                numberStart.reset();
            }
        }
        handleNumber(curRow.size());
    }

    static constexpr std::size_t ROW_COUNT = 3;
    std::array<std::string, ROW_COUNT> rows{};
    std::size_t rowIdx = 0;
    PartHandler partHandler;
};

template<typename Handler>
requires PartHandler<std::decay_t<Handler>>
SchematicProcessor(Handler &&partHandler)
    -> SchematicProcessor<std::decay_t<Handler>>;
}

#endif
