#pragma once

#include <algorithm>
#include <vector>

namespace jngen {

class Dsu {
public:
    int getRoot(int x);

    bool unite(int x, int y);

    bool isConnected() const { return components <= 1; }

    int numComponents() const { return components; }

    void extend(size_t size);

private:
    std::vector<int> parent;
    std::vector<int> rank;

    int components = 0;
};

#ifndef JNGEN_DECLARE_ONLY

int Dsu::getRoot(int x) {
    extend(x);

    return parent[x] == x ? x : (parent[x] = getRoot(parent[x]));
}

bool Dsu::unite(int x, int y) {
    extend(std::max(x, y) + 1);

    x = getRoot(x);
    y = getRoot(y);
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

void Dsu::extend(size_t x) {
    size_t last = parent.size() - 1;
    while (parent.size() < x) {
        ++components;
        parent.push_back(++last);
        rank.push_back(0);
    }
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen
