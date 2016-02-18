#include <bits/stdc++.h>

#include "named_arguments.h"
#include "random.h"

using namespace std;

DECLARE_NAMED_PARAMETER(needParent);
DECLARE_NAMED_PARAMETER(sep);
DECLARE_NAMED_PARAMETER(another);


template<typename... Traits>
void func(Traits... traits) {
    auto traitMap = impl::collectTraits(traits...);

    for (auto kv: traitMap) {
        cout << kv.first << ": " << (int)kv.second << " " << (string)kv.second << endl;
    }
}


int main() {
    for (int i = 0; i < 10; ++i) {
        cout << rnd.next(10) << endl;
    }
}
