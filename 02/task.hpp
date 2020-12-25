#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <optional>
#include <string>
#include <string_view>
#include <tuple>

#include "../base.hpp"

namespace task
{
    using namespace std;

    struct Policy
    {
        char character;
        size_t fstArg;
        size_t sndArg;
    };

    namespace
    {
        optional<tuple<size_t, size_t>> parseRange(string_view val)
        {
            const string SEP("-");
            const auto sepPos = val.find(SEP);
            if(sepPos == string_view::npos)
            {
                return {};
            }
            const auto minRes = base::parseValue<size_t>(val.substr(0, sepPos));
            const auto maxRes = base::parseValue<size_t>(val.substr(sepPos+1));
            if(!minRes || !maxRes)
            {
                return {};
            }
            return make_tuple(*minRes, *maxRes);
        }
    }

    optional<Policy> parsePolicy(string_view val)
    {
        const auto spacePos = val.find(' ');
        if(spacePos == string_view::npos || spacePos+1 == val.size())
        {
            return {};
        }
        const auto rangeRes = parseRange(val.substr(0, spacePos));
        if(!rangeRes)
        {
            return {};
        }
        const auto &[fstArg, sndArg] = *rangeRes;
        return Policy{val[spacePos+1], fstArg, sndArg};
    }

    optional<tuple<string_view, Policy>> parseEntry(string_view entry)
    {
        const string SEP(": ");
        const auto sepPos = entry.find(SEP);
        if(sepPos == string_view::npos)
        {
            return {};
        }
        const auto policyRes = parsePolicy(entry.substr(0, sepPos));
        if(!policyRes)
        {
            return {};
        }
        return make_tuple(entry.substr(sepPos+SEP.size()), *policyRes);
    }
}

#endif
