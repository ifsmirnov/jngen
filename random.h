#pragma once

#include "common.h"
#include "pattern.h"

#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace jngen {

static void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ensure(engine() == 822569775);
    ensure(engine() == 2137449171);
    ensure(engine() == 2671936806);
}

class Random;

class BaseTypedRandom {
public:
    BaseTypedRandom(Random& random) : random(random) {}

protected:
    Random& random;
};

template<typename T>
struct TypedRandom;

uint64_t maskForBound(uint64_t bound) {
    --bound;
    uint64_t mask = ~0;
    if ((mask >> 32) >= bound) mask >>= 32;
    if ((mask >> 16) >= bound) mask >>= 16;
    if ((mask >> 8 ) >= bound) mask >>= 8 ;
    if ((mask >> 4 ) >= bound) mask >>= 4 ;
    if ((mask >> 2 ) >= bound) mask >>= 2 ;
    if ((mask >> 1 ) >= bound) mask >>= 1 ;
    return mask;
}

template<typename Result, typename Source>
Result uniformRandom(Result bound, Random& random, Source (Random::*method)()) {
    static_assert(sizeof(Result) <= sizeof(Source),
        "uniformRandom: Source type must be at least as large as Result type");
#ifdef JNGEN_FAST_RANDOM
    return (random.*method)() % bound;
#else
    Source mask = maskForBound(bound);
    while (true) {
        Source outcome = (random.*method)() & mask;
        if (outcome < static_cast<Source>(bound)) {
            return outcome;
        }
    }
#endif
};

class Random {
public:
    Random() {
        assertRandomEngineConsistency();
        seed(std::random_device{}());
    }

    void seed(uint32_t val) {
        randomEngine_.seed(val);
    }

    void seed(const std::vector<uint32_t>& seed) {
        std::seed_seq seq(seed.begin(), seed.end());
        randomEngine_.seed(seq);
    }

    uint32_t next() {
        return randomEngine_();
    }

    uint64_t next64() {
        uint64_t a = next();
        uint64_t b = next();
        return (a << 32) ^ b;
    }

    double nextf() {
        return (double)randomEngine_() / randomEngine_.max();
    }

    int next(int n) {
        ensure(n > 0);
        return uniformRandom(n, *this, (uint32_t (Random::*)())&Random::next);
    }

    long next(long n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    long long next(long long n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    size_t next(size_t n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    double next(double n) {
        return nextf() * n;
    }

    int next(int l, int r) {
        return l + next(r-l+1);
    }

    long next(long l, long r) {
        return l + next(r-l+1);
    }

    long long next(long long l, long long r) {
        return l + next(r-l+1);
    }

    size_t next(size_t l, size_t r) {
        return l + next(r-l+1);
    }

    double next(double l, double r) {
        return l + next(r-l);
    }

    std::string next(const std::string& pattern) {
        return Pattern(pattern).next([this](int n) { return next(n); });
    }

    template<typename T, typename ... Args>
    T tnext(Args... args) {
        return TypedRandom<T>{*this}.next(args...);
    }

    template<typename ... Args>
    std::pair<int, int> nextp(Args... args) {
        return tnext<std::pair<int, int>>(args...);
    }

private:
    std::mt19937 randomEngine_;
};

Random rnd;

template<>
struct TypedRandom<int> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    int next(int n) { return random.next(n); }
    int next(int l, int r) { return random.next(l, r); }
};

template<>
struct TypedRandom<double> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    double next(double n) { return random.next(n); }
    double next(double l, double r) { return random.next(l, r); }
};

template<>
struct TypedRandom<long long> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    long long next(long long n) { return random.next(n); }
    long long next(long long l, long long r) { return random.next(l, r); }
};

template<>
struct TypedRandom<size_t> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    size_t next(size_t n) { return random.next(n); }
    size_t next(size_t l, size_t r) { return random.next(l, r); }
};

template<>
struct TypedRandom<char> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    char next(char n) { return random.next(n); }
    char next(char l, char r) { return random.next(l, r); }
};


struct OrderedPairTag {} opair;

template<>
struct TypedRandom<std::pair<int, int>> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;

    std::pair<int, int> next(int n) {
        // can't write 'return {random.next(n), random.next(n)}' because order of
        // evaluation of function arguments is unspecified.
        int first = random.next(n);
        int second = random.next(n);
        return {first, second};
    }
    std::pair<int, int> next(int l, int r) {
        int first = random.next(l, r);
        int second = random.next(l, r);
        return {first, second};
    }

    std::pair<int, int> next(int n, OrderedPairTag) {
        return ordered(next(n));
    }
    std::pair<int, int> next(int l, int r, OrderedPairTag) {
        return ordered(next(l, r));
    }

private:
    std::pair<int, int> ordered(std::pair<int, int> pair) const {
        if (pair.first > pair.second) {
            std::swap(pair.first, pair.second);
        }
        return pair;
    }
};

} // namespace jngen

using jngen::Random;

using jngen::rnd;
using jngen::opair;

void registerGen(int argc, char *argv[], int version = 1) {
    (void)version; // unused, only for testlib.h compatibility

    std::vector<uint32_t> seed;
    for (int i = 1; i < argc; ++i) {
        int startPosition = seed.size();
        seed.emplace_back();
        for (char *s = argv[i]; *s; ++s) {
            ++seed[startPosition];
            seed.push_back(*s);
        }
    }
    rnd.seed(seed);
}
