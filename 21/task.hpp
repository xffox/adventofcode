#ifndef TASK_H
#define TASK_H

#include <algorithm>
#include <optional>
#include <regex>
#include <vector>
#include <string>
#include <ranges>
#include <iterator>
#include <cstddef>
#include <unordered_set>
#include <unordered_map>
#include <set>
#include <tuple>
#include <utility>
#include <cassert>
#include <stdexcept>

namespace task
{
    using namespace std;

    using StringCol = vector<string>;
    using Food = tuple<StringCol, StringCol>;

    namespace inner
    {
        inline optional<Food> parseFood(const string &food)
        {
            static const regex FOOD_REGEX(
                "(([a-zA-Z]+ ?)+) \\(contains (([a-zA-Z]+(, )?)+)\\)");
            smatch matches;
            if(!regex_match(food, matches, FOOD_REGEX))
            {
                return {};
            }
            StringCol ingridients;
            const auto ingridientsStr = matches.str(1);
            const auto allergensStr = matches.str(3);
            ranges::copy(ingridientsStr
                | ranges::views::split(' ')
                | ranges::views::transform([](auto &&ingridient){
                        string ingridientStr;
                        ranges::copy(ingridient, back_inserter(ingridientStr));
                        return ingridientStr;
                    }),
                back_inserter(ingridients));
            StringCol allergens;
            ranges::copy(allergensStr
                | ranges::views::split(',')
                | ranges::views::transform([](auto &&allergen){
                        string allergenStr;
                        ranges::copy(allergen, back_inserter(allergenStr));
                        erase(allergenStr, ' ');
                        return allergenStr;
                    }),
                back_inserter(allergens));
            return make_tuple(move(ingridients), move(allergens));
        }
    }

    inline optional<vector<Food>> parseFoodList(
        const string &foodList)
    {
        vector<Food> result;
        for(auto &&line : foodList | ranges::views::split('\n'))
        {
            string foodStr;
            ranges::copy(line, back_inserter(foodStr));
            auto foodRes = inner::parseFood(foodStr);
            if(!foodRes)
            {
                return {};
            }
            result.push_back(move(*foodRes));
        }
        return result;
    }

    namespace inner
    {
        using Cand = tuple<vector<unordered_set<size_t>>, size_t>;

        inline Cand invertCand(const Cand &candidates)
        {
            const auto &[cands, sz] = candidates;
            Cand result(tuple_element<0, Cand>::type{sz}, cands.size());
            for(size_t i = 0; i < cands.size(); ++i)
            {
                const auto &cand = cands[i];
                for(auto c : cand)
                {
                    get<0>(result)[c].insert(i);
                }
            }
            return result;
        }

        inline void cleanUnique(Cand &candidates)
        {
            auto &[cands, sz] = candidates;
            unordered_set<size_t> solvedFields;
            for(size_t i = 0; i < cands.size(); ++i)
            {
                const auto &cand = cands[i];
                if(cand.size() == 1)
                {
                    solvedFields.insert(*begin(cand));
                }
            }
            for(auto &cand : cands)
            {
                if(cand.size() > 1)
                {
                    for(auto field : solvedFields)
                    {
                        cand.erase(field);
                    }
                }
            }
        }

        inline Cand solve(const Cand &candidates)
        {
            auto cands = candidates;
            unordered_set<size_t> solvedFields;
            bool newSolved = false;
            do
            {
                cleanUnique(cands);
                auto invertedCands = invertCand(cands);
                cleanUnique(invertedCands);
                cands = invertCand(invertedCands);
                newSolved = false;
                for(size_t i = 0; i < get<0>(cands).size(); ++i)
                {
    //                if(cands[i].empty())
    //                {
    //                    return {};
    //                }
                    if(get<0>(cands)[i].size() <= 1)
                    {
                        newSolved = newSolved || (solvedFields.insert(i).second);
                    }
                }
            }
            while(newSolved);
            return cands;
        }

        class Numerator
        {
        public:
            void add(const string &val)
            {
                assert(idxToValue.size() == valueToIdx.size());
                if(valueToIdx.insert(make_pair(val, idx)).second)
                {
                    idxToValue.push_back(val);
                    ++idx;
                }
            }

            const auto &fromIdx(size_t i) const
            {
                if(i >= idxToValue.size())
                {
                    throw invalid_argument("");
                }
                return idxToValue[i];
            }

            size_t fromValue(const string &val) const
            {
                auto iter = valueToIdx.find(val);
                if(iter == end(valueToIdx))
                {
                    throw invalid_argument("");
                }
                return iter->second;
            }

            size_t size() const
            {
                return idx;
            }

        private:
            size_t idx = 0;
            vector<string> idxToValue{};
            unordered_map<string, size_t> valueToIdx{};
        };
    }

    inline auto findIngridients(
        const vector<Food> &foodList)
    {
        inner::Numerator ingridientIndices;
        inner::Numerator allergenIndices;
        for(const auto &food : foodList)
        {
            ranges::for_each(get<0>(food), [&ingridientIndices](const auto &v){
                    ingridientIndices.add(v);
                });
            ranges::for_each(get<1>(food), [&allergenIndices](const auto &v){
                    allergenIndices.add(v);
                });
        }
        vector<set<size_t>> seen;
        {
            set<size_t> allCands;
            ranges::copy(ranges::views::iota(
                    static_cast<size_t>(0), ingridientIndices.size()),
                inserter(allCands, end(allCands)));
            for(size_t i = 0; i < allergenIndices.size(); ++i)
            {
                seen.push_back(allCands);
            }
        }
        for(const auto &food : foodList)
        {
            for(const auto &allergen : get<1>(food))
            {
                const auto allergenIdx = allergenIndices.fromValue(allergen);
                auto &ingridientSeen = seen.at(allergenIdx);
                set<size_t> curIngridients;
                for(const auto &ingridient : get<0>(food))
                {
                    curIngridients.insert(
                        ingridientIndices.fromValue(ingridient));
                }
                set<size_t> nextIngridients;
                ranges::set_intersection(ingridientSeen, curIngridients,
                    inserter(nextIngridients, end(nextIngridients)));
                ingridientSeen = move(nextIngridients);
            }
        }
        inner::Cand candidates(
            tuple_element<0, inner::Cand>::type{allergenIndices.size()},
            ingridientIndices.size());
        for(size_t i = 0; i < seen.size(); ++i)
        {
            const auto &ingridientSeen = seen.at(i);
            for(auto v : ingridientSeen)
            {
                get<0>(candidates)[i].insert(v);
            }
        }
        const auto [solvedCandidates, solvedSz] = inner::invertCand(
            inner::solve(candidates));
        unordered_map<string, optional<string>> result;
        for(size_t i = 0; i < solvedCandidates.size(); ++i)
        {
            assert(solvedCandidates[i].size() <= 1);
            auto &allergen = result[ingridientIndices.fromIdx(i)];
            if(!solvedCandidates[i].empty())
            {
                allergen = allergenIndices.fromIdx(
                    *begin(solvedCandidates[i]));
            }
        }
        return result;
    }
}

#endif
