#include <iostream>
#include <vector>
#include <string>
#include <utility>
#include <cassert>

#include "../task.hpp"

using namespace std;

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
    task::Machine machine(*instructionsRes);
    machine.run();
    cout<<machine.accumulator()<<endl;
    return 0;
}
