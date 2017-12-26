#ifndef JNGEN_INCLUDE_TREE_INL_H
#error File "tree_inl.h" must not be included directly.
#include "../tree.h" // for completion emgine
#endif

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

bool Tree::canAddEdge(int u, int v) {
    u = vertexByLabel(u);
    v = vertexByLabel(v);
    return dsu_.getRoot(u) != dsu_.getRoot(v);
}

Array Tree::parents(int root) const {
    ensure(isConnected(), "Tree::parents(int): Tree is not connected");
    root = vertexByLabel(root);

    Array parents(n());
    parents[root] = -1;
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
        if (x != -1) {
            x = vertexLabel(x);
        }
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

inline Tree& Tree::shuffleAllBut(const Array& except) {
    doShuffleAllBut(except);
    return *this;
}

inline Tree Tree::shuffledAllBut(const Array& except) const {
    Tree g(*this);
    return g.shuffleAllBut(except);
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

Tree Tree::random(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    if (size == 1) {
        return Tree();
    }
    return fromPruferSequence(Array::random(size - 2, size));
}

Tree Tree::randomPrim(int size, int elongation) {
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

Tree Tree::randomKruskal(int size) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);
    Tree t;
    t.extend(size);
    while (!t.isConnected()) {
        auto e = rnd.nextp(size, dpair);
        if (t.canAddEdge(e.first, e.second)) {
            t.addEdge(e.first, e.second);
        }
    }
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

Tree Tree::binary(int size) {
    return kary(size, 2);
}

Tree Tree::kary(int size, int k) {
    ensure(size > 0, "Number of vertices in the tree must be positive");
    checkLargeParameter(size);

    Tree t;
    for (int i = 1; i < size; ++i) {
        t.addEdge((i - 1) / k, i);
    }
    t.normalizeEdges();
    return t;
}

Tree Tree::fromPruferSequence(const Array& code) {
    std::vector<int> degree(code.size() + 2, 1);
    for (int v: code) {
        ++degree[v];
    }

    std::set<int> leaves;
    for (size_t v = 0; v != degree.size(); ++v) {
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

void Tree::doPrintParents(std::ostream& out, const OutputModifier& mod) const {
    int root = mod.printParents;
    if (root == -1) {
        root = 0;
    }

    auto parents = this->parents(root);
    if (mod.printParents == -1) {
        parents.erase(parents.begin());
    }

    if (mod.printN) {
        out << n() << "\n";
    }

    // TODO: avoid copy-paste from doPrintEdges
    if (mod.printWeights && vertexWeights_.hasNonEmpty()) {
        auto vertexWeights = prepareWeightArray(vertexWeights_, n());
        for (int i = 0; i < n(); ++i) {
            if (i > 0) {
                out << " ";
            }
            JNGEN_PRINT_NO_MOD(vertexWeights[vertexByLabel(i)]);
        }
        out << "\n";
    }

    auto t(mod);
    {
        auto mod(t);
        mod.printN = false;

        if (mod.printWeights && edgeWeights_.hasNonEmpty()) {
            ensure(false, "Printing parents and edge weights is not supported");
            ensure(
                mod.printParents == -1,
                "Root must not be set to any exact value when printing a tree "
                "with edge weights. To fix it, either set printParents() "
                "or printWeights(false)");
            ENSURE(root == 0);
            // TODO: some code to be here
        } else {
            JNGEN_PRINT(parents);
        }
    }
}

