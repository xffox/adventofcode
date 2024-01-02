#ifndef GRAPHEXPORT_HPP
#define GRAPHEXPORT_HPP

#include <array>
#include <cstddef>
#include <format>
#include <iterator>
#include <ostream>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>

namespace graphutil
{
class GraphExporter
{
public:
    void addNode(std::string node, std::string type)
    {
        nodes.emplace(std::move(node), std::move(type));
    }
    void addEdge(std::string from, std::string to)
    {
        edges[std::move(from)].insert(std::move(to));
    }

    // TODO: proper escapes
    void dump(std::ostream &stream) const
    {
        std::unordered_map<std::string, std::size_t> shapeTypes;
        const auto prepareShape = [&shapeTypes](const auto &type) {
            auto iter = shapeTypes.find(type);
            if(iter == end(shapeTypes))
            {
                iter = shapeTypes.emplace(type, shapeTypes.size()).first;
            }
            if(iter->second < SHAPES.size())
            {
                return SHAPES[iter->second];
            }
            return "note";
        };
        stream << "digraph name {\n";
        for(const auto &[node, type] : nodes)
        {
            const auto shape = prepareShape(type);
            stream << std::format("{} [label=\"{}\" shape=\"{}\"];\n", node,
                                  node, shape);
            if(const auto iter = edges.find(node); iter != end(edges))
            {
                for(const auto &child : iter->second)
                {
                    stream << std::format("{} -> {};\n", node, child);
                }
            }
        }
        stream << "}\n";
    }

private:
    static constexpr auto SHAPES = std::to_array({
        "box",
        "oval",
        "diamond",
        "triangle",
    });

    std::unordered_map<std::string, std::unordered_set<std::string>> edges;
    std::unordered_map<std::string, std::string> nodes;
};
}

#endif
