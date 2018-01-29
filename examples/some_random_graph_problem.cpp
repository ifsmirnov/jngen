#include "jngen.h"
#include <bits/stdc++.h>
using namespace std;
#define forn(i, n) for (int i = 0; i < (int)(n); ++i)
#define for2(cur, prev, a) for (auto _it1 = std::begin(a),\
        _it2 = _it1 == std::end(a) ? _it1 : std::next(_it1);\
        _it2 != std::end(a); ++_it1, ++_it2)\
        for (bool _ = true; _;)\
        for (auto &cur = *_it1, &prev = *_it2; _; _ = false)

Array getw(int m) {
    int minc = 0, maxc = 9;
    getNamed(minc, maxc);
    return Array::random(m, minc, maxc);
}

int main(int argc, char *argv[]) {
    registerGen(argc, argv);
    parseArgs(argc, argv);

    setMod().printN().printM().add1();

    if (int id = getOpt("manual", 0)) {
        int n = getOpt(0, -1);
        int m = getOpt(1, -1);
        (void)(n+m);

        if (id == 1) {
            cout << "2 1\n1 2 5\n";
        } else if (id == 2) {
            cout << "2 1\n1 2 0\n";
        } else if (id == 3) {
            Graph g = Tree::bamboo(n);
            g.setEdgeWeights(Array::random(n-1, 0, 9));
            g.shuffleAllBut({0, n-1});
            cout << g << endl;
        } else if (id == 4) {
            Graph g = Tree::bamboo(n);
            g.setEdgeWeights(Array::random(n-1, 0, 0));
            g.shuffleAllBut({0, n-1});
            cout << g << endl;
        } else if (id == 5) {
            Graph g = Tree::bamboo(n);
            g.setEdgeWeights(Array::random(n-1, 1, 9));
            g.shuffleAllBut({0, n-1});
            cout << g << endl;
        } else if (id == 6) {
            Graph g = Tree::star(n);
            g.setEdgeWeights(Array::random(n-1, 1, 9));
            g.shuffle();
            cout << g << endl;
        } else if (id == 7) {
            Graph g(n);
            forn(i, n-1) {
                g.addEdge(i, i+1);
                g.setEdgeWeight(g.m()-1, rnd.next(0, 9));
                g.addEdge(i, i+1);
                g.setEdgeWeight(g.m()-1, rnd.next(0, 9));
            }
            g.shuffleAllBut({0, n-1});
            cout << g << endl;
        } else {
            ensure(false, format("manual test id unknown: %d", id));
        }

        return 0;
    }


    int n = getOpt(0);
    int m = getOpt(1);

    string type = getOpt("type", "random");

    if (type == "random") {
        auto g = Graph::random(n, m).connected().allowMulti(true).g();
        g.setEdgeWeights(getw(m));
        g.shuffle();
        cout << g << endl;
    } else if (type == "stretched") {
        int elong = getOpt("elong", 10);
        int spread = getOpt("spread", 5);

        auto g = Graph::randomStretched(n, m, elong, spread).
            connected().allowMulti(true).g();
        g.setEdgeWeights(getw(m));
        g.shuffleAllBut({0, n-1});

        cout << g << endl;
    } else if (type == "levels") {
        int mn = getOpt("min", 1);
        int mx = getOpt("max", 10);
        auto levels = rndm.partition(Array::id(n-2, 1), (n-2) / ((mn + mx)/2), mn, mx);
        levels.insert(levels.begin(), {0});
        levels.push_back({n-1});

        Graph g;

        for2(prev, cur, levels) {
            for (auto v: cur) {
                g.addEdge(v, prev.choice());
                --m;
            }
        }
        while (m) {
            int l1 = rnd.next(1u, levels.size() - 1);
            int v = levels[l1-1].choice();
            int to = levels[l1].choice();
            g.addEdge(v, to);
            --m;
        }
        g.setEdgeWeights(getw(g.m()));

        cout << g << endl;
    } else {
        ensure(false, "Unknown test type");
    }

    return 0;
}
