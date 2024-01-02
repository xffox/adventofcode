#include <iostream>
#include <algorithm>
#include <string>
#include <ranges>

#include "../task.hpp"

using namespace std;
using namespace task;

int main()
{
    cout<<ranges::max(ranges::istream_view<string>(cin)
        | ranges::views::transform(parseID))<<endl;
    return 0;
}
