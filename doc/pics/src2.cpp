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

    int n = 40;

    d.setStroke("blue");

    for (int i = 0; i < n; ++i) {
        d.setOpacity(1.0 * i / n);
        d.circle(i * 2, 0, i);
    }

    d.dumpSvg("image.svg");
}
