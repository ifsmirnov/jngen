#include "array.h"
#include "structures.h"
#include <bits/stdc++.h>

#define forn(i, n) for (int i = 0; i < int(n); ++i)

using namespace std;

Array generateCycle(const Array& elements) {
    return elements;
}

Array combineCycles(const TArray<Array>& cycles) {
    int n = 0;
    for (const auto& c: cycles) {
        n += c.size();
    }

    Array result(n);

    for (const auto& c: cycles) {
        if (c.empty()) {
            continue;
        }
        for (size_t i = 1; i < c.size(); ++i) {
            result[c[i-1]] = c[i];
        }
        result[c.back()] = c.front();
    }

    return result;
}

int main() {
    auto perm = composition(Array::id(10), 2, generateCycle, combineCycles);
    cout << perm.add1() << endl;
}
