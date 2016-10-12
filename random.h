#pragma once

#include "common.h"

namespace impl {

std::mt19937 randomEngine;

__attribute__((constructor))
static void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ensure(engine() == 822569775);
    ensure(engine() == 2137449171);
    ensure(engine() == 2671936806);
}

class Random {
public:
    Random() {
        randomEngine.seed(std::random_device{}());
    }

    void seed(size_t val) {
        randomEngine.seed(val);
    }

    uint32_t next() {
        return randomEngine();
    }

    int next(int n) {
        // TODO(ifsmirnov): make random more uniform
        return randomEngine() % n;
    }

    long long next(long long n) {
        // TODO(ifsmirnov): make random more uniform
        return ((randomEngine() << 32) ^ randomEngine()) % n;
    }

    size_t next(size_t n) {
        // TODO(ifsmirnov): make random more uniform
        return ((randomEngine() << 32) ^ randomEngine()) % n;
    }

    double next(double n) {
        return (double)randomEngine() / randomEngine.max() * n;
    }

    int next(int l, int r) {
        return l + next(r-l+1);
    }

    long long next(long long l, long long r) {
        return r + next(r-l+1);
    }

    size_t next(size_t l, size_t r) {
        return l + next(r-l+1);
    }

    double next(double l, double r) {
        return l + next(r-l);
    }
};

} // namespace impl

impl::Random rnd;

void registerGen(int argc, char *argv[]) {
    size_t val = 0;
    for (int i = 0; i < argc; ++i) {
        for (char *s = argv[i]; *s; ++s) {
            val = val * 10099 + *s;
        }
    }
    rnd.seed(val);
}
