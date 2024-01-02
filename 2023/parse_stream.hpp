#ifndef PARSER_PARSESTREAM_HPP
#define PARSER_PARSESTREAM_HPP

#include <cstddef>
#include <optional>
#include <string_view>

namespace parser
{
class ParseStream
{
public:
    explicit constexpr ParseStream(std::string_view str) : str(str)
    {
    }

    [[nodiscard]] constexpr std::optional<char> peek() const
    {
        if(idx >= str.size())
        {
            return std::nullopt;
        }
        return str[idx];
    }

    constexpr std::optional<char> read()
    {
        if(idx >= str.size())
        {
            return std::nullopt;
        }
        const auto val = str[idx];
        ++idx;
        return val;
    }

private:
    std::size_t idx{};
    std::string_view str;
};
}

#endif
