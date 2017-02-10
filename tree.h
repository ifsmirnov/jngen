#pragma once

#include <bits/stdc++.h>

#include "array.h"
#include "printers.h"
#include "generic_graph.h"

namespace impl {

class Tree : public ReprProxy<Tree>, public GenericGraph {
public:
    Tree() {
        extend(1);
    }
    Tree(const GenericGraph& gg) : GenericGraph(gg) {
        extend(1);
    }

    void addEdge(int u, int v);

    Tree& shuffle();
    Tree shuffled() const;

    Tree link(int vInThis, const Tree& other, int vInOther);
    Tree glue(int vInThis, const Tree& other, int vInOther);

    static Tree bamboo(size_t size);
    static Tree randomPrufer(size_t size);
    static Tree random(size_t size, double elongation = 1.0);
};

inline void Tree::addEdge(int u, int v) {
    extend(std::max(u, v) + 1);

    u = vertexByLabel(u);
    v = vertexByLabel(v);

    int ret = dsu_.link(u, v);
    ensure(ret, "A cycle appeared in the tree :(");

    addEdgeUnsafe(u, v);
}

inline Tree& Tree::shuffle() {
    doShuffle();
    return *this;
}

inline Tree Tree::shuffled() const {
    Tree t = *this;
    return t.shuffle();
}

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 2) {
    ensure(t.connected(), "Tree is not connected :(");

    if (mod.printParents) {
        out << "Printing parents is not supported yet";
    } else if (mod.printEdges) {
        t.doPrintEdges(out, mod);
    } else {
        ensure(false, "Print mode is unknown");
    }
}

// Tree generators go here

inline Tree Tree::bamboo(size_t size) {
    Tree t;
    for (size_t i = 0; i + 1 < size; ++i) {
        t.addEdge(i, i+1);
    }
    return t;
}

inline Tree Tree::randomPrufer(size_t size) {
    if (size == 1) {
        return Tree();
    }

    Array code = Array::random(size - 2, size);
    std::vector<int> degree(size, 1);
    for (int v: code) {
        ++degree[v];
    }

    std::set<int> leaves;
    for (size_t v = 0; v < size; ++v) {
        if (degree[v] == 1) {
            leaves.insert(v);
        }
    }

    Tree t;
    for (int v: code) {
        ensure(!leaves.empty());
        int to = *leaves.begin();
        leaves.erase(leaves.begin());
        if (--degree[v] == 1) {
            leaves.insert(v);
        }

        t.addEdge(v, to);
    }

    ensure(leaves.size() == 2u);
    t.addEdge(*leaves.begin(), *leaves.rbegin());
    return t;
}

inline Tree Tree::random(size_t size, double elongation) {
    Tree t;
    for (size_t v = 1; v < size; ++v) {
        int parent = rnd.tnext<int>(v-1 - (v-1) * elongation, v-1);
        t.addEdge(parent, v);
    }
    return t;
}

Tree Tree::link(int vInThis, const Tree& other, int vInOther) {
    Tree t(*this);

    for (const auto& e: other.edges()) {
        t.addEdge(e.first + n(), e.second + n());
    }

    t.addEdge(vInThis, vInOther + n());

    return t;
}

Tree Tree::glue(int vInThis, const Tree& other, int vInOther) {
    auto newLabel = [vInThis, vInOther, &other, this] (int v) {
        if (v < vInOther) {
            return n() + v;
        } else if (v == vInOther) {
            return vInThis;
        } else {
            return n() + v - 1;
        }
    };

    Tree t(*this);

    for (const auto& e: other.edges()) {
        t.addEdge(newLabel(e.first), newLabel(e.second));
    }

    assert(t.n() == n() + other.n() - 1);

    return t;
}

} // namespace impl

using impl::Tree;
