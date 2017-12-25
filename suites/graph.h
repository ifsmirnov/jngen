#pragma once

#include "base_suite.h"
#include "../graph.h"

#include <algorithm>
#include <cstdlib>

namespace jngen {
namespace suites {

JNGEN_CHAINING_TRAITS(GeneralGraphSuiteTraits, allowLoops, allowMulti, connected)

class GeneralGraphSuite : public BaseTestSuite<Graph, GeneralGraphSuiteTraits, int, int> {
    graph_detail::BuilderProxy&& apply(graph_detail::BuilderProxy&& builder) const {
        builder.allowLoops(conf_._allowLoops);
        builder.allowMulti(conf_._allowMulti);
        builder.connected(conf_._connected);
        return std::move(builder);
    }

    int selectMForRandom(int n, int m, double ratio) const {
        if (conf_._connected) {
            return std::min(m, n + static_cast<int>((m - n + 1) * ratio));
        } else {
            return std::max(1, static_cast<int>(m * ratio));
        }
    }

    Graph randomWithRatio(int n, int m, double ratio) const {
        return apply(Graph::random(n, selectMForRandom(n, m, ratio))).g();
    }

public:
    GeneralGraphSuite() : BaseTestSuite("GeneralGraphSuite") {
#define JNGEN_PRODUCER_ARGS int n, int m

        JNGEN_ADD_PRODUCER(random0.001) {
            return randomWithRatio(n, m, 0.001);
        };

        JNGEN_ADD_PRODUCER(random0.005) {
            return randomWithRatio(n, m, 0.005);
        };

        JNGEN_ADD_PRODUCER(random0.01) {
            return randomWithRatio(n, m, 0.01);
        };

        JNGEN_ADD_PRODUCER(random0.02) {
            return randomWithRatio(n, m, 0.02);
        };

        JNGEN_ADD_PRODUCER(random0.05) {
            return randomWithRatio(n, m, 0.05);
        };

        JNGEN_ADD_PRODUCER(random0.1) {
            return randomWithRatio(n, m, 0.1);
        };

        JNGEN_ADD_PRODUCER(random0.2) {
            return randomWithRatio(n, m, 0.2);
        };

        JNGEN_ADD_PRODUCER(random0.3) {
            return randomWithRatio(n, m, 0.3);
        };

        JNGEN_ADD_PRODUCER(random0.4) {
            return randomWithRatio(n, m, 0.4);
        };

        JNGEN_ADD_PRODUCER(random0.6) {
            return randomWithRatio(n, m, 0.6);
        };

        JNGEN_ADD_PRODUCER(random0.7) {
            return randomWithRatio(n, m, 0.7);
        };

        JNGEN_ADD_PRODUCER(random0.9) {
            return randomWithRatio(n, m, 0.9);
        };

        JNGEN_ADD_PRODUCER(random1) {
            return randomWithRatio(n, m, 1);
        };

        JNGEN_ADD_PRODUCER(cycle) {
            (void)m;
            return Graph::cycle(n);
        };

        JNGEN_ADD_PRODUCER(complete) {
            int size = 1;
            while (size <= n && size * (size - 1) / 2 +
                    (conf_._allowLoops ? size : 0) <= m) {
                ++size;
            }
            return apply(Graph::complete(size - 1));
        };

#undef JNGEN_PRODUCER_ARGS
    }
};

} // namespace test_suites
} // namespace jngen
