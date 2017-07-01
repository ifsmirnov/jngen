// #define JNGEN_DECLARE_ONLY
#include "drawer/drawer.h"
#include "geometry.h"
#include "options.h"
#include "rnda.h"
#include <iostream>
#include <cmath>
using namespace std;
#define forn(i, n) for (int i = 0; i < int(n); ++i)

int main(int argc, char *argv[]) {
    (void)argc, (void)argv;
    parseArgs(argc, argv);

    Drawer d;

    int k = 5;
    int n = 8;
    int C = 150;
    int sz = 45;

    auto polygons = rnda.randomf(k, [n, C, sz] () {
        auto pt = rndg.point(C, C);
        auto poly = rndg.convexPolygon(n, sz, sz).shifted(pt);
        return poly;
    });

    for (const auto& t: polygons) {
        d.setFill(choice({"green", "red", "blue"}));
        d.polygon(t);
    }

    d.dumpSvg("image.svg");
}
