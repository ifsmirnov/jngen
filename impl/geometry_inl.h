#ifndef JNGEN_INCLUDE_GEOMETRY_INL_H
#error File "geometry_inl.h" must not be included directly.
#include "../geometry.h" // for completion engine
#endif

namespace jngen {

Point GeometryRandom::point(long long X, long long Y) {
    long long x = rnd.tnext<long long>(0, X);
    long long y = rnd.tnext<long long>(0, Y);
    return {x, y};
}

Point GeometryRandom::point(long long C) {
    return point(C, C);
}

Pointf GeometryRandom::pointf(long double X, long double Y) {
    long double x = rnd.tnext<long double>(0, X);
    long double y = rnd.tnext<long double>(0, Y);
    return {x, y};
}

Pointf GeometryRandom::pointf(long double C) {
    return point(C, C);
}

Polygon GeometryRandom::convexPolygon(int n, long long X, long long Y) {
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

Polygon GeometryRandom::convexPolygon(int n, long long C) {
    return convexPolygon(n, C, C);
}

TArray<Point> GeometryRandom::pointsInGeneralPosition(int n, long long C) {
    return pointsInGeneralPosition(n, C, C);
}

TArray<Point> GeometryRandom::pointsInGeneralPosition(
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

} // namespace jngen
