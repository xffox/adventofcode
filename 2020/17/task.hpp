#ifndef TASK_H
#define TASK_H

#include <cstddef>
#include <functional>
#include <unordered_set>
#include <utility>
#include <string>
#include <ranges>
#include <array>

namespace task
{
    template<std::size_t N>
    using Pos = std::array<int, N>;
}

namespace std
{
    template<std::size_t N>
    class hash<task::Pos<N>>
    {
    public:
        std::size_t operator()(const task::Pos<N> &p) const noexcept
        {
            return go<N>(p, 0);
        }

    private:
        using Hash = std::hash<int>;

    private:
        template<std::size_t I>
        static std::size_t go(const task::Pos<N> &p, std::size_t prev) noexcept
        {
            if constexpr(I > 0)
            {
                return go<I-1>(p, Hash{}(p[I-1])^(prev<<1));
            }
            return prev;
        }
    };
}

namespace task
{
    using namespace std;

    using uint = unsigned int;
    template<size_t N>
    using Board = unordered_set<Pos<N>>;

    template<size_t N_>
    class CubeEval
    {
    public:
        using BoardType = Board<N_>;
        static constexpr auto N = N_;

    public:
        explicit CubeEval(const BoardType &board)
            :board_(board)
        {}

        void step()
        {
            constexpr uint KEEP_COUNT = 2;
            constexpr uint ACTIVATE_COUNT = 3;
            BoardType nextBoard;
            NeighbourCountMap counts;
            for(const auto &pos : board_)
            {
                countNeighbours(counts, pos);
            }
            for(const auto &p : counts)
            {
                if((p.second == KEEP_COUNT && board_.contains(p.first)) ||
                    (p.second == ACTIVATE_COUNT))
                {
                    nextBoard.insert(p.first);
                }
            }
            board_ = move(nextBoard);
        }

        const BoardType &board() const
        {
            return board_;
        }

    private:
        using NeighbourCountMap = std::unordered_map<Pos<N>, uint>;

    private:
        template<typename... Ds>
        void countNeighbours(NeighbourCountMap &counts,
            const Pos<N> &pos, Ds... ds)
        {
            constexpr auto I = sizeof...(Ds);
            if constexpr(I < N)
            {
                for(int d = -1; d <= 1; ++d)
                {
                    countNeighbours(counts, pos, ds..., d);
                }
            }
            else
            {
                if(!((ds == 0) && ...))
                {
                    [&counts, &pos, ds...]<size_t... Is>(
                        std::index_sequence<Is...>){
                        const Pos<N> np{(pos[Is]+ds)...};
                        counts[np] += 1;
                    }(std::make_index_sequence<N>{});
                }
            }
        }

    private:
        BoardType board_;
    };

    template<size_t N>
    inline optional<Board<N>> parseBoard(const string &board)
    {
        int y = 0;
        Board<N> result;
        for(auto &&line : board | ranges::views::split('\n'))
        {
            int x = 0;
            for(auto c : line)
            {
                if(c == '#')
                {
                    result.insert(Pos<N>{x, y});
                }
                else if(c != '.')
                {
                    return {};
                }
                ++x;
            }
            ++y;
        }
        return result;
    }
}

#endif
