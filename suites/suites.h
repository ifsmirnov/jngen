#pragma once

#include "tree.h"

namespace jngen {

struct TestSuites {
    suites::GeneralTreeSuite tree;
};

JNGEN_EXTERN TestSuites testSuites;

} // namespace jngen

using jngen::testSuites;
