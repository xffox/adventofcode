#include <algorithm>
#include <array>
#include <cassert>
#include <cstddef>
#include <format>
#include <iostream>
#include <iterator>
#include <map>
#include <numeric>
#include <ranges>
#include <set>
#include <stdexcept>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../../base.hpp"
#include "../task.hpp"

namespace
{
template<typename R>
task::ulval commonCycle(const R &counters)
{
    return std::ranges::fold_left(counters, static_cast<task::ulval>(1),
                                  [](const auto res, const auto val) {
                                      const task::ulval g = std::gcd(res, val);
                                      return g * (res / g) * (val / g);
                                  });
}

template<typename NodesHandler>
void traverseComponents(const task::network::Network &network,
                        NodesHandler handler)
{
    std::unordered_set<task::input::Name> statefulNodes;
    network.traverse(
        [&](const auto &descr, const auto &, const auto &maybeSignal) {
            if(maybeSignal)
            {
                statefulNodes.insert(descr.name);
            }
        });
    std::unordered_map<task::input::Name, std::unordered_set<task::input::Name>>
        components;
    while(!statefulNodes.empty())
    {
        auto iter = begin(statefulNodes);
        auto &curComponent = components[*iter];
        curComponent.insert(*iter);
        network.traverse(
            [&](const auto &descr, const auto &, const auto &maybeSignal) {
                if(descr.name == *iter || !maybeSignal)
                {
                    return true;
                }
                statefulNodes.erase(descr.name);
                if(const auto prevIter = components.find(descr.name);
                   prevIter != end(components))
                {
                    curComponent.insert(begin(prevIter->second),
                                        end(prevIter->second));
                    components.erase(prevIter);
                    return false;
                }
                curComponent.insert(descr.name);
                return true;
            },
            *iter);
        statefulNodes.erase(iter);
    }
    for(const auto &[head, component] : components)
    {
        handler(component);
    }
}
}

int main()
{
    const auto maybeConfiguration = task::input::parseConfiguration(std::cin);
    if(!maybeConfiguration)
    {
        throw std::runtime_error(std::format("input parsing failed: {}",
                                             maybeConfiguration.error()));
    }
    const std::string rootName("broadcaster");
    task::network::Network network(*maybeConfiguration, rootName);
    std::vector<std::vector<task::input::Name>> counters;
    traverseComponents(network, [&](const auto &nodes) {
        auto &counter = counters.emplace_back();
        for(const auto &name : nodes)
        {
            counter.push_back(name);
        }
    });
    std::vector<task::ulval> prevValues(counters.size());
    std::vector<std::optional<task::ulval>> cycleSizes(counters.size());
    const std::string targeName = "rx";
    const auto extractState = [](const auto &values, const auto &names) {
        std::string str;
        for(const auto &name : names)
        {
            str.push_back(values.at(std::string(name)) ? '1' : '0');
        }
        return *base::parseValue<task::ulval>(str, 2);
    };
    for(task::ulval cnt = 0;; ++cnt)
    {
        network.exec(task::network::Signal::LOW,
                     [&](const auto &, const auto) { return true; });
        std::map<std::string, int> values;
        network.traverse(
            [&](const auto &cur, const auto &, const auto maybeSignal) {
                if(maybeSignal)
                {
                    assert(maybeSignal);
                    values[cur.name] = std::to_underlying(*maybeSignal);
                }
            });
        for(const auto &[idx, counter] : std::views::enumerate(counters))
        {
            const auto state = extractState(values, counter);
            if(!state)
            {
                auto &curCycle = cycleSizes[idx];
                if(!curCycle)
                {
                    curCycle = cnt + 1;
                }
            }
            prevValues[idx] = state;
        }
        if(std::ranges::all_of(cycleSizes,
                               [](const auto &val) { return val.has_value(); }))
        {
            break;
        }
    }
    std::cout << commonCycle(cycleSizes |
                             std::views::transform(
                                 [](const auto &val) { return *val; }))
              << '\n';
    return 0;
}
