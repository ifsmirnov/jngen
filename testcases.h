#pragma once

#include "common.h"

#include <cstdio>
#include <cstdlib>

namespace jngen {

int getInitialTestNo() {
    char *envvar = getenv("TESTNO");
    int testno;
    if (!envvar || 1 != std::sscanf(envvar, "%d", &testno)) {
        return 1;
    }
    return testno;
}

int nextTestNo = -1;

void startTest(int testNo) {
    nextTestNo = testNo + 1;
    char filename[10];
    std::sprintf(filename, "%d", testNo);
    if (!std::freopen(filename, "w", stdout)) {
        ensure(false, "Cannot open the file");
    }
}

void startTest() {
    if (nextTestNo == -1) {
        nextTestNo = getInitialTestNo();
    }

    startTest(nextTestNo);
}

void setNextTestNumber(int testNo) {
    nextTestNo = testNo;
}

} // namespace jngen

using jngen::startTest;
using jngen::setNextTestNumber;
