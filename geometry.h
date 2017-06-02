#pragma once

#include "array.h"
#include "common.h"
#include "printers.h"
#include "random.h"
#include "random.h"
#include "repr.h"
#include "rnda.h"

#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace jngen {

// TODO: why do we need this shit?
class EpsHolder {
private:
    EpsHolder() : eps(1e-9) {}

public:
    long double eps;

    static EpsHolder& instance() {
        static EpsHolder holder;
        return holder;
    }
};

inline void setEps(long double eps) {
    EpsHolder::instance().eps = eps;
}

inline long double eps() {
    return EpsHolder::instance().eps;
}

template<typename T, typename Enable = void>
class Comparator {
public:
    static bool eq(T a, T b) { return a == b; }
    static bool ne(T a, T b) { return !(a == b); }
    static bool lt(T a, T b) { return a < b; }
    static bool le(T a, T b) { return a <= b; }
    static bool gt(T a, T b) { return a > b; }
    static bool ge(T a, T b) { return a >= b; }
};

template<typename T>
class Comparator<T,
    typename std::enable_if<std::is_floating_point<T>::value, void>::type>
{
    static bool eq(T a, T b) { return std::abs(b - a) < eps(); }
    static bool ne(T a, T b) { return !(a == b); }
    static bool lt(T a, T b) { return a < b - eps; }
    static bool le(T a, T b) { return a <= b + eps; }
    static bool gt(T a, T b) { return a > b + eps; }
    static bool ge(T a, T b) { return a >= b - eps; }
};

// TODO: do something with eq(int, long long)
template<typename T> bool eq(T a, T b) { return Comparator<T>::eq(a, b); }
template<typename T> bool ne(T a, T b) { return Comparator<T>::ne(a, b); }
template<typename T> bool lt(T a, T b) { return Comparator<T>::lt(a, b); }
template<typename T> bool le(T a, T b) { return Comparator<T>::le(a, b); }
template<typename T> bool gt(T a, T b) { return Comparator<T>::gt(a, b); }
template<typename T> bool ge(T a, T b) { return Comparator<T>::ge(a, b); }

template<typename T>
struct TPoint : public ReprProxy<TPoint<T>> {
    T x, y;

    TPoint() : x(0), y(0) {}
    TPoint(T x, T y) : x(x), y(y) {}

    template<typename U>
    TPoint(const TPoint<U>& other) : x(other.x), y(other.y) {}

    TPoint<T> operator+(const TPoint<T>& other) const {
        return TPoint(x + other.x, y + other.y);
    }

    TPoint<T>& operator+=(const TPoint<T>& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    TPoint<T> operator-(const TPoint<T>& other) const {
        return TPoint(x - other.x, y - other.y);
    }

    TPoint<T>& operator-=(const TPoint<T>& other) {
        x -= other.x;
        y -= other.y;
        return *this;
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

template<typename T>
JNGEN_DECLARE_SIMPLE_PRINTER(TPoint<T>, 3) {
    (void)mod;
    out << t.x << " " << t.y;
}

// TODO: make polygon a class to support, e.g., shifting by a point
template<typename T>
using TPolygon = GenericArray<TPoint<T>>;

using Polygon = TPolygon<long long>;
using Polygonf = TPolygon<long double>;

template<typename T>
JNGEN_DECLARE_SIMPLE_PRINTER(TPolygon<T>, 5) {
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

// Please forgive me the liberty of using TPolygon instead of Array<Point<T>> :)
// (laxity?)
template<typename T>
TPolygon<T> convexHull(TPolygon<T> points) {
    points.sort().unique();

    if (points.size() <= 2u) {
        return points;
    }

    TPolygon<T> upper(points.begin(), points.begin() + 2);
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

    TPolygon<T> lower(points.begin(), points.begin() + 2);
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
    return convexHull(TPolygon<T>::randomf(
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

    // point in [0, X] x [0, Y]
    static Point point(long long X, long long Y) {
        long long x = rnd.tnext<long long>(0, X);
        long long y = rnd.tnext<long long>(0, Y);
        return {x, y};
    }

    // point in [0, C] x [0, C]
    static Point point(long long C) {
        return point(C, C);
    }

    // Point in [x1, x2] x [y1, y2]
    static Point point(long long x1, long long y1, long long x2, long long y2) {
        long long x = rnd.tnext<long long>(x1, x2);
        long long y = rnd.tnext<long long>(y1, y2);
        return {x, y};
    }

    static Polygon convexPolygon(int n, long long X, long long Y) {
        Polygon res = detail::convexPolygonByEllipse<long long>(
            n * 10, // BUBEN!
            Point(X/2, Y/2),
            Point(X/2, 0),
            Point(0, Y/2)
        );
        for (auto& x: res) {
            ensure(x.x >= 0);
            ensure(x.x <= X);
            ensure(x.y >= 0);
            ensure(x.y <= Y);
        }

        ensure(
            static_cast<int>(res.size()) >= n,
            "Cannot generate a convex polygon with so much vertices");

        return res.subseq(Array::id(res.size()).choice(n).sort());
    }

    static TArray<Point> pointsInCommonPosition(
            int n, long long X, long long Y) {
        TArray<Point> res;
        while (static_cast<int>(res.size()) != n) {
            Point p = point(X, Y);
            bool ok = true;
            for (size_t i = 0; i < res.size(); ++i) {
                if (p == res[i]) {
                    ok = false;
                    break;
                }
                for (size_t j = 0; j < i; ++j) {
                    if ((res[i] - res[j]) % (res[i] - p) == 0) {
                        ok = false;
                        break;
                    }
                }
                if (!ok) {
                    break;
                }
            }
            if (ok) {
                res.push_back(p);
            }
        }
        return res;
    }
};

JNGEN_EXTERN GeometryRandom rndg;

} // namespace jngen

using jngen::Point;
using jngen::Pointf;

using jngen::Polygon;
using jngen::Polygonf;

using jngen::rndg;

using jngen::eps;
using jngen::setEps;
