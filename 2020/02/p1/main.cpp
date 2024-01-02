#include <cstddef>
#include <algorithm>
#include <iostream>
#include <string>
#include <string_view>

#include "../task.hpp"

namespace
{
    using namespace std;
    using namespace task;

    bool isValidPassword(string_view password, const Policy &policy)
    {
        const size_t count = ranges::count(password, policy.character);
        return policy.fstArg <= count && count <= policy.sndArg;
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
