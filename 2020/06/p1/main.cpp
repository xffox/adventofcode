#include <iostream>
#include <cstddef>
#include <iterator>
#include <string>
#include <unordered_set>
#include <utility>

#include "../task.hpp"

using namespace std;
using namespace task;

namespace
{
    size_t countQuestions(const StringCol &answers)
    {
        unordered_set<char> questions;
        for(const auto &person : answers)
        {
            questions.insert(begin(person), end(person));
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
