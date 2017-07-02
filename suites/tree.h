#pragma once

#include "test_suite.h"
#include "../tree.h"

namespace jngen {
namespace suites {

class GeneralTreeSuite : public BaseTestSuite<Tree, int> {
public:
    GeneralTreeSuite() : BaseTestSuite("GeneralTreeSuite") {  }

private:
    void populate() override {
#define ADD_PRODUCER() *std::back_inserter(producers_) = [](int n)

        // 0
        ADD_PRODUCER() {
            return Tree::bamboo(n);
        };

        ADD_PRODUCER() {
            return Tree::randomPrufer(n);
        };

        ADD_PRODUCER() {
            return Tree::random(n);
        };

        ADD_PRODUCER() {
            return Tree::random(n, 2);
        };

        ADD_PRODUCER() {
            return Tree::random(n, 20);
        };

        // 5
        ADD_PRODUCER() {
            return Tree::random(n, 200);
        };

        ADD_PRODUCER() {
            return Tree::random(n, -2);
        };

        ADD_PRODUCER() {
            return Tree::star(n);
        };

        ADD_PRODUCER() {
            if (n < 3) {
                throw 1;
            }
            return Tree::caterpillar(n, n/2);
        };

#undef ADD_PRODUCER
    }
};

} // namespace test_suites
} // namespace jngen
