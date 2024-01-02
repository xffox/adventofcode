#ifndef GEOM_HPP
#define GEOM_HPP

#include <cmath>
#include <cstddef>
#include <ostream>

#include <limits>

namespace geom
{
struct Pos
{
    int r;
    int c;

    friend auto operator<=>(const Pos &, const Pos &) = default;
};

constexpr Pos operator+(const Pos &left, const Pos &right)
{
    return Pos{left.r + right.r, left.c + right.c};
}
constexpr Pos operator-(const Pos &left, const Pos &right)
{
    return Pos{left.r - right.r, left.c - right.c};
}
constexpr Pos operator*(const Pos &left, const Pos &right)
{
    return Pos{left.r * right.r, left.c * right.c};
}
constexpr Pos operator*(const Pos &left, const int mult)
{
    return Pos{left.r * mult, left.c * mult};
}
constexpr Pos operator*(const int mult, const Pos &right)
{
    return right * mult;
}
constexpr Pos operator/(const Pos &left, const Pos &right)
{
    return Pos{left.r / right.r, left.c / right.c};
}
constexpr Pos norm(const Pos &pos)
{
    const auto normVal = [](auto val) {
        return (val != 0 ? val / std::abs(val) : val);
    };
    return Pos{
        normVal(pos.r),
        normVal(pos.c),
    };
}
constexpr int dist(const Pos &left, const Pos &right)
{
    return std::abs(left.r - right.r) + std::abs(left.c - right.c);
}

constexpr Pos rotate(const Pos &left, const Pos &right)
{
    return Pos{
        left.r * right.r - left.c * right.c,
        left.r * right.c + left.c * right.r,
    };
}

constexpr Pos div(const Pos &left, const Pos &right)
{
    const auto num = rotate(left, right);
    const auto den = rotate(right, right);
    return Pos{num.r / den.r, num.c / den.r};
}

std::ostream &operator<<(std::ostream &stream, const geom::Pos &pos)
{
    return stream << '(' << pos.r << ',' << pos.c << ')';
}
}

#endif
