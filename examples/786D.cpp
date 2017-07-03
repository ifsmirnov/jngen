#include "jngen.h"
using namespace std;

// http://codeforces.com/contest/786/problem/D
// tree with a letter on each edge, then pairs of distinct vertices
// run as ./main n, m [-elong=...]
int main(int argc, char *argv[]) {
    registerGen(argc, argv);
    parseArgs(argc, argv);

    int n, q, elong = 0;
    getPositional(n, q);
    getNamed(elong);

    cout << n << " " << q << "\n";
    auto t = Tree::random(n, elong).shuffled();
    t.setEdgeWeights(TArray<char>::random(n - 1, 'a', 'z'));
    cout << t.add1() << "\n";
    cout << Arrayp::random(q, 1, n, dpair) << "\n";
}
