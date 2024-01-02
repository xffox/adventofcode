#include <cstddef>
#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>
#include <array>

#include "../task.hpp"

namespace
{
    using namespace std;
    using namespace task;

    bool isValidPassword(string_view password, const Policy &policy)
    {
        const auto cnt = ranges::count_if(
            to_array<size_t>({policy.fstArg, policy.sndArg}),
            [password, character=policy.character](auto pos){
                return pos > 0 && pos <= password.size() &&
                    password[pos-1] == character;
            });
        return cnt == 1;
    }

    bool isValidPasswordEntry(const string &entry)
    {
        const auto res = parseEntry(entry);
        if(res)
        {
            const auto &[password, policy] = *res;
            return isValidPassword(password, policy);
        }
        return false;
    }
}

int main()
{
    size_t count = 0;
    while(cin)
    {
        string entry;
        getline(cin, entry);
        count += static_cast<size_t>(isValidPasswordEntry(entry));
    }
    cout<<count<<endl;
    return 0;
}
