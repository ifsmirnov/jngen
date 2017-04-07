#pragma once

#include "array.h"
#include "common.h"
#include "math.h"

#include <algorithm>
#include <cstdio>
#include <cstdlib>

namespace jngen {

inline int getInitialTestNo() {
    char *envvar = getenv("TESTNO");
    int testno;
    if (!envvar || 1 != std::sscanf(envvar, "%d", &testno)) {
        return 1;
    }
    return testno;
}

#ifdef JNGEN_DECLARE_ONLY
extern int nextTestNo;
#else
int nextTestNo = -1;
#endif // JNGEN_DECLARE_ONLY

void startTest(int testNo);

void startTest();

void setNextTestNumber(int testNo);

Array64 randomTestSizes(
    long long totalSize,
    int count,
    long long minSize,
    long long maxSize,
    const Array64& predefined);

Array randomTestSizes(
    int totalSize,
    int count,
    int minSize,
    int maxSize,
    const Array& predefined);

#ifndef JNGEN_DECLARE_ONLY

void startTest(int testNo) {
    nextTestNo = testNo + 1;
    char filename[10];
    std::sprintf(filename, "%d", testNo);
    if (!std::freopen(filename, "w", stdout)) {
        ensure(false, format("Cannot open the file `%s'", filename));
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

Array64 randomTestSizes(
    long long totalSize,
    int count,
    long long minSize,
    long long maxSize,
    const Array64& predefined)
{
    for (auto x: predefined) {
        totalSize -= x;
    }
    ensure(totalSize >= 0, "Sum of predefined test sizes exceeds total size");
    ensure(count * minSize <= totalSize, "minSize is too large");
    ensure(minSize <= maxSize);

    auto partition = rndm.partition(totalSize - count * minSize, count)
        .sort().reverse();
    long long remaining = 0;

    long long localMax = maxSize - minSize;
    for (auto& x: partition) {
        if (x > localMax) {
            remaining += x - localMax;
            x = localMax;
        } else {
            long long add = std::min(remaining, localMax - x);
            x += add;
            remaining -= add;
        }

        x += minSize;
    }

    ensure(remaining == 0, "maxSize is too small");

    return (partition + predefined).shuffled();
}

Array randomTestSizes(
    int totalSize,
    int count,
    int minSize,
    int maxSize,
    const Array& predefined)
{
    return arrayCast<int>(randomTestSizes(
        static_cast<long long>(totalSize),
        count,
        static_cast<long long>(minSize),
        static_cast<long long>(maxSize),
        arrayCast<long long>(predefined)
    ));
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

using jngen::startTest;
using jngen::setNextTestNumber;

using jngen::randomTestSizes;
