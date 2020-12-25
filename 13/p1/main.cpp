#include <iostream>
#include <algorithm>
#include <ranges>
#include <string>

#include "../../base.hpp"

using namespace std;

namespace
{
    using uint = unsigned int;

    constexpr uint minDiff(uint ts, uint id)
    {
        return (ts+(id-1))/id*id - ts;
    }

    template<ranges::view R>
    auto extractBusses(R buses)
    {
        return buses | ranges::views::split(',')
            | ranges::views::transform([](auto word){
                    string s;
                    ranges::for_each(word, [&s](auto v){
                            s.push_back(v);
                        });
                    return s;
                })
            | ranges::views::transform([](const auto &word){
                    return base::parseValue<uint>(word);
                })
            | ranges::views::filter([](const auto &val){
                    return static_cast<bool>(val);
                })
            | ranges::views::transform([](const auto &val){
                    return *val;
                });
    }

    template<ranges::view R>
    uint minBus(uint ts, R buses)
    {
        return ranges::min(buses, {}, [ts](auto v){return minDiff(ts, v);});
    }
}

int main()
{
    uint ts = 0;
    cin>>ts;
    cin.ignore();
    string buses;
    getline(cin, buses);
    const auto b = minBus(ts, extractBusses(ranges::views::all(buses)));
    cout<<b*minDiff(ts, b)<<endl;
    return 0;
}
