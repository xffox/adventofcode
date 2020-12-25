#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <algorithm>
#include <ranges>
#include <vector>
#include <string>
#include <iterator>
#include <cassert>
#include <type_traits>

#include <iostream>

#include "../base.hpp"

namespace task
{
    using namespace std;

    using uint = unsigned int;

    template<ranges::view R>
        requires requires(R r){
            {*begin(r)} -> convertible_to<char>;
        }
    vector<uint> parseValues(R values)
    {
        vector<uint> result;
        ranges::copy(values
            | ranges::views::split(',')
            | ranges::views::transform([](auto val){
                    string valStr;
                    ranges::copy(val, back_inserter(valStr));
                    const auto res = base::parseValue<uint>(valStr);
                    assert(res);
                    return *res;
                }),
            back_inserter(result));
        return result;
    }

    inline uint nthRecitation(const vector<uint> &seed, size_t n)
    {
        if(n == 0 || seed.empty())
        {
            return 0;
        }
        if(n <= seed.size())
        {
            return seed[n-1];
        }
        vector<size_t> prevs(n, 0);
        for(size_t i = 0; i+1 < seed.size(); ++i)
        {
            prevs[seed[i]] = i+1;
        }
        uint prev = seed.back();
        for(size_t i = seed.size(); i != n; ++i)
        {
            const auto pos = prevs[prev];
            const auto val = (!!pos)*(i - pos);
            prevs[prev] = i;
            prev = val;
        }
        return prev;
    }
}

#endif
