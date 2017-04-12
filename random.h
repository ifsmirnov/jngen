#pragma once

#include "common.h"
#include "pattern.h"

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace jngen {

static void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ENSURE(engine() == 822569775,
        "std::mt19937 doesn't conform to the C++ standard");
    ENSURE(engine() == 2137449171,
        "std::mt19937 doesn't conform to the C++ standard");
    ENSURE(engine() == 2671936806,
        "std::mt19937 doesn't conform to the C++ standard");
}

static void assertIntegerSizes() {
    static_assert(
        std::numeric_limits<unsigned char>::max() == 255,
        "max(unsigned char) != 255");
    static_assert(sizeof(int) == 4, "sizeof(int) != 4");
    static_assert(sizeof(long long) == 8, "sizeof(int) != 8");
    static_assert(
        sizeof(size_t) == 4 || sizeof(size_t) == 8,
        "sizeof(size_t) is neither 4 nor 8");
    static_assert(
        sizeof(std::size_t) == sizeof(size_t),
        "sizeof(size_t) != sizeof(std::size_t)");
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

inline uint64_t maskForBound(uint64_t bound) {
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
}

class Random {
public:
    Random() {
        assertRandomEngineConsistency();
        assertIntegerSizes();
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

    long long next(long long n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    size_t next(size_t n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    double next(double n) {
        ensure(n >= 0);
        return nextf() * n;
    }

    int next(int l, int r) {
        ensure(l <= r);
        uint32_t n = static_cast<uint32_t>(r) - l + 1;
        return l + uniformRandom(
            n, *this, (uint32_t (Random::*)())&Random::next);
    }

    long long next(long long l, long long r) {
        ensure(l <= r);
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + uniformRandom(n, *this, &Random::next64);
    }

    size_t next(size_t l, size_t r) {
        ensure(l <= r);
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + uniformRandom(n, *this, &Random::next64);
    }

    double next(double l, double r) {
        ensure(l <= r);
        return l + next(r-l);
    }

    //  implemented in random_inl.h
    int wnext(int n, int w);
    long long wnext(long long n, int w);
    size_t wnext(size_t n, int w);
    double wnext(double n, int w);

    int wnext(int l, int r, int w);
    long long wnext(long long l, long long r, int w);
    size_t wnext(size_t l, size_t r, int w);
    double wnext(double l, double r, int w);

    std::string next(const std::string& pattern) {
        return Pattern(pattern).next([this](int n) { return next(n); });
    }

    template<typename ... Args>
    std::string next(const std::string& pattern, Args... args) {
        return next(format(pattern, args...));
    }

    template<typename T, typename ... Args>
    T tnext(Args... args) {
        return TypedRandom<T>{*this}.next(args...);
    }

    template<typename ... Args>
    std::pair<int, int> nextp(Args... args) {
        return tnext<std::pair<int, int>>(args...);
    }

    template<typename Iterator>
    typename Iterator::value_type choice(Iterator begin, Iterator end) {
        auto length = std::distance(begin, end);
        ensure(length > 0, "Cannot select from a range of negative length");
        size_t index = tnext<size_t>(length);
        std::advance(begin, index);
        return *begin;
    }

    template<typename Container>
    typename Container::value_type choice(const Container& container) {
        ensure(!container.empty(), "Cannot select from an empty container");
        return choice(container.begin(), container.end());
    }

private:
    template<typename T, typename ...Args>
    T smallWnext(int w, Args... args) {
        ENSURE(std::abs(w) <= WNEXT_LIMIT);
        T result = next(args...);
        while (w > 0) {
            result = std::max(result, next(args...));
            --w;
        }
        while (w < 0) {
            result = std::min(result, next(args...));
            ++w;
        }
        return result;
    }

    double realWnext(int w) {
        if (w == 0) {
            return nextf();
        } else if (w > 0) {
            return std::pow(nextf(), 1.0 / (w + 1));
        } else {
            return 1.0 - std::pow(nextf(), 1.0 / (-w + 1));
        }
    }

    std::mt19937 randomEngine_;
    constexpr static int WNEXT_LIMIT = 8;
};

JNGEN_EXTERN Random rnd;

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

template<typename T>
struct TypedRandom : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    template<typename ... Args>
    T next(Args... args) { return random.next(args...); }
};

struct RandomPairTraits {
    const bool ordered;
    const bool distinct;
};

#ifdef JNGEN_DECLARE_ONLY
extern RandomPairTraits opair, dpair, odpair, dopair;
#else
RandomPairTraits opair{true, false};
RandomPairTraits dpair{false, true};
RandomPairTraits odpair{true, true};
RandomPairTraits dopair{true, true};
#endif

template<>
struct TypedRandom<std::pair<int, int>> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;

    std::pair<int, int> next(int n) {
        return next(n, {false, false});
    }
    std::pair<int, int> next(int l, int r) {
        return next(l, r, {false, false});
    }

    std::pair<int, int> next(int n, RandomPairTraits traits) {
        int first = rnd.next(n);
        int second;
        do {
            second = rnd.next(n);
        } while (traits.distinct && first == second);
        if (traits.ordered && first > second) {
            std::swap(first, second);
        }
        return {first, second};
    }
    std::pair<int, int> next(int l, int r, RandomPairTraits traits) {
        auto res = next(r-l+1, traits);
        res.first += l;
        res.second += l;
        return res;
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
using jngen::dpair;
using jngen::dopair;
using jngen::odpair;

inline void registerGen(int argc, char *argv[], int version = 1) {
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

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_RANDOM_INL_H
#include "random_inl.h"
#undef JNGEN_INCLUDE_RANDOM_INL_H
#endif // JNGEN_DECLARE_ONLY
