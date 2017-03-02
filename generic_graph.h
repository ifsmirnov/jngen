#pragma once

#include "array.h"
#include "dsu.h"
#include "printers.h"

#include <algorithm>
#include <iostream>
#include <iterator>
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

    virtual Array edges(int v) const {
        v = vertexByLabel(v);

        Array result;
        std::transform(
            adjList_[v].begin(),
            adjList_[v].end(),
            std::back_inserter(result),
            [this](int x) { return vertexLabel(x); }
        );
        return result;
    }

    virtual Arrayp edges() const {
        Arrayp result;
        for (int id = 0; id < n(); ++id) {
            int v = vertexByLabel(id);
            size_t pos = result.size();
            for (int to: edges(v)) {
                if (v <= to) {
                    result.emplace_back(vertexLabel(v), vertexLabel(to));
                }
            }
            std::sort(result.begin() + pos, result.end());
        }
        return result;
    }

    // TODO: should it really be public?
    virtual void doPrintEdges(
        std::ostream& out, const OutputModifier& mod) const;

    // TODO: more operators!
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
        ++numEdges_;
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

// TODO: this should compare by vertex labels actually
inline int GenericGraph::compareTo(const GenericGraph& other) const {
    if (n() != other.n()) {
        return n() < other.n() ? -1 : 1;
    }
    for (int i = 0; i < n(); ++i) {
        Array e1 = Array(edges(i)).sorted();
        Array e2 = Array(other.edges(i)).sorted();
        if (e1 != e2) {
            return e1 < e2 ? -1 : 1;
        }
    }
    return 0;
}

} // namespace jngen

