#define GLIBCXX_DEBUG
#include <bits/stdc++.h>

#include "array.h"

#define forn(i, n) for (int i = 0; i < n; ++i)

using namespace std;

int main() {
    rnd.seed(123);

    cout << Array::id(10).shuffled().add1().printN() << endl;
}
