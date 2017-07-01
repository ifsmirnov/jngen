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

const char* colorToString(const Color& color) {
    const static char* NONE = "none";
    if (color.empty()) {
        return NONE;
    }
    return color.c_str();
}

// Given x \in [l, r], return linear interpolation to [L, R]
double lerp(double x, double l, double r, double L, double R) {
    return L + (R - L) * ((x - l) / (r - l));
}

} // namespace

SvgEngine::SvgEngine(double x1, double y1, double x2, double y2) :
    width_(1.0),
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

