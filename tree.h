#pragma once

#include "array.h"
#include "generic_graph.h"
#include "printers.h"

#include <algorithm>
#include <vector>

namespace jngen {

class Tree : public ReprProxy<Tree>, public GenericGraph {
public:
    Tree() {
        extend(1);
    }

    Tree(const GenericGraph& gg) : GenericGraph(gg) {
        extend(1);
        ensure(
                dsu_.numComponents() == n() - m(),
                "Cannot create a tree from a graph with cycles");
    }

    void addEdge(int u, int v, const Weight& w = Weight{}) override;

    bool canAddEdge(int u, int v);

    Array parents(int root) const;

    Tree& shuffle();
    Tree shuffled() const;
    Tree& shuffleAllBut(const Array& except);
    Tree shuffledAllBut(const Array& except) const;

    Tree link(int vInThis, const Tree& other, int vInOther);
    Tree glue(int vInThis, const Tree& other, int vInOther);

    static Tree bamboo(int size);
    static Tree random(int size);
    static Tree randomPrim(int size, int elongation = 0);
    static Tree randomKruskal(int size);
    static Tree star(int size);
    static Tree caterpillar(int size, int length);
    static Tree binary(int size);
    static Tree kary(int size, int k);

    static Tree fromPruferSequence(const Array& code);

    void doPrintParents(std::ostream& out, const OutputModifier& mod) const;
};

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 2) {
    ensure(t.isConnected(), "Cannot print a tree: it is not connected");

    if (mod.printEdges) {
        t.doPrintEdges(out, mod);
    } else {
        t.doPrintParents(out, mod);
    }
}

template<>
struct Hash<Tree> {
    uint64_t operator()(const Tree& t) const {
        return Hash<GenericGraph>{}(t);
    }
};

} // namespace jngen

JNGEN_DEFINE_STD_HASH(jngen::Tree);

using jngen::Tree;

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_TREE_INL_H
#include "impl/tree_inl.h"
#undef JNGEN_INCLUDE_TREE_INL_H
#endif // JNGEN_DECLARE_ONLY
