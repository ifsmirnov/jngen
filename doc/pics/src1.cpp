// #define JNGEN_DECLARE_ONLY
#include "drawer/drawer.h"
#include "geometry.h"
#include "options.h"
#include <iostream>
#include <cmath>
using namespace std;
#define forn(i, n) for (int i = 0; i < int(n); ++i)

int main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    parseArgs(argc, argv);

    Drawer d;

    int n = 6;
    double R = 10;
    TArray<Pointf> pts;
    double pi = acos(-1.0);
    for (int i = 0; i < n; ++i) {
        double sina = sin(2 * pi * i / n);
        double cosa = cos(2 * pi * i / n);
        pts.emplace_back(cosa * R, sina * R);
    }

    pts.shuffle();

    for (int i = 0; i < n; ++i) {
        for (int j = 0; j < n; ++j) {
            if (i < 3 && j < 3) {
                d.setWidth(2);
                d.setColor("red");
            } else {
                d.setWidth(1);
                d.setColor(rnd.next(2) ? "red" : "blue");
            }
            d.segment(pts[i], pts[j]);
        }
    }

    d.setFill("green");
    d.setStroke("black");
    d.setWidth(1);

    for (auto pt: pts) {
        d.circle(pt, 1);
    }

    d.dumpSvg("image.svg");
}
