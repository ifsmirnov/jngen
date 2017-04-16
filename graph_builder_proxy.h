#pragma once

#include <functional>

namespace jngen {

class Graph;

namespace graph_detail {

class GraphRandom;

struct Traits {
    int n;
    int m;
    bool directed = false;
    bool allowLoops = false;
    bool allowMulti = false;
    bool connected = false;

    Traits() {}
    explicit Traits(int n) : n(n) {}
    Traits(int n, int m) : n(n), m(m) {}
};

class BuilderProxy {
public:
    BuilderProxy(
            Traits traits,
            std::function<Graph(Traits)> builder) :
        traits_(traits),
        builder_(builder)
    {  }

    Graph g() const;

    operator Graph() const;

    BuilderProxy& allowLoops(bool value = true) {
        traits_.allowLoops = value;
        return *this;
    }

    BuilderProxy& allowMulti(bool value = true) {
        traits_.allowMulti = value;
        return *this;
    }

    BuilderProxy& connected(bool value = true) {
        traits_.connected = value;
        return *this;
    }

private:
    Traits traits_;
    std::function<Graph(Traits)> builder_;
};

} // namespace graph_detail

} // namespace jngen
