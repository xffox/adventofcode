#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <memory>
#include <optional>
#include <utility>
#include <cassert>
#include <string_view>
#include <cctype>
#include <stack>
#include <tuple>
#include <unordered_map>

#include "../base.hpp"

namespace task
{
    using namespace std;

    using lint = long long int;

    enum class Operator
    {
        PLUS,
        MULT
    };

    using PriorityMap = unordered_map<Operator, int>;

    class Node
    {
    public:
        virtual ~Node() = default;

        virtual lint eval() const = 0;
    };

    class LeafNode: public Node
    {
    public:
        explicit LeafNode(lint value)
            :value(value)
        {}

        lint eval() const override
        {
            return value;
        }

    private:
        lint value;
    };

    class OperatorNode: public Node
    {
    public:
        OperatorNode(unique_ptr<Node> left, unique_ptr<Node> right,
            Operator op)
            :op(op), left(move(left)), right(move(right))
        {}

        lint eval() const override
        {
            assert(left);
            assert(right);
            const auto leftVal = left->eval();
            const auto rightVal = right->eval();
            switch(op)
            {
            case Operator::PLUS:
                return leftVal + rightVal;
            case Operator::MULT:
                return leftVal * rightVal;
            default:
                assert(false);
                return 0;
            }
        }

    private:
        Operator op;
        unique_ptr<Node> left;
        unique_ptr<Node> right;
    };

    namespace
    {
        struct Context
        {
            string_view expression;
            PriorityMap priorities;
        };

        void parseSpaces(size_t &pos,
            const Context &context)
        {
            while(pos < context.expression.size() &&
                isspace(context.expression[pos]))
            {
                ++pos;
            }
        }

        optional<Operator> parseOperator(size_t &pos,
            const Context &context)
        {
            if(pos == context.expression.size())
            {
                return {};
            }
            optional<Operator> res;
            switch(context.expression[pos])
            {
                case '+':
                    res = Operator::PLUS;
                    break;
                case '*':
                    res = Operator::MULT;
                    break;
            }
            if(res)
            {
                ++pos;
            }
            return res;
        }

        optional<lint> parseValue(size_t &pos,
            const Context &context)
        {
            if(pos < context.expression.size() &&
                isdigit(context.expression[pos]))
            {
                const auto valueStart = pos;
                ++pos;
                while(pos < context.expression.size() &&
                    isdigit(context.expression[pos]))
                {
                    ++pos;
                }
                return base::parseValue<lint>(
                    context.expression.substr(valueStart, pos - valueStart));
            }
            return nullopt;
        }

        optional<unique_ptr<Node>> parseSubexpression(
            size_t &pos, const Context &context);

        optional<unique_ptr<Node>> parseBracedExpression(
            size_t &pos, const Context &context)
        {
            parseSpaces(pos, context);
            if(pos == context.expression.size() ||
                context.expression[pos] != '(')
            {
                return {};
            }
            ++pos;
            auto res = parseSubexpression(pos, context);
            if(!res || pos == context.expression.size() ||
                context.expression[pos] != ')')
            {
                return {};
            }
            ++pos;
            return res;
        }

        optional<unique_ptr<Node>> parseNode(
            size_t &pos, const Context &context)
        {
            parseSpaces(pos, context);
            std::unique_ptr<Node> left;
            auto bracedExpressionRes =
                parseBracedExpression(pos, context);
            if(bracedExpressionRes)
            {
                return bracedExpressionRes;
            }
            const auto valueRes = parseValue(pos, context);
            if(valueRes)
            {
                return std::make_unique<LeafNode>(*valueRes);
            }
            return {};
        }

        optional<unique_ptr<Node>> parseSubexpression(
            size_t &pos, const Context &context)
        {
            static constexpr PriorityMap::mapped_type DEFAULT_PRIORITY = 0;
            auto lookupPriority = [&context](auto op){
                return base::lookupWithDefault(
                    context.priorities, op, DEFAULT_PRIORITY);
            };
            using ValStack = stack<unique_ptr<Node>>;
            using OpStack = stack<Operator>;
            auto leftRes = parseNode(pos, context);
            if(!leftRes)
            {
                return {};
            }
            ValStack valStack;
            OpStack opStack;
            valStack.push(move(*leftRes));
            auto evalStack = [&valStack, &opStack](){
                assert(!opStack.empty());
                assert(!valStack.empty());
                auto right = move(valStack.top());
                valStack.pop();
                assert(!valStack.empty());
                auto left = move(valStack.top());
                valStack.pop();
                const auto curOp = opStack.top();
                opStack.pop();
                valStack.push(
                    make_unique<OperatorNode>(
                        move(left),
                        move(right),
                        curOp));
            };
            while(true)
            {
                parseSpaces(pos, context);
                const auto operatorRes = parseOperator(pos, context);
                if(!operatorRes)
                {
                    break;
                }
                auto rightRes = parseNode(pos, context);
                if(!rightRes)
                {
                    return {};
                }
                const auto op = *operatorRes;
                const auto opPriority = lookupPriority(op);
                while(!opStack.empty())
                {
                    const auto curOp = opStack.top();
                    if(lookupPriority(curOp) < opPriority)
                    {
                        break;
                    }
                    evalStack();
                }
                valStack.push(move(*rightRes));
                opStack.push(op);
            }
            while(!opStack.empty())
            {
                evalStack();
            }
            assert(valStack.size() == 1);
            return move(valStack.top());
        }
    }

    inline optional<unique_ptr<Node>> parseExpression(string_view expression,
        const PriorityMap &priorities = {})
    {
        size_t pos = 0;
        auto res = parseSubexpression(pos,
            Context{expression, priorities});
        if(!res || pos != expression.size())
        {
            return {};
        }
        return res;
    }

    inline lint eval(const Node &node)
    {
        return node.eval();
    }
}

#endif
