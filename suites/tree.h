#pragma once

#include "base_suite.h"
#include "../tree.h"

#include <cstdlib>

namespace jngen {
namespace suites {

class GeneralTreeSuite : public BaseTestSuite<Tree, int> {
public:
    GeneralTreeSuite() : BaseTestSuite("GeneralTreeSuite") {
#define JNGEN_PRODUCER_ARGS int n

        JNGEN_ADD_PRODUCER(random1) {
            return Tree::random(n);
        };

        JNGEN_ADD_PRODUCER(random2) {
            return Tree::random(n);
        };

        JNGEN_ADD_PRODUCER(random3) {
            return Tree::random(n);
        };

        JNGEN_ADD_PRODUCER(bamboo) {
            return Tree::bamboo(n);
        };

        JNGEN_ADD_PRODUCER(shuffled_bamboo) {
            return Tree::bamboo(n).shuffled();
        };

        JNGEN_ADD_PRODUCER(3branches) {
            int k = (n - 1) / 3 + 1;

            Tree t = Tree::bamboo(k);
            t = t.glue(0, Tree::bamboo(k), 0);
            t = t.glue(0, Tree::bamboo(k), 0);

            ENSURE(t.n() <= n);

            return t;
        };

        JNGEN_ADD_PRODUCER(sqrt_branches) {
            int k = std::sqrt(n) + 1;
            Tree t = Tree::bamboo(k);
            while (t.n() + k - 1 <= n) {
                t = t.glue(0, Tree::bamboo(k), 0);
            }
            ENSURE(t.n() <= n);
            return t;
        };

        JNGEN_ADD_PRODUCER(branches_123) {
            Tree t = Tree::bamboo(2);
            for (int i = 2; t.n() + i <= n; ++i) {
                t = t.link(0, Tree::bamboo(i), 0);
            }
            ENSURE(t.n() < n);
            return t;
        };

        JNGEN_ADD_PRODUCER(binary) {
            return Tree::binary(n);
        };

        JNGEN_ADD_PRODUCER(3ary) {
            return Tree::kary(n, 3);
        };

        JNGEN_ADD_PRODUCER(4ary) {
            return Tree::kary(n, 4);
        };

        JNGEN_ADD_PRODUCER(50ary) {
            return Tree::kary(n, 50);
        };

        JNGEN_ADD_PRODUCER(500ary) {
            return Tree::kary(n, 500);
        };

        JNGEN_ADD_PRODUCER(star) {
            return Tree::star(n);
        };

        JNGEN_ADD_PRODUCER(shuffled_star) {
            return Tree::star(n).shuffled();
        };

        JNGEN_ADD_PRODUCER(caterpillar_len90) {
            return Tree::caterpillar(n, n * 0.9);
        };

        JNGEN_ADD_PRODUCER(caterpillar_len50) {
            return Tree::caterpillar(n, n * 0.5);
        };

        JNGEN_ADD_PRODUCER(caterpillar_len10) {
            return Tree::caterpillar(n, n * 0.1);
        };

        JNGEN_ADD_PRODUCER(broom_n/2) {
            auto t1 = Tree::bamboo(n/2);
            auto t2 = Tree::star(n - n/2);
            return t1.link(n/2 - 1, t2, 0);
        };

        JNGEN_ADD_PRODUCER(random_w-100) {
            return Tree::randomPrim(n, -100);
        };

        JNGEN_ADD_PRODUCER(random_w-50) {
            return Tree::randomPrim(n, -50);
        };

        JNGEN_ADD_PRODUCER(random_w-10) {
            return Tree::randomPrim(n, -10);
        };

        JNGEN_ADD_PRODUCER(random_w-5) {
            return Tree::randomPrim(n, -5);
        };

        JNGEN_ADD_PRODUCER(random_w0) {
            return Tree::randomPrim(n, 0);
        };

        JNGEN_ADD_PRODUCER(random_w5) {
            return Tree::randomPrim(n, 5);
        };

        JNGEN_ADD_PRODUCER(random_w10) {
            return Tree::randomPrim(n, 10);
        };

        JNGEN_ADD_PRODUCER(random_w50) {
            return Tree::randomPrim(n, 50);
        };

        JNGEN_ADD_PRODUCER(random_w100) {
            return Tree::randomPrim(n, 100);
        };

#undef JNGEN_PRODUCER_ARGS
    }
};

} // namespace test_suites
} // namespace jngen
