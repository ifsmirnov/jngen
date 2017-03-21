#pragma once

#include "array.h"
#include "generic_graph.h"
#include "tree.h"

#include <memory>
#include <set>
#include <utility>
#include <vector>

/* Directed graphs are not supported yet, and Graph class itself
 * is pretty useless. Sorry for now.
 */

namespace jngen {

// TODO: make GraphBuilder subclass of Graph
class GraphBuilder;

class Graph : public ReprProxy<Graph>, public GenericGraph {
    friend class GraphBuilder;
public:
    virtual ~Graph() {}
    Graph() {}

    Graph(int n) {
        extend(n);
    }

    Graph(const GenericGraph& gg) : GenericGraph(gg) {}

    void setN(int n);

    static Graph random(int n, int m);

    Graph& allowLoops(bool value = true);
    Graph& allowMulti(bool value = true);
    Graph& connected(bool value = true);

    int n() const override { return self().GenericGraph::n(); }
    int m() const override { return self().GenericGraph::m(); }
    void addEdge(int u, int v, const Weight& w = Weight{}) override {
        self().GenericGraph::addEdge(u, v, w);
    }
    bool isConnected() const override {
        return self().GenericGraph::isConnected();
    }
    Array edges(int v) const override {
        return self().GenericGraph::edges(v);
    }
    Arrayp edges() const override {
        return self().GenericGraph::edges();
    }
    virtual void setVertexWeights(const WeightArray& weights) override {
        self().GenericGraph::setVertexWeights(weights);
    }
    virtual void setVertexWeight(int v, const Weight& weight) override {
        self().GenericGraph::setVertexWeight(v, weight);
    }
    virtual void setEdgeWeights(const WeightArray& weights) override {
        self().GenericGraph::setEdgeWeights(weights);
    }
    virtual void setEdgeWeight(size_t index, const Weight& weight) override {
        self().GenericGraph::setEdgeWeight(index, weight);
    }
    virtual Weight vertexWeight(int v) const override {
        return self().GenericGraph::vertexWeight(v);
    }
    virtual Weight edgeWeight(size_t index) const override {
        return self().GenericGraph::edgeWeight(index);
    }
    int vertexLabel(int v) const override {
        return self().GenericGraph::vertexLabel(v);
    }
    int vertexByLabel(int v) const override {
        return self().GenericGraph::vertexByLabel(v);
    }

    Graph& shuffle();
    Graph shuffled() const;

private:
    void setBuilder(std::shared_ptr<GraphBuilder> builder) {
        builder_ = builder;
    }

    const Graph& self() const;
    Graph& self();

    std::shared_ptr<GraphBuilder> builder_;
};

class GraphBuilder {
public:
    const Graph& graph() {
        if (!finalized_) {
            build();
        }
        return graph_;
    }

    GraphBuilder(int n, int m) :
        n_(n), m_(m)
    {  }

    void allowLoops(bool value) {
        loops_ = value;
    }

    void allowMulti(bool value) {
        multiEdges_ = value;
    }

    void connected(bool value) {
        connected_ = value;
    }

private:
    void build();

    int n_;
    int m_;
    bool connected_ = false;
    bool multiEdges_ = false;
    bool loops_ = false;

    bool finalized_ = false;
    Graph graph_;
};

inline void Graph::setN(int n) {
    ensure(n >= this->n(), "Cannot lessen number of vertices in the graph");
    extend(n);
}

inline Graph& Graph::allowLoops(bool value) {
    ensure(builder_, "Cannot modify the graph which is already built");
    builder_->allowLoops(value);
    return *this;
}

inline Graph& Graph::allowMulti(bool value) {
    ensure(builder_, "Cannot modify the graph which is already built");
    builder_->allowMulti(value);
    return *this;
}

inline Graph& Graph::connected(bool value) {
    ensure(builder_, "Cannot modify the graph which is already built");
    builder_->connected(value);
    return *this;
}

inline void GraphBuilder::build() {
    // the probability distribution is not uniform in some cases
    // but we forget about it for now.

    ensure(!finalized_);
    finalized_ = true;

    int n = n_;
    int m = m_;

    if (!multiEdges_) {
        long long maxEdges = static_cast<long long>(n) *
            (n + (loops_ ? 1 : -1)) / 2;
        ensure(m_ <= maxEdges, "Too many edges in the graph");
    }

    std::set<std::pair<int, int>> usedEdges;

    if (connected_) {
        ensure(m_ >= n_ - 1, "Not enough edges for a connected graph");
        auto treeEdges = Tree::randomPrufer(n).edges();
        usedEdges.insert(treeEdges.begin(), treeEdges.end());
        ensure(usedEdges.size() == static_cast<size_t>(n - 1));
    }

    auto edgeIsGood = [&usedEdges, this](const std::pair<int, int>& edge) {
        if (!loops_ && edge.first == edge.second) {
            return false;
        }
        if (!multiEdges_ && usedEdges.count(edge)) {
            return false;
        }
        return true;
    };

    Arrayp result(usedEdges.begin(), usedEdges.end());

    while (result.size() < static_cast<size_t>(m)) {
        auto edge = rnd.tnext<std::pair<int, int>>(n, opair);
        if (edgeIsGood(edge)) {
            usedEdges.insert(edge);
            result.push_back(edge);
        }
    }

    ensure(result.size() == static_cast<size_t>(m),
        "[INTERNAL ASSERT] Not enough edges found");

    graph_.setN(n);
    for (const auto& edge: result) {
        graph_.addEdge(edge.first, edge.second);
    }

    graph_.normalizeEdges();
}

Graph Graph::random(int n, int m) {
    Graph g;
    auto builder = std::make_shared<GraphBuilder>(n, m);
    g.setBuilder(builder);
    return g;
}

inline Graph& Graph::shuffle() {
    self().doShuffle();
    return *this;
}

inline Graph Graph::shuffled() const {
    Graph g = self();
    return g.shuffle();
}

const Graph& Graph::self() const {
    return builder_ ? builder_->graph() : *this;
}

Graph& Graph::self() {
    if (builder_) {
        *this = builder_->graph();
        builder_.reset();
    }
    return *this;
}

JNGEN_DECLARE_SIMPLE_PRINTER(Graph, 2) {
    t.doPrintEdges(out, mod);
}

} // namespace jngen

using jngen::Graph;
