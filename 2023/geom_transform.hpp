#ifndef GEOMTRANSFORM_HPP
#define GEOMTRANSFORM_HPP

#include <array>
#include <cmath>
#include <cstddef>

#include "geom.hpp"
#include "matrix.hpp"

namespace geom
{
namespace detail
{
    constexpr std::size_t TRANSFORM_SIZE = 2;
}

using TransformMatrix = numutil::Matrix<int>;

inline TransformMatrix prepareTransform(
    const std::array<std::array<int, detail::TRANSFORM_SIZE>,
                     detail::TRANSFORM_SIZE> &values)
{
    TransformMatrix result(values.size(), values.front().size());
    for(std::size_t r = 0; r < values.size(); ++r)
    {
        for(std::size_t c = 0; c < values.size(); ++c)
        {
            result.ix(r, c) = values[r][c];
        }
    }
    return result;
}

inline Pos transform(const Pos &pos, const TransformMatrix &transform)
{
    return Pos{
        pos.r * transform.ix(0, 0) + pos.c * transform.ix(0, 1),
        pos.r * transform.ix(1, 0) + pos.c * transform.ix(1, 1),
    };
}

template<typename T>
auto rotate(const numutil::Matrix<T> &board, const Pos &rotation)
{
    constexpr auto posAbs = [](const Pos &pos) {
        return Pos{std::abs(pos.r), std::abs(pos.c)};
    };
    Pos size{static_cast<int>(board.rows()), static_cast<int>(board.columns())};
    auto resultSize = posAbs(rotate(size, rotation));
    numutil::Matrix<T> result(resultSize.r, resultSize.c);
    const Pos direction = rotate(Pos{1, 1}, rotation);
    const Pos offset{
        (resultSize.r - 1) * (direction.r < 0),
        (resultSize.c - 1) * (direction.c < 0),
    };
    for(std::size_t row = 0; row < board.rows(); ++row)
    {
        for(std::size_t col = 0; col < board.columns(); ++col)
        {
            const auto dst = offset + rotate(
                                          Pos{
                                              static_cast<int>(row),
                                              static_cast<int>(col),
                                          },
                                          rotation);
            result.ix(dst.r, dst.c) = board.ix(row, col);
        }
    }
    return result;
}
}

#endif
