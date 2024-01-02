#ifndef TASK_H
#define TASK_H

#include <ranges>
#include <unordered_map>
#include <string>
#include <vector>
#include <iterator>

namespace task
{
    using namespace std;

    using FieldMap = unordered_map<string, string>;

    template<ranges::range Batch>
    vector<FieldMap> parsePassports(Batch batch)
    {
        vector<string> passports;
        {
            string cur;
            char prev = '\0';
            for(auto c : batch)
            {
                if(c != '\n')
                {
                    cur.push_back(c);
                }
                else
                {
                    if(prev != '\n')
                    {
                        cur.push_back(' ');
                    }
                    else
                    {
                        passports.push_back(move(cur));
                        cur.clear();
                    }
                }
                prev = c;
            };
            if(!cur.empty())
            {
                passports.push_back(move(cur));
            }
        }
        auto entries = passports | views::transform([](auto passport){
                auto fields = passport
                    | views::split(' ')
                    | views::transform([](auto pair){
                            string tmp;
                            for(auto c : pair)
                            {
                                tmp.push_back(c);
                            }
                            const auto sepPos = tmp.find(':');
                            if(sepPos == string::npos)
                                return make_pair(string(), string());
                            return make_pair(
                                tmp.substr(0, sepPos),
                                tmp.substr(sepPos+1));
                        });
                return FieldMap(begin(fields), end(fields));
            });
        return vector<FieldMap>(begin(entries), end(entries));
    }
}

#endif
