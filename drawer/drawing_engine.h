#pragma once

#include <string>

namespace jngen {
namespace drawing {

enum class Color {
    White,
    Black,
    Red,
    Green,
    Blue,
    Grey,
    LightGrey
};

class DrawingEngine {
public:
    DrawingEngine() {}
    virtual ~DrawingEngine() {}

    virtual void drawPoint(double x, double y) = 0;
    virtual void drawCircle(double x, double y, double r) = 0;
    virtual void drawSegment(double x1, double y1, double x2, double y2) = 0;
    virtual void drawText(
        double x, double y, const std::string& s) = 0;

    virtual void setWidth(double width) = 0;
    virtual void setColor(Color color) = 0;

    virtual Color color() const = 0;
    virtual double width() const = 0;
};

}} // namespace jngen::drawing
