#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <cassert>
#include <optional>
#include <array>

#include "../task.hpp"

using namespace std;

namespace
{
    optional<task::Instruction> tryFixInstruction(
        const task::Instruction &instr)
    {
        constexpr auto FIXES = to_array<task::Operation>({
            task::Operation::JMP,
            task::Operation::NOP
        });
        for(size_t i = 0; i < FIXES.size(); ++i)
        {
            if(instr.operation == FIXES[i])
            {
                return task::Instruction{
                    FIXES[(i+1)%FIXES.size()],
                    instr.argument
                };
            }
        }
        return {};
    }

    optional<task::ValueType> tryFinish(
        const vector<task::Instruction> &instructions)
    {
        task::Machine machine(instructions);
        machine.run();
        if(machine.instructionPointer() == instructions.size())
        {
            return machine.accumulator();
        }
        return {};
    }
}

int main()
{
    vector<string> lines;
    while(cin)
    {
        string line;
        getline(cin, line);
        if(!line.empty())
        {
            lines.push_back(move(line));
        }
    }
    const auto instructionsRes = task::parseInstructions(lines);
    assert(instructionsRes);
    auto instructions = *instructionsRes;
    {
        const auto res = tryFinish(instructions);
        if(res)
        {
            cout<<*res<<endl;
            return 0;
        }
    }
    for(size_t i = 0; i < instructions.size(); ++i)
    {
        const auto prev = instructions[i];
        const auto fix = tryFixInstruction(instructions[i]);
        if(fix)
        {
            instructions[i] = *fix;
            const auto res = tryFinish(instructions);
            if(res)
            {
                cout<<*res<<endl;
                return 0;
            }
            instructions[i] = prev;
        }
    }
    return 0;
}
