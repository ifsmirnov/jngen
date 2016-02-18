#include <bits/stdc++.h>

#include "named_arguments.h"
#include "random.h"
#include "array.h"

using namespace std;

int main() {
    auto a = Arrayf::random({5, 10}, 100, 150);
    cout << repr(a, $sep="  ", $printSize=false, $addOne=true) << endl;
}
