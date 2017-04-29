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
    }

    void addEdge(int u, int v, const Weight& w = Weight{}) override;

    Array parents(int root) const;

    Tree& shuffle();
    Tree shuffled() const;

    Tree link(int vInThis, const Tree& other, int vInOther);
    Tree glue(int vInThis, const Tree& other, int vInOther);

    static Tree bamboo(int size);
    static Tree randomPrufer(int size);
    static Tree random(int size, int elongation = 0);
    static Tree star(int size);
    static Tree caterpillar(int size, int length);
};

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 2) {
    ensure(t.isConnected(), "Cannot print a tree: it is not connected");

    if (mod.printParents) {
        ensure(false, "Printing parents is not implemented");
    } else if (mod.printEdges) {
        t.doPrintEdges(out, mod);
    } else {
        ensure(false, "Print mode is not set, select one of 'printParents'"
            " or 'printEdges'");
    }
}


#ifndef JNGEN_DECLARE_ONLY

void Tree::addEdge(int u, int v, const Weight& w) {
    extend(std::max(u, v) + 1);

    u = vertexByLabel(u);
    v = vertexByLabel(v);

    int ret = dsu_.unite(u, v);
    ensure(ret, "A cycle appeared in the tree");

    addEdgeUnsafe(u, v);

    if (!w.empty()) {
        setEdgeWeight(m() - 1, w);
    }
}

Array Tree::parents(int root) const {
    ensure(isConnected(), "Tree::parents(int): Tree is not connected");
    root = vertexByLabel(root);

    Array parents(n());
    parents[root] = root;
    std::vector<int> used(n());
    std::vector<int> queue{root};
    for (size_t i = 0; i < queue.size(); ++i) {
        int v = queue[i];
        used[v] = true;
        for (auto to: internalEdges(v)) {
            if (!used[to]) {
                parents[to] = v;
                queue.push_back(to);
            }
        }
    }

    for (auto& x: parents) {
        x = vertexLabel(x);
    }

    return parents;
}

Tree& Tree::shuffle() {
    doShuffle();
    return *this;
}

Tree Tree::shuffled() const {
    Tree t = *this;
    return t.shuffle();
}

Tree Tree::link(int vInThis, const Tree& other, int vInOther) {
    ensure(vInThis < n(), "Cannot link a nonexistent vertex");
    ensure(vInOther < other.n(), "Cannot link to a nonexistent vertex");

    Tree t(*this);

    for (const auto& e: other.edges()) {
        t.addEdge(e.first + n(), e.second + n());
    }

    t.addEdge(vInThis, vInOther + n());

    return t;
}

Tree Tree::glue(int vInThis, const Tree& other, int vInOther) {
    ensure(vInThis < n(), "Cannot glue a nonexistent vertex");
    ensure(vInOther < other.n(), "Cannot glue to a nonexistent vertex");

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

    ensure(t.n() == n() + other.n() - 1);

    return t;
}

// Tree generators go here

Tree Tree::bamboo(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    Tree t;
    for (int i = 0; i + 1 < size; ++i) {
        t.addEdge(i, i+1);
    }
    t.normalizeEdges();
    return t;
}

Tree Tree::randomPrufer(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    if (size == 1) {
        return Tree();
    }

    Array code = Array::random(size - 2, size);
    std::vector<int> degree(size, 1);
    for (int v: code) {
        ++degree[v];
    }

    std::set<int> leaves;
    for (int v = 0; v < size; ++v) {
        if (degree[v] == 1) {
            leaves.insert(v);
        }
    }

    Tree t;
    for (int v: code) {
        ENSURE(!leaves.empty());
        int to = *leaves.begin();
        leaves.erase(leaves.begin());
        if (--degree[v] == 1) {
            leaves.insert(v);
        }

        t.addEdge(v, to);
    }

    ENSURE(leaves.size() == 2u);
    t.addEdge(*leaves.begin(), *leaves.rbegin());
    t.normalizeEdges();
    return t;
}

Tree Tree::random(int size, int elongation) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    Tree t;
    for (int v = 1; v < size; ++v) {
        int parent = rnd.wnext(v, elongation);
        t.addEdge(parent, v);
    }
    t.normalizeEdges();
    return t;
}

Tree Tree::star(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    Tree t;
    for (int i = 1; i < size; ++i) {
        t.addEdge(0, i);
    }
    t.normalizeEdges();
    return t;
}

Tree Tree::caterpillar(int size, int length) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    ensure(length > 0, "Length of the caterpillar must be positive");
    checkLargeParameter(size);
    ensure(length <= size);
    Tree t = Tree::bamboo(length);
    for (int i = length; i < size; ++i) {
        t.addEdge(rnd.next(length), i);
    }
    t.normalizeEdges();
    return t;
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

using jngen::Tree;
