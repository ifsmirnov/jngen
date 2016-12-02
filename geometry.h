#pragma once

#include "common.h"
#include "random.h"
#include "repr.h"
#include "printers.h"
#include "random.h"

namespace impl {

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
std::ostream& operator<<(std::ostream& out, const TPoint<T>& t) {
    return out << t.x << " " << t.y;
}

template<typename T>
JNGEN_DECLARE_SIMPLE_PRINTER(TPoint<T>, 2) {
    (void)mod;
    out << t;
}

class GeometryRandom {
public:
    GeometryRandom() {
        static bool created = false;
        ensure(!created, "impl::GeometryRandom should be created only once");
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
} rndg;

} // namespace impl

using impl::Point;
using impl::Pointf;

using impl::rndg;

using impl::eps;
using impl::setEps;
