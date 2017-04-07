#pragma once

#include <algorithm>
#include <vector>

namespace jngen {

class Dsu {
public:
    int getParent(int x);

    bool link(int x, int y);

    bool isConnected() const { return components <= 1; }

private:
    std::vector<int> parent;
    std::vector<int> rank;

    int components = 0;

    void extend(size_t x);
};

#ifndef JNGEN_DECLARE_ONLY

int Dsu::getParent(int x) {
    extend(x);

    return parent[x] == x ? x : (parent[x] = getParent(parent[x]));
}

bool Dsu::link(int x, int y) {
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

void Dsu::extend(size_t x) {
    size_t last = parent.size() - 1;
    while (parent.size() <= x) {
        ++components;
        parent.push_back(++last);
        rank.push_back(0);
    }
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

