#pragma once

#include "array.h"
#include "dsu.h"
#include "printers.h"

#include <algorithm>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

namespace jngen {

class GenericGraph {
public:
    virtual ~GenericGraph() {}

    virtual int n() const { return adjList_.size(); }
    virtual int m() const { return numEdges_; }

    virtual void addEdge(int u, int v);
    virtual bool connected() const { return dsu_.connected(); }

    virtual int vertexLabel(int v) const { return vertexLabel_[v]; }
    virtual int vertexByLabel(int v) const { return vertexByLabel_[v]; }

    virtual const std::vector<int>& edges(int v) const {
        return adjList_[v];
    }

    virtual std::vector<std::pair<int, int>> edges() const {
        std::vector<std::pair<int, int>> result;
        for (int v = 0; v < n(); ++v) {
            for (int to: edges(v)) {
                if (v <= to) {
                    result.emplace_back(vertexLabel(v), vertexLabel(to));
                }
            }
        }
        return result;
    }

    virtual void doPrintEdges(
        std::ostream& out, const OutputModifier& mod) const;

    virtual bool operator==(const GenericGraph& other) const;
    virtual bool operator<(const GenericGraph& other) const;

protected:
    void doShuffle() {
        if (vertexLabel_.size() < static_cast<size_t>(n())) {
            vertexLabel_ = Array::id(n());
        }
        vertexLabel_.shuffle();
        vertexByLabel_ = vertexLabel_.inverse();
        edgesShuffled_ = true;
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
        if (u != v) {
            adjList_[v].push_back(u);
        }
    }

    int compareTo(const GenericGraph& other) const;

    int numEdges_ = 0;

    bool edgesShuffled_ = false;

    Dsu dsu_;
    std::vector<std::vector<int>> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;
};

inline void GenericGraph::addEdge(int u, int v) {
    extend(std::max(u, v) + 1);
    dsu_.link(u, v);
    addEdgeUnsafe(u, v);
    ++numEdges_;
}

inline void GenericGraph::doPrintEdges(
    std::ostream& out, const OutputModifier& mod) const
{
    Arrayp edges;
    for (int v = 0; v < n(); ++v) {
        for (int to: this->edges(v)) {
            if (v <= to) {
                edges.emplace_back(vertexLabel(v), vertexLabel(to));
            }
        }
    }

    if (edgesShuffled_) {
        edges.shuffle();
    }

    if (mod.printN) {
        out << n();
        if (mod.printM) {
            out << " " << m();
        }
        out << "\n";
    } else if (mod.printM) {
        out << m() << "\n";
    }

    auto t(mod);
    {
        auto mod(t);
        mod.printN = false;
        JNGEN_PRINT(edges);
    }
}

inline bool GenericGraph::operator==(const GenericGraph& other) const {
    return compareTo(other) == 0;
}

inline bool GenericGraph::operator<(const GenericGraph& other) const {
    return compareTo(other) == -1;
}

inline int GenericGraph::compareTo(const GenericGraph& other) const {
    if (n() < other.n()) {
        return -1;
    }
    if (n() > other.n()) {
        return 1;
    }
    for (int i = 0; i < n(); ++i) {
        std::set<int> edges1(edges(i).begin(), edges(i).end());
        std::set<int> edges2(other.edges(i).begin(), other.edges(i).end());
        if (edges1 < edges2) {
            return -1;
        }
        if (edges1 > edges2) {
            return 1;
        }
    }
    return 0;
}

} // namespace jngen

