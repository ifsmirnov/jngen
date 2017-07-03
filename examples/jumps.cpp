#ifdef LOCAL
#define JNGEN_DECLARE_ONLY
#endif
#include "jngen.h"
#include <bits/stdc++.h>
using namespace std;
#define forn(i, n) for (int i = 0; i < (int)(n); ++i)

int main(int argc, char *argv[]) {
    registerGen(argc, argv);
    parseArgs(argc, argv);
    setMod().printN();

    int n;
    ensure(getOpt(0, n));

    string type = getOptOr("type", "random");

    if (type == "random") {
        int min = 1, max = n-1;
        getNamed(min, max);

        auto a = Array::random(n, min, max);

        cout << a << "\n";
    } else if (type == "manual") {
        int id;
        ensure(getNamed(id));

        if (id == 1) {
            Array a(n, 1);
            a[0] = a[n-1] = n-1;
            cout << a << "\n";
        } else if (id == 2) {
            cout << Array(n, 1) << "\n";
        } else if (id == 3) {
            cout << Array(n, n-1) << "\n";
        } else if (id == 4) {
            cout << Array{1, 2} * (n/2) << "\n";
        } else {
            ensure(false, format("Incorrect manual test id: '%d'", id));
        }
    } else if (type == "sides") {
        int sidelen = 0, smin = 1, smax = n-1, min = 1, max = n-1;
        getNamed(sidelen, smin, smax, min, max);
        ensure(2 * sidelen <= n);

        auto lhs = Array::random(rnd.wnext(1, sidelen, 3), smin, smax);
        auto rhs = Array::random(rnd.wnext(1, sidelen, 3), smin, smax);
        auto mid = Array::random(n - lhs.size() - rhs.size(), min, max);

        cout << lhs + mid + rhs << "\n";
    } else if (type == "islands") {
        int cnt = 1, size = n, min = 1, max = n-1;
        getNamed(cnt, size, min, max);
        ensure(cnt * (size + 1) - 1 <= n);
        auto landSizes = rndm.partitionNonEmpty(n - cnt*size, cnt+1);
        Array a;
        forn(i, cnt) {
            a += Array(landSizes[i], n-1);
            a += Array::random(size, min, max);
        }
        a += Array(landSizes.back(), n-1);
        cout << a << "\n";
    } else {
        ensure(false, format("Incorrect type: '%s'", type.c_str()));
    }

    return 0;
}
