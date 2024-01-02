#include <algorithm>
#include <array>
#include <format>
#include <functional>
#include <iostream>
#include <iterator>
#include <map>
#include <queue>
#include <ranges>
#include <stack>
#include <stdexcept>
#include <tuple>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../../matrix.hpp"
#include "../task.hpp"

namespace
{
using Edge = std::tuple<task::ID, task::ID>;

using EdgeCountMap = numutil::Matrix<task::uval>;

constexpr Edge normalizeID(const Edge &id)
{
    if(std::get<0>(id) >= std::get<1>(id))
    {
        return id;
    }
    return std::tuple{std::get<1>(id), std::get<0>(id)};
}

template<typename Func>
void traverseBreadth(const task::Graph &graph, const task::ID &src, Func &&func)
{
    std::queue<task::ID> front;
    std::vector<bool> seen(graph.size(), false);
    front.push(src);
    seen[src] = true;
    while(!front.empty())
    {
        const auto cur = std::move(front.front());
        front.pop();
        if(const auto iter = graph.find(cur); iter != end(graph))
        {
            for(const auto &nxt : iter->second)
            {
                auto &&curSeen = seen[nxt];
                if(!curSeen)
                {
                    curSeen = true;
                    func(cur, nxt);
                    front.push(nxt);
                }
            }
        }
    }
}

task::ulval solve(const task::Graph &graph)
{
    constexpr std::size_t EDGES = 3;
    const auto nodes = graph.size();
    EdgeCountMap counts(nodes, nodes);
    for(const auto &[node, connections] : graph)
    {
        std::vector<task::ulval> currentCounts(nodes, 1);
        std::stack<Edge> visits;
        traverseBreadth(graph, node, [&](const auto &cur, const auto &nxt) {
            visits.push(Edge{cur, nxt});
        });
        while(!visits.empty())
        {
            const auto [cur, nxt] = visits.top();
            visits.pop();
            const auto [fstID, sndID] = normalizeID(std::tuple{cur, nxt});
            const auto cnt = currentCounts[nxt];
            counts.ix(fstID, sndID) += cnt;
            currentCounts[cur] += cnt;
        }
    }
    std::map<task::ulval, Edge> selectedEdges;
    for(task::uval row = 0; row < nodes; ++row)
    {
        for(task::uval col = 0; col < row; ++col)
        {
            selectedEdges.emplace(counts.ix(row, col), Edge{row, col});
            if(selectedEdges.size() > EDGES)
            {
                selectedEdges.erase(begin(selectedEdges));
            }
        }
    }
    auto cutGraph = graph;
    for(const auto &[cnt, edge] : selectedEdges)
    {
        const auto &[fst, snd] = edge;
        cutGraph[fst].erase(snd);
        cutGraph[snd].erase(fst);
    }
    task::ulval fstCnt = 1;
    traverseBreadth(cutGraph, std::get<0>(begin(selectedEdges)->second),
                    [&fstCnt](const auto &, const auto &) { ++fstCnt; });
    const auto sndCnt = nodes - fstCnt;
    return fstCnt * sndCnt;
}
}

int main()
{
    const auto maybeGraph = task::input::parseGraph(std::cin);
    if(!maybeGraph)
    {
        throw std::unexpected(
            std::format("input parsing failed: {}", maybeGraph.error()));
    }
    std::cout << solve(*maybeGraph) << '\n';
    return 0;
}
