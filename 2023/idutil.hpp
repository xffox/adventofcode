#ifndef IDUTIL_HPP
#define IDUTIL_HPP

#include <concepts>
#include <unordered_map>

namespace idutil
{
template<typename ID, std::unsigned_integral NumID>
class IDAlloc
{
public:
    NumID alloc(const ID &origID)
    {
        auto iter = ids.find(origID);
        if(iter == end(ids))
        {
            iter = ids.emplace(origID, nxtID++).first;
        }
        return iter->second;
    }

private:
    std::unordered_map<ID, NumID> ids;
    NumID nxtID = 0;
};
}

#endif
