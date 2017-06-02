#pragma once

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
