#include <iostream>
#include <complex>
#include <ranges>
#include <string>
#include <cassert>
#include <cmath>

#include "../task.hpp"

using namespace std;

int main()
{
    const auto instructionsRes =
        task::parse(ranges::istream_view<string>(cin));
    assert(instructionsRes);
    task::State state{task::Pos(0), task::Pos(1, 0)};
    const auto endState = task::exec(state,
        ranges::views::all(*instructionsRes));
    const auto s = abs(round(endState.position.real())) +
        abs(round(endState.position.imag()));
    cout<<s<<endl;
    return 0;
}
