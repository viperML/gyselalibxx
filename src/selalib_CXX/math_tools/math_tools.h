#ifndef MATH_TOOLS_H
#define MATH_TOOLS_H
#include <experimental/mdspan>
#include <cmath>

typedef std::experimental::mdspan<double,std::experimental::dynamic_extent> mdspan_1d;
typedef std::experimental::mdspan<double,std::experimental::dynamic_extent,std::experimental::dynamic_extent> mdspan_2d;

template<typename T>
inline T sum(T* array, int size)
{
    T val(0.0);
    for (int i(0); i<size; ++i)
    {
        val += array[i];
    }
    return val;
}

inline double sum(mdspan_1d const& array)
{
    double val(0.0);
    for (int i(0); i<array.extent(0); ++i)
    {
        val += array[i];
    }
    return val;
}

inline double sum(mdspan_1d const& array, int start, int end)
{
    double val(0.0);
    for (int i(start); i<end; ++i)
    {
        val += array[i];
    }
    return val;
}

template<typename T>
inline T modulo(T x, T y)
{
    return x - y * std::floor(double(x)/y);
}

inline double ipow(double a, int i)
{
    double r(1.0);
    if (i>0)
    {
        for (int j(0); j<i; ++j)
        {
            r *= a;
        }
    }
    else if (i<0)
    {
        for (int j(0); j<-i; ++j)
        {
            r *= a;
        }
        r = 1.0/r;
    }
    return r;
}

template<typename T>
inline T min(T a, T b)
{
    return (a<b) ? a : b;
}

template<typename T>
inline T max(T a, T b)
{
    return (a>b) ? a : b;
}

#endif // MATH_TOOLS_H
