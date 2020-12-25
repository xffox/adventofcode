#ifndef BASE_H
#define BASE_H

#include <cstddef>
#include <string>
#include <optional>
#include <string_view>
#include <type_traits>
#include <iterator>
#include <concepts>

namespace base
{
    using namespace std;

    template<typename T, typename V>
    struct MatchRefTuple
    {
        static constexpr bool value = false;
    };
    template<typename V, typename... Args>
    struct MatchRefTuple<tuple<Args...>, V>
    {
        static constexpr bool value =
            ((is_same_v<decay_t<Args>, V> &&
              (is_lvalue_reference<Args>::value ||
               is_rvalue_reference<Args>::value)) && ...);
    };

    template<integral T>
    optional<T> parseValue(string_view val)
    {
        try
        {
            T res = T{};
            if constexpr(is_signed<T>::value)
            {
                res = stoll(string(val));
                if(res < numeric_limits<T>::min() ||
                    res > numeric_limits<T>::max())
                {
                    return {};
                }
            }
            else
            {
                res = stoull(string(val));
                if(res > numeric_limits<T>::max())
                {
                    return {};
                }
            }
            return res;
        }
        catch(const exception&)
        {
            return {};
        }
    }

    template<typename M>
    auto lookupWithDefault(const M &map,
        typename M::key_type key,
        const typename M::mapped_type &defaulValue) ->
            typename M::mapped_type
    {
        auto iter = map.find(key);
        if(iter != std::end(map))
        {
            return iter->second;
        }
        return defaulValue;
    }
}

#endif
