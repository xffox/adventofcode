#include <iostream>
#include <algorithm>
#include <iterator>
#include <cstddef>
#include <utility>
#include <string>
#include <unordered_map>
#include <ranges>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    bool validPassport(const FieldMap &fields)
    {
        const unordered_map<string, bool> FIELDS = {
            std::make_pair("byr", true),
            std::make_pair("iyr", true),
            std::make_pair("eyr", true),
            std::make_pair("hgt", true),
            std::make_pair("hcl", true),
            std::make_pair("ecl", true),
            std::make_pair("pid", true),
            std::make_pair("cid", false)
        };
        const size_t requiredFieldCount = ranges::count_if(FIELDS,
            [](const auto &p){return p.second;});
        size_t fieldCount = 0;
        for(const auto &field : fields)
        {
            auto iter = FIELDS.find(field.first);
            if(iter == end(FIELDS))
            {
                return false;
            }
            if(iter->second)
            {
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
