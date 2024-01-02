#include <iostream>
#include <algorithm>
#include <iterator>
#include <string>
#include <ranges>
#include <cassert>

#include "../task.hpp"

namespace
{
    using namespace std;
    using namespace task;

    template<ranges::view R>
    uint sum(R values)
    {
        uint s = 0;
        ranges::for_each(values, [&s](auto v){s += v;});
        return s;
    }

    uint scanningErrorRate(const Notes &notes)
    {
        return sum(ranges::views::transform(notes.tickets
                | ranges::views::drop(1),
            [&notes](const auto &ticket){
                return sum(ticket
                    | ranges::views::filter([&notes](auto v){
                            return !ranges::any_of(notes.rules,
                                [v](const auto &rule){
                                    return matchesRule(rule, v);
                                });
                        }));
            }));
    }
}

int main()
{
    string inp;
    copy(istreambuf_iterator<char>(cin), {}, back_inserter(inp));
    const auto notesRes = parseNotes(inp);
    assert(notesRes);
    cout<<scanningErrorRate(*notesRes)<<endl;
    return 0;
}
