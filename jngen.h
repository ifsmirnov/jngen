
#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

#ifdef JNGEN_DECLARE_ONLY
#define JNGEN_EXTERN extern
#else
#define JNGEN_EXTERN
#endif

namespace jngen {

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& s) :
        std::runtime_error("Assertion `" + s + "' failed.")
    {  }

    Exception(const std::string& assertMsg, const std::string& expl) :
        std::runtime_error(expl + " (assertion `" + assertMsg + "' failed).")
    {  }
};

class InternalException : public Exception {
public:
    explicit InternalException(const std::string& s) : Exception(s) {}

    InternalException(const std::string& assertMsg, const std::string& expl) :
        Exception(assertMsg, expl)
    {  }
};

} // namespace jngen

#define JNGEN_ENSURE1(exType, cond)\
do\
    if (!(cond)) {\
        throw exType(#cond);\
    }\
while (false)

#define JNGEN_ENSURE2(exType, cond, msg)\
do\
    if (!(cond)) {\
        throw exType(#cond, msg);\
    }\
while (false)

#define JNGEN_GET_MACRO(_1, _2, NAME, ...) NAME

#define ensure(...) JNGEN_GET_MACRO(__VA_ARGS__, JNGEN_ENSURE2, JNGEN_ENSURE1)\
    (jngen::Exception, __VA_ARGS__)
#define ENSURE(...) JNGEN_GET_MACRO(__VA_ARGS__, JNGEN_ENSURE2, JNGEN_ENSURE1)\
    (jngen::InternalException, __VA_ARGS__)

namespace jngen {

template<typename ... Args>
std::string format(const std::string& fmt, Args... args) {
    constexpr static char BUF_SIZE = 64;
    static char BUFFER[BUF_SIZE];

    int bufSize = BUF_SIZE;
    char *buf = BUFFER;

    while (true) {
        int ret = std::snprintf(buf, bufSize, fmt.c_str(), args...);
        if (ret < bufSize) {
            break;
        }

        if (bufSize != BUF_SIZE) {
            delete[] buf;
        }

        bufSize *= 2;
        buf = new char[bufSize];
    }

    std::string result(buf);

    if (bufSize != BUF_SIZE) {
        delete[] buf;
    }

    return result;
}

class ContextTimer {
public:
    ContextTimer(const std::string& name) : name_(name) {
        start_ = std::chrono::steady_clock::now();
    }

    ContextTimer() : ContextTimer("") {}

    ContextTimer(const ContextTimer&) = delete;
    ContextTimer& operator=(const ContextTimer&) = delete;
    ContextTimer(ContextTimer&&) = delete;
    ContextTimer& operator=(ContextTimer&&) = delete;

    ~ContextTimer() {
        auto dif = std::chrono::steady_clock::now() - start_;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dif);
        if (!name_.empty()) {
            std::cerr << "[" << name_ << "] ";
        }
        std::cerr << ms.count() << " ms\n";
    }

private:
    std::string name_;
    std::chrono::steady_clock::time_point start_;
};

template<typename F>
auto distribution(int n, F&& f) -> std::map<decltype(f()), int> {
    std::map<decltype(f()), int> dist;
    for (int i = 0; i < n; ++i) {
        ++dist[f()];
    }
    return dist;
}

inline void checkLargeParameter(int n) {
#ifdef JNGEN_I_WANT_LARGE_OBJECTS
    (void)n;
#else
    constexpr static int BOUND = 5e6;
    ensure(
        n <= BOUND,
        "If you want to generate an object of size > 5'000'000, please use "
        "#define JNGEN_I_WANT_LARGE_OBJECTS prior to including Jngen");
#endif // JNGEN_I_WANT_LARGE_OBJECTS
}

// Some type traits helpers. Based on ideas from TCPPPL v4.
template<bool B, typename T>
using enable_if_t = typename std::enable_if<B, T>::type;

namespace util {

inline long long gcd(long long a, long long b) {
    if (a < 0) {
        a = -a;
    }
    if (b < 0) {
        b = -b;
    }

    while (a && b) {
        if (a > b) {
            a %= b;
        } else {
            b %= a;
        }
    }
    return a + b;
}

} // namespace util

} // namespace jngen

using jngen::format;
using jngen::ContextTimer;
using jngen::distribution;

#include <string>
#include <utility>
#include <vector>

namespace jngen {
namespace drawing {

enum class Color {
    None,
    White,
    Black,
    Red,
    Green,
    Blue,
    Grey,
    LightGrey
};

struct DrawingEngineState {
    double width;
    Color stroke;
    Color fill;
    double opacity;
};

class DrawingEngine {
public:
    DrawingEngine() {}
    virtual ~DrawingEngine() {}

    virtual void drawPoint(double x, double y) = 0;
    virtual void drawCircle(double x, double y, double r) = 0;
    virtual void drawSegment(double x1, double y1, double x2, double y2) = 0;
    virtual void drawPolygon(
        const std::vector<std::pair<double, double>>& vertices) = 0;
    virtual void drawText(
        double x, double y, const std::string& s) = 0;

    virtual void setWidth(double width) = 0;
    virtual void setStroke(Color color) = 0;
    virtual void setFill(Color color) = 0;
    virtual void setOpacity(double opacity) = 0;

    virtual double width() const = 0;
    virtual Color stroke() const = 0;
    virtual Color fill() const = 0;
    virtual double opacity() const = 0;

    DrawingEngineState saveState() {
        return { width(), stroke(), fill(), opacity() };
    }

    void restoreState(const DrawingEngineState& state) {
        setWidth(state.width);
        setStroke(state.stroke);
        setFill(state.fill);
        setOpacity(state.opacity);
    }
};

}} // namespace jngen::drawing


#include <sstream>
#include <string>

namespace jngen {
namespace drawing {

class SvgEngine : public DrawingEngine {
public:
    SvgEngine(double x1 = 0, double y1 = 0, double x2 = 50, double y2 = 50);

    virtual ~SvgEngine() {}

    virtual void drawPoint(double x, double y) override;
    virtual void drawCircle(double x, double y, double r) override;
    virtual void drawSegment(
        double x1, double y1, double x2, double y2) override;
    virtual void drawPolygon(
        const std::vector<std::pair<double, double>>& vertices) override;
    virtual void drawText(
        double x, double y, const std::string& s) override;

    virtual void setWidth(double width) override;
    virtual void setStroke(Color color) override;
    virtual void setFill(Color color) override;
    virtual void setOpacity(double opacity) override;

    virtual double width() const override { return width_; }
    virtual Color stroke() const override { return strokeColor_; }
    virtual Color fill() const override { return fillColor_; }
    virtual double opacity() const override { return opacity_; }

    std::string serialize() const;

private:
    static const char* colorToString(Color color);

    double lerpX(double x) const;
    double lerpY(double y) const;
    double scaleSize(double size) const;

    std::string getStyle() const;

    std::ostringstream output_;

    double width_;
    Color strokeColor_;
    Color fillColor_;
    double opacity_;

    double x1_, y1_, x2_, y2_; // borders
};

inline const char* SvgEngine::colorToString(Color color) {
    switch (color) {
        case Color::None: return "none";
        case Color::White: return "white";
        case Color::Black: return "black";
        case Color::Red: return "red";
        case Color::Green: return "green";
        case Color::Blue: return "blue";
        case Color::Grey: return "grey";
        case Color::LightGrey: return "lightgrey";
        default: return "none";
    }
}

}} // namespace jngen::drawing

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_SVG_ENGINE_INL_H
#ifndef JNGEN_INCLUDE_SVG_ENGINE_INL_H
#error File "svg_engine_inl.h" must not be included directly.
#endif

#include <cmath>
#include <cstdlib>

namespace jngen {
namespace drawing {

static char buf[10000];
constexpr static double WIDTH_SCALE = 8;
constexpr static double CANVAS_SIZE = 2000;
constexpr static int FONT_SIZE = 64;

namespace {

// Given x \in [l, r], return linear interpolation to [L, R]
double lerp(double x, double l, double r, double L, double R) {
    return L + (R - L) * ((x - l) / (r - l));
}

} // namespace

SvgEngine::SvgEngine(double x1, double y1, double x2, double y2) :
    width_(1.0),
    strokeColor_(Color::Black),
    fillColor_(Color::Black),
    opacity_(1.0),
    x1_(x1),
    y1_(y1),
    x2_(x2),
    y2_(y2)
{  }

void SvgEngine::drawPoint(double x, double y) {
    x = lerpX(x);
    y = lerpY(y);
    double w = width_ * WIDTH_SCALE * 1.5;
    std::sprintf(
        buf,
        "<circle cx='%f' cy='%f' r='%f' fill='%s' opacity='%f'/>",
        x, y, w, colorToString(strokeColor_), opacity_
    );
    output_ << buf << "\n";
}

void SvgEngine::drawCircle(double x, double y, double r) {
    x = lerpX(x);
    y = lerpY(y);
    r = scaleSize(r);
    std::sprintf(
        buf,
        "<circle cx='%f' cy='%f' r='%f' style='%s'/>",
        x, y, r, getStyle().c_str()
    );
    output_ << buf << "\n";
}

void SvgEngine::drawPolygon(
    const std::vector<std::pair<double, double>>& points)
{
    output_ << "<polygon points='";
    for (const auto& point: points) {
        output_ << lerpX(point.first) << "," << lerpY(point.second) << " ";
    }
    output_ << "' style='" << getStyle() << "'/>\n";
}

void SvgEngine::drawSegment(
        double x1, double y1, double x2, double y2)
{
    x1 = lerpX(x1);
    y1 = lerpY(y1);
    x2 = lerpX(x2);
    y2 = lerpY(y2);
    if (std::fabs(x1 - x2) < 1e-9) {
        x1 = std::round(x1);
        x2 = std::round(x2);
    }
    if (std::fabs(y1 - y2) < 1e-9) {
        y1 = std::round(y1);
        y2 = std::round(y2);
    }
    std::sprintf(
        buf,
        "<line x1='%f' y1='%f' x2='%f' y2='%f' style='%s'/>",
        x1, y1, x2, y2, getStyle().c_str()
    );
    output_ << buf << "\n";
}

void SvgEngine::drawText(
    double x, double y, const std::string& s)
{
    x = std::round(lerpX(x));
    y = std::round(lerpY(y));
    std::sprintf(
        buf,
        "<text x='%f' y='%f' font-size='%d' font-family='Helvetica'>%s</text>",
        x, y, FONT_SIZE, s.c_str()
    );
    output_ << buf << "\n";
}

void SvgEngine::setWidth(double width) {
    width_ = width;
}

void SvgEngine::setStroke(Color color) {
    strokeColor_ = color;
}

void SvgEngine::setFill(Color color) {
    fillColor_ = color;
}

void SvgEngine::setOpacity(double opacity) {
    opacity_ = opacity;
}

std::string SvgEngine::serialize() const {
    int offset = std::sprintf(
        buf,
        "<svg xmlns='http://www.w3.org/2000/svg' "
        "viewBox='%f %f %f %f'>\n",
        0.0, 0.0, CANVAS_SIZE, CANVAS_SIZE * (y2_ - y1_) / (x2_ - x1_)
    );
    std::sprintf(
        buf + offset,
        "<circle cx='%f' cy='%f' r='%f' fill='white'/>\n",
        CANVAS_SIZE/2, CANVAS_SIZE/2,
        std::hypot(CANVAS_SIZE, CANVAS_SIZE * (y2_ - y1_) / (x2_ - x1_))
    );

    return buf + output_.str() + "</svg>\n";
}

double SvgEngine::lerpX(double x) const {
    return lerp(x, x1_, x2_, 0., CANVAS_SIZE);
}

double SvgEngine::lerpY(double y) const {
    return lerp(y, y2_, y1_, 0., CANVAS_SIZE * (y2_ - y1_) / (x2_ - x1_));
}

double SvgEngine::scaleSize(double size) const {
    return size * CANVAS_SIZE / (x2_ - x1_);
}

std::string SvgEngine::getStyle() const {
    static char buf[1024];
    std::sprintf(
        buf,
        "stroke-width:%f;stroke:%s;fill:%s;opacity:%f",
        width_ * WIDTH_SCALE,
        colorToString(strokeColor_),
        colorToString(fillColor_),
        opacity_
    );
    return buf;
}

}} // namespace jngen::drawing

#undef JNGEN_INCLUDE_SVG_ENGINE_INL_H
#endif // JNGEN_DECLARE_ONLY


#include <algorithm>
#include <cmath>
#include <fstream>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <utility>
#include <vector>

namespace jngen {
namespace drawing {

class Drawer {
public:
    Drawer();

    template<typename P>
    void point(const P& p);

    template<typename P>
    void circle(const P& p, double radius);

    template<typename P>
    void segment(const P& p1, const P& p2);

    template<typename P>
    void polygon(const std::vector<P>& points);

    void setWidth(double width);

    void setColor(Color color);
    void setColor(const std::string& desc);

    void setStroke(Color color);
    void setStroke(const std::string& desc);

    void setFill(Color color);
    void setFill(const std::string& desc);

    void setOpacity(double opacity);

    void dumpSvg(const std::string& filename);

private:
    struct Point {
        double x, y;
        Point() {}
        Point(double x, double y) : x(x), y(y) {}
    };

    typedef std::function<void(DrawingEngine*)> DrawRequest;

    typedef std::pair<Point, Point> Bbox;

    static Color colorByName(const std::string& name);

    static Bbox emptyBbox();

    static Bbox unite(const Bbox& lhs, const Bbox& rhs);

    static Bbox bbox(const Point& p);
    static Bbox bbox(const std::pair<Point, double>& circle);

    Bbox getBbox() const;

    static Bbox viewportByBbox(const Bbox& bbox);

    void drawAll();
    void drawGrid(const Bbox& bbox);

    std::vector<DrawRequest> requests_;

    DrawingEngine* engine_;
    Bbox bbox_;
    int requestId_ = 0;
};

template<typename P>
void Drawer::point(const P& p) {
    bbox_ = unite(bbox_ , bbox(Point(p.x, p.y)));
    requests_.push_back([p](DrawingEngine* engine) {
        engine->drawPoint(p.x, p.y);
    });
}

template<typename P>
void Drawer::circle(const P& p, double radius) {
    bbox_ = unite(bbox_ , bbox({Point(p.x, p.y), radius}));
    requests_.push_back([p, radius](DrawingEngine* engine) {
        engine->drawCircle(p.x, p.y, radius);
    });
}

template<typename P>
void Drawer::segment(const P& p1, const P& p2) {
    bbox_ = unite(bbox_ , bbox(Point(p1.x, p1.y)));
    bbox_ = unite(bbox_ , bbox(Point(p2.x, p2.y)));
    requests_.push_back([p1, p2](DrawingEngine* engine) {
        engine->drawSegment(p1.x, p1.y, p2.x, p2.y);
    });
}

template<typename P>
void Drawer::polygon(const std::vector<P>& points) {
    for (const auto& p: points) {
        bbox_ = unite(bbox_, bbox(Point(p.x, p.y)));
    }

    requests_.push_back([points](DrawingEngine* engine) {
        std::vector<std::pair<double, double>> enginePoints;
        for (const auto& p: points) {
            enginePoints.emplace_back(p.x, p.y);
        }
        engine->drawPolygon(enginePoints);
    });
}

#ifndef JNGEN_DECLARE_ONLY

Drawer::Drawer() : bbox_(emptyBbox()) {
    setFill("none");
    setStroke("black");
}

Color Drawer::colorByName(const std::string& name) {
    const static std::map<std::string, Color> colors = {
        {"", Color::None},
        {"none", Color::None},
        {"white", Color::White},
        {"black", Color::Black},
        {"red", Color::Red},
        {"green", Color::Green},
        {"blue", Color::Blue},
        {"grey", Color::Grey},
        {"lightgrey", Color::LightGrey},
        {"gray", Color::Grey},
        {"lightgray", Color::LightGrey}
    };

    if (!colors.count(name)) {
        throw std::logic_error("Color `" + name+ "' is not supported");
    }

    return colors.at(name);
}

void Drawer::setWidth(double width) {
    requests_.push_back([width](DrawingEngine* engine) {
        engine->setWidth(width);
    });
}

void Drawer::setColor(Color color) {
    setStroke(color);
    setFill(color);
}

void Drawer::setStroke(Color color) {
    requests_.push_back([color](DrawingEngine* engine) {
        engine->setStroke(color);
    });
}

void Drawer::setFill(Color color) {
    requests_.push_back([color](DrawingEngine* engine) {
        engine->setFill(color);
    });
}

void Drawer::setColor(const std::string& desc) {
    setColor(colorByName(desc));
}

void Drawer::setStroke(const std::string& desc) {
    setStroke(colorByName(desc));
}

void Drawer::setFill(const std::string& desc) {
    setFill(colorByName(desc));
}

void Drawer::setOpacity(double opacity) {
    requests_.push_back([opacity](DrawingEngine* engine) {
        engine->setOpacity(opacity);
    });
}

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
    for (const auto& request: requests_) {
        request(engine_);
    }
}

void Drawer::drawGrid(const Bbox& bbox) {
    const static std::vector<int> STEP_DELTA = {20, 25, 20};
    // Step goes like 1, 2, 5, 10, 20, 50, 100, ...
    constexpr static int SMALL_IN_BIG = 5;
    constexpr static int THRESHOLD = 8;
    constexpr static int MAX_SPREAD_TO_DRAW_ALL_TICKS = 13;
    constexpr static double TEXT_OFFSET_RATIO = 0.01;

    auto savedState = engine_->saveState();

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
    engine_->setStroke(Color::LightGrey);

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
    engine_->setStroke(Color::Grey);

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

    engine_->restoreState(savedState);
}

void Drawer::dumpSvg(const std::string& filename) {
    if (requests_.empty()) {
        return;
    }

    auto bbox = bbox_;
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

#include <algorithm>
#include <vector>

namespace jngen {

class Dsu {
public:
    int getRoot(int x);

    bool unite(int x, int y);

    bool isConnected() const { return components <= 1; }

    void extend(size_t size);

private:
    std::vector<int> parent;
    std::vector<int> rank;

    int components = 0;
};

#ifndef JNGEN_DECLARE_ONLY

int Dsu::getRoot(int x) {
    extend(x);

    return parent[x] == x ? x : (parent[x] = getRoot(parent[x]));
}

bool Dsu::unite(int x, int y) {
    extend(std::max(x, y) + 1);

    x = getRoot(x);
    y = getRoot(y);
    if (x == y) {
        return false;
    }

    if (rank[x] > rank[y]) {
        std::swap(x, y);
    }
    if (rank[y] == rank[x]) {
        ++rank[y];
    }
    parent[x] = y;

    --components;

    return true;
}

void Dsu::extend(size_t x) {
    size_t last = parent.size() - 1;
    while (parent.size() < x) {
        ++components;
        parent.push_back(++last);
        rank.push_back(0);
    }
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

#include <functional>

namespace jngen {

class Graph;

namespace graph_detail {

class GraphRandom;

struct Traits {
    int n;
    int m;
    bool directed = false;
    bool allowLoops = false;
    bool allowMulti = false;
    bool connected = false;

    Traits() {}
    explicit Traits(int n) : n(n) {}
    Traits(int n, int m) : n(n), m(m) {}
};

class BuilderProxy {
public:
    BuilderProxy(
            Traits traits,
            std::function<Graph(Traits)> builder) :
        traits_(traits),
        builder_(builder)
    {  }

    Graph g() const;

    operator Graph() const;

    BuilderProxy& allowLoops(bool value = true) {
        traits_.allowLoops = value;
        return *this;
    }

    BuilderProxy& allowMulti(bool value = true) {
        traits_.allowMulti = value;
        return *this;
    }

    BuilderProxy& connected(bool value = true) {
        traits_.connected = value;
        return *this;
    }

private:
    Traits traits_;
    std::function<Graph(Traits)> builder_;
};

} // namespace graph_detail

} // namespace jngen


#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace jngen {

struct VariableMap {
    std::vector<std::string> positional;
    std::map<std::string, std::string> named;

    int count(size_t pos) const {
        return pos < positional.size();
    }

    int count(const std::string& name) const {
        return named.count(name);
    }

    std::string operator[](size_t pos) const {
        if (!count(pos)) {
            return "";
        }
        return positional.at(pos);
    }

    std::string operator[](const std::string& name) const {
        if (!count(name)) {
            return "";
        }
        return named.at(name);
    }

    bool initialized = false;
};

// TODO: think about seed as a last argument
inline VariableMap parseArguments(const std::vector<std::string>& args) {
    VariableMap result;

    auto setNamedVar = [&result](
            const std::string& name,
            const std::string& value)
    {
        ensure(
            !result.count(value),
            "Named arguments must have distinct names");
        result.named[name] = value;
    };

    std::string pendingVarName;

    for (const std::string& s: args) {
        if (s == "-") {
            continue;
        }
        if (s == "--") {
            break;
        }

        if (s[0] != '-') {
            if (!pendingVarName.empty()) {
                setNamedVar(pendingVarName, s);
                pendingVarName = "";
            } else {
                result.positional.push_back(s);
            }
            continue;
        }

        if (!pendingVarName.empty()) {
            result.named[pendingVarName] = "1";
            pendingVarName = "";
        }

        std::string name;
        std::string value;
        bool foundEq = false;
        for (char c: s.substr(1)) {
            if (!foundEq && c == '=') {
                foundEq = true;
            } else {
                if (foundEq) {
                    value += c;
                } else {
                    name += c;
                }
            }
        }
        if (foundEq) {
            setNamedVar(name, value);
        } else {
            pendingVarName = name;
        }

        setNamedVar(name, value);
    }

    if (!pendingVarName.empty()) {
        result.named[pendingVarName] = "1";
    }

    result.initialized = true;
    return result;
}

JNGEN_EXTERN VariableMap vmap;

template<typename T>
bool readVariable(const std::string& value, T& var) {
    std::istringstream ss(value);

    T t;
    if (ss >> t) {
        var = t;
        return true;
    }
    return false;
}

template<typename T>
bool getOpt(size_t index, T& var) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getOpt(...)");
    if (!vmap.count(index)) {
        return false;
    }
    return readVariable(vmap[index], var);
}

template<typename T>
bool getOpt(const std::string& name, T& var) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getOpt(...)");
    if (!vmap.count(name)) {
        return false;
    }
    return readVariable(vmap[name], var);
}

template<typename T>
T getOptOr(size_t index, T def) {
    getOpt(index, def);
    return def;
}

template<typename T>
T getOptOr(const std::string& name, T def) {
    getOpt(name, def);
    return def;
}

inline std::string getOptOr(size_t index, const char* def) {
    std::string defString(def);
    getOpt(index, defString);
    return defString;
}

inline std::string getOptOr(const std::string& name, const char* def) {
    std::string defString(def);
    getOpt(name, defString);
    return defString;
}

inline void parseArgs(int argc, char *argv[]) {
    vmap = parseArguments(std::vector<std::string>(argv + 1, argv + argc));
}

namespace detail {

inline std::vector<std::string> splitByComma(std::string s) {
    auto strip = [](std::string s) {
        size_t l = 0;
        while (l < s.size() && s[l] == ' ') {
            ++l;
        }
        s = s.substr(l);
        while (!s.empty() && s.back() == ' ') {
            s.pop_back();
        }
        return s;
    };

    std::vector<std::string> result;
    s += ',';
    std::string cur;

    for (char c: s) {
        if (c == ',') {
            result.push_back(strip(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }

    return result;
}

inline int getNamedImpl(std::vector<std::string>::const_iterator) { return 0; }

template<typename T, typename ... Args>
int getNamedImpl(
    std::vector<std::string>::const_iterator it, T& var, Args&... args)
{
    int res = getOpt(*it, var);
    res += getNamedImpl(++it, args...);
    return res;
}

inline int getPositionalImpl(size_t) { return 0; }

template<typename T, typename ... Args>
int getPositionalImpl(size_t index, T& var, Args&... args) {
    int res = getOpt(index, var);
    res += getPositionalImpl(index + 1, args...);
    return res;
}

} // namespace detail

template<typename ... Args>
int doGetNamed(const std::string& names, Args&... args) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getNamed(...)");

    auto namesSplit = detail::splitByComma(names);

    ENSURE(
        namesSplit.size() == sizeof...(args),
        "Number of names is not equal to number of variables");

    return detail::getNamedImpl(namesSplit.begin(), args...);
}

template<typename ... Args>
int getPositional(Args&... args) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getPositional(...)");

    return detail::getPositionalImpl(0, args...);
}

} // namespace jngen

using jngen::parseArgs;
using jngen::getOpt;
using jngen::getOptOr;

using jngen::getPositional;

#define getNamed(...) jngen::doGetNamed(#__VA_ARGS__, __VA_ARGS__)


#include <algorithm>
#include <cctype>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

// TODO: adequate error messages

namespace jngen {

class Pattern {
    friend class Parser;
public:
    Pattern() : isOrPattern(false), min(1), max(1) {}
    Pattern(const std::string& s);

    std::string next(std::function<int(int)> rnd) const;

private:
    Pattern(Pattern p, std::pair<int, int> quantity) :
        isOrPattern(false),
        min(quantity.first),
        max(quantity.second)
    {
        children.push_back(std::move(p));
    }

    Pattern(std::vector<char> chars, std::pair<int, int> quantity) :
        chars(std::move(chars)),
        isOrPattern(false),
        min(quantity.first),
        max(quantity.second)
    {  }

    std::vector<char> chars;
    std::vector<Pattern> children;
    bool isOrPattern;
    int min;
    int max;
};

class Parser {
public:
    Pattern parse(const std::string& s) {
        this->s = s;
        pos = 0;
        return parsePattern();
    }

private:
    static bool isControl(char c) {
        static const std::string CONTROL_CHARS = "()[]{}|?";
        return CONTROL_CHARS.find(c) != std::string::npos;
    }

    static int control(int c) {
        return c >> 8;
    }

    int next() {
        size_t newPos;
        int result = peekAndMove(newPos);
        pos = newPos;
        return result;
    }

    int peek() const {
        size_t dummy;
        return peekAndMove(dummy);
    }

    int peekAndMove(size_t& newPos) const {
        newPos = pos;
        if (pos == s.size()) {
            return -1;
        }
        if (s[pos] == '\\') {
            ensure(
                pos+1 < s.size(),
                "Backslash at the end of the pattern is illegal");
            newPos += 2;
            return s[pos+1];
        }

        ++newPos;
        int ret = s[pos];
        return isControl(ret) ? (ret << 8) : ret;
    }

    // TODO: catch overflows
    int readInt() {
        ENSURE(std::isdigit(peek()));

        int res = 0;
        while (std::isdigit(peek())) {
            res = res * 10 + next() - '0';
        }
        return res;
    }

    std::pair<int, int> parseRange() {
        ENSURE(control(next()) == '{');

        int from = readInt();

        int nxt = next();
        if (control(nxt) == '}') {
            return {from, from};
        } else if (nxt == ',' || nxt == '-') {
            int to = readInt();
            ENSURE(control(next()) == '}');
            return {from, to};
        } else {
            ensure(false, "cannot parse character range");
        }
    }

    std::pair<int, int> tryParseQuantity() {
        std::pair<int, int> quantity = {1, 1};

        int qchar = peek();
        if (control(qchar) == '?') {
            quantity = {0, 1};
            next();
        } else if (control(qchar) == '{') {
            quantity = parseRange();
        }

        return quantity;
    }

    std::vector<char> parseBlock() {
        std::vector<char> allowed;
        char last = -1;
        bool inRange = false;
        while (control(peek()) != ']') {
            char c = next(); // buggy on cases like [a-}]
            ENSURE(c != -1);

            if (c == '-') {
                ensure(!inRange, "invalid pattern");
                inRange = true;
            } else if (inRange) {
                ensure(c >= last, "invalid pattern");
                for (char i = last; i <= c; ++i) {
                    allowed.push_back(i);
                }
                inRange = false;
                last = -1;
            } else {
                if (last != -1) {
                    allowed.push_back(last);
                }
                last = c;
            }
        }

        ENSURE(control(next()) == ']');

        ENSURE(!inRange);
        if (last != -1) {
            allowed.push_back(last);
        }

        std::sort(allowed.begin(), allowed.end());
        return allowed;
    }

    Pattern parsePattern() {
        std::vector<Pattern> orPatterns;
        Pattern cur;

        while (true) {
            int nxt = next();
            if (nxt == -1 || control(nxt) == ')') {
                break;
            } else if (control(nxt) == '(') {
                Pattern p = parsePattern();
                cur.children.push_back(Pattern(p, tryParseQuantity()));
            } else if (control(nxt) == '|') {
                orPatterns.emplace_back();
                std::swap(orPatterns.back(), cur);
            } else {
                std::vector<char> chars;
                if (control(nxt) == '[') {
                    chars = parseBlock();
                } else {
                    ENSURE(!control(nxt));
                    chars = {static_cast<char>(nxt)};
                }

                cur.children.push_back(Pattern(chars, tryParseQuantity()));
            }
        }

        if (orPatterns.empty()) {
            return cur;
        } else {
            orPatterns.emplace_back();
            std::swap(orPatterns.back(), cur);

            Pattern p;
            p.isOrPattern = true;
            p.children = orPatterns;
            return p;
        }
    }

    std::string s;
    size_t pos;
};

#ifndef JNGEN_DECLARE_ONLY
Pattern::Pattern(const std::string& s) {
    *this = Parser().parse(s);
}

std::string Pattern::next(std::function<int(int)> rnd) const {
    if (isOrPattern) {
        ENSURE(!children.empty());
        return children[rnd(children.size())].next(rnd);
    }

    ENSURE( (!!chars.empty()) ^ (!!children.empty()) );

    int count;
    if (min == max) {
        count = min;
    } else {
        count = min + rnd(max - min + 1);
    }

    std::string result;
    for (int i = 0; i < count; ++i) {
        if (!children.empty()) {
            for (const Pattern& p: children) {
                result += p.next(rnd);
            }
        } else {
            result += chars[rnd(chars.size())];
        }
    }

    return result;
}
#endif

} // namespace jngen

using jngen::Pattern;


#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace jngen {

static void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ENSURE(engine() == 822569775,
        "std::mt19937 doesn't conform to the C++ standard");
    ENSURE(engine() == 2137449171,
        "std::mt19937 doesn't conform to the C++ standard");
    ENSURE(engine() == 2671936806,
        "std::mt19937 doesn't conform to the C++ standard");
}

static void assertIntegerSizes() {
    static_assert(
        std::numeric_limits<unsigned char>::max() == 255,
        "max(unsigned char) != 255");
    static_assert(sizeof(int) == 4, "sizeof(int) != 4");
    static_assert(sizeof(long long) == 8, "sizeof(long long) != 8");
    static_assert(
        sizeof(size_t) == 4 || sizeof(size_t) == 8,
        "sizeof(size_t) is neither 4 nor 8");
    static_assert(
        sizeof(std::size_t) == sizeof(size_t),
        "sizeof(size_t) != sizeof(std::size_t)");
}

class Random;

class BaseTypedRandom {
public:
    BaseTypedRandom(Random& random) : random(random) {}

protected:
    Random& random;
};

template<typename T>
struct TypedRandom;

inline uint64_t maskForBound(uint64_t bound) {
    --bound;
    uint64_t mask = ~0;
    if ((mask >> 32) >= bound) mask >>= 32;
    if ((mask >> 16) >= bound) mask >>= 16;
    if ((mask >> 8 ) >= bound) mask >>= 8 ;
    if ((mask >> 4 ) >= bound) mask >>= 4 ;
    if ((mask >> 2 ) >= bound) mask >>= 2 ;
    if ((mask >> 1 ) >= bound) mask >>= 1 ;
    return mask;
}

template<typename Result, typename Source>
Result uniformRandom(Result bound, Random& random, Source (Random::*method)()) {
    static_assert(sizeof(Result) <= sizeof(Source),
        "uniformRandom: Source type must be at least as large as Result type");
#ifdef JNGEN_FAST_RANDOM
    return (random.*method)() % bound;
#else
    Source mask = maskForBound(bound);
    while (true) {
        Source outcome = (random.*method)() & mask;
        if (outcome < static_cast<Source>(bound)) {
            return outcome;
        }
    }
#endif
}

class Random {
public:
    Random() {
        assertRandomEngineConsistency();
        assertIntegerSizes();
        seed(std::random_device{}());
    }

    void seed(uint32_t val) {
        randomEngine_.seed(val);
    }

    void seed(const std::vector<uint32_t>& seed) {
        std::seed_seq seq(seed.begin(), seed.end());
        randomEngine_.seed(seq);
    }

    uint32_t next() {
        return randomEngine_();
    }

    uint64_t next64() {
        uint64_t a = next();
        uint64_t b = next();
        return (a << 32) ^ b;
    }

    double nextf() {
        return (double)randomEngine_() / randomEngine_.max();
    }

    int next(int n) {
        ensure(n > 0);
        return uniformRandom(n, *this, (uint32_t (Random::*)())&Random::next);
    }

    long long next(long long n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    size_t next(size_t n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    double next(double n) {
        ensure(n >= 0);
        return nextf() * n;
    }

    int next(int l, int r) {
        ensure(l <= r);
        uint32_t n = static_cast<uint32_t>(r) - l + 1;
        return l + uniformRandom(
            n, *this, (uint32_t (Random::*)())&Random::next);
    }

    long long next(long long l, long long r) {
        ensure(l <= r);
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + uniformRandom(n, *this, &Random::next64);
    }

    size_t next(size_t l, size_t r) {
        ensure(l <= r);
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + uniformRandom(n, *this, &Random::next64);
    }

    double next(double l, double r) {
        ensure(l <= r);
        return l + next(r-l);
    }

    //  implemented in random_inl.h
    int wnext(int n, int w);
    long long wnext(long long n, int w);
    size_t wnext(size_t n, int w);
    double wnext(double n, int w);

    int wnext(int l, int r, int w);
    long long wnext(long long l, long long r, int w);
    size_t wnext(size_t l, size_t r, int w);
    double wnext(double l, double r, int w);

    std::string next(const std::string& pattern) {
        return Pattern(pattern).next([this](int n) { return next(n); });
    }

    template<typename ... Args>
    std::string next(const std::string& pattern, Args... args) {
        return next(format(pattern, args...));
    }

    template<typename T, typename ... Args>
    T tnext(Args... args) {
        return TypedRandom<T>{*this}.next(args...);
    }

    template<typename ... Args>
    std::pair<int, int> nextp(Args... args) {
        return tnext<std::pair<int, int>>(args...);
    }

    template<typename Iterator>
    typename Iterator::value_type choice(Iterator begin, Iterator end) {
        auto length = std::distance(begin, end);
        ensure(length > 0, "Cannot select from a range of negative length");
        size_t index = tnext<size_t>(length);
        std::advance(begin, index);
        return *begin;
    }

    template<typename Container>
    typename Container::value_type choice(const Container& container) {
        ensure(!container.empty(), "Cannot select from an empty container");
        return choice(container.begin(), container.end());
    }

private:
    template<typename T, typename ...Args>
    T smallWnext(int w, Args... args) {
        ENSURE(std::abs(w) <= WNEXT_LIMIT);
        T result = next(args...);
        while (w > 0) {
            result = std::max(result, next(args...));
            --w;
        }
        while (w < 0) {
            result = std::min(result, next(args...));
            ++w;
        }
        return result;
    }

    double realWnext(int w) {
        if (w == 0) {
            return nextf();
        } else if (w > 0) {
            return std::pow(nextf(), 1.0 / (w + 1));
        } else {
            return 1.0 - std::pow(nextf(), 1.0 / (-w + 1));
        }
    }

    std::mt19937 randomEngine_;
    constexpr static int WNEXT_LIMIT = 8;
};

JNGEN_EXTERN Random rnd;

template<>
struct TypedRandom<int> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    int next(int n) { return random.next(n); }
    int next(int l, int r) { return random.next(l, r); }
};

template<>
struct TypedRandom<double> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    double next(double n) { return random.next(n); }
    double next(double l, double r) { return random.next(l, r); }
};

template<>
struct TypedRandom<long long> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    long long next(long long n) { return random.next(n); }
    long long next(long long l, long long r) { return random.next(l, r); }
};

template<>
struct TypedRandom<size_t> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    size_t next(size_t n) { return random.next(n); }
    size_t next(size_t l, size_t r) { return random.next(l, r); }
};

template<>
struct TypedRandom<char> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    char next(char n) { return random.next(n); }
    char next(char l, char r) { return random.next(l, r); }
};

template<typename T>
struct TypedRandom : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    template<typename ... Args>
    T next(Args... args) { return random.next(args...); }
};

struct RandomPairTraits {
    const bool ordered;
    const bool distinct;
};

#ifdef JNGEN_DECLARE_ONLY
extern RandomPairTraits opair, dpair, odpair, dopair;
#else
RandomPairTraits opair{true, false};
RandomPairTraits dpair{false, true};
RandomPairTraits odpair{true, true};
RandomPairTraits dopair{true, true};
#endif

template<>
struct TypedRandom<std::pair<int, int>> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;

    std::pair<int, int> next(int n) {
        return next(n, {false, false});
    }
    std::pair<int, int> next(int l, int r) {
        return next(l, r, {false, false});
    }

    std::pair<int, int> next(int n, RandomPairTraits traits) {
        int first = rnd.next(n);
        int second;
        do {
            second = rnd.next(n);
        } while (traits.distinct && first == second);
        if (traits.ordered && first > second) {
            std::swap(first, second);
        }
        return {first, second};
    }
    std::pair<int, int> next(int l, int r, RandomPairTraits traits) {
        auto res = next(r-l+1, traits);
        res.first += l;
        res.second += l;
        return res;
    }

private:
    std::pair<int, int> ordered(std::pair<int, int> pair) const {
        if (pair.first > pair.second) {
            std::swap(pair.first, pair.second);
        }
        return pair;
    }
};

} // namespace jngen

using jngen::Random;

using jngen::rnd;
using jngen::opair;
using jngen::dpair;
using jngen::dopair;
using jngen::odpair;

inline void registerGen(int argc, char *argv[], int version = 1) {
    (void)version; // unused, only for testlib.h compatibility

    std::vector<uint32_t> seed;
    for (int i = 1; i < argc; ++i) {
        int startPosition = seed.size();
        seed.emplace_back();
        for (char *s = argv[i]; *s; ++s) {
            ++seed[startPosition];
            seed.push_back(*s);
        }
    }
    rnd.seed(seed);
}

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_RANDOM_INL_H
#ifndef JNGEN_INCLUDE_RANDOM_INL_H
#error File "random_inl.h" must not be included directly.
#endif

namespace jngen {

int Random::wnext(int n, int w) {
    ensure(n > 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<int>(w, n);
    } else {
        double t = realWnext(w);
        return n * t;
    }
}

long long Random::wnext(long long n, int w) {
    ensure(n > 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<long long>(w, n);
    } else {
        return n * realWnext(w);
    }
}

size_t Random::wnext(size_t n, int w) {
    ensure(n > 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<size_t>(w, n);
    } else {
        return n * realWnext(w);
    }
}

double Random::wnext(double n, int w) {
    ensure(n >= 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<double>(w, n);
    } else {
        return realWnext(w) * n;
    }
}

int Random::wnext(int l, int r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<int>(w, l, r);
    } else {
        uint32_t n = static_cast<uint32_t>(r) - l + 1;
        return l + static_cast<uint32_t>(n * realWnext(w));
    }
}

long long Random::wnext(long long l, long long r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<long long>(w, l, r);
    } else {
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + static_cast<uint64_t>(n * realWnext(w));
    }
}

size_t Random::wnext(size_t l, size_t r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<size_t>(w, l, r);
    } else {
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + static_cast<uint64_t>(n * realWnext(w));
    }
}

double Random::wnext(double l, double r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<double>(w, l, r);
    } else {
        return realWnext(w) * (r - l) + l;
    }
}

} // namespace jngen
#undef JNGEN_INCLUDE_RANDOM_INL_H
#endif // JNGEN_DECLARE_ONLY

#include <iostream>
#include <type_traits>

namespace jngen {

template<int N> struct PTag : PTag<N-1> {};
template<> struct PTag<0> {};
struct PTagMax : PTag<20> {};

struct OutputModifier {
    int addition = 0;
    bool printN = false;
    bool printM = false;

    bool printParents = false;
    bool printEdges = true;

    char sep = ' ';
};

JNGEN_EXTERN OutputModifier defaultMod;

template<typename T>
class Repr {
    friend std::ostream& operator<<(std::ostream& out, const Repr& repr) {
        repr.print(out);
        return out;
    }

    template<typename P>
    friend Repr<P> repr(const P& t);

    template<typename P>
    friend class ReprProxy;

protected:
    Repr() = delete;
    Repr(const Repr<T>&) = default;
    Repr<T>& operator=(const Repr<T>&) = default;
    Repr(Repr<T>&&) = default;
    Repr<T>& operator=(Repr<T>&&) = default;

public:
    Repr(const T& object) :
        object_(object),
        mod_(defaultMod)
    {  }

    Repr<T>& add1(bool value = true) {
        mod_.addition = value;
        return *this;
    }

    Repr<T>& printN(bool value = true) {
        mod_.printN = value;
        return *this;
    }

    Repr<T>& printM(bool value = true) {
        mod_.printM = value;
        return *this;
    }

    Repr<T>& printParents(bool value = true) {
        mod_.printParents = value;
        mod_.printEdges = !value;
        return *this;
    }

    Repr<T>& printEdges(bool value = true) {
        mod_.printEdges = value;
        mod_.printParents = !value;
        return *this;
    }

    Repr<T>& endl(bool value = true) {
        mod_.sep = value ? '\n' : ' ';
        return *this;
    }

private:
    void print(std::ostream& out) const {
        printValue(out, object_, mod_, PTagMax{});
    }

    const T& object_;

protected:
    OutputModifier mod_;
};

class BaseReprProxy {};

template<typename T>
class ReprProxy : public BaseReprProxy {
    friend std::ostream& operator<<(std::ostream& out, const ReprProxy& proxy) {
        Repr<T> repr(static_cast<const T&>(proxy));
        return out << repr;
    }

public:
    Repr<T> add1(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.add1(value);
        return repr;
    }

    Repr<T> printN(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printN(value);
        return repr;
    }

    Repr<T> printM(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printM(value);
        return repr;
    }

    Repr<T> printParents(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printParents(value);
        return repr;
    }

    Repr<T> printEdges(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printEdges(value);
        return repr;
    }

    Repr<T> endl(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.endl(value);
        return repr;
    }

protected:
    ReprProxy() {
        static_assert(
            std::is_base_of<ReprProxy<T>, T>::value,
            "ReprProxy<T> must be inherited by T");
    }
};

template<typename T>
Repr<T> repr(const T& t) {
    return Repr<T>(t);
}

class DefaultModSetter : public Repr<int> {
    friend DefaultModSetter setMod();

private:
    DefaultModSetter(int val) :
        Repr<int>(val)
    {  }

public:
    ~DefaultModSetter() {
        defaultMod = mod_;
    }

    Repr<int>& reset() {
        mod_ = OutputModifier();
        return *this;
    }
};

inline DefaultModSetter setMod() {
    static int dummy = 0;
    return DefaultModSetter(dummy);
}

} // namespace jngen

using jngen::repr;
using jngen::setMod;


#include <iostream>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

namespace jngen {

namespace detail {

// TODO: maybe make it more clear SFINAE, like boost::has_left_shift<X,Y>?
// TODO: make these defines namespace independent

#define JNGEN_DEFINE_FUNCTION_CHECKER(name, expr)\
template<typename T, typename Enable = void>\
class Has ## name ## Helper: public std::false_type {};\
\
template<typename T>\
class Has ## name ## Helper<T,\
    decltype(void(\
        expr\
    ))\
> : public std::true_type {};\

#define JNGEN_HAS_FUNCTION(name)\
    detail::Has ## name ## Helper<T>::value

JNGEN_DEFINE_FUNCTION_CHECKER(
    OstreamMethod,
    std::declval<std::ostream&>().operator<< (std::declval<T>())
)

JNGEN_DEFINE_FUNCTION_CHECKER(
    OstreamFreeFunction,
    std::operator<<(std::declval<std::ostream&>(), std::declval<T>())
)

JNGEN_DEFINE_FUNCTION_CHECKER(
    Plus,
    T(std::declval<T>() + 1)
)

JNGEN_DEFINE_FUNCTION_CHECKER(
    Container,
    std::distance(std::declval<T>().begin(), std::declval<T>().end())
)

#define JNGEN_HAS_OSTREAM()\
    (JNGEN_HAS_FUNCTION(OstreamMethod) ||\
        JNGEN_HAS_FUNCTION(OstreamFreeFunction))

template<typename T>
struct VectorDepth {
    constexpr static int value = 0;
};

template<typename T, template <typename...> class C>
struct VectorDepth<C<T>> {
    constexpr static int value =
        std::is_base_of<
            std::vector<T>,
            C<T>
        >::value ? VectorDepth<T>::value + 1 : 0;
};

} // namespace detail

#define JNGEN_DECLARE_PRINTER(constraint, priority)\
template<typename T>\
auto printValue(\
    std::ostream& out, const T& t, const OutputModifier& mod, PTag<priority>)\
    -> enable_if_t<constraint, void>

#define JNGEN_DECLARE_SIMPLE_PRINTER(type, priority)\
inline void printValue(std::ostream& out, const type& t,\
    const OutputModifier& mod, PTag<priority>)

#define JNGEN_PRINT(value)\
printValue(out, value, mod, PTagMax{})

#define JNGEN_PRINT_NO_MOD(value)\
printValue(out, value, OutputModifier{}, PTagMax{})

JNGEN_DECLARE_PRINTER(!JNGEN_HAS_OSTREAM(), 0)
{
    // can't just write 'false' here because assertion always fails
    static_assert(!std::is_same<T, T>::value, "operator<< is undefined");
    (void)out;
    (void)mod;
    (void)t;
}

JNGEN_DECLARE_PRINTER(JNGEN_HAS_OSTREAM(), 10)
{
    (void)mod;
    out << t;
}

JNGEN_DECLARE_PRINTER(
    JNGEN_HAS_OSTREAM() && JNGEN_HAS_FUNCTION(Plus), 11)
{
    if (std::is_integral<T>::value) {
        out << T(t + mod.addition);
    } else {
        out << t;
    }
}

JNGEN_DECLARE_PRINTER(detail::VectorDepth<T>::value == 1, 3)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }
    bool first = true;
    for (const auto& x: t) {
        if (first) {
            first = false;
        } else {
            out << mod.sep;
        }
        JNGEN_PRINT(x);
    }
}

JNGEN_DECLARE_PRINTER(detail::VectorDepth<T>::value == 1 &&
    std::tuple_size<typename T::value_type>::value == 2, 4)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }

    bool first = true;
    for (const auto& x: t) {
        if (first) {
            first = false;
        } else {
            out << "\n";
        }
        JNGEN_PRINT(x);
    }
}

JNGEN_DECLARE_PRINTER(detail::VectorDepth<T>::value == 2, 4)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }
    for (const auto& x: t) {
        JNGEN_PRINT(x);
        out << "\n";
    }
}

JNGEN_DECLARE_PRINTER(JNGEN_HAS_FUNCTION(Container), 2)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }
    bool first = true;
    for (const auto& x: t) {
        if (first) {
            first = false;
        } else {
            out << " ";
        }
        JNGEN_PRINT(x);
    }
}

JNGEN_DECLARE_PRINTER(JNGEN_HAS_FUNCTION(Container)
    && std::tuple_size<typename T::value_type>::value == 2, 3)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }
    bool first = true;
    for (const auto& x: t) {
        if (first) {
            first = false;
        } else {
            out << "\n";
        }
        JNGEN_PRINT(x);
    }
}

// http://stackoverflow.com/a/19841470/2159939
#define JNGEN_COMMA ,

template<typename Lhs, typename Rhs>
JNGEN_DECLARE_SIMPLE_PRINTER(std::pair<Lhs JNGEN_COMMA Rhs>, 3)
{
    JNGEN_PRINT(t.first);
    out << " ";
    JNGEN_PRINT(t.second);
}

#undef JNGEN_COMMA

// Following snippet allows writing
//     cout << pair<int, int>(1, 2) << endl;
// in user code. I have to put it into separate namespace because
//   1) I don't want to 'use' all operator<< from jngen
//   2) I cannot do it in global namespace because JNGEN_HAS_OSTREAM relies
// on that it is in jngen.
namespace namespace_for_fake_operator_ltlt {

template<typename T>
auto operator<<(std::ostream& out, const T& t)
    -> enable_if_t<
            !JNGEN_HAS_OSTREAM() && !std::is_base_of<BaseReprProxy, T>::value,
            std::ostream&
        >
{
    // not jngen::printValue, because relying on ADL here for printers declared
    // later (see, e.g., http://stackoverflow.com/questions/42833134)
    printValue(out, t, jngen::defaultMod, jngen::PTagMax{});
    return out;
}

} // namespace namespace_for_fake_operator_ltlt

// Calling this operator inside jngen namespace doesn't work without this line.
using namespace jngen::namespace_for_fake_operator_ltlt;

} // namespace jngen

using namespace jngen::namespace_for_fake_operator_ltlt;


#include <algorithm>

namespace jngen {

// TODO: deprecate random_shuffle as done in testlib.h

template<typename Iterator>
void shuffle(Iterator begin, Iterator end) {
    ensure(end >= begin, "Cannot shuffle range of negative length");
    size_t size = end - begin;
    for (size_t i = 1; i < size; ++i) {
        std::swap(*(begin + i), *(begin + rnd.next(i + 1)));
    }
}

template<typename Iterator>
typename Iterator::value_type choice(Iterator begin, Iterator end) {
    return rnd.choice(begin, end);
}

template<typename Container>
typename Container::value_type choice(const Container& container) {
    return rnd.choice(container);
}

} // namespace jngen

using jngen::shuffle;
using jngen::choice;


#include <algorithm>
#include <initializer_list>
#include <numeric>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace jngen {

template<typename T>
class GenericArray : public ReprProxy<GenericArray<T>>, public std::vector<T> {
public:
    typedef std::vector<T> Base;

    using Base::Base;

    GenericArray() {}
    GenericArray(const GenericArray<T>&) = default;
    GenericArray& operator=(const GenericArray<T>&) = default;
    GenericArray(GenericArray<T>&&) = default;
    GenericArray& operator=(GenericArray<T>&&) = default;

    ~GenericArray() {}

    /* implicit */ GenericArray(const Base& base) :
            Base(base)
    {  }

    using Base::at;
    using Base::size;
    using Base::resize;
    using Base::begin;
    using Base::end;
    using Base::insert;
    using Base::clear;
    using Base::erase;

    void extend(size_t requiredSize) {
        checkLargeParameter(requiredSize);
        if (requiredSize > size()) {
            resize(requiredSize);
        }
    }

    template<typename F, typename ...Args>
    static GenericArray<T> randomf(size_t size, F func, const Args& ... args);
    template<typename F, typename ...Args>
    static GenericArray<T> randomfUnique(
            size_t size, F func, const Args& ... args);
    template<typename F, typename ...Args>
    static GenericArray<T> randomfAll(F func, const Args& ... args);

    template<typename ...Args>
    static GenericArray<T> random(size_t size, const Args& ... args);
    template<typename ...Args>
    static GenericArray<T> randomUnique(size_t size, const Args& ... args);
    template<typename ...Args>
    static GenericArray<T> randomAll(const Args& ... args);

    static GenericArray<T> id(size_t size, T start = T{});

    GenericArray<T>& shuffle();
    GenericArray<T> shuffled() const;

    GenericArray<T>& reverse();
    GenericArray<T> reversed() const;

    GenericArray<T>& sort();
    GenericArray<T> sorted() const;

    template<typename Comp>
    GenericArray<T>& sort(Comp&& comp);
    template<typename Comp>
    GenericArray<T> sorted(Comp&& comp) const;

    GenericArray<T>& unique();
    GenericArray<T> uniqued() const;

    GenericArray<T> inverse() const;

    template<typename Integer>
    GenericArray<T> subseq(const std::vector<Integer>& indices) const;

    template<typename Integer>
    GenericArray<T> subseq(
        const std::initializer_list<Integer>& indices) const;

    T choice() const;
    GenericArray<T> choice(size_t count) const;
    GenericArray<T> choiceWithRepetition(size_t count) const;

    GenericArray<T>& operator+=(const GenericArray<T>& other);
    GenericArray<T> operator+(const GenericArray<T>& other) const;

    GenericArray<T>& operator*=(int k);
    GenericArray<T> operator*(int k) const;

    operator std::string() const;
};

template<typename T>
template<typename ...Args>
GenericArray<T> GenericArray<T>::random(size_t size, const Args& ... args) {
    checkLargeParameter(size);
    GenericArray<T> result(size);
    for (T& x: result) {
        x = rnd.tnext<T>(args...);
    }
    return result;
}

template<typename T>
template<typename F, typename ...Args>
GenericArray<T> GenericArray<T>::randomf(
        size_t size,
        F func,
        const Args& ... args)
{
    checkLargeParameter(size);
    GenericArray<T> result(size);
    for (T& x: result) {
        x = func(args...);
    }
    return result;
}

namespace detail {

template<typename T, typename Enable = std::size_t>
struct DictContainer {
    typedef std::set<T> type;
};

template<typename T>
struct DictContainer<T, typename std::hash<T>::result_type>
{
    typedef std::unordered_set<T> type;
};

} // namespace detail

template<typename T>
template<typename F, typename ...Args>
GenericArray<T> GenericArray<T>::randomfUnique(
        size_t size,
        F func,
        const Args& ... args)
{
    typename detail::DictContainer<T>::type set;
    checkLargeParameter(size);
    GenericArray<T> result;
    result.reserve(size);

    size_t retries = (size + 10) * log(size + 10) * 2;

    while (result.size() != size) {
        T t = func(args...);
        if (!set.count(t)) {
            set.insert(t);
            result.push_back(t);
        }

        if (--retries == 0) {
            ensure(false, "There are not enough unique elements");
        }

    }

    return result;
}

template<typename T>
template<typename ...Args>
GenericArray<T> GenericArray<T>::randomUnique(
        size_t size, const Args& ... args)
{
    return GenericArray<T>::randomfUnique(
        size,
        [](Args... args) { return rnd.tnext<T>(args...); },
        args...);
}

template<typename T>
template<typename F, typename ...Args>
GenericArray<T> GenericArray<T>::randomfAll(
        F func,
        const Args& ... args)
{
    typename detail::DictContainer<T>::type set;
    GenericArray<T> result;

    int timeAfterLastHit = 0;

    while (true) {
        T t = func(args...);
        if (!set.count(t)) {
            set.insert(t);
            result.push_back(t);
            timeAfterLastHit = 0;
        }

        ++timeAfterLastHit;

        // Probability of finding not all elements is about e^{-20} ~= 1e-9
        if (timeAfterLastHit > (result.size() + 10) * 20) {
            return result;
        }
    }
}

template<typename T>
template<typename ...Args>
GenericArray<T> GenericArray<T>::randomAll(const Args& ... args)
{
    return GenericArray<T>::randomfAll(
        [](Args... args) { return rnd.tnext<T>(args...); },
        args...);
}

template<typename T>
GenericArray<T> GenericArray<T>::id(size_t size, T start) {
    constexpr bool enable = std::is_integral<T>::value;
    static_assert(enable, "Cannot call Array<T>::id with non-integral T");
    checkLargeParameter(size);

    if (enable) {
        GenericArray<T> result(size);
        std::iota(result.begin(), result.end(), start);
        return result;
    } else {
        return {};
    }
}

template<typename T>
GenericArray<T>& GenericArray<T>::shuffle() {
    jngen::shuffle(begin(), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::shuffled() const {
    auto res = *this;
    res.shuffle();
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::reverse() {
    std::reverse(begin(), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::reversed() const {
    auto res = *this;
    res.reverse();
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::sort() {
    std::sort(begin(), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::sorted() const {
    auto res = *this;
    res.sort();
    return res;
}

template<typename T>
template<typename Comp>
GenericArray<T>& GenericArray<T>::sort(Comp&& comp) {
    std::sort(begin(), end(), comp);
    return *this;
}

template<typename T>
template<typename Comp>
GenericArray<T> GenericArray<T>::sorted(Comp&& comp) const {
    auto res = *this;
    res.sort(comp);
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::unique() {
    erase(std::unique(begin(), end()), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::uniqued() const {
    auto res = *this;
    res.unique();
    return res;
}

template<typename T>
GenericArray<T> GenericArray<T>::inverse() const {
    static_assert(
        std::is_integral<T>::value,
        "Can only take inverse permutation of integral array");
    int n = size();

    if (n == 0) {
        return *this;
    }

    // sanity check
    ensure(*max_element(begin(), end()) == n-1 &&
        *min_element(begin(), end()) == 0,
        "Trying to take inverse of the array which is not a permutation");

    const static T NONE = static_cast<T>(-1);
    GenericArray<T> result(n, NONE);
    for (int i = 0; i < n; ++i) {
        ensure(result[at(i)] == NONE,
            "Trying to take inverse of the array which is not a permutation");
        result[at(i)] = i;
    }

    return result;
}

template<typename T>
template<typename Integer>
GenericArray<T> GenericArray<T>::subseq(
        const std::vector<Integer>& indices) const
{
    GenericArray<T> result;
    result.reserve(indices.size());
    for (Integer idx: indices) {
        result.push_back(at(idx));
    }
    return result;
}

// TODO(ifsmirnov): ever need to make it faster?
template<typename T>
template<typename Integer>
GenericArray<T> GenericArray<T>::subseq(
        const std::initializer_list<Integer>& indices) const
{
    return subseq(std::vector<T>(indices));
}

template<typename T>
T GenericArray<T>::choice() const {
    return jngen::choice(begin(), end());
}

template<typename T>
GenericArray<T> GenericArray<T>::choice(size_t count) const {
    ensure(
        count <= size(),
        "Use Array::choiceWithRepetition to select more than size() elements");

    size_t n = size();

    std::unordered_map<size_t, size_t> used;
    std::vector<size_t> res;
    for (size_t i = 0; i < count; ++i) {
        size_t oldValue = used.count(n-i-1) ? used[n-i-1] : n-i-1;
        size_t index = rnd.tnext<size_t>(n-i);
        res.push_back(used.count(index) ? used[index] : index);
        used[index] = oldValue;
    }

    return subseq(res);
}

template<typename T>
GenericArray<T> GenericArray<T>::choiceWithRepetition(size_t count) const {
    checkLargeParameter(count);
    GenericArray<T> res(count);
    for (T& t: res) {
        t = choice();
    }
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::operator+=(const GenericArray<T>& other) {
    insert(end(), other.begin(), other.end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::operator+(const GenericArray<T>& other) const {
    GenericArray<T> copy(*this);
    return copy += other;
}

template<typename T>
GenericArray<T>& GenericArray<T>::operator*=(int k) {
    if (k == 0) {
        clear();
        return *this;
    }

    this->reserve(this->size() * k);

    size_t size = this->size();
    while (k-- > 1) {
        insert(end(), begin(), begin() + size);
    }
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::operator*(int k) const {
    GenericArray<T> copy(*this);
    return copy *= k;
}

template<typename T>
GenericArray<T>::operator std::string() const {
    static_assert(std::is_same<T, char>::value, "Must not cast"
        " TArray<T> to std::string with 'T' != 'char'");
    return std::string(begin(), end());
}

template<typename T>
using TArray = GenericArray<T>;

} // namespace jngen

using jngen::TArray;

using Array = jngen::GenericArray<int>;
using Array2d = jngen::GenericArray<jngen::GenericArray<int>>;
using Array64 = jngen::GenericArray<long long>;
using Arrayf = jngen::GenericArray<double>;
using Arrayp = jngen::GenericArray<std::pair<int, int>>;

namespace jngen {

template<typename T>
jngen::GenericArray<T> makeArray(const std::vector<T>& values) {
    return jngen::GenericArray<T>(values);
}

template<typename T>
jngen::GenericArray<T> makeArray(const std::initializer_list<T>& values) {
    return jngen::GenericArray<T>(values);
}

template<typename T, typename U>
TArray<std::pair<T, U>> zip(const TArray<T>& lhs, const TArray<U>& rhs) {
    ensure(
        lhs.size() == rhs.size(),
        "In zip(a, b), a and b must have the same size");
    TArray<std::pair<T, U>> result;
    for (size_t i = 0; i < lhs.size(); ++i) {
        result.emplace_back(lhs[i], rhs[i]);
    }
    return result;
}

template<typename T, typename U>
TArray<T> arrayCast(const TArray<U>& array) {
    return TArray<T>(array.begin(), array.end());
}

} // namespace jngen

using jngen::makeArray;
using jngen::zip;
using jngen::arrayCast;


#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace jngen {

namespace detail {

inline int multiply(int x, int y, int mod) {
    return static_cast<long long>(x) * y % mod;
}

inline long long multiply(long long x, long long y, long long mod) {
#if defined(__SIZEOF_INT128__)
    return static_cast<__int128>(x) * y % mod;
#else
    long long res = 0;
    while (y) {
        if (y&1) {
            res = (static_cast<unsigned long long>(res) + x) % mod;
        }
        x = (static_cast<unsigned long long>(x) + x) % mod;
        y >>= 1;
    }
    return res;
#endif
}

inline int power(int x, int k, int mod) {
    int res = 1;
    while (k) {
        if (k&1) {
            res = multiply(res, x, mod);
        }
        x = multiply(x, x, mod);
        k >>= 1;
    }
    return res;
}

inline long long power(long long x, long long k, long long mod) {
    long long res = 1;
    while (k) {
        if (k&1) {
            res = multiply(res, x, mod);
        }
        x = multiply(x, x, mod);
        k >>= 1;
    }
    return res;
}

template<typename I>
bool millerRabinTest(I n, const std::vector<I>& witnesses) {
    static_assert(
        std::is_same<I, int>::value || std::is_same<I, long long>::value,
        "millerRabinTest<int/long long> only is supported");

    if (n == 1) {
        return false;
    }

    constexpr int LIMIT = 10000;

    if (n <= LIMIT) {
        for (int i = 2; i*i <= n; ++i) {
            if (n%i == 0) {
                return false;
            }
        }
        return true;
    }

    int r = 0;
    I d = n - 1;
    while (d % 2 == 0) {
        ++r;
        d /= 2;
    }

    for (I a: witnesses) {
        I x = power(a, d, n);
        if (x == 1 || x == n - 1) {
            continue;
        }

        bool composite = true;
        for (int i = 0; i < r - 1; ++i) {
            x = multiply(x, x, n);
            if (x == 1) {
                return false;
            }
            if (x == n - 1) {
                i = r;
                composite = false;
                continue;
            }
        }
        if (composite) {
            return false;
        }
    }
    return true;
}

} // namespace detail

inline bool isPrime(long long n) {
    const static std::vector<int> INT_WITNESSES{2, 7, 61};
    const static std::vector<long long> LONG_LONG_WITNESSES
        {2, 3, 5, 7, 11, 13, 17, 19, 23};
    // todo: experiment with base
    // 2, 325, 9375, 28178, 450775, 9780504, and 1795265022
    // (guaranteed for all integers < 2^64)

    // first strong pseudoprime to i64 bases is 3825123056546413051 ~= 3.8e18
    ensure(n > 0, "isPrime() is undefined for negative numbers");
    ensure(
        n <= static_cast<long long>(3.8e18),
        "isPrime() supports only numbers not greater than 3.8 * 10^18");

    if (n < std::numeric_limits<int>::max()) {
        return detail::millerRabinTest<int>(n, INT_WITNESSES);
    } else {
        return detail::millerRabinTest<long long>(n, LONG_LONG_WITNESSES);
    }
}

class MathRandom {
public:
    MathRandom() {
        static bool created = false;
        ensure(!created, "jngen::MathRandom should be created only once");
        created = true;
    }

    static long long randomPrime(long long n) {
        ensure(n > 2, format("There are no primes below %lld", n));
        return randomPrime(2, n - 1);
    }

    static long long randomPrime(long long l, long long r) {
        ensure(l <= r);
        std::unordered_set<long long> used;
        while (static_cast<long long>(used.size()) < r - l + 1) {
            long long x = rnd.next(l, r);
            if (used.count(x)) {
                continue;
            }
            used.insert(x);
            if (isPrime(x)) {
                return x;
            }
        }
        ensure(
            false,
            format(
                "There are no primes between %lld and %lld",
                l, r)
        );
    }

    static Array partition(int n, int numParts) {
        auto res = partition(
            static_cast<long long>(n), static_cast<long long>(numParts));
        return Array(res.begin(), res.end());
    }

    static Array64 partition(long long n, int numParts) {
        auto res = partitionNonEmpty(n + numParts, numParts);
        for (auto& x: res) {
            --x;
        }
        return res;
    }

    static Array partitionNonEmpty(int n, int numParts) {
        auto res = partitionNonEmpty(
            static_cast<long long>(n), static_cast<long long>(numParts));
        return Array(res.begin(), res.end());
    }

    static Array64 partitionNonEmpty(long long n, int numParts) {
        ensure(numParts > 0);
        ensure(
            numParts <= n,
            format("Cannot divide %lld into %lld nonempty parts",
                n, numParts));

        auto delimiters = Array64::randomUnique(numParts - 1, 1, n - 1).sorted();
        delimiters.insert(delimiters.begin(), 0);
        delimiters.push_back(n);
        Array64 res(numParts);
        for (long long i = 0; i < numParts; ++i) {
            res[i] = delimiters[i + 1] - delimiters[i];
        }
        return res;
    }

    template<typename T>
    TArray<TArray<T>> partition(TArray<T> elements, int numParts) {
        return partition(
            std::move(elements),
            partition(static_cast<int>(elements.size()), numParts));
    }

    template<typename T>
    TArray<TArray<T>> partitionNonEmpty(TArray<T> elements, int numParts) {
        return partition(
            std::move(elements),
            partitionNonEmpty(static_cast<int>(elements.size()), numParts));
    }

    template<typename T>
    TArray<TArray<T>> partition(TArray<T> elements, const Array& sizes) {
        size_t total = std::accumulate(sizes.begin(), sizes.end(), size_t(0));
        ensure(total == elements.size(), "sum(sizes) != elements.size()");
        elements.shuffle();
        TArray<TArray<T>> res;
        auto it = elements.begin();
        for (int size: sizes) {
            res.emplace_back();
            std::copy(it, it + size, std::back_inserter(res.back()));
            it += size;
        }

        return res;
    }
};

JNGEN_EXTERN MathRandom rndm;

} // namespace jngen

using jngen::isPrime;

using jngen::rndm;


namespace jngen {

class ArrayRandom {
public:
    ArrayRandom() {
        static bool created = false;
        ENSURE(!created, "jngen::ArrayRandom should be created only once");
        created = true;
    }

    template<typename F, typename ...Args>
    static auto randomf(
            size_t size,
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomf(size, func, args...);
    }

    template<typename F, typename ...Args>
    static auto randomfUnique(
            size_t size,
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomfUnique(size, func, args...);
    }

    template<typename F, typename ...Args>
    static auto randomfAll(
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomfAll(func, args...);
    }
};

JNGEN_EXTERN ArrayRandom rnda;

} // namespace jngen

using jngen::rnda;


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


#include <algorithm>
#include <utility>

namespace jngen {

class QueryBuilder {
public:
    QueryBuilder() {}

    QueryBuilder(int n);
    QueryBuilder(int l, int r);

    QueryBuilder& minLen(int value);
    QueryBuilder& maxLen(int value);
    QueryBuilder& range(int n);
    QueryBuilder& range(int l, int r);
    QueryBuilder& small();
    QueryBuilder& large();
    QueryBuilder& ordered(bool value);

    std::pair<int, int> next();
    Arrayp next(int m);

private:
    enum class QueryType {
        Default, Large, Small
    };

    QueryType queryType_ = QueryType::Default;

    std::pair<int, int> range_; // half-interval
    std::pair<int, int> lenRange_; // segment
    bool ordered_;
};

#ifndef JNGEN_DECLARE_ONLY

QueryBuilder::QueryBuilder(int n) :
    QueryBuilder(0, n)
{  }

QueryBuilder::QueryBuilder(int l, int r) :
    range_(l, r + 1),
    lenRange_(1, r - l + 1),
    ordered_(true)
{
    ensure(l <= r);
}

QueryBuilder& QueryBuilder::minLen(int value) {
    lenRange_.first = value;
    return *this;
}

QueryBuilder& QueryBuilder::maxLen(int value) {
    lenRange_.second = value;
    return *this;
}

QueryBuilder& QueryBuilder::range(int n) {
    ensure(n > 0);
    return this->range(0, n - 1);
}

QueryBuilder& QueryBuilder::range(int l, int r) {
    ensure(l <= r);
    range_ = {l, r+1};
    lenRange_.second = std::min(lenRange_.second, r - l + 1);
    lenRange_.first = std::min(lenRange_.first, lenRange_.second);
    return *this;
}

QueryBuilder& QueryBuilder::small() {
    queryType_ = QueryType::Small;
    return *this;
}

QueryBuilder& QueryBuilder::large() {
    queryType_ = QueryType::Large;
    return *this;
}

QueryBuilder& QueryBuilder::ordered(bool value) {
    ordered_ = value;
    return *this;
}

std::pair<int, int> QueryBuilder::next() {
    switch (queryType_) {
    case QueryType::Default: {
        // This is inaccurate to say the least. I don't know how to
        // generate a random segment with length from l to r without
        // calling sqrt.
        int len = rnd.wnext(lenRange_.first, lenRange_.second, -1);
        int l = rnd.next(range_.first, range_.second - len - 1);
        if (ordered_ || rnd.next(2)) {
            return {l, l + len - 1};
        } else {
            return {l + len - 1, l};
        }
    }
    case QueryType::Large: {
        ENSURE(false, "not implemented");
        break;
    }
    case QueryType::Small: {
        ENSURE(false, "not implemented");
        break;
    }
    default: ENSURE(false, "Nonexistent option");
    }
}

Arrayp QueryBuilder::next(int m) {
    return rnda.randomf(m, [this]() { return next(); });
}

#endif

template<typename ... Args>
QueryBuilder rndq(Args... args) {
    return QueryBuilder(std::forward(args)...);
}

} // namespace jngen

using jngen::rndq;


#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace jngen {

typedef std::pair<long long, long long> HashBase; // (mod, base)
typedef std::pair<std::string, std::string> StringPair;

class StringRandom {
public:
    StringRandom() {
        static bool created = false;
        ENSURE(!created, "jngen::StringRandom should be created only once");
        created = true;
    }

    static std::string thueMorse(int len, char first = 'a', char second = 'b');

    static std::string abacaba(int len, char first = 'a');

    static StringPair antiHash(
            const std::vector<HashBase>& bases,
            const std::string& alphabet = "a-z",
            int length = -1);
};

JNGEN_EXTERN StringRandom rnds;

namespace detail {

inline int popcount(long long x) {
    int res = 0;
    while (x) {
        ++res;
        x &= x-1;
    }
    return res;
}

inline int trailingZeroes(long long x) {
    int res = 0;
    ENSURE(x != 0);
    while (!(x&1)) {
        ++res;
        x >>= 1;
    }
    return res;
}

inline std::string parseAllowedChars(std::string pattern) {
    std::set<char> result;
    pattern += "\0\0";
    for (size_t i = 0; i < pattern.length(); ++i) {
        if (pattern[i] == '-') {
            result.insert('-');
        } else if(pattern[i+1] == '-' && pattern[i+2] != '\0') {
            for (char c = pattern[i]; c <= pattern[i+2]; ++c) {
                result.insert(c);
            }
            i += 2;
        } else {
            result.insert(pattern[i]);
        }
    }
    return std::string(result.begin(), result.end());
}

inline std::vector<std::string> extendAntiHash(
        const std::vector<std::string>& chars,
        HashBase base,
        int count)
{
    ENSURE(count == 2, "Count != 2 is not supported (yet)");

    size_t baseLength = chars[0].size();
    for (const auto& s: chars) {
        ensure(s.size() == baseLength);
    }

    long long mod = base.first;
    long long p = base.second;

    long long pPower = 1;
    for (size_t i = 0; i != baseLength; ++i) {
        pPower = (pPower * p) % mod;
    }

    std::vector<long long> charHashes;
    for (const auto& s: chars) {
        long long hash = 0;
        for (char c: s) {
            hash = (hash * p + c) % mod;
        }
        charHashes.push_back(hash);
    }

    auto computeHash = [&charHashes, mod, pPower](const std::vector<int>& a) {
        long long hash = 0;
        for (int x: a) {
            hash = (hash * pPower + charHashes[x]) % mod;
        }
        return hash;
    };

    // This bounds were achieved empirically and should be justified.
    int needForMatch;
    if (count == 2) {
        needForMatch = 5 * pow(double(mod), 0.5);
    } else {
        ENSURE(false, "Only count = 2 is supported yet");
    }

    int length = 2;
    double wordCount = pow(double(chars.size()), double(length));

    while (true) {
        ++length;
        wordCount *= chars.size();
        if (wordCount < needForMatch) {
            continue;
        }

        std::vector<std::pair<long long, Array>> words;
        std::map<long long, int> hashCount;
        std::set<Array> used;

        for (int i = 0; i < needForMatch; ++i) {
            Array w = Array::random(length, chars.size());
            if (used.count(w)) {
                --i;
                continue;
            }
            used.insert(w);
            long long hash = computeHash(w);
            words.emplace_back(hash, w);
            if (++hashCount[hash] == count) {
                std::vector<std::string> result;
                for (const auto& kv: words) {
                    if (kv.first == hash) {
                        std::string word;
                        for (int c: kv.second) {
                            word += chars[c];
                        }
                        result.push_back(word);
                    }
                }
                return result;
            }
        }
    }
}

inline StringPair minimalAntiHashTest(
        std::vector<HashBase> bases,
        const std::string allowedChars)
{
    for (auto base: bases) {
        ensure(base.first >= 0, "0 < MOD must hold");
        ensure(
            base.first <= (long long)(2e9),
            "Modules larger than 2'000'000'000 are not supported yet");
        ensure(
            0 < base.second && base.second < base.first,
            "0 <= P < MOD must hold");
    }

    std::vector<int> counts;
    if (bases.size() == 1) {
        counts = {2};
    } else if (bases.size() == 2) {
        counts = {2, 2};
    } else {
        counts.assign(bases.size(), 2);
    }

    std::vector<std::string> cur;
    for (char c: allowedChars) {
        cur.emplace_back(1, c);
    }

    for (size_t i = 0; i != bases.size(); ++i) {
        cur = extendAntiHash(cur, bases[i], counts[i]);
        ensure(static_cast<int>(cur.size()) == counts[i],
            "Cannot generate long enough pair with same hash");
    }

    return {cur[0], cur[1]};
}

} // namespace detail

#ifndef JNGEN_DECLARE_ONLY

std::string StringRandom::thueMorse(int len, char first, char second) {
    ensure(len >= 0);
    std::string res(len, ' ');
    for (int i = 0; i < len; ++i) {
        res[i] = detail::popcount(i)%2 == 0 ? first : second;
    }
    return res;
}

std::string StringRandom::abacaba(int len, char first) {
    ensure(len >= 0);
    std::string res(len, ' ');
    for (int i = 0; i < len; ++i) {
        res[i] = first + detail::trailingZeroes(~i);
    }
    return res;
}

StringPair StringRandom::antiHash(
        const std::vector<HashBase>& bases,
        const std::string& alphabet,
        int length)
{
    std::string allowedChars = detail::parseAllowedChars(alphabet);
    StringPair result = detail::minimalAntiHashTest(bases, allowedChars);

    if (length == -1) {
        return result;
    }

    ensure(
        static_cast<int>(result.first.length()) <= length,
        "Cannot generate enough long anti-hash test");

    int extraLength = length - result.first.length();
    int leftSize = rnd.next(0, extraLength);

    std::string left = rnd.next(format("[%s]{%d}", alphabet.c_str(), leftSize));
    std::string right =
        rnd.next(format("[%s]{%d}", alphabet.c_str(), extraLength - leftSize));

    return {
        left + result.first + right,
        left + result.second + right
    };
}

#endif

} // namespace jngen

using jngen::rnds;


#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace jngen {

inline int getInitialTestNo() {
    char *envvar = getenv("TESTNO");
    int testno;
    if (!envvar || 1 != std::sscanf(envvar, "%d", &testno)) {
        return 1;
    }
    return testno;
}

#ifdef JNGEN_DECLARE_ONLY
extern int nextTestNo;
#else
int nextTestNo = -1;
#endif // JNGEN_DECLARE_ONLY

void startTest(int testNo);

void startTest();

void setNextTestNumber(int testNo);

Array64 randomTestSizes(
    long long totalSize,
    int count,
    long long minSize,
    long long maxSize,
    const Array64& predefined);

Array randomTestSizes(
    int totalSize,
    int count,
    int minSize,
    int maxSize,
    const Array& predefined);

#ifndef JNGEN_DECLARE_ONLY

void startTest(int testNo) {
    nextTestNo = testNo + 1;
    char filename[10];
    std::sprintf(filename, "%d", testNo);
    if (!std::freopen(filename, "w", stdout)) {
        ensure(false, format("Cannot open the file `%s'", filename));
    }
}

void startTest() {
    if (nextTestNo == -1) {
        nextTestNo = getInitialTestNo();
    }

    startTest(nextTestNo);
}

void setNextTestNumber(int testNo) {
    nextTestNo = testNo;
}

Array64 randomTestSizes(
    long long totalSize,
    int count,
    long long minSize,
    long long maxSize,
    const Array64& predefined)
{
    for (auto x: predefined) {
        totalSize -= x;
    }
    ensure(totalSize >= 0, "Sum of predefined test sizes exceeds total size");
    ensure(count * minSize <= totalSize, "minSize is too large");
    ensure(minSize <= maxSize);

    auto partition = rndm.partition(totalSize - count * minSize, count)
        .sort().reverse();
    long long remaining = 0;

    long long localMax = maxSize - minSize;
    for (auto& x: partition) {
        if (x > localMax) {
            remaining += x - localMax;
            x = localMax;
        } else {
            long long add = std::min(remaining, localMax - x);
            x += add;
            remaining -= add;
        }

        x += minSize;
    }

    ensure(remaining == 0, "maxSize is too small");

    return (partition + predefined).shuffled();
}

Array randomTestSizes(
    int totalSize,
    int count,
    int minSize,
    int maxSize,
    const Array& predefined)
{
    return arrayCast<int>(randomTestSizes(
        static_cast<long long>(totalSize),
        count,
        static_cast<long long>(minSize),
        static_cast<long long>(maxSize),
        arrayCast<long long>(predefined)
    ));
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

using jngen::startTest;
using jngen::setNextTestNumber;

using jngen::randomTestSizes;


#include <cstring>
#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace jngen {

namespace variant_detail {

constexpr static int NO_TYPE = -1;

template<typename T>
struct PlainType {
    using type = typename std::remove_cv<
        typename std::remove_reference<T>::type>::type;
};

template<size_t Size, typename ... Args>
class VariantImpl;

template<size_t Size>
class VariantImpl<Size> {
public:
    VariantImpl() {
        type_ = NO_TYPE;
    }

private:
    int type_;
    char data_[Size];

protected:
    int& type() { return type_; }
    int type() const { return type_; }

    char* data() { return data_; }
    const char* data() const { return data_; }

    void doDestroy() {
        throw;
    }

    template<typename P>
    constexpr static int typeId() {
        return NO_TYPE;
    }

    void copy(char*) const {
        throw;
    }

    void move(char* dst) const {
        memmove(dst, data(), Size);
    }

    void setType(int) {
        throw;
    }

    template<typename V>
    void applyVisitor(V&&) const {
        throw;
    }

    void assign() {}
};

template<size_t Size, typename T, typename ... Args>
class VariantImpl<Size, T, Args...> : public VariantImpl<
        (sizeof(T) > Size ? sizeof(T) : Size),
        Args...
    >
{
    using Base = VariantImpl<(sizeof(T) > Size ? sizeof(T) : Size), Args...>;

    constexpr static int MY_ID = sizeof...(Args);

protected:
    void doDestroy() {
        if (this->type() == MY_ID) {
            this->type() = NO_TYPE;
            reinterpret_cast<T*>(this->data())->~T();
        } else {
            Base::doDestroy();
        }
    }

    template<typename P>
    constexpr static int typeId() {
        return std::is_same<P, T>::value ?
            MY_ID :
            Base::template typeId<P>();
    }

    void copy(char* dst) const {
        if (this->type() == MY_ID) {
            new(dst) T(*reinterpret_cast<const T*>(this->data()));
        } else {
            Base::copy(dst);
        }
    }

    void setType(int typeIndex) {
        if (typeIndex == MY_ID) {
            if (this->type() != NO_TYPE) {
                throw;
            }
            assign(T{});
        } else {
            Base::setType(typeIndex);
        }
    }

    template<typename V>
    void applyVisitor(V&& v) const {
        if (this->type() == MY_ID) {
            v(*reinterpret_cast<const T*>(this->data()));
        } else {
            Base::applyVisitor(v);
        }
    }

    using Base::assign;

    void assign(const T& t) {
        if (this->type() == NO_TYPE) {
            new(this->data()) T;
            this->type() = MY_ID;
        }

        ref() = t;
    }

private:
    T& ref() { return *reinterpret_cast<T*>(this->data()); }

public:
    operator T() const {
        if (this->type() == MY_ID) {
            return *reinterpret_cast<const T*>(this->data());
        } else {
            return T();
        }
    }
};

template<typename ... Args>
class Variant : public VariantImpl<0, Args...> {
    using Base = VariantImpl<0, Args...>;

public:
    Variant() {}

    Variant(const Variant<Args...>& other) {
        if (other.type() != NO_TYPE) {
            other.copy(this->data());
            unsafeType() = other.type();
        }
    }

    Variant& operator=(const Variant<Args...>& other) {
        if (this->type() != NO_TYPE) {
            this->doDestroy();
        }
        if (other.type() != NO_TYPE) {
            other.copy(this->data());
            unsafeType() = other.type();
        }
        return *this;
    }

    Variant(Variant<Args...>&& other) {
        if (other.type() != NO_TYPE) {
            other.move(this->data());
            unsafeType() = other.type();
            other.unsafeType() = NO_TYPE;
        }
    }

    Variant& operator=(Variant<Args...>&& other) {
        if (this->type() != NO_TYPE) {
            this->doDestroy();
        }
        if (other.type() != NO_TYPE) {
            other.move(this->data());
            unsafeType() = other.type();
            other.unsafeType() = NO_TYPE;
        }
        return *this;
    }

    ~Variant() {
        if (type() != NO_TYPE) {
            this->doDestroy();
        }
    }

    template<typename T>
    Variant(const T& t) : Variant() {
        this->assign(t);
    }

    template<typename T>
    T& ref() {
        return *ptr<T>();
    }

    template<typename T>
    const T& cref() {
        auto ptr = cptr<T>();
        if (ptr == 0) {
            throw std::logic_error("jngen::Variant: taking a reference for"
                " a type which is not active now");
        }
        return *ptr;
    }

    template<typename V>
    void applyVisitor(V&& v) const {
        Base::applyVisitor(v);
    }

    int type() const { return Base::type(); }

    void setType(int typeIndex) {
        if (typeIndex == NO_TYPE) {
            throw std::logic_error("jngen::Variant::setType():"
                " calling with NO_TYPE is invalid");
        }
        if (this->type() == typeIndex) {
            return;
        }
        if (this->type() != NO_TYPE) {
            this->doDestroy();
        }
        Base::setType(typeIndex);
    }

    bool empty() const { return Base::type() == NO_TYPE; }

    template<typename T>
    constexpr static bool hasType() {
        return Base::template typeId<T>() != NO_TYPE;
    }

private:
    template<typename T_, typename T = typename PlainType<T_>::type>
    T* ptr() {
        if (type() != this->template typeId<T>()) {
            if (type() != NO_TYPE) {
                this->doDestroy();
            }
            ::new(this->data()) T;
            unsafeType() = this->template typeId<T>();
        }
        return reinterpret_cast<T*>(this->data());
    }

    template<typename T_, typename T = typename PlainType<T_>::type>
    const T* cptr() const {
        if (type() != this->template typeId<T>()) {
            return nullptr;
        }
        return reinterpret_cast<const T*>(this->data());
    }

    int& unsafeType() {
        return Base::type();
    }
};

struct OstreamVisitor {
    template<typename T>
    void operator()(const T& t) {
        JNGEN_PRINT(t);
    }
    std::ostream& out;
    const OutputModifier& mod;
};

} // namespace variant_detail

using variant_detail::Variant;

template<typename ... Args>
JNGEN_DECLARE_SIMPLE_PRINTER(Variant<Args...>, 5) {
    if (t.type() == jngen::variant_detail::NO_TYPE) {
        out << "{empty variant}";
    } else {
        t.applyVisitor(jngen::variant_detail::OstreamVisitor{out, mod});
    }
}

} // namespace jngen


#include <iterator>
#include <vector>
#include <type_traits>

namespace jngen {

template<typename ... Args>
class VariantArray : public GenericArray<Variant<Args...>> {
public:
    using Base = GenericArray<Variant<Args...>>;
    using BaseVariant = Variant<Args...>;

    using Base::Base;

    VariantArray() {}

    /* implicit */ VariantArray(const Base& base) :
            Base(base)
    {  }

    template<typename T, typename = typename std::enable_if<
        BaseVariant::template hasType<T>()>::type>
    VariantArray(const std::vector<T>& other) {
        std::copy(other.begin(), other.end(), std::back_inserter(*this));
    }

    template<typename T, typename = typename std::enable_if<
        BaseVariant::template hasType<T>()>::type>
    VariantArray(std::vector<T>&& other) {
        std::move(other.begin(), other.end(), std::back_inserter(*this));
        GenericArray<T>().swap(other);
    }

    template<typename T, typename = typename std::enable_if<
        BaseVariant::template hasType<T>()>::type>
    operator GenericArray<T>() const
    {
        return GenericArray<T>(this->begin(), this->end());
    }

    bool hasNonEmpty() const {
        for (const auto& x: *this) {
            if (!x.empty()) {
                return true;
            }
        }
        return false;
    }

    int anyType() const {
        for (const auto& x: *this) {
            if (!x.empty()) {
                return x.type();
            }
        }
        return 0;
    }

};

} // namespace jngen

#include <string>
#include <utility>


namespace jngen {

#define JNGEN_DEFAULT_WEIGHT_TYPES int, double, std::string, char, std::pair<int, int>

#if defined(JNGEN_EXTRA_WEIGHT_TYPES)
#define JNGEN_WEIGHT_TYPES JNGEN_DEFAULT_WEIGHT_TYPES , JNGEN_EXTRA_WEIGHT_TYPES
#else
#define JNGEN_WEIGHT_TYPES JNGEN_DEFAULT_WEIGHT_TYPES
#endif

using Weight = Variant<JNGEN_WEIGHT_TYPES>;
using WeightArray = VariantArray<JNGEN_WEIGHT_TYPES>;

} // namespace jngen

using jngen::Weight;
using jngen::WeightArray;


#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

namespace jngen {

class GenericGraph {
public:
    virtual ~GenericGraph() {}

    virtual int n() const { return adjList_.size(); }
    virtual int m() const { return numEdges_; }

    // u, v: labels
    virtual void addEdge(int u, int v, const Weight& w = Weight{});
    virtual bool isConnected() const { return dsu_.isConnected(); }

    virtual int vertexLabel(int v) const { return vertexLabel_[v]; }
    virtual int vertexByLabel(int v) const { return vertexByLabel_[v]; }

    // v: label
    // return: array<label>
    virtual Array edges(int v) const;

    // return: array<label, label>
    virtual Arrayp edges() const;

    // order: by labels
    // TODO: think about ordering here
    virtual void setVertexWeights(const WeightArray& weights) {
        ensure(
            static_cast<int>(weights.size()) == n(),
            "The argument of setVertexWeights must have exactly n elements");
        vertexWeights_.resize(n());
        for (int i = 0; i < n(); ++i) {
            vertexWeights_[i] = weights[vertexByLabel(i)];
        }
    }

    // v: label
    virtual void setVertexWeight(int v, const Weight& weight) {
        ensure(v < n(), "setVertexWeight");
        v = vertexByLabel(v);

        vertexWeights_.extend(v + 1);
        vertexWeights_[v] = weight;
    }

    virtual void setEdgeWeights(const WeightArray& weights) {
        ensure(
            static_cast<int>(weights.size()) == m(),
            "The argument of setEdgeWeights must have exactly m elements");
        edgeWeights_ = weights;
    }

    virtual void setEdgeWeight(size_t index, const Weight& weight) {
        ensure(static_cast<int>(index) < m(), "setEdgeWeight");
        edgeWeights_.extend(index + 1);
        edgeWeights_[index] = weight;
    }

    // v: label
    virtual Weight vertexWeight(int v) const {
        ensure(v < n(), "vertexWeight");
        size_t index = vertexByLabel(v);
        if (index >= vertexWeights_.size()) {
            return Weight{};
        }
        return vertexWeights_[index];
    }

    virtual Weight edgeWeight(size_t index) const {
        ensure(static_cast<int>(index) < m(), "edgeWeight");
        if (index >= edgeWeights_.size()) {
            return Weight{};
        }
        return edgeWeights_[index];
    }

    // TODO: should it really be public?
    virtual void doPrintEdges(
        std::ostream& out, const OutputModifier& mod) const;

    virtual bool operator==(const GenericGraph& other) const;
    virtual bool operator!=(const GenericGraph& other) const;
    virtual bool operator< (const GenericGraph& other) const;
    virtual bool operator> (const GenericGraph& other) const;
    virtual bool operator<=(const GenericGraph& other) const;
    virtual bool operator>=(const GenericGraph& other) const;

protected:
    void doShuffle();

    void extend(size_t size);

    // v: vertex number
    // returns: array<number>
    Array internalEdges(int v) const;

    // u, v: vertex numbers
    void addEdgeUnsafe(int u, int v);

    // v: vertex number
    // returns: vertex number
    int edgeOtherEnd(int v, int edgeId) const;

    void permuteEdges(const Array& order);

    void normalizeEdges();

    int compareTo(const GenericGraph& other) const;

    int numEdges_ = 0;

    bool directed_ = false;

    Dsu dsu_;
    std::vector<Array> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;
    Arrayp edges_;

    WeightArray vertexWeights_;
    WeightArray edgeWeights_;
};

#ifndef JNGEN_DECLARE_ONLY

Array GenericGraph::edges(int v) const {
    ensure(v < n(), "Graph::edges(v)");
    v = vertexByLabel(v);

    Array result = internalEdges(v);
    for (auto& x: result) {
        x = vertexLabel(x);
    }

    return result;
}

Arrayp GenericGraph::edges() const {
    auto edges = edges_;
    for (auto& e: edges) {
        e.first = vertexLabel(e.first);
        e.second = vertexLabel(e.second);
    }
    return edges;
}

void GenericGraph::doShuffle() {
    // this if is to be removed after all checks pass
    if (vertexLabel_.size() < static_cast<size_t>(n())) {
        ENSURE(false, "GenericGraph::doShuffle");
        vertexLabel_ = Array::id(n());
    }

    vertexLabel_.shuffle();
    vertexByLabel_ = vertexLabel_.inverse();

    if (!directed_) {
        for (auto& edge: edges_) {
            if (rnd.next(2)) {
                std::swap(edge.first, edge.second);
            }
        }
    }

    permuteEdges(Array::id(numEdges_).shuffled());
}

void GenericGraph::extend(size_t size) {
    checkLargeParameter(size);
    size_t oldSize = n();
    if (size > oldSize) {
        adjList_.resize(size);
        vertexLabel_ += Array::id(size - oldSize, oldSize);
        vertexByLabel_ += Array::id(size - oldSize, oldSize);
        dsu_.extend(size);
    }
}

Array GenericGraph::internalEdges(int v) const {
    Array result;
    std::transform(
        adjList_[v].begin(),
        adjList_[v].end(),
        std::back_inserter(result),
        [this, v](int x) { return edgeOtherEnd(v, x); }
    );
    return result;
}

void GenericGraph::addEdgeUnsafe(int u, int v) {
    int id = numEdges_++;
    edges_.emplace_back(u, v);

    ENSURE(u < n() && v < n(), "GenericGraph::addEdgeUnsafe");

    adjList_[u].push_back(id);
    if (!directed_ && u != v) {
        adjList_[v].push_back(id);
    }
}

int GenericGraph::edgeOtherEnd(int v, int edgeId) const {
    ENSURE(edgeId < numEdges_);
    const auto& edge = edges_[edgeId];
    if (edge.first == v) {
        return edge.second;
    }
    ENSURE(!directed_);
    ENSURE(edge.second == v);
    return edge.first;
}

void GenericGraph::permuteEdges(const Array& order) {
    ENSURE(static_cast<int>(order.size()) == m(), "GenericGraph::permuteEdges");

    edges_ = edges_.subseq(order);

    auto newByOld = order.inverse();
    for (int v = 0; v < n(); ++v) {
        for (auto& x: adjList_[v]) {
            x = newByOld[x];
        }
    }

    if (edgeWeights_.hasNonEmpty()) {
        edgeWeights_.extend(m());
        edgeWeights_ = edgeWeights_.subseq(order);
    }
}

void GenericGraph::normalizeEdges() {
#ifndef JNGEN_NO_NORMALIZE_EDGES
    ENSURE(
        vertexLabel_ == Array::id(n()),
        "Can call normalizeEdges() only on newly created graph");

    if (!directed_) {
        for (auto& edge: edges_) {
            if (edge.first > edge.second) {
                std::swap(edge.first, edge.second);
            }
        }
    }

    auto order = Array::id(numEdges_).sorted(
        [this](int i, int j) {
            return edges_[i] < edges_[j];
        });

    permuteEdges(order);
#endif // JNGEN_NO_NORMALIZE_EDGES
}

void GenericGraph::addEdge(int u, int v, const Weight& w) {
    extend(std::max(u, v) + 1);

    u = vertexByLabel(u);
    v = vertexByLabel(v);

    dsu_.unite(u, v);
    addEdgeUnsafe(u, v);

    if (!w.empty()) {
        setEdgeWeight(m() - 1, w);
    }
}

namespace {

WeightArray prepareWeightArray(WeightArray a, int requiredSize) {
    ENSURE(a.hasNonEmpty(), "Attempt to print empty weight array");

    a.extend(requiredSize);
    int type = a.anyType();
    for (auto& x: a) {
        if (x.empty()) {
            x.setType(type);
        }
    }

    return a;
}

} // namespace

void GenericGraph::doPrintEdges(
    std::ostream& out, const OutputModifier& mod) const
{
    if (mod.printN) {
        out << n();
        if (mod.printM) {
            out << " " << m();
        }
        out << "\n";
    } else if (mod.printM) {
        out << m() << "\n";
    }

    if (vertexWeights_.hasNonEmpty()) {
        auto vertexWeights = prepareWeightArray(vertexWeights_, n());
        for (int i = 0; i < n(); ++i) {
            if (i > 0) {
                out << " ";
            }
            JNGEN_PRINT_NO_MOD(vertexWeights[vertexByLabel(i)]);
        }
        out << "\n";
    }

    auto t(mod);
    {
        auto mod(t);

        Arrayp edges = this->edges();
        mod.printN = false;
        if (edgeWeights_.hasNonEmpty()) {
            auto edgeWeights = prepareWeightArray(edgeWeights_, m());
            for (int i = 0; i < m(); ++i) {
                if (i > 0) {
                    out << "\n";
                }
                JNGEN_PRINT(edges[i]);
                out << " ";
                JNGEN_PRINT_NO_MOD(edgeWeights[i]);
            }
        } else {
            JNGEN_PRINT(edges);
        }
    }
}

bool GenericGraph::operator==(const GenericGraph& other) const {
    return compareTo(other) == 0;
}

bool GenericGraph::operator!=(const GenericGraph& other) const {
    return compareTo(other) != 0;
}

bool GenericGraph::operator<(const GenericGraph& other) const {
    return compareTo(other) == -1;
}

bool GenericGraph::operator>(const GenericGraph& other) const {
    return compareTo(other) == 1;
}

bool GenericGraph::operator<=(const GenericGraph& other) const {
    return compareTo(other) != 1;
}

bool GenericGraph::operator>=(const GenericGraph& other) const {
    return compareTo(other) != -1;
}

int GenericGraph::compareTo(const GenericGraph& other) const {
    if (n() != other.n()) {
        return n() < other.n() ? -1 : 1;
    }
    for (int i = 0; i < n(); ++i) {
        Array e1 = Array(edges(i)).sorted();
        Array e2 = Array(other.edges(i)).sorted();
        if (e1 != e2) {
            return e1 < e2 ? -1 : 1;
        }
    }
    return 0;
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen



#include <algorithm>
#include <vector>

namespace jngen {

class Tree : public ReprProxy<Tree>, public GenericGraph {
public:
    Tree() {
        extend(1);
    }
    Tree(const GenericGraph& gg) : GenericGraph(gg) {
        extend(1);
    }

    void addEdge(int u, int v, const Weight& w = Weight{}) override;

    Array parents(int root) const;

    Tree& shuffle();
    Tree shuffled() const;

    Tree link(int vInThis, const Tree& other, int vInOther);
    Tree glue(int vInThis, const Tree& other, int vInOther);

    static Tree bamboo(int size);
    static Tree randomPrufer(int size);
    static Tree random(int size, int elongation = 0);
    static Tree star(int size);
    static Tree caterpillar(int size, int length);
};

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 2) {
    ensure(t.isConnected(), "Cannot print a tree: it is not connected");

    if (mod.printParents) {
        ensure(false, "Printing parents is not implemented");
    } else if (mod.printEdges) {
        t.doPrintEdges(out, mod);
    } else {
        ensure(false, "Print mode is not set, select one of 'printParents'"
            " or 'printEdges'");
    }
}


#ifndef JNGEN_DECLARE_ONLY

void Tree::addEdge(int u, int v, const Weight& w) {
    extend(std::max(u, v) + 1);

    u = vertexByLabel(u);
    v = vertexByLabel(v);

    int ret = dsu_.unite(u, v);
    ensure(ret, "A cycle appeared in the tree");

    addEdgeUnsafe(u, v);

    if (!w.empty()) {
        setEdgeWeight(m() - 1, w);
    }
}

Array Tree::parents(int root) const {
    ensure(isConnected(), "Tree::parents(int): Tree is not connected");
    root = vertexByLabel(root);

    Array parents(n());
    parents[root] = root;
    std::vector<int> used(n());
    std::vector<int> queue{root};
    for (size_t i = 0; i < queue.size(); ++i) {
        int v = queue[i];
        used[v] = true;
        for (auto to: internalEdges(v)) {
            if (!used[to]) {
                parents[to] = v;
                queue.push_back(to);
            }
        }
    }

    for (auto& x: parents) {
        x = vertexLabel(x);
    }

    return parents;
}

Tree& Tree::shuffle() {
    doShuffle();
    return *this;
}

Tree Tree::shuffled() const {
    Tree t = *this;
    return t.shuffle();
}

Tree Tree::link(int vInThis, const Tree& other, int vInOther) {
    ensure(vInThis < n(), "Cannot link a nonexistent vertex");
    ensure(vInOther < other.n(), "Cannot link to a nonexistent vertex");

    Tree t(*this);

    for (const auto& e: other.edges()) {
        t.addEdge(e.first + n(), e.second + n());
    }

    t.addEdge(vInThis, vInOther + n());

    return t;
}

Tree Tree::glue(int vInThis, const Tree& other, int vInOther) {
    ensure(vInThis < n(), "Cannot glue a nonexistent vertex");
    ensure(vInOther < other.n(), "Cannot glue to a nonexistent vertex");

    auto newLabel = [vInThis, vInOther, &other, this] (int v) {
        if (v < vInOther) {
            return n() + v;
        } else if (v == vInOther) {
            return vInThis;
        } else {
            return n() + v - 1;
        }
    };

    Tree t(*this);

    for (const auto& e: other.edges()) {
        t.addEdge(newLabel(e.first), newLabel(e.second));
    }

    ensure(t.n() == n() + other.n() - 1);

    return t;
}

// Tree generators go here

Tree Tree::bamboo(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    Tree t;
    for (int i = 0; i + 1 < size; ++i) {
        t.addEdge(i, i+1);
    }
    t.normalizeEdges();
    return t;
}

Tree Tree::randomPrufer(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    if (size == 1) {
        return Tree();
    }

    Array code = Array::random(size - 2, size);
    std::vector<int> degree(size, 1);
    for (int v: code) {
        ++degree[v];
    }

    std::set<int> leaves;
    for (int v = 0; v < size; ++v) {
        if (degree[v] == 1) {
            leaves.insert(v);
        }
    }

    Tree t;
    for (int v: code) {
        ENSURE(!leaves.empty());
        int to = *leaves.begin();
        leaves.erase(leaves.begin());
        if (--degree[v] == 1) {
            leaves.insert(v);
        }

        t.addEdge(v, to);
    }

    ENSURE(leaves.size() == 2u);
    t.addEdge(*leaves.begin(), *leaves.rbegin());
    t.normalizeEdges();
    return t;
}

Tree Tree::random(int size, int elongation) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    Tree t;
    for (int v = 1; v < size; ++v) {
        int parent = rnd.wnext(v, elongation);
        t.addEdge(parent, v);
    }
    t.normalizeEdges();
    return t;
}

Tree Tree::star(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    Tree t;
    for (int i = 1; i < size; ++i) {
        t.addEdge(0, i);
    }
    t.normalizeEdges();
    return t;
}

Tree Tree::caterpillar(int size, int length) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    ensure(length > 0, "Length of the caterpillar must be positive");
    checkLargeParameter(size);
    ensure(length <= size);
    Tree t = Tree::bamboo(length);
    for (int i = length; i < size; ++i) {
        t.addEdge(rnd.next(length), i);
    }
    t.normalizeEdges();
    return t;
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

using jngen::Tree;


#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace jngen {

class Graph : public ReprProxy<Graph>, public GenericGraph {
    using BuilderProxy = graph_detail::BuilderProxy;
    using Traits = graph_detail::Traits;

    friend class graph_detail::GraphRandom;

public:
    virtual ~Graph() {}
    Graph() {}

    Graph(int n) {
        extend(n);
    }

    Graph(const GenericGraph& gg) : GenericGraph(gg) {}

    void setN(int n);

    Graph& shuffle();
    Graph shuffled() const;

    static BuilderProxy random(int n, int m);
    static BuilderProxy complete(int n);
    static BuilderProxy empty(int n);
    static BuilderProxy cycle(int n);
    static BuilderProxy randomStretched(
            int n, int m, int elongation, int spread);
};

inline void Graph::setN(int n) {
    ensure(n >= this->n(), "Cannot lessen number of vertices in the graph");
    extend(n);
}

inline Graph& Graph::shuffle() {
    doShuffle();
    return *this;
}

inline Graph Graph::shuffled() const {
    Graph g(*this);
    return g.shuffle();
}

JNGEN_DECLARE_SIMPLE_PRINTER(Graph, 2) {
    t.doPrintEdges(out, mod);
}

JNGEN_DECLARE_SIMPLE_PRINTER(graph_detail::BuilderProxy, 2) {
    JNGEN_PRINT(t.g());
}

} // namespace jngen

using jngen::Graph;

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_GRAPH_INL_H
#ifndef JNGEN_INCLUDE_GRAPH_INL_H
#error File "graph_inl.h" must not be included directly.
#endif


namespace jngen {

namespace graph_detail {

Graph BuilderProxy::g() const {
    return builder_(traits_);
}

BuilderProxy::operator Graph() const {
    return g();
}

class GraphRandom {
    using BuilderProxy = graph_detail::BuilderProxy;
    using Traits = graph_detail::Traits;

public:
    GraphRandom() {
        static bool created = false;
        ensure(!created, "jngen::GraphRandom should be created only once");
        created = true;
    }

    static BuilderProxy random(int n, int m) {
        ensure(
            n >= 0 && m >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        checkLargeParameter(m);
        return BuilderProxy(Traits(n, m), &doRandom);
    }

    static BuilderProxy complete(int n) {
        ensure(
            n >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            for (int i = 0; i < t.n; ++i) {
                for (int j = 0; j < t.n; ++j) {
                    if (i < j ||
                        (i == j && t.allowLoops) ||
                        (i > j && t.directed))
                    {
                        g.addEdge(i, j);
                    }
                }
            }
            g.normalizeEdges();
            return g;
        });
    }

    static BuilderProxy empty(int n) {
        ensure(
            n >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            g.setN(t.n);
            return g;
        });
    }

    static BuilderProxy cycle(int n) {
        ensure(
            n >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            for (int i = 0; i < t.n; ++i) {
                g.addEdge(i, (i+1)%t.n);
            }
            g.normalizeEdges();
            return g;
        });
    }

    static BuilderProxy randomStretched(
            int n, int m, int elongation, int spread)
    {
        ensure(
            n >= 0 && m >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        checkLargeParameter(m);
        return BuilderProxy(Traits(n, m), [elongation, spread](Traits t) {
            return doRandomStretched(t, elongation, spread);
        });
    }

private:
    static Graph doRandom(Traits t) {
        int n = t.n;
        int m = t.m;

        if (!t.allowMulti) {
            ensure(m <= maxEdges(n, t), "Too many edges in the graph");
        }

        std::set<std::pair<int, int>> usedEdges;

        if (t.connected) {
            ensure(m >= n - 1, "Not enough edges for a connected graph");
            auto treeEdges = Tree::randomPrufer(n).edges();
            usedEdges.insert(treeEdges.begin(), treeEdges.end());
            ENSURE(usedEdges.size() == static_cast<size_t>(n - 1));
        }

        auto edgeIsGood = [&usedEdges, t](std::pair<int, int> edge) {
            // TODO: move this check to edges generation loop
            if (!t.allowLoops && edge.first == edge.second) {
                ENSURE(false);
                return false;
            }
            if (!t.directed && edge.first > edge.second) {
                ENSURE(false);
                std::swap(edge.first, edge.second);
            }

            if (!t.allowMulti && usedEdges.count(edge)) {
                return false;
            }
            return true;
        };

        Arrayp result(usedEdges.begin(), usedEdges.end());

        while (result.size() < static_cast<size_t>(m)) {
            auto edge = randomEdge(n, t);
            if (edgeIsGood(edge)) {
                usedEdges.insert(edge);
                result.push_back(edge);
            }
        }

        ENSURE(
            result.size() == static_cast<size_t>(m),
            "Not enough edges found");

        Graph graph;

        graph.setN(n);
        for (const auto& edge: result) {
            graph.addEdge(edge.first, edge.second);
        }

        graph.normalizeEdges();

        return graph;
    }

    static Graph doRandomStretched(Traits t, int elongation, int spread) {
        Tree tree = Tree::random(t.n, elongation);
        Array parents = tree.parents(0);

        Graph graph(tree);

        auto treeEdges = tree.edges();
        std::set<std::pair<int, int>> usedEdges(
            treeEdges.begin(), treeEdges.end());

        while (graph.m() != t.m) {
            int u = rnd.next(t.n);
            int up = rnd.next(0, spread);
            int v = u;
            for (int iter = 0; iter < up; ++iter) {
                v = parents[v];
            }

            ENSURE(v <= u);

            if (!t.allowLoops && u == v) {
                continue;
            }

            if (!t.allowMulti && usedEdges.count({v, u})) {
                continue;
            }

            graph.addEdge(u, v);
            usedEdges.emplace(u, v);
        }

        graph.normalizeEdges();
        return graph;
    }

    static std::pair<int, int> randomEdge(int n, const Traits& t) {
        return rnd.nextp(n, RandomPairTraits{!t.directed, !t.allowLoops});
    }

    static long long maxEdges(int n, const Traits& t) {
        ENSURE(!t.allowMulti);
        long long res = static_cast<long long>(n) * (n-1);
        if (!t.directed) {
            res /= 2;
        }
        if (t.allowLoops) {
            res += n;
        }
        return res;
    }
};

} // namespace graph_detail

Graph::BuilderProxy Graph::random(int n, int m) {
    return graph_detail::GraphRandom::random(n, m);
}

Graph::BuilderProxy Graph::complete(int n) {
    return graph_detail::GraphRandom::complete(n);
}

Graph::BuilderProxy Graph::empty(int n) {
    return graph_detail::GraphRandom::empty(n);
}

Graph::BuilderProxy Graph::cycle(int n) {
    return graph_detail::GraphRandom::cycle(n);
}

Graph::BuilderProxy Graph::randomStretched(
        int n, int m, int elongation, int spread) {
    return graph_detail::GraphRandom::randomStretched(n, m, elongation, spread);
}

} // namespace jngen
#undef JNGEN_INCLUDE_GRAPH_INL_H
#endif // JNGEN_DECLARE_ONLY
