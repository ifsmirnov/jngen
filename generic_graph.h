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

    // u, v: labels
    virtual void addEdge(int u, int v);
    virtual bool isConnected() const { return dsu_.isConnected(); }

    virtual int vertexLabel(int v) const { return vertexLabel_[v]; }
    virtual int vertexByLabel(int v) const { return vertexByLabel_[v]; }

    // v: label
    // return: array<label>
    virtual Array edges(int v) const {
        v = vertexByLabel(v);

        Array result;
        std::transform(
            adjList_[v].begin(),
            adjList_[v].end(),
            std::back_inserter(result),
            [this, v](int x) { return vertexLabel(edgeOtherEnd(v, x)); }
        );
        return result;
    }

    // return: array<label, label>
    virtual Arrayp edges() const {
        auto edges = edges_;
        for (auto& e: edges) {
            e.first = vertexLabel(e.first);
            e.second = vertexLabel(e.second);
        }
        return edges;
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

        if (!directed_) {
            for (auto& edge: edges_) {
                if (rnd.next(2)) {
                    std::swap(edge.first, edge.second);
                }
            }
        }

        permuteEdges(Array::id(numEdges_).shuffled());
    }

    void extend(size_t size) {
        size_t oldSize = n();
        if (size > oldSize) {
            adjList_.resize(size);
            vertexLabel_ += Array::id(size - oldSize, oldSize);
            vertexByLabel_ += Array::id(size - oldSize, oldSize);
        }
    }

    // u, v: edge numbers
    void addEdgeUnsafe(int u, int v) {
        int id = numEdges_++;
        edges_.emplace_back(u, v);

        adjList_[u].push_back(id);
        if (u != v) {
            adjList_[v].push_back(id);
        }
    }

    // v: edge number
    // returns: edge number
    int edgeOtherEnd(int v, int edgeId) {
        ensure(edgeId < numEdges_);
        const auto& edge = edges_[edgeId];
        if (edge.first == v) {
            return edge.second;
        }
        ensure(!directed_);
        ensure(edge.second == v);
        return edge.first;
    }

    void permuteEdges(const Array& order) {
        edges_ = edges_.subseq(order);

        auto newByOld = order.inverse();
        for (int v = 0; v < n(); ++v) {
            for (auto& x: adjList_[v]) {
                x = newByOld[x];
            }
        }
    }

    void normalizeEdges() {
        ensure(
            vertexLabel_ == Array::id(n()),
            "Can call normalizeEdges() only on newly created graph");

        if (!directed_) {
            for (auto& edge: edges_) {
                if (edge.first > edge.second) {
                    std::swap(edge.first, edge.second);
                }
            }
        }

        auto order = Array::id(numEdges_).sorted(
            [this](int i, int j) {
                return edges_[i] < edges_[j];
            });

        permuteEdges(order);
    }

    int compareTo(const GenericGraph& other) const;

    int numEdges_ = 0;

    bool directed_ = false;

    Dsu dsu_;
    std::vector<Array> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;
    Arrayp edges_;
};

inline void GenericGraph::addEdge(int u, int v) {
    extend(std::max(u, v) + 1);

    u = vertexByLabel(u);
    v = vertexByLabel(v);

    dsu_.link(u, v);
    addEdgeUnsafe(u, v);
}

inline void GenericGraph::doPrintEdges(
    std::ostream& out, const OutputModifier& mod) const
{
    Arrayp edges = this->edges();

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

