#include <iostream>
#include <cstddef>
#include <algorithm>
#include <iterator>
#include <string>
#include <ranges>
#include <vector>
#include <cassert>
#include <unordered_set>

#include "../task.hpp"

namespace
{
    using namespace std;
    using namespace task;

    using Cand = vector<unordered_set<size_t>>;

    bool validTicket(const vector<Rule> &rules, const ValCol &ticket)
    {
        return !ranges::any_of(ticket, [&rules](auto v){
                return !ranges::any_of(rules,
                    [v](const auto &rule){
                        return matchesRule(rule, v);
                    });
                });
    }

    Cand invertCand(const Cand &cands)
    {
        Cand result(cands.size());
        for(size_t i = 0; i < cands.size(); ++i)
        {
            const auto &cand = cands[i];
            for(auto c : cand)
            {
                result[c].insert(i);
            }
        }
        return result;
    }

    void cleanUnique(Cand &cands)
    {
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

    optional<vector<string>> findFields(const Notes &notes)
    {
        auto validTickets = notes.tickets | ranges::views::filter(
            [&notes](const auto &ticket){
                    return validTicket(notes.rules, ticket);
                });
        vector<unordered_set<size_t>> candidates;
        {
            unordered_set<size_t> init;
            ranges::copy(ranges::views::iota(
                    static_cast<size_t>(0), notes.rules.size()),
                inserter(init, end(init)));
            for(size_t i = 0; i < notes.rules.size(); ++i)
            {
                candidates.push_back(init);
            }
        }
        ranges::for_each(validTickets, [&candidates, &notes](const auto &ticket){
                for(size_t i = 0; i < ticket.size(); ++i)
                {
                    for(size_t j = 0; j < notes.rules.size(); ++j)
                    {
                        if(!matchesRule(notes.rules[j], ticket[i]))
                        {
                            candidates[i].erase(j);
                        }
                    }
                }
            });
        unordered_set<size_t> solvedFields;
        bool newSolved = false;
        do
        {
            cleanUnique(candidates);
            auto invertedCands = invertCand(candidates);
            cleanUnique(invertedCands);
            candidates = invertCand(invertedCands);
            newSolved = false;
            for(size_t i = 0; i < candidates.size(); ++i)
            {
                if(candidates[i].empty())
                {
                    return {};
                }
                if(candidates[i].size() == 1)
                {
                    newSolved = newSolved || (solvedFields.insert(i).second);
                }
            }
        }
        while(newSolved);
        vector<string> result(candidates.size());
        for(size_t i = 0; i < candidates.size(); ++i)
        {
            const auto &cand = candidates[i];
            if(cand.size() != 1)
            {
                return {};
            }
            result[i] = notes.rules[*begin(cand)].name;
        }
        return result;
    }
}

int main()
{
    const string PREFIX = "departure";
    string inp;
    copy(istreambuf_iterator<char>(cin), {}, back_inserter(inp));
    const auto notesRes = parseNotes(inp);
    assert(notesRes);
    const auto &notes = *notesRes;
    const auto fieldsRes = findFields(*notesRes);
    assert(fieldsRes);
    const auto &fields = *fieldsRes;
    assert(!notes.tickets.empty());
    ulint res = 1;
    for(size_t i = 0; i < fields.size(); ++i)
    {
        if(fields[i].starts_with(PREFIX))
        {
            res *= notes.tickets[0][i];
        }
    }
    cout<<res<<endl;
    return 0;
}
