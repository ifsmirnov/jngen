#pragma once

#include "array.h"
#include "common.h"
#include "hash.h"
#include "printers.h"
#include "random.h"
#include "repr.h"
#include "rnda.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <set>
#include <tuple>
#include <type_traits>
#include <unordered_set>

namespace jngen {

#ifdef JNGEN_DECLARE_ONLY
extern long double eps;
#else
long double eps = 1e-9;
#endif

inline void setEps(long double value) {
    eps = value;
}

template<typename T, typename U, typename Enable = void>
struct Comparator {
    static bool eq(T a, U b) { return a == b; }
    static bool lt(T a, U b) { return a < b; }
};

template<typename T, typename U>
struct Comparator<T, U, enable_if_t<
        std::is_floating_point<T>::value || std::is_floating_point<U>::value,
        void>>
{
    static bool eq(T a, U b) { return std::abs(b - a) < eps; }
    static bool lt(T a, U b) { return a < b - eps; }
};

template<typename T, typename U>
bool eq(T t, U u) {
    return Comparator<T, U>().eq(t, u);
}

template<typename T, typename U>
bool lt(T t, U u) {
    return Comparator<T, U>().lt(t, u);
}

template<typename T, typename U> bool ne(T t, U u) { return !eq(t, u); }
template<typename T, typename U> bool le(T t, U u) { return !lt(u, t); }
template<typename T, typename U> bool gt(T t, U u) { return  lt(u, t); }
template<typename T, typename U> bool ge(T t, U u) { return !lt(t, u); }

template<typename T>
struct TPoint : public ReprProxy<TPoint<T>> {
    T x, y;

    TPoint() : x(0), y(0) {}
    TPoint(T x, T y) : x(x), y(y) {}

    template<typename U>
    TPoint(const TPoint<U>& other) : x(other.x), y(other.y) {}

    TPoint<T> operator+(const TPoint<T>& other) const {
        return TPoint<T>(x + other.x, y + other.y);
    }

    TPoint<T>& operator+=(const TPoint<T>& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    TPoint<T> operator-(const TPoint<T>& other) const {
        return TPoint<T>(x - other.x, y - other.y);
    }

    TPoint<T>& operator-=(const TPoint<T>& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    TPoint<T> operator-() const {
        return TPoint<T>(-x, -y);
    }

    TPoint<T> operator*(T factor) const {
        return TPoint<T>(x * factor, y * factor);
    }

    TPoint<T>& operator*=(T factor) {
        x *= factor;
        y *= factor;
        return *this;
    }

    T operator*(const TPoint<T>& other) const {
        return x * other.x + y * other.y;
    }

    T operator%(const TPoint<T>& other) const {
        return x * other.y - y * other.x;
    }

    bool operator==(const TPoint<T>& other) const {
        return eq(x, other.x) && eq(y, other.y);
    }

    bool operator!=(const TPoint<T>& other) const {
        return !(*this == other);
    }

    bool operator<(const TPoint<T>& other) const {
        if (eq(x, other.x)) {
            return lt(y, other.y);
        }
        return lt(x, other.x);
    }
};

using Point = TPoint<long long>;
using Pointf = TPoint<long double>;

template<>
struct Hash<Point> {
    uint64_t operator()(const Point& point) const {
        uint64_t h = 0;
        impl::hashCombine(h, Hash<long long>{}(point.x));
        impl::hashCombine(h, Hash<long long>{}(point.y));
        return h;
    }
};

template<typename T>
JNGEN_DECLARE_SIMPLE_PRINTER(TPoint<T>, 3) {
    (void)mod;
    out << t.x << " " << t.y;
}

template<typename T>
class TPolygon : public GenericArray<TPoint<T>> {
public:
    using Base = GenericArray<TPoint<T>>;
    using Base::Base;

    TPolygon<T>& shift(const TPoint<T>& vector) {
        for (auto &pt: *this) {
            pt += vector;
        }
        return *this;
    }

    TPolygon<T> shifted(const TPoint<T>& vector) const {
        auto res = *this;
        res.shift(vector);
        return res;
    }

    TPolygon<T>& reflect() {
        for (auto& pt: *this) {
            pt = -pt;
        }
        return *this;
    }

    TPolygon<T> reflected() const {
        auto res = *this;
        res.reflect();
        return res;
    }
};

using Polygon = TPolygon<long long>;
using Polygonf = TPolygon<long double>;

template<>
struct Hash<Polygon> {
    uint64_t operator()(const Polygon& p) const {
        return Hash<TArray<Point>>{}(p);
    }
};

template<typename T>
JNGEN_DECLARE_SIMPLE_PRINTER(TArray<TPoint<T>>, 5) {
    // I should avoid copy-paste from array printer here but need to output
    // points with '\n' separator. Maybe 'mod' should be made non-const?
    if (mod.printN) {
        out << t.size() << "\n";
    }
    bool first = true;
    for (const auto& x: t) {
        if (first) {
            first = false;
        } else {
            out << '\n';
        }
        JNGEN_PRINT(x);
    }
}

namespace detail {

template<typename T>
TPolygon<T> convexHull(TArray<TPoint<T>> points) {
    points.sort().unique();

    if (points.size() <= 2u) {
        return points;
    }

    TArray<TPoint<T>> upper(points.begin(), points.begin() + 2);
    upper.reserve(points.size());
    int top = 1;
    for (size_t i = 2; i < points.size(); ++i) {
        while (top >= 1 && ge(
                (upper[top] - upper[top-1]) % (points[i] - upper[top]), 0ll))
        {
            upper.pop_back();
            --top;
        }
        upper.push_back(points[i]);
        ++top;
    }

    TArray<TPoint<T>> lower(points.begin(), points.begin() + 2);
    lower.reserve(points.size());
    top = 1;
    for (size_t i = 2; i < points.size(); ++i) {
        while (top >= 1 && le(
                (lower[top] - lower[top-1]) % (points[i] - lower[top]), 0ll))
        {
            lower.pop_back();
            --top;
        }
        lower.push_back(points[i]);
        ++top;
    }
    upper.pop_back();
    upper.erase(upper.begin());
    return lower + upper.reversed();
}

template<typename T>
TPolygon<T> convexPolygonByEllipse(
        int n, Pointf center, Pointf xAxis, Pointf yAxis)
{
    return convexHull(rnda.randomf(
        n,
        [center, xAxis, yAxis] () -> TPoint<T> {
            static const long double PI = acosl(-1.0);
            long double angle = rnd.next(0., PI*2);
            long double sina = sinl(angle);
            long double cosa = cosl(angle);
            return center + xAxis * cosa + yAxis * sina;
        }
    ));
}

} // namespace detail

class GeometryRandom {
public:
    GeometryRandom() {
        static bool created = false;
        ensure(!created, "jngen::GeometryRandom should be created only once");
        created = true;
    }

    // point in [0, C] x [0, C]
    static Point point(long long C);

    // point in [min, max] x [min, max]
    static Point point(long long min, long long max);

    // point in [X1, Y1] x [X2, Y2]
    static Point point(
            long long X1, long long Y1,
            long long X2, long long Y2);

    // point in [0, C] x [0, C]
    static Point pointf(long double C);

    // point in [min, max] x [min, max]
    static Point pointf(long double min, long double max);

    // point in [X1, Y1] x [X2, Y2]
    static Point pointf(
            long double X1, long double Y1,
            long double X2, long double Y2);


    static Polygon convexPolygon(int n, long long C);
    static Polygon convexPolygon(int n, long long min, long long max);
    static Polygon convexPolygon(
            int n,
            long long X1, long long Y1,
            long long X2, long long Y2);


    static TArray<Point> pointsInGeneralPosition(int n, long long C);

    static TArray<Point> pointsInGeneralPosition(
            int n, long long min, long long max);
    static TArray<Point> pointsInGeneralPosition(
            int n,
            long long X1, long long Y1,
            long long X2, long long Y2);

};


JNGEN_EXTERN GeometryRandom rndg;

JNGEN_EXTERN template struct jngen::TPoint<long long>;
JNGEN_EXTERN template struct jngen::TPoint<long double>;
JNGEN_EXTERN template class jngen::TPolygon<long long>;
JNGEN_EXTERN template class jngen::TPolygon<long double>;

JNGEN_EXTERN template TPolygon<long long> detail::convexHull<long long>(
        TArray<TPoint<long long>> points);
JNGEN_EXTERN template TPolygon<long double> detail::convexHull<long double>(
        TArray<TPoint<long double>> points);

} // namespace jngen

JNGEN_DEFINE_STD_HASH(jngen::Point);
JNGEN_DEFINE_STD_HASH(jngen::Polygon);

using jngen::Point;
using jngen::Pointf;

using jngen::Polygon;
using jngen::Polygonf;

using jngen::rndg;

using jngen::setEps;

// workaround for g++-7
namespace std {

JNGEN_EXTERN template class std::allocator<Point>;
JNGEN_EXTERN template class std::allocator<Pointf>;

} // namespace std

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_GEOMETRY_INL_H
#include "impl/geometry_inl.h"
#undef JNGEN_INCLUDE_GEOMETRY_INL_H
#endif // JNGEN_DECLARE_ONLY
