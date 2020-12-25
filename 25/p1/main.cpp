#include <iostream>

#include "../../base.hpp"

using namespace std;

namespace
{
    using uint = unsigned int;
    using ulint = unsigned long long int;


    uint findLoopSize(ulint mod, ulint subject, ulint result)
    {
        uint i = 0;
        for(ulint v = 1; v != result; ++i)
        {
            v = (v*subject)%mod;
        }
        return i;
    }

    ulint transform(ulint mod, ulint subject, uint loopSize)
    {
        ulint v = 1;
        for(uint i = 0; i < loopSize; ++i)
        {
            v = (v*subject)%mod;
        }
        return v;
    }
}

int main()
{
    constexpr ulint INIT_SUBJECT = 7;
    constexpr ulint MOD = 20201227;
    ulint cardPubkey = 0;
    ulint doorPubkey = 0;
    cin>>cardPubkey>>doorPubkey;
    cout<<transform(MOD, cardPubkey,
        findLoopSize(MOD, INIT_SUBJECT, doorPubkey))<<endl;
    return 0;
}
