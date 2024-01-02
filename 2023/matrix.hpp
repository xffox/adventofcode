#ifndef NUMUTIL_MATRIX_HPP
#define NUMUTIL_MATRIX_HPP

#include <cassert>
#include <cstddef>
#include <format>
#include <initializer_list>
#include <iterator>
#include <limits>
#include <stdexcept>
#include <vector>

namespace numutil
{
template<typename T>
class Matrix
{
public:
    Matrix(std::size_t rows, std::size_t columns, const T &init = T())
        : data((rows == 0 ||
                std::numeric_limits<std::size_t>::max() / rows >= columns)
                   ? rows * columns
                   : throw std::invalid_argument("invalid dimensions"),
               init),
          row_count(rows), column_count(columns)
    {
    }
    Matrix(std::initializer_list<std::initializer_list<T>> values)
        : Matrix(values.size(),
                 !std::empty(values) ? std::begin(values)->size() : 0)
    {
        auto rowIter = std::begin(values);
        for(std::size_t i = 0; i < values.size(); ++i, ++rowIter)
        {
            auto colIter = std::begin(*rowIter);
            for(std::size_t j = 0; j < rowIter->size(); ++j, ++colIter)
            {
                ix(i, j) = *colIter;
            }
        }
    }
    T &ix(std::size_t row, std::size_t column)
    {
        assert(row < row_count && column < column_count);
        return data[row * column_count + column];
    }
    const T &ix(std::size_t row, std::size_t column) const
    {
        return const_cast<Matrix *>(this)->ix(row, column);
    }
    std::size_t rows() const
    {
        return row_count;
    }
    std::size_t columns() const
    {
        return column_count;
    }

    Matrix transpose() const
    {
        Matrix result(columns(), rows());
        for(std::size_t row = 0; row < rows(); ++row)
        {
            for(std::size_t col = 0; col < columns(); ++col)
            {
                result.ix(col, row) = ix(row, col);
            }
        }
        return result;
    }

    auto operator<=>(const Matrix &) const = default;

private:
    typedef std::vector<T> DataCollection;
    DataCollection data;
    std::size_t row_count;
    std::size_t column_count;
};

template<typename T>
inline auto mult(const Matrix<T> &left, const Matrix<T> &right)
{
    if(left.columns() != right.rows())
    {
        throw std::invalid_argument("invalid matrix multiplication");
    }
    Matrix<T> result(left.rows(), right.columns());
    for(std::size_t r = 0; r < result.rows(); ++r)
    {
        for(std::size_t c = 0; c < result.columns(); ++c)
        {
            for(std::size_t k = 0; k < left.columns(); ++k)
            {
                result.ix(r, c) += left.ix(r, k) * right.ix(k, c);
            }
        }
    }
    return result;
}
}

#endif
