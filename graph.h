#pragma once

#include "array.h"
#include "generic_graph.h"
#include "tree.h"

#include <memory>
#include <set>
#include <utility>
#include <vector>

namespace jngen {

class Graph : public ReprProxy<Graph>, public GenericGraph {
    friend class GraphRandom;
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

JNGEN_DECLARE_SIMPLE_PRINTER(Graph, 2) {
    t.doPrintEdges(out, mod);
}

} // namespace jngen

using jngen::Graph;
