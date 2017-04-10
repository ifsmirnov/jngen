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

    static Bbox emptyBbox();

    static Bbox unite(const Bbox& lhs, const Bbox& rhs);

    static Bbox bbox(const Point& p);
    static Bbox bbox(const std::pair<Point, double>& circle);
    static Bbox bbox(const std::pair<Point, Point>& segment);

    Bbox getBbox() const;

    static Bbox viewportByBbox(const Bbox& bbox);

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

#ifndef JNGEN_DECLARE_ONLY

Drawer::Bbox Drawer::emptyBbox() {
    const static double inf = 1e18;
    return { Point{inf, inf}, Point{-inf, -inf} };
}

Drawer::Bbox Drawer::unite(const Bbox& lhs, const Bbox& rhs) {
    return Bbox{
            Point{
                std::min(lhs.first.x, rhs.first.x),
                std::min(lhs.first.y, rhs.first.y)},
            Point{
                std::max(lhs.second.x, rhs.second.x),
                std::max(lhs.second.y, rhs.second.y)}
    };
}

Drawer::Bbox Drawer::bbox(const Point& p) {
    return {p, p};
}

Drawer::Bbox Drawer::bbox(const std::pair<Point, double>& circle) {
    Point p;
    double radius;
    std::tie(p, radius) = circle;
    return {
            Point{p.x - radius, p.y - radius},
            Point{p.x + radius, p.y + radius}
    };
}

Drawer::Bbox Drawer::bbox(const std::pair<Point, Point>& segment) {
    return unite(bbox(segment.first), bbox(segment.second));
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
    return result;
}

/*
Given a bbox of points, returns a bbox with following properties:
    - at least 5% margin at each side is blank
    - side lengths differ by at most 1.6
    - side length is at least 10
    - if it is possible to include (0, 0), include it explicitly
 */
Drawer::Bbox Drawer::viewportByBbox(const Bbox& bbox) {
    constexpr static double MIN_SIZE = 10.0;
    constexpr static double MAX_RATIO = 1.6;
    constexpr static double MARGIN_RATIO = 0.05;
    constexpr static double MAX_RELATIVE_DISTANCE_TO_ZERO = 0.2;

    double lx = bbox.first.x;
    double rx = bbox.second.x;
    double ly = bbox.first.y;
    double ry = bbox.second.y;

    auto extendToSize = [&](double& l, double &r, double size) {
        double shift = (size - (r - l)) / 2;
        l -= shift;
        r += shift;
    };

    auto extendInterval = [&](double& l, double &r) {
        if (r - l < MIN_SIZE) {
            if (l >= -1e-9 && r < MIN_SIZE) {
                l = 0;
                r = MIN_SIZE;

            } else if (r <= 1e-9 && l >= -MIN_SIZE) {
                l = -MIN_SIZE;
                r = 0;
            } else {
                extendToSize(l, r, MIN_SIZE);
            }
        }

        if ((l > 0 || r < 0) && std::min(std::abs(l), std::abs(r)) <=
                (r - l) * MAX_RELATIVE_DISTANCE_TO_ZERO)
        {
            if (l > 0) {
                l = 0;
            } else {
                r = 0;
            }
        }

        double margin = (r - l) * MARGIN_RATIO;
        l -= margin;
        r += margin;
    };

    extendInterval(lx, rx);
    extendInterval(ly, ry);

    if ((rx - lx) / (ry - ly) > MAX_RATIO) {
        extendToSize(ly, ry, (rx - lx) / MAX_RATIO);
    } else if ((ry - ly) / (rx - lx) > MAX_RATIO) {
        extendToSize(lx, rx, (ry - ly) / MAX_RATIO);
    }

    return { Point(lx, ly), Point(rx, ry) };
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
    constexpr static int MAX_SPREAD_TO_DRAW_ALL_TICKS = 13;
    constexpr static double TEXT_OFFSET_RATIO = 0.01;

    Color savedColor = engine_->color();
    double savedWidth = engine_->width();

    int step = 5;
    double spread = std::min(
        bbox.second.x - bbox.first.x,
        bbox.second.y - bbox.first.y);
    size_t deltaPos = 2;
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

    for (
            double tick = std::ceil(bbox.first.x / step) * step;
            tick < bbox.second.x;
            tick += step)
    {
        engine_->drawSegment(tick, bbox.first.y, tick, bbox.second.y);
    }

    for (
            double tick = std::ceil(bbox.first.y / step) * step;
            tick < bbox.second.y;
            tick += step)
    {
        engine_->drawSegment(bbox.first.x, tick, bbox.second.x, tick);
    }

    const double textOffsetX =
        (bbox.second.x - bbox.first.x) * TEXT_OFFSET_RATIO;
    const double textOffsetY =
        (bbox.second.y - bbox.first.y) * TEXT_OFFSET_RATIO;

    auto format = [](double x) {
        static char buf[10];
        std::sprintf(buf, "%d", int(std::lround(x)));
        return std::string(buf);
    };

    if (spread < MAX_SPREAD_TO_DRAW_ALL_TICKS) {
        step = 1;
    }

    for (
            double tick = std::ceil(bbox.first.y / step) * step;
            tick < bbox.second.y;
            tick += step)
    {
        engine_->drawText(
            bbox.first.x + textOffsetX, tick + textOffsetX, format(tick));
    }

    for (
            double tick = std::ceil(bbox.first.x / step) * step;
            tick < bbox.second.x;
            tick += step)
    {
        engine_->drawText(
            tick + textOffsetY, bbox.first.y + textOffsetY, format(tick));
    }


    if (spread <= MAX_SPREAD_TO_DRAW_ALL_TICKS) {
        step = 1;
    }


    engine_->setColor(savedColor);
    engine_->setWidth(savedWidth);
}

void Drawer::dumpSvg(const std::string& filename) {
    if (points_.empty() && circles_.empty() && segments_.empty()) {
        return;
    }

    auto bbox = getBbox();
    auto viewport = viewportByBbox(bbox);
    std::unique_ptr<SvgEngine> svgEngine(new SvgEngine(
        viewport.first.x, viewport.first.y,
        viewport.second.x, viewport.second.y));

    engine_ = svgEngine.get();
    drawGrid(viewport);
    drawAll();

    std::string svg = svgEngine->serialize();

    std::ofstream out(filename);
    out << svg;
    out.close();
}

#endif // JNGEN_DECLARE_ONLY

}} // namespace jngen::drawing

using jngen::drawing::Drawer;
using jngen::drawing::Color;
