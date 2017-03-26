#pragma once

#include <algorithm>
#include <set>

#include "array.h"
#include "common.h"
#include "graph.h"
#include "printers.h"

namespace jngen {

class GraphRandom;

namespace graph_detail {

struct Traits {
    int n;
    int m;
    bool directed = false;
    bool allowLoops = false;
    bool allowMulti = false;
    bool connected = false;

    Traits() {}
    explicit Traits(int n) : n(n) {}
    Traits(int n, int m) : n(n), m(m) {}
};

class BuilderProxy {
public:
    BuilderProxy(
            Traits traits,
            std::function<Graph(Traits)> builder) :
        traits_(traits),
        builder_(builder)
    {  }

    Graph g() const {
        return builder_(traits_);
    }

    operator Graph() const { return g(); };

    BuilderProxy& allowLoops(bool value = true) {
        traits_.allowLoops = value;
        return *this;
    }

    BuilderProxy& allowMulti(bool value = true) {
        traits_.allowMulti = value;
        return *this;
    }

    BuilderProxy& connected(bool value = true) {
        traits_.connected = value;
        return *this;
    }

private:
    Traits traits_;
    std::function<Graph(Traits)> builder_;
};

} // namespace graph_detail

// TODO: set directedness in graphs
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
        return BuilderProxy(Traits(n, m), &doRandom);
    }

    static BuilderProxy complete(int n) {
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            for (int i = 0; i < t.n; ++i) {
                for (int j = 0; j < t.n; ++j) {
                    if (i < j ||
                        (i == j && t.allowLoops) ||
                        (i > j && t.directed))
                    {
                        g.addEdge(i, j);
                    }
                }
            }
            g.normalizeEdges();
            return g;
        });
    }

    static BuilderProxy empty(int n) {
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            g.setN(t.n);
            return g;
        });
    }

    static BuilderProxy cycle(int n) {
        return BuilderProxy(Traits(n), [](Traits t) {
            Graph g;
            for (int i = 0; i < t.n; ++i) {
                g.addEdge(i, (i+1)%t.n);
            }
            g.normalizeEdges();
            return g;
        });
    }

    static BuilderProxy randomStretched(
            int n, int m, int elongation, int spread) {
        return BuilderProxy(Traits(n, m), [elongation, spread](Traits t) {
            return doRandomStretched(t, elongation, spread);
        });
    }

public:
    static Graph doRandom(Traits t) {
        int n = t.n;
        int m = t.m;

        if (!t.allowMulti) {
            ensure(m <= maxEdges(n, t), "Too many edges in the graph");
        }

        std::set<std::pair<int, int>> usedEdges;

        if (t.connected) {
            ensure(m >= n - 1, "Not enough edges for a connected graph");
            auto treeEdges = Tree::randomPrufer(n).edges();
            usedEdges.insert(treeEdges.begin(), treeEdges.end());
            ensure(usedEdges.size() == static_cast<size_t>(n - 1));
        }

        auto edgeIsGood = [&usedEdges, t](std::pair<int, int> edge) {
            // TODO: move this check to edges generation loop
            if (!t.allowLoops && edge.first == edge.second) {
                ensure(false);
                return false;
            }
            if (!t.directed && edge.first > edge.second) {
                ensure(false);
                std::swap(edge.first, edge.second);
            }

            if (!t.allowMulti && usedEdges.count(edge)) {
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

        ensure(result.size() == static_cast<size_t>(m),
            "[INTERNAL ASSERT] Not enough edges found");

        Graph graph;

        graph.setN(n);
        for (const auto& edge: result) {
            graph.addEdge(edge.first, edge.second);
        }

        graph.normalizeEdges();

        return graph;
    }

    static Graph doRandomStretched(Traits t, int elongation, int spread) {
        Tree tree = Tree::random(t.n, elongation);
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

            ensure(v <= u);

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

    static std::pair<int, int> randomEdge(int n, const Traits& t) {
        return rnd.nextp(n, RandomPairTraits{!t.directed, !t.allowLoops});
    }

    static long long maxEdges(int n, const Traits& t) {
        ensure(!t.allowMulti);
        long long res = static_cast<long long>(n) * (n-1);
        if (!t.directed) {
            res /= 2;
        }
        if (t.allowLoops) {
            res += n;
        }
        return res;
    }
};

JNGEN_EXTERN GraphRandom rndg;

JNGEN_DECLARE_SIMPLE_PRINTER(graph_detail::BuilderProxy, 2) {
    JNGEN_PRINT(t.g());
}

} // namespace jngen

using jngen::rndg;
