#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cctype>
#include <expected>
#include <istream>
#include <iterator>
#include <optional>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

#include "../geom.hpp"
#include "../idutil.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;
using ID = uval;
using Graph = std::unordered_map<ID, std::unordered_set<ID>>;

namespace input
{
    namespace detail
    {
        using StrID = std::string;
        using IDAlloc = idutil::IDAlloc<StrID, ID>;
    }

    inline std::expected<Graph, std::string> parseGraph(std::istream &stream)
    {
        detail::IDAlloc idAlloc;
        Graph graph;
        for(const auto lineView :
            std::ranges::subrange(std::istreambuf_iterator<char>{stream},
                                  std::istreambuf_iterator<char>{}) |
                std::views::lazy_split('\n'))
        {
            std::string line;
            std::ranges::copy(lineView, std::back_inserter(line));
            if(line.empty())
            {
                continue;
            }
            std::optional<ID> srcID;
            for(const auto word : line | std::views::split(' '))
            {
                std::string strID;
                std::ranges::copy(word | std::views::filter([](const auto val) {
                                      return std::isalnum(val);
                                  }),
                                  std::back_inserter(strID));
                const auto curID = idAlloc.alloc(strID);
                if(srcID)
                {
                    graph[*srcID].insert(curID);
                    graph[curID].insert(*srcID);
                }
                else
                {
                    srcID = curID;
                }
            }
        }
        return graph;
    }
}
}

#endif
