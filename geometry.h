#pragma once

#include "array.h"
#include "common.h"
#include "printers.h"
#include "random.h"
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

} // namespace jngen

namespace std {

template<>
struct hash<jngen::Point> {
    // Credits to boost::hash
    static void hash_combine_impl(uint64_t& h, uint64_t k) {
        const uint64_t m = 0xc6a4a7935bd1e995;
        const int r = 47;

        k *= m;
        k ^= k >> r;
        k *= m;

        h ^= k;
        h *= m;

        // Completely arbitrary number, to prevent 0's
        // from hashing to 0.
        h += 0xe6546b64;
    }

    size_t operator()(const jngen::Point& p) const {
        uint64_t h = 0;
        hash_combine_impl(h, std::hash<long long>{}(p.x));
        hash_combine_impl(h, std::hash<long long>{}(p.y));
        return h;
    }
};

} // namespace std

namespace jngen {

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

    TPolygon<T> shifted(const TPoint<T>& vector) {
        auto res = *this;
        res.shift(vector);
        return res;
    }
};

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

    // point in [0, X] x [0, Y]
    static Pointf pointf(long double X, long double Y) {
        long double x = rnd.tnext<long double>(0, X);
        long double y = rnd.tnext<long double>(0, Y);
        return {x, y};
    }

    // point in [0, C] x [0, C]
    static Pointf pointf(long double C) {
        return point(C, C);
    }

    static Polygon convexPolygon(int n, long long X, long long Y) {
        ensure(n >= 0);
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
            "Cannot generate a convex polygon with so many vertices");

        return res.subseq(Array::id(res.size()).choice(n).sort());
    }

    static Polygon convexPolygon(int n, long long C) {
        return convexPolygon(n, C, C);
    }

    static TArray<Point> pointsInGeneralPosition(
            int n, long long X, long long Y)
    {
        struct Line {
            long long A, B, C; // Ax + By + C = 0
            Line() {}
            Line(const Point& p1, const Point& p2) {
                A = p1.y - p2.y;
                B = p2.x - p1.x;
                C = -(p1.x * A + p1.y * B);

                ENSURE(A != 0 || B != 0);

                long long g = util::gcd(A, util::gcd(B, C));
                A /= g;
                B /= g;
                C /= g;
                if (A < 0 || (A == 0 && B < 0)) {
                    A = -A;
                    B = -B;
                    C = -C;
                }
            }

            bool operator<(const Line& other) const {
                return std::tie(A, B, C) < std::tie(other.A, other.B, other.C);
            }
        };

        const long long LIMIT = 2e9;
        ensure(
            X <= LIMIT && Y <= LIMIT,
            "rndg.pointsInGeneralPosition must not be called with coordinates "
            "larger than 2e9");

        std::set<Line> lines;
        std::unordered_set<Point> points;

        TArray<Point> res;

        while (static_cast<int>(res.size()) != n) {
            Point p = point(X, Y);

            if (points.count(p)) {
                continue;
            }

            if (std::none_of(
                    res.begin(),
                    res.end(),
                    [&lines, &p] (const Point& q) {
                        return lines.count(Line(p, q));
                    }))
            {
                points.insert(p);
                for (const auto& q: res) {
                    lines.emplace(p, q);
                }
                res.push_back(p);
            }
        }
        return res;
    }

    static TArray<Point> pointsInGeneralPosition(int n, long long C) {
        return pointsInGeneralPosition(n, C, C);
    }
};

JNGEN_EXTERN GeometryRandom rndg;

} // namespace jngen

using jngen::Point;
using jngen::Pointf;

using jngen::Polygon;
using jngen::Polygonf;

using jngen::rndg;

using jngen::setEps;
