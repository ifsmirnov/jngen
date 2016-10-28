#pragma once

#include <bits/stdc++.h>

#include "dsu.h"
#include "array.h"
#include "printers.h"

namespace impl {

class GenericGraph {
public:
    int n() const { return adjList_.size(); }
    int m() const { return numEdges_; }

    void addEdge(int u, int v);
    bool connected() const { return dsu_.connected(); }

    int vertexLabel(int v) const { return vertexLabel_[v]; }
    int vettexByLabel(int v) const { return vertexByLabel_[v]; }

    const std::vector<int>& edges(int v) const {
        return adjList_[v];
    }

    void printEdges(std::ostream& out, const OutputModifier& mod) const;

protected:
    void doShuffle() {
        if (vertexLabel_.size() < static_cast<size_t>(n())) {
            vertexLabel_ = Array::id(n());
        }
        vertexLabel_.shuffle();
        vertexByLabel_ = vertexLabel_.inverse();
    }

    void extend(size_t size) {
        size_t oldSize = n();
        if (size > oldSize) {
            adjList_.resize(size);
            vertexLabel_ += Array::id(size - oldSize, oldSize);
            vertexByLabel_ += Array::id(size - oldSize, oldSize);
        }
    }

    void addEdgeUnsafe(int u, int v) {
        adjList_[u].push_back(v);
        adjList_[v].push_back(u);
    }

    int numEdges_ = 0;

    Dsu dsu_;
    std::vector<std::vector<int>> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;
};

inline void GenericGraph::addEdge(int u, int v) {
    extend(std::max(u, v) + 1);
    dsu_.link(u, v);
    addEdgeUnsafe(u, v);
}

inline void GenericGraph::printEdges(
    std::ostream& out, const OutputModifier& mod) const
{
    Arrayp edges;
    for (int v = 0; v < n(); ++v) {
        for (int to: this->edges(v)) {
            if (v < to) {
                edges.emplace_back(vertexLabel(v), vertexLabel(to));
            }
        }
    }

    JNGEN_PRINT(edges);
}

} // namespace impl

