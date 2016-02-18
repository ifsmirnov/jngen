#include <bits/stdc++.h>

#include "named_arguments.h"
#include "random.h"
#include "array.h"
#include "perm.h"

using namespace std;

int main() {
    impl::randomEngine.seed(1235);

    auto a = Array::random({5, 10}, 10, 50);
    cout << a << endl;

    Perm p(a.size());
    p.apply(a);
    cout << a << endl;

    a.shuffle();
    cout << a << endl;

    cout << choice(a, 50, $allowRepeats = true) << endl;
}
