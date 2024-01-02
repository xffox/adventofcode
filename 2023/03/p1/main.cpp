#include <iostream>
#include <string>

#include "../task.hpp"

int main()
{
    task::ulval sum = 0;
    task::SchematicProcessor proc(
        [&sum](const auto &, const auto val, const auto &symbols) {
            if(!symbols.empty())
            {
                sum += val;
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
    std::cout << sum << '\n';
    return 0;
}
