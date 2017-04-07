#pragma once

#include "svg_engine.h"

#include <algorithm>
#include <cmath>
#include <fstream>
#include <memory>
#include <utility>
#include <vector>

namespace jngen {
namespace drawing {

class Drawer {
public:
    template<typename P>
    void point(const P& p);

    template<typename P>
    void circle(const P& p, double radius);

    template<typename P>
    void segment(const P& p1, const P& p2);

    template<typename P>
    void polygon(const std::vector<P>& points);

    void dumpSvg(const std::string& filename);

private:
    struct Point {
        double x, y;
        Point() {}
        Point(double x, double y) : x(x), y(y) {}
    };

    typedef std::pair<Point, Point> Bbox;

    static Bbox emptyBbox() {
        const static double inf = 1e18;
        return { Point{inf, inf}, Point{-inf, -inf} };
    }

    static Bbox unite(const Bbox& lhs, const Bbox& rhs) {
        return Bbox{
                Point{
                    std::min(lhs.first.x, rhs.first.x),
                    std::min(lhs.first.y, rhs.first.y)},
                Point{
                    std::max(lhs.second.x, rhs.second.x),
                    std::max(lhs.second.y, rhs.second.y)}
        };
    }

    static Bbox bbox(const Point& p) {
        return {p, p};
    }

    static Bbox bbox(const std::pair<Point, double>& circle) {
        Point p;
        double radius;
        std::tie(p, radius) = circle;
        return {
                Point{p.x - radius, p.y - radius},
                Point{p.x + radius, p.y + radius}
        };
    }

    static Bbox bbox(const std::pair<Point, Point>& segment) {
        return unite(bbox(segment.first), bbox(segment.second));
    }

    Bbox getBbox() const;
    void drawAll();
    void drawGrid(const Bbox& bbox);

    std::vector<Point> points_;
    std::vector<std::pair<Point, double>> circles_;
    std::vector<std::pair<Point, Point>> segments_;

    DrawingEngine* engine_;
};

template<typename P>
void Drawer::point(const P& p) {
    points_.push_back(Point(p.x, p.y));
}

template<typename P>
void Drawer::circle(const P& p, double radius) {
    circles_.emplace_back(Point(p.x, p.y), radius);
}

template<typename P>
void Drawer::segment(const P& p1, const P& p2) {
    segments_.emplace_back(Point(p1.x, p1.y), Point(p2.x, p2.y));
}

template<typename P>
void Drawer::polygon(const std::vector<P>& points) {
    for (size_t i = 0; i + 1 < points.size(); ++i) {
        segment(points[i], points[i+1]);
    }
    segment(points.back(), points.front());
}

Drawer::Bbox Drawer::getBbox() const {
    Bbox result = emptyBbox();
    for (const auto& t: points_) {
        result = unite(result, bbox(t));
    }
    for (const auto& t: circles_) {
        result = unite(result, bbox(t));
    }
    for (const auto& t: segments_) {
        result = unite(result, bbox(t));
    }

    double dx = result.second.x - result.first.x;
    double dy = result.second.y - result.first.y;
    double add = std::min(dx, dy) * 0.05;
    result.first.x -= add;
    result.second.x += add;
    result.first.y -= add;
    result.second.y += add;

    return result;
}

void Drawer::drawAll() {
    for (const auto& t: points_) {
        engine_->drawPoint(t.x, t.y);
    }
    for (const auto& t: circles_) {
        engine_->drawCircle(t.first.x, t.first.y, t.second);
    }
    for (const auto& t: segments_) {
        engine_->drawSegment(t.first.x, t.first.y, t.second.x, t.second.y);
    }
}

void Drawer::drawGrid(const Bbox& bbox) {
    const static std::vector<int> STEP_DELTA = {20, 25, 20};
    // Step goes like 1, 2, 5, 10, 20, 50, 100, ...
    constexpr static int SMALL_IN_BIG = 5;
    constexpr static int THRESHOLD = 8;

    Color savedColor = engine_->color();
    double savedWidth = engine_->width();

    int step = 1;
    double spread = std::min(
        bbox.second.x - bbox.first.x,
        bbox.second.y - bbox.first.y);
    size_t deltaPos = 0;
    while (spread / step > THRESHOLD) {
        step = step * STEP_DELTA[deltaPos] / 10;
        if (++deltaPos == STEP_DELTA.size()) {
            deltaPos = 0;
        }
    }

    engine_->setWidth(0.5);
    engine_->setColor(Color::LightGrey);

    double smallStep = 1.0 * step / SMALL_IN_BIG;

    for (
            double tick = std::ceil(bbox.first.x / smallStep) * smallStep;
            tick < bbox.second.x;
            tick += smallStep)
    {
        if (std::lround(tick) % step != 0) {
            engine_->drawSegment(tick, bbox.first.y, tick, bbox.second.y);
        }
    }

    for (
            double tick = std::ceil(bbox.first.y / smallStep) * smallStep;
            tick < bbox.second.y;
            tick += smallStep)
    {
        if (std::lround(tick) % step != 0) {
            engine_->drawSegment(bbox.first.x, tick, bbox.second.x, tick);
        }
    }

    engine_->setWidth(0.75);
    engine_->setColor(Color::Grey);
    const double textOffset = smallStep / 4.0;

    auto format = [](double x) {
        static char buf[10];
        std::sprintf(buf, "%d", int(std::lround(x)));
        return std::string(buf);
    };

    for (
            double tick = std::ceil(bbox.first.x / step) * step;
            tick < bbox.second.x;
            tick += step)
    {
        engine_->drawSegment(tick, bbox.first.y, tick, bbox.second.y);
        engine_->drawText(
            tick + textOffset, bbox.first.y + textOffset, format(tick));
    }

    for (
            double tick = std::ceil(bbox.first.y / step) * step;
            tick < bbox.second.y;
            tick += step)
    {
        engine_->drawSegment(bbox.first.x, tick, bbox.second.x, tick);
        engine_->drawText(
            bbox.first.x + textOffset, tick + textOffset, format(tick));
    }

    engine_->setColor(savedColor);
    engine_->setWidth(savedWidth);
}

void Drawer::dumpSvg(const std::string& filename) {
    if (points_.empty() && circles_.empty() && segments_.empty()) {
        return;
    }

    auto bbox = getBbox();
    std::unique_ptr<SvgEngine> svgEngine(new SvgEngine(
        bbox.first.x, bbox.first.y, bbox.second.x, bbox.second.y));

    engine_ = svgEngine.get();
    drawGrid(bbox);
    drawAll();

    std::string svg = svgEngine->serialize();

    std::ofstream out(filename);
    out << svg;
    out.close();
}

}} // namespace jngen::drawing

using jngen::drawing::Drawer;
using jngen::drawing::Color;
