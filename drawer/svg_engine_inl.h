#ifndef JNGEN_INCLUDE_SVG_ENGINE_INL_H
#error File "svg_engine_inl.h" must not be included directly.
#endif

#include <cmath>

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
    color_(Color::Black),
    x1_(x1),
    y1_(y1),
    x2_(x2),
    y2_(y2)
{  }

void SvgEngine::drawPoint(double x, double y) {
    x = lerpX(x);
    y = lerpY(y);
    double w = width_ * WIDTH_SCALE;
    sprintf(
        buf,
        "<circle cx='%f' cy='%f' r='%f' fill='%s'/>",
        x, y, w, colorToString(color_)
    );
    output_ << buf << "\n";
}

void SvgEngine::drawCircle(double x, double y, double r) {
    x = lerpX(x);
    y = lerpY(y);
    r = scaleSize(r);
    sprintf(
        buf,
        "<circle cx='%f' cy='%f' r='%f' fill='%s'/>",
        x, y, r, colorToString(color_)
    );
    output_ << buf << "\n";
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
    sprintf(
        buf,
        "<line x1='%f' y1='%f' x2='%f' y2='%f' stroke='%s' stroke-width='%f'/>",
        x1, y1, x2, y2, colorToString(color_), width_ * WIDTH_SCALE
    );
    output_ << buf << "\n";
}

void SvgEngine::drawText(
    double x, double y, const std::string& s)
{
    x = std::round(lerpX(x));
    y = std::round(lerpY(y));
    sprintf(
        buf,
        "<text x='%f' y='%f' font-size='%d' font-family='Helvetica'>%s</text>",
        x, y, FONT_SIZE, s.c_str()
    );
    output_ << buf << "\n";
}

void SvgEngine::setWidth(double width) {
    width_ = width;
}

void SvgEngine::setColor(Color color) {
    color_ = color;
}

std::string SvgEngine::serialize() const {
    int offset = sprintf(
        buf,
        "<svg xmlns='http://www.w3.org/2000/svg' "
        "viewBox='%f %f %f %f'>\n",
        0.0, 0.0, CANVAS_SIZE, CANVAS_SIZE * (y2_ - y1_) / (x2_ - x1_)
    );
    sprintf(
        buf + offset,
        "<circle cx='%f' cy='%f' r='%f' fill='white'/>\n",
        CANVAS_SIZE/2, CANVAS_SIZE/2, CANVAS_SIZE
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

}} // namespace jngen::drawing

