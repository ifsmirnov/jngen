#pragma once

#include <bits/stdc++.h>

#include "array.h"
#include "repr.h"

namespace impl {

class Dsu {
public:
    int getParent(int x) {
        extend(x);

        return parent[x] == x ? x : (parent[x] = getParent(parent[x]));
    }

    bool link(int x, int y) {
        extend(std::max(x, y));

        x = parent[x];
        y = parent[y];
        if (x == y) {
            return false;
        }

        if (rank[x] > rank[y]) {
            std::swap(x, y);
        }
        if (rank[y] == rank[x]) {
            ++rank[y];
        }
        parent[x] = y;

        --components;

        return true;
    }

    bool connected() const { return components == 1; }

private:
    std::vector<int> parent;
    std::vector<int> rank;

    int components = 0;

    void extend(size_t x) {
        size_t last = parent.size() - 1;
        while (parent.size() <= x) {
            ++components;
            parent.push_back(++last);
            rank.push_back(0);
        }
    }
};

class Tree : public Repr<Tree> {
public:
    void addEdge(int u, int v);

    void setRoot(int v) { root_ = v; }
    int root() const { return root_; };

    int n() const { return adjList_.size(); }

    bool connected() const { return dsu_.connected(); }

    Tree& shuffle();
    Tree shuffled() const;

    int vertexLabel(size_t v) const {
        return v < vertexLabel_.size() ? vertexLabel_[v] : v;
    }

    int vertexByLabel(size_t v) const {
        return v < vertexByLabel_.size() ? vertexByLabel_[v] : v;
    }

    const std::vector<int>& edges(int v) const { return adjList_[v]; }

private:
    std::vector<std::vector<int>> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;

    Dsu dsu_;

    int root_ = 0;
};

inline void Tree::addEdge(int u, int v) {
    adjList_[u].push_back(v);
    adjList_[v].push_back(u);

    ensure(dsu_.link(u, v), "A cycle appeared in the tree :(");
}

inline Tree& Tree::shuffle() {
    if (vertexLabel_.size() < static_cast<size_t>(n())) {
        vertexLabel_ = Array::id(n());
    }
    vertexLabel_.shuffle();
    vertexByLabel_ = vertexLabel_.inverse();
    return *this;
}

inline Tree Tree::shuffled() const {
    Tree t = *this;
    return t.shuffle();
}

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 0) {
    ensure(t.connected(), "Tree is not connected :(");

    if (mod.printN) {
        out << t.n() << "\n";
    }

    if (mod.printParents) {
        out << "Printing parents is not supported yet";
    } else if (mod.printEdges) {
        /*
        int count = 0;
        for (int v = 0; v < t.n(); ++v) {
            for (int u: t.edges(v)) {
                if (v < u) {
                    printValue(
                }
            }
        }
        */
    } else {
        ensure(false, "Print mode is unknown");
    }
}

} // namespace impl
