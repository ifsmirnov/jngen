#pragma once

#include <bits/stdc++.h>

#include "array.h"
#include "printers.h"
#include "generic_graph.h"

namespace impl {

class Tree : public ReprProxy<Tree>, public GenericGraph {
public:
    void addEdge(int u, int v);

    Tree& shuffle();
    Tree shuffled() const;
};

inline void Tree::addEdge(int u, int v) {
    extend(std::max(u, v) + 1);

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

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 0) {
    ensure(t.connected(), "Tree is not connected :(");

    if (mod.printParents) {
        out << "Printing parents is not supported yet";
    } else if (mod.printEdges) {
        t.GenericGraph::printEdges(out, mod);
    } else {
        ensure(false, "Print mode is unknown");
    }
}

} // namespace impl

using impl::Tree;
