#ifndef TASK_H
#define TASK_H

#include <iostream>
#include <iterator>
#include <cstddef>
#include <vector>
#include <optional>
#include <ranges>
#include <string_view>
#include <limits>
#include <concepts>
#include <utility>
#include <cassert>

namespace task
{
    using namespace std;

    enum class Operation
    {
        ACC,
        JMP,
        NOP
    };

    using ValueType = long long int;

    struct Instruction
    {
        Operation operation;
        ValueType argument;
    };

    template<signed_integral T>
    inline optional<T> parseValue(string_view s)
    {
        size_t sz = 0;
        const auto res = stoll(string(s), &sz);
        if(sz != s.size() ||
            res < numeric_limits<T>::min() || res > numeric_limits<T>::max())
        {
            return {};
        }
        return res;
    }

    inline optional<Instruction> parseInstruction(string_view line)
    {
        const auto sepPos = line.find(' ');
        if(sepPos == string_view::npos)
        {
            return {};
        }
        const auto operation =
            line.substr(0, sepPos);
        const auto argumentRes =
            parseValue<ValueType>(line.substr(sepPos+1));
        if(!argumentRes)
        {
            return {};
        }
        const auto argument = *argumentRes;
        if(operation == "acc")
        {
            return Instruction{Operation::ACC, argument};
        }
        if(operation == "jmp")
        {
            return Instruction{Operation::JMP, argument};
        }
        if(operation == "nop")
        {
            return Instruction{Operation::NOP, argument};
        }
        return {};
    }

    template<typename R>
        requires ranges::range<R> &&
            requires(R v) {
                {*begin(v)} -> convertible_to<string_view>;
            }
    optional<vector<Instruction>> parseInstructions(const R &lines)
    {
        vector<Instruction> instructions;
        for(const auto &line : lines)
        {
            const auto instructionRes = parseInstruction(line);
            if(!instructionRes)
            {
                return {};
            }
            instructions.push_back(*instructionRes);
        }
        return instructions;
    }

    class Machine
    {
    public:
        explicit Machine(const vector<Instruction> &instructions)
            :instructions(instructions)
        {}

        void run()
        {
            vector<bool> seen(instructions.size(), false);
            while(true)
            {
                if(instructionPointer_ < 0)
                {
                    break;
                }
                const size_t ip = instructionPointer_;
                if(ip >= instructions.size())
                {
                    break;
                }
                if(seen[ip])
                {
                    break;
                }
                seen[ip] = true;
                const auto &instruction = instructions[ip];
                ++instructionPointer_;
                exec(instruction);
            }
        }

        [[nodiscard]]
        ValueType accumulator() const
        {
            return accumulator_;
        }

        [[nodiscard]]
        size_t instructionPointer() const
        {
            return instructionPointer_;
        }

    private:
        void exec(const Instruction &instr)
        {
            switch(instr.operation)
            {
            case Operation::ACC:
                {
                    accumulator_ += instr.argument;
                    break;
                }
            case Operation::JMP:
                {
                    instructionPointer_ =
                        currentInstructionPointer() + instr.argument;
                    break;
                }
            case Operation::NOP:
                {
                    break;
                }
            default:
                {
                    assert(false);
                }
            }
        }

        [[nodiscard]]
        ptrdiff_t currentInstructionPointer() const
        {
            return instructionPointer_-1;
        }

    private:
        vector<Instruction> instructions;
        ptrdiff_t instructionPointer_ = 0;
        ValueType accumulator_ = 0;
    };
}

#endif
