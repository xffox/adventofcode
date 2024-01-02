#ifndef BASE_HPP
#define BASE_HPP

#include <concepts>
#include <cstdlib>
#include <exception>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <type_traits>

namespace base
{
template<std::integral T>
std::optional<T> parseValue(std::string_view val, unsigned int base = 10)
{
    try
    {
        T res = T{};
        if constexpr(std::is_signed<T>::value)
        {
            res = std::stoll(std::string(val), nullptr, base);
            if(res < std::numeric_limits<T>::min() ||
               res > std::numeric_limits<T>::max())
            {
                return {};
            }
        }
        else
        {
            res = std::stoull(std::string(val), nullptr, base);
            if(res > std::numeric_limits<T>::max())
            {
                return {};
            }
        }
        return res;
    }
    catch(const std::exception &)
    {
        return {};
    }
}
}

#endif
