#pragma once

#include "drawing_engine.h"

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
#include "svg_engine_inl.h"
#undef JNGEN_INCLUDE_SVG_ENGINE_INL_H
#endif // JNGEN_DECLARE_ONLY
