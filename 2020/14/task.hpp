#ifndef TASK_H
#define TASK_H

#include <vector>
#include <optional>
#include <iterator>
#include <string_view>
#include <regex>
#include <ranges>
#include <unordered_map>
#include <variant>
#include <climits>

#include "../base.hpp"

namespace task
{
    using namespace std;

    using uint = unsigned int;
    using ulint = unsigned long long int;

    struct Mem
    {
        uint addr;
        ulint value;
    };

    struct Mask
    {
        ulint select;
        ulint value;
    };

    using Operation = variant<Mask, Mem>;

    struct Program
    {
        vector<Operation> operations;
    };

    template<ranges::range R>
    optional<Mask> parseMask(R str)
    {
        static const regex MASK_REGEX("mask = ([X01]+)");
        smatch matches;
        string val;
        ranges::copy(str, back_inserter(val));
        if(!regex_match(val, matches, MASK_REGEX))
        {
            return {};
        }
        ulint select = 0;
        ulint value = 0;
        for(auto c : matches.str(1))
        {
            select *= 2;
            value *= 2;
            if(c != 'X')
            {
                value += (c == '1');
            }
            else
            {
                select += 1;
            }
        }
        return Mask{~select, value};
    }

    template<ranges::range R>
    optional<Mem> parseMem(R str)
    {
        static const regex MEM_REGEX("mem\\[([0-9]+)\\] = ([0-9]+)");
        smatch matches;
        string val;
        ranges::copy(str, back_inserter(val));
        if(!regex_match(val, matches, MEM_REGEX))
        {
            return {};
        }
        const auto addRes = base::parseValue<uint>(matches.str(1));
        const auto valRes = base::parseValue<ulint>(matches.str(2));
        if(!addRes || !valRes)
        {
            return {};
        }
        return Mem{*addRes, *valRes};
    }

    inline optional<Program> parseProgram(string_view str)
    {
        auto lines = str | ranges::views::split('\n');
        vector<Operation> operations;
        for(auto line : lines)
        {
            const auto maskRes = parseMask(line);
            if(maskRes)
            {
                operations.push_back(*maskRes);
                continue;
            }
            const auto memRes = parseMem(line);
            if(memRes)
            {
                operations.push_back(*memRes);
                continue;
            }
            return {};
        }
        return Program{operations};
    }

    constexpr uint countBits(ulint val)
    {
        uint res = 0;
        while(val)
        {
            res += 1;
            val = (val & (val-1));
        }
        return res;
    }

    template<class Spec>
    class ProcBase
    {
    public:
        using Memory = unordered_map<ulint, ulint>;

    public:
        explicit ProcBase(const Program &program)
            :program(program)
        {}

        Memory exec() const
        {
            Memory mem;
            Visitor visitor(mem);
            for(const auto &op : program.operations)
            {
                visit(visitor, op);
            }
            return mem;
        }

    private:
        class Visitor
        {
        public:
            explicit Visitor(Memory &mem)
                :mem(mem)
            {}

            void operator()(const Mask &mask)
            {
                this->mask = mask;
            }

            void operator()(const Mem &write)
            {
                Spec::write(mem, mask, write);
            }

        private:
            Memory &mem;
            Mask mask = {0, 0};
        };

    private:
        Program program;
    };

    class SpecV1
    {
        template<typename V>
        friend class ProcBase;
    private:
        SpecV1() = delete;

        static void write(ProcBase<void>::Memory &mem,
            const Mask &mask, const Mem &write)
        {
            mem[write.addr] =
                (write.value&(~mask.select)) |
                mask.value;
        }
    };

    class SpecV2
    {
        template<typename V>
        friend class ProcBase;
    private:
        SpecV2() = delete;

        static void write(ProcBase<void>::Memory &mem,
            const Mask &mask, const Mem &write)
        {
            const auto select = ~mask.select;
            constexpr auto MASK_SZ = (CHAR_BIT*sizeof(ulint)-1);
            constexpr ulint lastBit = 1UL<<(MASK_SZ-1);
            const auto bits = countBits(select);
            const ulint rng = 1UL<<bits;
            for(ulint v = 0; v < rng; ++v)
            {
                ulint setter = 0;
                auto curVal = v;
                auto sel = select;
                for(uint i = 0; i < MASK_SZ; ++i, sel<<=1)
                {
                    setter<<=1;
                    if(sel&lastBit)
                    {
                        setter += curVal%2;
                        curVal /= 2;
                    }
                }
                const auto addr =
                    ((write.addr | mask.value)&(~select)) | setter;
                mem[addr] = write.value;
            }
        }
    };

    using Proc = ProcBase<SpecV1>;
    using ProcV2 = ProcBase<SpecV2>;
}

#endif
