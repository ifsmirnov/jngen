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

void assertRandomEngineConsistency();
void assertIntegerSizes();
void registerGen(int argc, char *argv[], int version = 1);

class Random;

class BaseTypedRandom {
public:
    BaseTypedRandom(Random& random) : random(random) {}

protected:
    Random& random;
};

template<typename T>
struct TypedRandom;

uint64_t maskForBound(uint64_t bound);

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

    void seed(uint32_t val);
    void seed(const std::vector<uint32_t>& seed);

    uint32_t next();
    uint64_t next64();
    double nextf();

    int next(int n);
    long long next(long long n);
    size_t next(size_t n);
    double next(double n);

    int next(int l, int r);
    long long next(long long l, long long r);
    size_t next(size_t l, size_t r);
    double next(double l, double r);

    int wnext(int n, int w);
    long long wnext(long long n, int w);
    size_t wnext(size_t n, int w);
    double wnext(double n, int w);

    int wnext(int l, int r, int w);
    long long wnext(long long l, long long r, int w);
    size_t wnext(size_t l, size_t r, int w);
    double wnext(double l, double r, int w);

    std::string next(const std::string& pattern);

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
    auto choice(Iterator begin, Iterator end)
            -> typename std::iterator_traits<Iterator>::value_type
    {
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
        int first = random.next(n);
        int second;
        do {
            second = random.next(n);
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

using jngen::registerGen;

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_RANDOM_INL_H
#include "impl/random_inl.h"
#undef JNGEN_INCLUDE_RANDOM_INL_H
#endif // JNGEN_DECLARE_ONLY
