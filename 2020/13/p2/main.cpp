#include <iostream>
#include <cstddef>
#include <algorithm>
#include <ranges>
#include <string>
#include <tuple>
#include <vector>
#include <cassert>
#include <iterator>

#include "../../base.hpp"

using namespace std;

namespace
{
    using ulint = unsigned long long int;

    constexpr ulint modinvert(ulint value, ulint mod)
    {
        if(value%mod == 0)
        {
            return 0;
        }
        ulint mult = 1;
        while(value > 1)
        {
            ulint m = 1;
            for(ulint c = 1;; c+=1)
            {
                m = (c*mod+value-1)/value;
                if(mod%m != 0)
                {
                    value = m*value-mod*c;
                    break;
                }
            }
            mult = (mult*m)%mod;
        }
        assert(value > 0);
        return mult;
    }

    template<ranges::view R>
    auto extractBusses(R buses)
    {
        auto vs = buses | ranges::views::split(',')
            | ranges::views::transform([](auto word){
                    string s;
                    ranges::for_each(word, [&s](auto v){
                            s.push_back(v);
                        });
                    return s;
                })
            | ranges::views::transform([](const auto &word){
                    return base::parseValue<ulint>(word);
                });
        vector<tuple<ulint, ulint>> result;
        size_t i = 0;
        for(const auto &word : vs)
        {
            if(word)
            {
                result.emplace_back(*word, i);
            }
            ++i;
        }
        return result;
    }

    template<ranges::random_access_range R>
    ulint solve(const R &values)
    {
        if(values.size() == 0)
        {
            return 0;
        }
        if(values.size() == 1)
        {
            return std::get<1>(*begin(values));
        }
        const size_t modIdx =
            ranges::max_element(values, {},
                [](const auto &p){return get<0>(p);}) - begin(values);
        const auto &[mod, leftOffset] = values[modIdx];
        assert(mod >= leftOffset);
        const auto offset = (mod - leftOffset)%mod;
        vector<tuple<ulint, ulint>> cands;
        for(size_t i = 0; i < values.size(); ++i)
        {
            if(i == modIdx)
            {
                continue;
            }
            const auto t = get<0>(values[i]);
            assert(mod >= get<1>(values[i]));
            const ulint d = (mod + (mod - get<1>(values[i])) - offset)%mod;
            const auto m = modinvert(t, mod)*d%mod * t;
            cands.emplace_back(t, (m+get<1>(values[i]))/mod);
        }
        const auto r = mod*solve(cands)+leftOffset;
        return r;
    }

    template<ranges::range R>
    ulint findSeqTimestamp(const R &buses)
    {
        const auto &[mod, offset] =
            ranges::max(buses, {}, [](const auto &p){return get<0>(p);});
        vector<tuple<ulint, ulint>> cands;
        for(const auto &p : buses)
        {
            cands.emplace_back(get<0>(p), (get<0>(p) - get<1>(p)%get<0>(p))%mod);
        }
        return solve(cands);
    }
}

int main()
{
    ulint ts = 0;
    cin>>ts;
    cin.ignore();
    string buses;
    getline(cin, buses);
    auto parsedBuses = extractBusses(ranges::views::all(buses));
    const auto b = findSeqTimestamp(parsedBuses);
    for(const auto &v : parsedBuses)
    {
        if((b+get<1>(v))%get<0>(v) != 0)
        {
            assert(false);
        }
    }
    cout<<b<<endl;
    return 0;
}
