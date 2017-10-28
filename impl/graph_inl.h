#ifndef JNGEN_INCLUDE_GRAPH_INL_H
#error File "graph_inl.h" must not be included directly.
#include "../graph.h" // for completion emgine
#endif

#include "../array.h"
#include "../common.h"
#include "../printers.h"

namespace jngen {

namespace graph_detail {

Graph BuilderProxy::g() const {
    return builder_(traits_);
}

BuilderProxy::operator Graph() const {
    return g();
}

class GraphRandom {
    using BuilderProxy = graph_detail::BuilderProxy;
    using Traits = graph_detail::Traits;

public:
    GraphRandom() {
        static bool created = false;
        ensure(!created, "jngen::GraphRandom should be created only once");
        created = true;
    }

    static BuilderProxy random(int n, int m) {
        ensure(
            n >= 0 && m >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        checkLargeParameter(m);
        return BuilderProxy(Traits(n, m), &doRandom);
    }

    static BuilderProxy complete(int n) {
        ensure(
            n >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n * n);
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            if (t.directed) {
                g.directed_ = true;
            }
            for (int i = 0; i < t.n; ++i) {
                for (int j = 0; j <= i; ++j) {
                    if (i == j) {
                        if (t.allowLoops) {
                            g.addEdge(i, j);
                        }
                        continue;
                    }

                    if (t.directed) {
                        if (t.acyclic) {
                            g.addEdge(i, j);
                        } else if (t.allowAntiparallel) {
                            g.addEdge(i, j);
                            g.addEdge(j, i);
                        } else {
                            if (rnd.next(2)) {
                                g.addEdge(i, j);
                            } else {
                                g.addEdge(j, i);
                            }
                        }
                    } else {
                        g.addEdge(i, j);
                        if (t.allowMulti) {
                            g.addEdge(j, i);
                        }
                    }
                }
            }
            g.normalizeEdges();
            return g;
        });
    }

    static BuilderProxy empty(int n) {
        ensure(
            n >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            if (t.directed) {
                g.directed_ = true;
            }
            g.setN(t.n);
            return g;
        });
    }

    static BuilderProxy cycle(int n) {
        ensure(
            n >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            if (t.directed) {
                g.directed_ = true;
            }
            for (int i = 0; i < t.n; ++i) {
                g.addEdge(i, (i+1)%t.n);
            }
            g.normalizeEdges();
            return g;
        });
    }

    static BuilderProxy randomStretched(
            int n, int m, int elongation, int spread)
    {
        ensure(
            n >= 0 && m >= 0,
            "Number of vertices and edges in the graph must be nonnegative");
        checkLargeParameter(n);
        checkLargeParameter(m);
        return BuilderProxy(Traits(n, m), [elongation, spread](Traits t) {
            return doRandomStretched(t, elongation, spread);
        });
    }

private:
    static Graph doRandom(Traits t) {
        if (t.directed && t.acyclic) {
            return doDag(t);
        }
        int n = t.n;
        int m = t.m;

        if (!t.allowMulti) {
            ensure(m <= maxEdges(n, t), "Too many edges in the graph");
        }

        std::set<std::pair<int, int>> usedEdges;

        if (t.connected) {
            ensure(m >= n - 1, "Not enough edges for a connected graph");
            auto treeEdges = Tree::random(n).edges();
            if (t.directed) {
                for (auto& edge: treeEdges) {
                    if (rnd.next(2)) {
                        std::swap(edge.first, edge.second);
                    }
                }
            }
            usedEdges.insert(treeEdges.begin(), treeEdges.end());
            ENSURE(usedEdges.size() == static_cast<size_t>(n - 1));
        }

        auto edgeIsGood = [&usedEdges, t](std::pair<int, int> edge) {
            // TODO: move this check to edges generation loop
            if (!t.allowLoops && edge.first == edge.second) {
                ENSURE(false);
                return false;
            }
            if (!t.directed && edge.first > edge.second) {
                ENSURE(false);
                std::swap(edge.first, edge.second);
            }
            if (!t.allowMulti && usedEdges.count(edge)) {
                return false;
            }
            if (t.directed && !t.allowAntiparallel &&
                    usedEdges.count({edge.second, edge.first}))
            {
                return false;
            }
            return true;
        };

        Arrayp result(usedEdges.begin(), usedEdges.end());

        while (result.size() < static_cast<size_t>(m)) {
            auto edge = randomEdge(n, t);
            if (edgeIsGood(edge)) {
                usedEdges.insert(edge);
                result.push_back(edge);
            }
        }

        ENSURE(
            result.size() == static_cast<size_t>(m),
            "Not enough edges found");

        Graph graph;
        if (t.directed) {
            graph.directed_ = true;
        }

        graph.setN(n);
        for (const auto& edge: result) {
            graph.addEdge(edge.first, edge.second);
        }

        graph.normalizeEdges();

        return graph;
    }

    static Graph doRandomStretched(Traits t, int elongation, int spread) {
        ensure(
                !t.directed,
                "randomStretched not available for directed graphs");

        Tree tree = Tree::randomPrim(t.n, elongation);
        Array parents = tree.parents(0);

        Graph graph(tree);

        auto treeEdges = tree.edges();
        std::set<std::pair<int, int>> usedEdges(
            treeEdges.begin(), treeEdges.end());

        while (graph.m() != t.m) {
            int u = rnd.next(t.n);
            int up = rnd.next(0, spread);
            int v = u;
            for (int iter = 0; iter < up; ++iter) {
                v = parents[v];
            }

            ENSURE(v <= u);

            if (!t.allowLoops && u == v) {
                continue;
            }

            if (!t.allowMulti && usedEdges.count({v, u})) {
                continue;
            }

            graph.addEdge(u, v);
            usedEdges.emplace(u, v);
        }

        graph.normalizeEdges();
        return graph;
    }

    static Graph doDag(Traits t) {
        (void)t;
        ENSURE(false, "No dags at the moment");

    }

    static std::pair<int, int> randomEdge(int n, const Traits& t) {
        return rnd.nextp(n, RandomPairTraits{!t.directed, !t.allowLoops});
    }

    static long long maxEdges(int n, const Traits& t) {
        ENSURE(!t.allowMulti);
        long long res = static_cast<long long>(n) * (n-1);
        if (!(t.directed && t.allowAntiparallel)) {
            res /= 2;
        }
        if (t.allowLoops) {
            res += n;
        }
        return res;
    }
};

} // namespace graph_detail

Graph::BuilderProxy Graph::random(int n, int m) {
    return graph_detail::GraphRandom::random(n, m);
}

Graph::BuilderProxy Graph::complete(int n) {
    return graph_detail::GraphRandom::complete(n);
}

Graph::BuilderProxy Graph::empty(int n) {
    return graph_detail::GraphRandom::empty(n);
}

Graph::BuilderProxy Graph::cycle(int n) {
    return graph_detail::GraphRandom::cycle(n);
}

Graph::BuilderProxy Graph::randomStretched(
        int n, int m, int elongation, int spread) {
    return graph_detail::GraphRandom::randomStretched(n, m, elongation, spread);
}

} // namespace jngen
