#include "jngen.h"
#include <bits/stdc++.h>
#define forn(i, n) for (int i = 0; i < (int)(n); ++i)
using namespace std;

Tree uniDepthTree(const vector<int>& layers) {
    ensure(is_sorted(layers.begin(), layers.end()));

    Tree t;
    Array last{0};
    int n = 1;
    for (int d: layers) {
        Array nxt = Array::id(d, n);
        n += d;
        Array cnt(last.size(), 1);
        forn(i, d - last.size()) ++cnt[rnd.next() % cnt.size()];
        int ptr = 0;
        forn(i, cnt.size()) {
            forn(j, cnt[i]) {
                t.addEdge(last[i], nxt[ptr++]);
            }
        }
        last = nxt;
    }
    return t;
}

Array depthVector(int n, int depth) {
    ensure(n >= depth);

    Array a(depth, 1);
    n -= depth;

    while (n) {
        int k = rnd.next(1, min(depth, n));
        forn(i, k) {
            ++a[depth - i - 1];
        }
        n -= k;
    }
    return a;
}

Tree goodTree(int n, int a, int b) {
    int deg = rnd.next(1, int(sqrt(n)));

    Array sz(deg, 1);
    forn(i, n - deg - 1) ++sz[rnd.next(sz.size())];

    Tree t;
    for (int x: sz) {
        int d;
        if (min(a, b) > x) {
            continue;
        } else if (max(a, b) > x) {
            d = min(a, b);
        } else {
            d = rnd.next(0, 1) ? a : b;
        }

        auto u = uniDepthTree(depthVector(x, d));
        t = t.glue(0, u, 0);
    }

    return t;
}

Tree distort(Tree t, int cnt) {
    int n = t.n();
    forn(i, cnt) {
        t.addEdge(rnd.next(n), n);
        ++n;
    }
    return t.shuffle();
}

void genSpecial(int id) {
    if (id == 1) {
        cout << distort(Tree::bamboo(180001), 50).shuffled() << endl;
    } else if (id == 2) {
        cout << Tree::star(200000).shuffled() << endl;
    } else if (id == 3) {
        cout << distort(Tree::star(190000), 1000).shuffled() << endl;
    } else if (id == 4 || id == 5) {
        Tree a = Tree::bamboo(98000);
        Tree b = Tree::star(98000);
        a = a.link(0, b, 0);

        if (id == 5) {
            a = distort(a, 200);
        }

        cout << a.shuffled() << endl;
    } else if (id == 6) {
        cout << Tree::caterpillar(200000, 50000).shuffled() << endl;
    } else if (id == 7) {
        cout << Tree::caterpillar(20000, 150000).shuffled() << endl;
    }
}

int main(int argc, char *argv[]) {
    registerGen(argc, argv);
    parseArgs(argc, argv);

    setMod().printN().add1();

    string type;
    int n, a = -1, b = -1;

    getPositional(type, n, a, b);

    if (a == -1) {
        cerr << "a = -1" << endl;
        a = rnd.next(1, int(sqrt(n)));
    }
    if (b == -1) {
        cerr << "b = -1" << endl;
        b = rnd.next(1, int(sqrt(n)));
    }

    if (type == "yes") {
        cout << goodTree(n, a, b).shuffled() << endl;
    }

    if (type == "no") {
        int bad = rnd.next(1, min(n, 10));
        cout << distort(goodTree(n - bad, a, b).shuffled(), bad) << endl;
    }

    if (type == "bamboo") {
        cout << Tree::bamboo(n).shuffled() << endl;
    }

    if (type == "special") {
        genSpecial(n);
    }
}

