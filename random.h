#pragma once

#include "common.h"

namespace impl {

std::mt19937 randomEngine;

static void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ensure(engine() == 822569775);
    ensure(engine() == 2137449171);
    ensure(engine() == 2671936806);
}

template<typename T>
struct TypedRandom;

class Random {
public:
    Random() {
        assertRandomEngineConsistency();
        randomEngine.seed(std::random_device{}());

        static bool created = false;
        ensure(!created, "impl::Random should be created only once");
        created = true;
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

    template<typename T, typename ... Args>
    T tnext(Args... args) {
        return TypedRandom<T>::next(args...);
    }
};

Random rnd;

template<>
struct TypedRandom<int> {
    static int next(int n) { return rnd.next(n); }
    static int next(int l, int r) { return rnd.next(l, r); }
};

template<>
struct TypedRandom<double> {
    static double next(double n) { return rnd.next(n); }
    static double next(double l, double r) { return rnd.next(l, r); }
};

template<>
struct TypedRandom<long long> {
    static long long next(long long n) { return rnd.next(n); }
    static long long next(long long l, long long r) { return rnd.next(l, r); }
};

template<>
struct TypedRandom<size_t> {
    static size_t next(size_t n) { return rnd.next(n); }
    static size_t next(size_t l, size_t r) { return rnd.next(l, r); }
};

struct OrderedPairTag {} opair;

template<>
struct TypedRandom<std::pair<int, int>> {
    static std::pair<int, int> next(int n) {
        // can't write 'return {rnd.next(n), rnd.next(n)}' because order of
        // evaluation of function arguments is unspecified.
        int first = rnd.next(n);
        int second = rnd.next(n);
        return {first, second};
    }
    static std::pair<int, int> next(int l, int r) {
        int first = rnd.next(l, r);
        int second = rnd.next(l, r);
        return {first, second};
    }

    static std::pair<int, int> next(int n, OrderedPairTag) {
        return ordered(next(n));
    }
    static std::pair<int, int> next(int l, int r, OrderedPairTag) {
        return ordered(next(l, r));
    }

private:
    static std::pair<int, int> ordered(std::pair<int, int> pair) {
        if (pair.first > pair.second) {
            std::swap(pair.first, pair.second);
        }
        return pair;
    }
};

} // namespace impl

using impl::rnd;
using impl::opair;

void registerGen(int argc, char *argv[]) {
    size_t val = 0;
    for (int i = 0; i < argc; ++i) {
        for (char *s = argv[i]; *s; ++s) {
            val = val * 10099 + *s;
        }
    }
    rnd.seed(val);
}
