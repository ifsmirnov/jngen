#pragma once

#include "common.h"

#include <bits/stdc++.h>

namespace impl {

int getInitialTestNo() {
    char *envvar = getenv("TESTNO");
    int testno;
    if (!envvar || 1 != sscanf(envvar, "%d", &testno)) {
        return 1;
    }
    return testno;
}

int nextTestNo = -1;

void startTest(int testNo) {
    nextTestNo = testNo + 1;
    char filename[10];
    sprintf(filename, "%d", testNo);
    if (!freopen(filename, "w", stdout)) {
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

} // namespace impl

using impl::startTest;
using impl::setNextTestNumber;
