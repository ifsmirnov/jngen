#pragma once

#include "graph.h"
#include "tree.h"

namespace jngen {

struct TestSuites {
    suites::GeneralGraphSuite graph;
    suites::GeneralTreeSuite tree;
};

JNGEN_EXTERN TestSuites testSuites;

} // namespace jngen

using jngen::testSuites;
