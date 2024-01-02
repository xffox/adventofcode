#ifndef TASK_H
#define TASK_H

#include <ranges>
#include <vector>
#include <variant>
#include <string_view>
#include <cassert>
#include <complex>
#include <numbers>

#include "../base.hpp"

namespace task
{
    using namespace std;

    namespace
    {
        constexpr double degreeToRadian(double val)
        {
            constexpr double PI_DEG = 180;
            return val/PI_DEG*numbers::pi_v<double>;
        }
    }

    enum class MoveType
    {
        NORTH,
        SOUTH,
        EAST,
        WEST,
        FORWARD
    };
    enum class RotateType
    {
        LEFT,
        RIGHT
    };
    struct Move
    {
        MoveType type;
        int dist;
    };
    struct Rotate
    {
        RotateType type;
        int degrees;
    };

    using Instruction = variant<Move, Rotate>;
    using Pos = complex<double>;

    struct State
    {
        Pos position;
        Pos direction;
    };

    template<ranges::view R>
    optional<vector<Instruction>> parse(R lines)
    {
        vector<Instruction> instructions;
        for(const auto &line : lines)
        {
            if(line.empty())
            {
                return {};
            }
            const auto valueRes = base::parseValue<int>(
                string_view(line).substr(1));
            if(!valueRes)
            {
                return {};
            }
            optional<Instruction> instr;
            switch(line[0])
            {
            case 'N':
                instr = Move{MoveType::NORTH, *valueRes};
                break;
            case 'S':
                instr = Move{MoveType::SOUTH, *valueRes};
                break;
            case 'E':
                instr = Move{MoveType::EAST, *valueRes};
                break;
            case 'W':
                instr = Move{MoveType::WEST, *valueRes};
                break;
            case 'F':
                instr = Move{MoveType::FORWARD, *valueRes};
                break;
            case 'L':
                instr = Rotate{RotateType::LEFT, *valueRes};
                break;
            case 'R':
                instr = Rotate{RotateType::RIGHT, *valueRes};
                break;
            default:
                return {};
            }
            assert(instr);
            instructions.push_back(*instr);
        }
        return instructions;
    }

    template<ranges::view R>
    State exec(const State &state, R instructions, bool moveWaypoint = false)
    {
        State cur(state);
        struct Visitor
        {
            explicit Visitor(State &state, bool moveWaypoint)
                :state(state), moveWaypoint(moveWaypoint)
            {}

            void operator()(const Move &move)
            {
                Pos dir(0);
                switch(move.type)
                {
                case MoveType::NORTH:
                    dir = Pos(0, 1);
                    break;
                case MoveType::SOUTH:
                    dir = Pos(0, -1);
                    break;
                case MoveType::WEST:
                    dir = Pos(-1, 0);
                    break;
                case MoveType::EAST:
                    dir = Pos(1, 0);
                    break;
                case MoveType::FORWARD:
                    state.position += state.direction*Pos(move.dist);
                    break;
                default:
                    assert(false);
                    break;
                }
                if(moveWaypoint)
                {
                    state.direction += dir*Pos(move.dist);
                }
                else
                {
                    state.position += dir*Pos(move.dist);
                }
            }

            void operator()(const Rotate &rotate)
            {
                switch(rotate.type)
                {
                case RotateType::LEFT:
                    state.direction *=
                        polar(static_cast<double>(1),
                            degreeToRadian(rotate.degrees));
                    break;
                case RotateType::RIGHT:
                    state.direction *=
                        polar(static_cast<double>(1),
                            -degreeToRadian(rotate.degrees));
                    break;
                default:
                    assert(false);
                    break;
                }
            }

            State &state;
            bool moveWaypoint;
        };
        Visitor visitor(cur, moveWaypoint);
        for(const auto &instr : instructions)
        {
            visit(visitor, instr);
        }
        return visitor.state;
    }
}

#endif
