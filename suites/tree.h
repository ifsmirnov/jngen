#pragma once

#include "base_suite.h"
#include "../tree.h"

namespace jngen {
namespace suites {

class GeneralTreeSuite : public BaseTestSuite<Tree, int> {
public:
    GeneralTreeSuite() : BaseTestSuite("GeneralTreeSuite") {
#define JNGEN_PRODUCER_ARGS int n

        // 0
        JNGEN_ADD_PRODUCER(bamboo) {
            return Tree::bamboo(n);
        };

        JNGEN_ADD_PRODUCER() {
            return Tree::randomPrufer(n);
        };

        JNGEN_ADD_PRODUCER( ) {
            return Tree::random(n);
        };

        JNGEN_ADD_PRODUCER() {
            return Tree::random(n, 2);
        };

        JNGEN_ADD_PRODUCER( zloy    los) {
            return Tree::random(n, 20);
        };

        // 5
        JNGEN_ADD_PRODUCER(zloy los   ,) {
            return Tree::random(n, 200);
        };

        JNGEN_ADD_PRODUCER(zloy loewrs) {
            return Tree::random(n, -2);
        };

        JNGEN_ADD_PRODUCER() {
            return Tree::star(n);
        };

        JNGEN_ADD_PRODUCER() {
            if (n < 3) {
                throw 1;
            }
            return Tree::caterpillar(n, n/2);
        };

#undef JNGEN_PRODUCER_ARGS
    }
};

} // namespace test_suites
} // namespace jngen
