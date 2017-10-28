#pragma once

#include "array.h"
#include "dsu.h"
#include "printers.h"
#include "weight.h"

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

    bool directed() const { return directed_; }

    // u, v: labels
    virtual void addEdge(int u, int v, const Weight& w = Weight{});
    virtual bool isConnected() const { return dsu_.isConnected(); }

    virtual int vertexLabel(int v) const { return vertexLabel_.at(v); }
    virtual int vertexByLabel(int v) const { return vertexByLabel_.at(v); }

    // v: label
    // return: array<label>
    virtual Array edges(int v) const;

    // return: array<label, label>
    virtual Arrayp edges() const;

    // order: by labels
    // TODO: think about ordering here
    virtual void setVertexWeights(const WeightArray& weights);
    // v: label
    virtual void setVertexWeight(int v, const Weight& weight);

    virtual void setEdgeWeights(const WeightArray& weights);
    virtual void setEdgeWeight(size_t index, const Weight& weight);

    // v: label
    virtual Weight vertexWeight(int v) const;
    virtual Weight edgeWeight(size_t index) const;

    // TODO: should it really be public?
    virtual void doPrintEdges(
        std::ostream& out, const OutputModifier& mod) const;

    virtual bool operator==(const GenericGraph& other) const;
    virtual bool operator!=(const GenericGraph& other) const;
    virtual bool operator< (const GenericGraph& other) const;
    virtual bool operator> (const GenericGraph& other) const;
    virtual bool operator<=(const GenericGraph& other) const;
    virtual bool operator>=(const GenericGraph& other) const;

protected:
    static WeightArray prepareWeightArray(WeightArray a, int requiredSize);

    void doShuffle();

    void extend(size_t size);

    // v: vertex number
    // returns: array<number>
    Array internalEdges(int v) const;

    // u, v: vertex numbers
    void addEdgeUnsafe(int u, int v);

    // v: vertex number
    // returns: vertex number
    int edgeOtherEnd(int v, int edgeId) const;

    void permuteEdges(const Array& order);

    void normalizeEdges();

    int compareTo(const GenericGraph& other) const;

    int numEdges_ = 0;

    bool directed_ = false;

    Dsu dsu_;
    std::vector<Array> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;
    Arrayp edges_;

    WeightArray vertexWeights_;
    WeightArray edgeWeights_;
};

template<>
struct Hash<GenericGraph> {
    uint64_t operator()(const GenericGraph& graph) const {
        uint64_t h = 0;
        for (int i = 0; i < graph.n(); ++i) {
            impl::hashCombine(h, Hash<Array>{}(graph.edges(i)));
        }
        return h;
    }
};

} // namespace jngen

JNGEN_DEFINE_STD_HASH(jngen::GenericGraph);

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_GENERIC_GRAPH_INL_H
#include "impl/generic_graph_inl.h"
#undef JNGEN_INCLUDE_GENERIC_GRAPH_INL_H
#endif // JNGEN_DECLARE_ONLY
