#include <algorithm>
#include <cstddef>
#include <functional>
#include <iostream>
#include <ranges>
#include <string>
#include <unordered_map>
#include <vector>

#include "../task.hpp"

int main()
{
    constexpr char GEAR_SYMB = '*';
    constexpr std::size_t GEAR_VAL_COUNT = 2;
    std::unordered_map<task::Pos, std::vector<task::ulval>> parts;
    task::SchematicProcessor proc(
        [&parts](const auto &, const auto val, const auto &symbols) {
            for(const auto &[pos, symb] : symbols)
            {
                if(symb == GEAR_SYMB)
                {
                    parts[pos].push_back(val);
                }
            }
        });
    while(std::cin)
    {
        proc.produceRow([&](std::string &row) {
            std::getline(std::cin, row);
            return !row.empty();
        });
    }
    proc.produceEmptyRow();
    const auto sum = std::ranges::fold_left(
        parts | std::views::filter([](const auto &p) {
            return p.second.size() == GEAR_VAL_COUNT;
        }) | std::views::transform([](const auto &p) {
            return std::ranges::fold_left(p.second, static_cast<task::ulval>(1),
                                          std::multiplies<>{});
        }),
        static_cast<task::ulval>(0), std::plus<>{});
    std::cout << sum << '\n';
    return 0;
}
