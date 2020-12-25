#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include <utility>
#include <string>
#include <string_view>
#include <unordered_set>
#include <unordered_map>
#include <ranges>
#include <functional>
#include <optional>
#include <cctype>

#include "../../base.hpp"
#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    template<unsigned long long int MIN, unsigned long long int MAX>
    bool validNumber(string_view s)
    {
        const auto res = base::parseValue<unsigned long long int>(s);
        return *res && MIN <= *res && *res <= MAX;
    }

    bool validHeight(string_view s)
    {
        const auto iter = find_if(begin(s), end(s), [](auto c){return !isdigit(c);});
        const auto suffixPos = iter-begin(s);
        const auto value = string_view(s).substr(0, suffixPos);
        const auto suffix = string_view(s).substr(suffixPos);
        if(suffix == "cm")
        {
            return validNumber<150, 193>(value);
        }
        if(suffix == "in")
        {
            return validNumber<59, 76>(value);
        }
        return false;
    }

    template<size_t SZ, ranges::sized_range R, typename P>
    bool allOfWithSize(const R &values, P pred)
    {
        return values.size() == SZ &&
            ranges::all_of(values, pred);
    }

    bool validColor(string_view s)
    {
        constexpr size_t COLOR_LENGTH = 6;
        if(s.empty() || (s[0] != '#'))
        {
            return false;
        }
        return allOfWithSize<COLOR_LENGTH>(
            s.substr(1), [](auto c){return tolower(c) == c && isxdigit(c);});
    }

    bool validEyeColor(string_view s)
    {
        const unordered_set<string> COLORS =
            {"amb", "blu", "brn", "gry", "grn", "hzl", "oth"};
        return COLORS.contains(string(s));
    }

    bool validPID(string_view s)
    {
        constexpr size_t PID_LENGTH = 9;
        return allOfWithSize<PID_LENGTH>(s, [](auto c){return isdigit(c);});
    }

    bool validPassport(const FieldMap &fields)
    {
        struct FieldInfo
        {
            bool required;
            function<bool(const string&)> validator;
        };
        const unordered_map<string, FieldInfo> FIELDS = {
            std::make_pair("byr", FieldInfo{true, [](const auto &s){return validNumber<1920, 2002>(s);}}),
            std::make_pair("iyr", FieldInfo{true, [](const auto &s){return validNumber<2010, 2020>(s);}}),
            std::make_pair("eyr", FieldInfo{true, [](const auto &s){return validNumber<2020, 2030>(s);}}),
            std::make_pair("hgt", FieldInfo{true, validHeight}),
            std::make_pair("hcl", FieldInfo{true, validColor}),
            std::make_pair("ecl", FieldInfo{true, validEyeColor}),
            std::make_pair("pid", FieldInfo{true, validPID}),
            std::make_pair("cid", FieldInfo{false, [](const auto&){return true;}})
        };
        const size_t requiredFieldCount = ranges::count_if(FIELDS,
            [](const auto &p){return p.second.required;});
        size_t fieldCount = 0;
        for(const auto &field : fields)
        {
            auto iter = FIELDS.find(field.first);
            if(iter == end(FIELDS))
            {
                return false;
            }
            if(iter->second.required)
            {
                if(!iter->second.validator(field.second))
                {
                    return false;
                }
                ++fieldCount;
            }
        }
        return fieldCount == requiredFieldCount;
    }
}

int main()
{
    const string inp(istreambuf_iterator<char>(cin), {});
    const auto passports = parsePassports(inp);
    cout<<ranges::count_if(passports, [](const auto &passport){
            return validPassport(passport);
        })<<endl;
    return 0;
}
