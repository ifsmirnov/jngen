#ifndef JNGEN_INCLUDE_GEOMETRY_INL_H
#error File "geometry_inl.h" must not be included directly.
#include "../geometry.h" // for completion engine
#endif

namespace jngen {

Point GeometryRandom::point(long long C) {
    return point(0, 0, C, C);
}

Point GeometryRandom::point(long long min, long long max) {
    return point(min, min, max, max);
}

Point GeometryRandom::point(
        long long X1, long long Y1,
        long long X2, long long Y2) {
    long long x = rnd.tnext<long long>(X1, X2);
    long long y = rnd.tnext<long long>(Y1, Y2);
    return Point(x, y);
}

Point GeometryRandom::pointf(long double C) {
    return pointf(0, 0, C, C);
}

Point GeometryRandom::pointf(long double min, long double max) {
    return pointf(min, min, max, max);
}

Point GeometryRandom::pointf(
        long double X1, long double Y1,
        long double X2, long double Y2)
{
    long double x = rnd.tnext<long double>(X1, X2);
    long double y = rnd.tnext<long double>(Y1, Y2);
    return Pointf(x, y);
}


Polygon GeometryRandom::convexPolygon(int n, long long C) {
    return convexPolygon(n, 0, 0, C, C);
}

Polygon GeometryRandom::convexPolygon(int n, long long min, long long max) {
    return convexPolygon(n, min, min, max, max);
}

Polygon GeometryRandom::convexPolygon(
            int n,
            long long X1, long long Y1,
            long long X2, long long Y2)
{
    // todo: off-by-one error?
    auto dx = X2 - X1;
    auto dy = Y2 - Y1;
    ensure(n >= 0);
    Polygon res = detail::convexPolygonByEllipse<long long>(
        n * 10, // BUBEN!
        Point(dx / 2, dy / 2),
        Point(dx / 2, 0),
        Point(0, dy / 2)
    );
    res.shift(Point(X1, Y1));
    for (auto& x: res) {
        ENSURE(x.x >= X1);
        ENSURE(x.x <= X2);
        ENSURE(x.y >= Y1);
        ENSURE(x.y <= Y2);
    }

    ensure(
        static_cast<int>(res.size()) >= n,
        "Cannot generate a convex polygon with so many vertices");

    return res.subseq(Array::id(res.size()).choice(n).sort());
}


TArray<Point> GeometryRandom::pointsInGeneralPosition(int n, long long C) {
    return pointsInGeneralPosition(n, 0, 0, C, C);
}

TArray<Point> GeometryRandom::pointsInGeneralPosition(
        int n, long long min, long long max)
{
    return pointsInGeneralPosition(n, min, min, max, max);
}

TArray<Point> GeometryRandom::pointsInGeneralPosition(
        int n,
        long long X1, long long Y1,
        long long X2, long long Y2)
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
        std::abs(X2 - X1) <= LIMIT && X1 <= LIMIT && X2 <= LIMIT &&
            std::abs(Y2 - Y1) <= LIMIT && Y1 <= LIMIT && Y2 <= LIMIT,
        "rndg.pointsInGeneralPosition must not be called with coordinates "
        "larger than 2e9");

    std::set<Line> lines;
    std::unordered_set<Point> points;

    TArray<Point> res;

    while (static_cast<int>(res.size()) != n) {
        Point p = point(X1, Y1, X2, Y2);

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
