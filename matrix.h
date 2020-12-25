#ifndef MATRIX_MATRIX_H
#define MATRIX_MATRIX_H

#include <vector>
#include <stdexcept>
#include <cstddef>
#include <limits>

namespace numutil
{
    template<typename T>
    class Matrix
    {
    public:
        Matrix(std::size_t rows, std::size_t columns, const T &init = T())
            :data((rows == 0 ||
                    std::numeric_limits<std::size_t>::max()/rows >= columns)
                ?rows*columns:throw std::invalid_argument(
                    "invalid dimensions"), init),
            row_count(rows), column_count(columns) {}
        T &ix(std::size_t row, std::size_t column)
        {
            if(row >= row_count)
                throw std::invalid_argument("invalid row");
            if(column >= column_count)
                throw std::invalid_argument("invalid column");
            return data[row*column_count + column];
        }
        const T &ix(std::size_t row, std::size_t column) const
        {
            return const_cast<Matrix*>(this)->ix(row, column);
        }
        std::size_t rows() const{return row_count;}
        std::size_t columns() const{return column_count;}
    private:
        typedef std::vector<T> DataCollection;
        DataCollection data;
        std::size_t row_count;
        std::size_t column_count;
    };
}

#endif
