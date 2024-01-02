#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <string>
#include <string_view>

namespace task
{
    using namespace std;

    using uint = unsigned int;

    template<char... ONES>
    uint parseBin(string_view str)
    {
        uint res = 0;
        for(auto c : str)
        {
            res *= 2;
            res += ((c == ONES) || ...);
        }
        return res;
    }

    uint parseID(const string &str)
    {
        constexpr size_t LEN = 10;
        if(str.size() != LEN)
        {
            return 0;
        }
        return parseBin<'B', 'R'>(str);
    }

}

#endif
