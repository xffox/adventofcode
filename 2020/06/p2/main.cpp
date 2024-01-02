#include <iostream>
#include <cstddef>
#include <iterator>
#include <string>
#include <unordered_set>
#include <utility>
#include <ranges>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    size_t countQuestions(const StringCol &answers)
    {
        using QuestionsSet = unordered_set<char>;
        if(answers.empty())
        {
            return 0;
        }
        QuestionsSet questions(
            begin(answers.front()), end(answers.front()));
        for(const auto &person : answers | ranges::views::drop(1))
        {
            QuestionsSet nextQuestions;
            for(auto c : person)
            {
                if(questions.contains(c))
                {
                    nextQuestions.insert(c);
                }
            }
            questions = move(nextQuestions);
        }
        return questions.size();
    }
}

int main()
{
    StringCol answers;
    size_t questions = 0;
    while(cin)
    {
        string s;
        getline(cin, s);
        if(!s.empty())
        {
            answers.push_back(move(s));
        }
        else
        {
            questions += countQuestions(answers);
            answers.clear();
        }
    }
    questions += countQuestions(answers);
    cout<<questions<<endl;
    return 0;
}
