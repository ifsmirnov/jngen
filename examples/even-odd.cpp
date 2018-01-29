#include "jngen.h"
#include <bits/stdc++.h>
using namespace std;
#define forn(i, n) for (int i = 0; i < (int)(n); ++i)

#define se second
#define fi first

Graph connectedBipartite(int n, int m) {
    Tree t = Tree::random(n);
    vector<int> q{0};
    vector<int> col(n, -1);
    col[0] = 0;
    Array bc[2];
    bc[0] = {0};
    forn(i, n) {
        int v = q[i];
        for (int to: t.edges(v)) {
            if (col[to] == -1) {
                col[to] = !col[v];
                bc[col[to]].push_back(to);
                q.push_back(to);
            }
        }
    }
    m = min<long long>((long long)m, 1ll * bc[0].size() * bc[1].size());
    auto treeEdges = t.edges();
    Graph g(t);
    set<pair<int, int>> edges(treeEdges.begin(), treeEdges.end());
    while ((int)edges.size() != m) {
        int u = bc[0].choice();
        int v = bc[1].choice();
        if (!edges.count({v, u}) && edges.emplace(u, v).second) {
            g.addEdge(u, v);
        }
    }
    return g.shuffled();
}

Graph makeTreeOfGraphs(const std::vector<Graph>& graphs, bool line = false) {
    Array shifts;
    int s = 0;
    int n = graphs.size();
    forn(i, n) {
        shifts.push_back(s);
        s += graphs[i].n();
    }

    jngen::Dsu dsu;
    dsu.getRoot(s - 1);

    auto t = line ? Tree::bamboo(n) : Tree::random(n);
    for (auto e: t.edges()) {
        int v1 = rnd.next(shifts[e.fi], shifts[e.fi] + graphs[e.fi].n() - 1);
        int v2 = rnd.next(shifts[e.se], shifts[e.se] + graphs[e.se].n() - 1);
        dsu.unite(v1, v2);
    }

    map<int, int> id;
    forn(i, s) {
        int v = dsu.getRoot(i);
        if (!id.count(v)) {
            int t = id.size();
            id[v] = t;
        }
    }

    Graph res(id.size());
    set<pair<int, int>> edges;
    forn(i, n) for (auto e: graphs[i].edges()) {
        int v1 = e.first + shifts[i];
        int v2 = e.second + shifts[i];
        v1 = id[dsu.getRoot(v1)];
        v2 = id[dsu.getRoot(v2)];
        if (v1 != v2 && !edges.count({v1, v2}) && !edges.count({v2, v2})) {
            edges.emplace(v1, v2);
            res.addEdge(v1, v2);
        }
    }
    return res;
}

int main(int argc, char *argv[]) {
    registerGen(argc, argv);
    parseArgs(argc, argv);

    setMod().printN().printM().add1();

    string type = getOpt("type", "random");

    if (type == "random") {
        int n, m;
        ensure(getPositional(n, m) == 2);
        ensure(n >= 2);
        cout << Graph::random(n, m).connected().g().shuffled() << endl;
    } else if (type == "bipartite") {
        int n, m;
        ensure(getPositional(n, m) == 2);
        cout << connectedBipartite(n, m) << endl;
    } else if (type == "bipartite-tree") {
        int n, m;
        ensure(getPositional(n, m) == 2);
        int n_comps = getOpt("n_comps", 5);
        int n_bad = getOpt("n_bad", 0);
        Array vnums = rndm.partition(n, n_comps, /* min_size = */ 1);
        Array enums = vnums;
        for (int& x: enums) {
            --x;
            m -= x;
        }
        auto ePartition = rndm.partition(m, n_comps, /* min_size = */ 1);
        forn(i, n_comps) enums[i] += ePartition[i];
        TArray<Graph> parts;
        forn(i, n_comps) {
            if (rnd.next(n_comps - i) < n_bad) {
                --n_bad;
                parts.push_back(Graph::random(
                    vnums[i], min<long long>(enums[i], 1ll * vnums[i] * (vnums[i] - 1) / 2)).connected()
                    );
            } else {
                parts.push_back(connectedBipartite(vnums[i], enums[i]));
            }
        }
        auto g = makeTreeOfGraphs(parts);
//         cout << Array::id(g.n()).endl().printN(false)  << endl;
//         cout << g.printN(false).printM(false) << endl;
        cout << g.shuffled() << endl;
    } else if (type == "manual") {
        int n = getOpt(0);
        int id = getOpt("id");
        if (id == 1) {
            const int k = 100;
            vector<Graph> graphs;
            forn(i, k) {
                graphs.push_back(connectedBipartite(n / (k*2), n / k));
            }
            auto g = makeTreeOfGraphs(graphs, true);
            cout << g.shuffled() << endl;
        } else if (id == 2) {
            const int k = 100;
            vector<Graph> graphs;
            forn(i, k) {
                if (i%2 == 0) {
                    graphs.push_back(connectedBipartite(n / (k*2), n / k));
                } else {
                    graphs.push_back(Graph::complete(3));
                }
            }
            auto g = makeTreeOfGraphs(graphs, true);
            cout << g.shuffled() << endl;
        } else if (id == 3) {
            cout << Graph(Tree::bamboo(n)).shuffled() << endl;
        } else if (id == 4) {
            cout << Graph(Tree::star(n)).shuffled() << endl;
        } else {
            ensure(false, format("Incorrect manual test id: '%d'", id));
        }
    } else {
        ensure(false, format("Type '%s' is not supported", type.c_str()));
    }

    return 0;
}
