#ifndef TASK_HPP
#define TASK_HPP

#include <algorithm>
#include <cassert>
#include <expected>
#include <format>
#include <functional>
#include <istream>
#include <iterator>
#include <memory>
#include <optional>
#include <queue>
#include <ranges>
#include <string>
#include <unordered_map>
#include <unordered_set>
#include <utility>
#include <vector>

#include "../parse_stream.hpp"

namespace task
{
using uval = unsigned int;
using ulval = unsigned long long int;

namespace input
{
    using Name = std::string;

    enum class ModuleType
    {
        UNKNOWN,
        BROADCASTER,
        FLIP_FLOP,
        CONJUNCTION,
    };

    struct Module
    {
        Name name{};
        ModuleType type{};
    };

    struct ConnectedModule
    {
        Module module{};
        std::vector<Name> destinations{};
    };

    struct Configuration
    {
        std::vector<ConnectedModule> modules;
    };

    namespace detail
    {
        template<typename T>
        using ParseResult = std::expected<T, std::string>;

        inline ParseResult<std::string> parseName(parser::ParseStream &stream)
        {
            std::string result;
            while(const auto maybeVal = stream.peek())
            {
                if(!std::isalpha(*maybeVal))
                {
                    break;
                }
                result.push_back(*maybeVal);
                stream.read();
            }
            if(result.empty())
            {
                return std::unexpected("empty name");
            }
            return result;
        }

        inline ParseResult<Module> parseBroadcasterModule(
            parser::ParseStream &stream)
        {
            auto maybeName = parseName(stream);
            if(!maybeName)
            {
                return std::unexpected(
                    std::format("broadcaster module name parsing failed: {}",
                                maybeName.error()));
            }
            return Module{
                .name = std::move(*maybeName),
                .type = ModuleType::BROADCASTER,
            };
        }

        inline ParseResult<ModuleType> parseModuleType(
            parser::ParseStream &stream)
        {
            const auto maybeType = stream.read();
            if(!maybeType)
            {
                return std::unexpected("no module type");
            }
            switch(*maybeType)
            {
            case '%':
                return ModuleType::FLIP_FLOP;
            case '&':
                return ModuleType::CONJUNCTION;
            default:
                return std::unexpected(
                    std::format("invalid module type: {}", *maybeType));
            }
        }

        inline ParseResult<Module> parseRegularModule(
            parser::ParseStream &stream)
        {
            const auto maybeType = parseModuleType(stream);
            if(!maybeType)
            {
                return std::unexpected(std::format(
                    "module type parsing failed: {}", maybeType.error()));
            }
            auto maybeName = parseName(stream);
            if(!maybeName)
            {
                return std::unexpected(std::format(
                    "module name parsing failed: {}", maybeName.error()));
            }
            return Module{
                .name = std::move(*maybeName),
                .type = *maybeType,
            };
        }

        inline ParseResult<Module> parseModule(parser::ParseStream &stream)
        {
            {
                auto broadcasterModuleStream = stream;
                if(auto maybeBroadcasterModule =
                       parseBroadcasterModule(broadcasterModuleStream))
                {
                    stream = broadcasterModuleStream;
                    return maybeBroadcasterModule;
                }
            }
            auto maybeModule = parseRegularModule(stream);
            if(!maybeModule)
            {
                return std::unexpected(std::format("module parsing failed: {}",
                                                   maybeModule.error()));
            }
            return maybeModule;
        }

        inline void parseSpaces(parser::ParseStream &stream)
        {
            while(const auto maybeSpace = stream.peek())
            {
                if(*maybeSpace != ' ')
                {
                    break;
                }
                stream.read();
            }
        }

        inline bool parseCharacter(parser::ParseStream &stream, const char val)
        {
            return stream.read() == val;
        }

        inline ParseResult<std::vector<Name>> parseDestinationNames(
            parser::ParseStream &stream)
        {
            std::vector<Name> names;
            while(true)
            {
                auto maybeName = parseName(stream);
                if(!maybeName)
                {
                    return std::unexpected(
                        std::format("destination name parsing failed: {}",
                                    maybeName.error()));
                }
                names.push_back(std::move(*maybeName));
                {
                    auto sepStream = stream;
                    if(const auto maybeSep = sepStream.read(); maybeSep != ',')
                    {
                        break;
                    }
                    stream = sepStream;
                }
                parseSpaces(stream);
            }
            return names;
        }

        inline ParseResult<ConnectedModule> parseConnectedModule(
            parser::ParseStream &stream)
        {
            auto maybeModule = parseModule(stream);
            if(!maybeModule)
            {
                return std::unexpected(std::format("module parsing failed: {}",
                                                   maybeModule.error()));
            }
            parseSpaces(stream);
            parseCharacter(stream, '-');
            parseCharacter(stream, '>');
            parseSpaces(stream);
            auto maybeDestinationNames = parseDestinationNames(stream);
            if(!maybeDestinationNames)
            {
                return std::unexpected(
                    std::format("destination names parsing failed: {}",
                                maybeDestinationNames.error()));
            }
            if(stream.peek().has_value())
            {
                return std::unexpected("extra characters at the end");
            }
            return ConnectedModule{.module = std::move(*maybeModule),
                                   .destinations =
                                       std::move(*maybeDestinationNames)};
        }
    }

    inline std::expected<Configuration, std::string> parseConfiguration(
        std::istream &stream)
    {
        Configuration configuration{};
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
            parser::ParseStream stream(line);
            auto maybeModule = detail::parseConnectedModule(stream);
            if(!maybeModule)
            {
                return std::unexpected(
                    std::format("connected module parsing failed: {}",
                                maybeModule.error()));
            }
            configuration.modules.push_back(std::move(*maybeModule));
        }
        return configuration;
    }
}

namespace network
{
    enum class Signal
    {
        LOW = 0,
        HIGH,
    };

    constexpr Signal flip(Signal signal)
    {
        return Signal{!std::to_underlying(signal)};
    }

    namespace detail
    {
        class Module;
        using ModuleID = const Module *;

        class Module
        {
        public:
            virtual ~Module() = default;

            virtual void registerSource(ModuleID sourceID) = 0;
            virtual std::optional<Signal> process(Signal signal,
                                                  ModuleID sourceID) = 0;
            virtual std::optional<Signal> state() const = 0;
        };

        class BroadcasterModule : public Module
        {
        public:
            void registerSource(ModuleID) override
            {
            }
            std::optional<Signal> process(Signal signal, ModuleID) override
            {
                return signal;
            }
            std::optional<Signal> state() const override
            {
                return std::nullopt;
            }
        };

        class FlipFlopModule : public Module
        {
        public:
            void registerSource(ModuleID) override
            {
            }
            std::optional<Signal> process(Signal signal, ModuleID) override
            {
                if(signal == Signal::LOW)
                {
                    this->signal = flip(this->signal);
                    return this->signal;
                }
                return std::nullopt;
            }
            std::optional<Signal> state() const override
            {
                return signal;
            }

        private:
            Signal signal{Signal::LOW};
        };

        class ConjunctionModule : public Module
        {
        public:
            void registerSource(ModuleID sourceID) override
            {
                signals.emplace(sourceID, Signal::LOW);
            }
            std::optional<Signal> process(Signal signal,
                                          ModuleID sourceID) override
            {
                signals[sourceID] = signal;
                return std::ranges::all_of(
                           signals | std::views::transform([](const auto &p) {
                               return p.second;
                           }),
                           [](const auto s) { return s == Signal::HIGH; })
                           ? Signal::LOW
                           : Signal::HIGH;
            }
            std::optional<Signal> state() const override
            {
                return std::nullopt;
            }

        private:
            std::unordered_map<ModuleID, Signal> signals;
        };
    }

    class Network
    {
    public:
        struct NodeDescr
        {
            input::Name name{};
            input::ModuleType type{};
        };

    private:
        struct Node
        {
            NodeDescr descr;
            std::unique_ptr<detail::Module> module;
            std::vector<std::shared_ptr<Node>> children{};
        };

        using ModuleMap = std::unordered_map<
            std::string_view,
            std::reference_wrapper<const input::ConnectedModule>>;
        using NodeMap = std::unordered_map<input::Name, std::shared_ptr<Node>>;

    public:
        explicit Network(const input::Configuration &configuration,
                         const input::Name &rootName)
        {
            ModuleMap namedModules;
            std::ranges::copy(configuration.modules |
                                  std::views::transform([](const auto &module) {
                                      return std::pair{
                                          std::string_view(module.module.name),
                                          std::cref(module)};
                                  }),
                              std::inserter(namedModules, end(namedModules)));
            root = prepareNode(moduleNodes, rootName, namedModules);
        }

        template<typename SignalHandler>
        void exec(Signal signal, SignalHandler handler) const
        {
            std::queue<
                std::tuple<std::shared_ptr<Node>, Signal, detail::ModuleID>>
                propagationQueue;
            propagationQueue.push({
                root,
                signal,
                nullptr,
            });
            while(!propagationQueue.empty())
            {
                auto [node, signal, sourceID] = propagationQueue.front();
                propagationQueue.pop();
                assert(node);
                if(!handler(node->descr, signal))
                {
                    continue;
                }
                if(node->module)
                {
                    const auto maybeNextSignal =
                        node->module->process(signal, sourceID);
                    if(maybeNextSignal)
                    {
                        for(const auto &child : node->children)
                        {
                            propagationQueue.push(
                                {child, *maybeNextSignal, node->module.get()});
                        }
                    }
                }
            }
        }

        template<typename NodeHandler>
        void traverse(
            NodeHandler handler,
            const std::optional<input::Name> &startModuleName = {}) const
        {
            auto cur = root;
            if(startModuleName)
            {
                if(const auto iter = moduleNodes.find(*startModuleName);
                   iter != end(moduleNodes))
                {
                    cur = iter->second;
                }
                else
                {
                    return;
                }
            }
            traverse(cur, handler);
        }

        std::optional<Signal> state(const input::Name &moduleName) const
        {
            const auto iter = moduleNodes.find(moduleName);
            if(iter == end(moduleNodes) || !iter->second->module)
            {
                return std::nullopt;
            }
            return iter->second->module->state();
        }

    private:
        static std::shared_ptr<Node> prepareNode(NodeMap &nodes,
                                                 const input::Name &name,
                                                 const ModuleMap &namedModules)
        {
            if(const auto iter = nodes.find(name); iter != end(nodes))
            {
                return iter->second;
            }
            const auto moduleIter = namedModules.find(name);
            auto resultModule = [&]() -> std::unique_ptr<detail::Module> {
                if(moduleIter == end(namedModules))
                {
                    return {};
                }
                const auto &module = moduleIter->second.get();
                switch(module.module.type)
                {
                case input::ModuleType::UNKNOWN:
                    return {};
                case input::ModuleType::BROADCASTER:
                    return std::make_unique<detail::BroadcasterModule>();
                case input::ModuleType::FLIP_FLOP:
                    return std::make_unique<detail::FlipFlopModule>();
                case input::ModuleType::CONJUNCTION:
                    return std::make_unique<detail::ConjunctionModule>();
                }
                std::unreachable();
            }();
            auto node = std::make_shared<Node>(
                NodeDescr{
                    .name = name,
                },
                std::move(resultModule));
            nodes.emplace(name, node);
            if(moduleIter != end(namedModules))
            {
                node->descr.type = moduleIter->second.get().module.type;
                for(const auto &dst : moduleIter->second.get().destinations)
                {
                    auto &child = node->children.emplace_back(
                        prepareNode(nodes, dst, namedModules));
                    assert(child);
                    if(child->module)
                    {
                        child->module->registerSource(node->module.get());
                    }
                }
            }
            return node;
        }

        template<typename NodeHandler>
        static void traverse(const std::shared_ptr<Node> root,
                             NodeHandler handler)
        {
            std::queue<std::shared_ptr<Node>> front;
            std::unordered_set<const Node *> seen{root.get()};
            front.push(root);
            while(!front.empty())
            {
                auto cur = front.front();
                front.pop();
                assert(cur);
                std::vector<NodeDescr> childrenDescrs;
                for(const auto &child : cur->children)
                {
                    childrenDescrs.push_back(child->descr);
                    if(seen.insert(child.get()).second)
                    {
                        front.push(child);
                    }
                }
                handler(cur->descr, childrenDescrs,
                        cur->module ? cur->module->state() : std::nullopt);
            }
        }

        NodeMap moduleNodes;
        std::shared_ptr<Node> root{};
    };
}
}

#endif
