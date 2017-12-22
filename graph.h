#pragma once

#include "array.h"
#include "generic_graph.h"
#include "graph_builder_proxy.h"
#include "tree.h"

#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace jngen {

class Graph : public ReprProxy<Graph>, public GenericGraph {
    using BuilderProxy = graph_detail::BuilderProxy;
    using Traits = graph_detail::Traits;

    friend class graph_detail::GraphRandom;
    friend class graph_detail::BuilderProxy;

public:
    virtual ~Graph() {}
    Graph() {}

    Graph(int n) {
        extend(n);
    }

    Graph(const GenericGraph& gg) : GenericGraph(gg) {}

    void setN(int n);

    Graph& shuffle();
    Graph shuffled() const;
    Graph& shuffleAllBut(const Array& except);
    Graph shuffledAllBut(const Array& except) const;

    static BuilderProxy random(int n, int m);
    static BuilderProxy complete(int n);
    static BuilderProxy empty(int n);
    static BuilderProxy cycle(int n);
    static BuilderProxy randomStretched(
            int n, int m, int elongation, int spread);
};

inline void Graph::setN(int n) {
    ensure(n >= this->n(), "Cannot lessen number of vertices in the graph");
    extend(n);
}

inline Graph& Graph::shuffle() {
    doShuffle();
    return *this;
}

inline Graph Graph::shuffled() const {
    Graph g(*this);
    return g.shuffle();
}

inline Graph& Graph::shuffleAllBut(const Array& except) {
    doShuffleAllBut(except);
    return *this;
}

inline Graph Graph::shuffledAllBut(const Array& except) const {
    Graph g(*this);
    return g.shuffleAllBut(except);
}

JNGEN_DECLARE_SIMPLE_PRINTER(Graph, 2) {
    t.doPrintEdges(out, mod);
}

JNGEN_DECLARE_SIMPLE_PRINTER(graph_detail::BuilderProxy, 2) {
    JNGEN_PRINT(t.g());
}

template<>
struct Hash<Graph> {
    uint64_t operator()(const Graph& g) const {
        return Hash<GenericGraph>{}(g);
    };
};

} // namespace jngen

using jngen::Graph;

JNGEN_DEFINE_STD_HASH(jngen::Graph);

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_GRAPH_INL_H
#include "impl/graph_inl.h"
#undef JNGEN_INCLUDE_GRAPH_INL_H
#endif // JNGEN_DECLARE_ONLY
