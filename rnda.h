#pragma once

#include "array.h"
#include "common.h"
#include "gcc_primes_list.h"
#include "math_jngen.h"

#include <algorithm>
#include <cstdlib>

namespace jngen {

enum class UnorderedSetCompiler {
    Gcc4,
    Gcc5or6,
    Clang
};

class ArrayRandom {
public:
    ArrayRandom() {
        static bool created = false;
        ENSURE(!created, "jngen::ArrayRandom should be created only once");
        created = true;
    }

    template<typename F, typename ...Args>
    static auto randomf(
            size_t size,
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomf(size, func, args...);
    }

    template<typename F, typename ...Args>
    static auto randomfUnique(
            size_t size,
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomfUnique(size, func, args...);
    }

    template<typename F, typename ...Args>
    static auto randomfAll(
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomfAll(func, args...);
    }

    static Array64 antiUnorderedSet(
        int n,
        double maxLoadFactor = 1.0,
        bool reserve = false,
        UnorderedSetCompiler compiler = UnorderedSetCompiler::Gcc4);

private:
    static Array64 numbersDividingPrime(int n, long long p);

    static long long nextPrime(
        unsigned long long x,
        UnorderedSetCompiler compiler);
};

JNGEN_EXTERN ArrayRandom rnda;

#ifndef JNGEN_DECLARE_ONLY

Array64 ArrayRandom::antiUnorderedSet(
    int n,
    double maxLoadFactor,
    bool reserve,
    UnorderedSetCompiler compiler)
{
    ensure(
        compiler == UnorderedSetCompiler::Gcc4,
        "unordered set antitest supported only for gcc-4.x yet");

    ensure(
        n <= 1000000,
        "unordered set antitest supported only for n <= 1e7");

    int buckets;

    if (reserve) {
        buckets = nextPrime(std::ceil(n / maxLoadFactor), compiler);
    } else {
        buckets = 2;
        for (int size = 1; size <= n; ++size) {
            if (size + 1 > buckets * maxLoadFactor) {
                buckets = nextPrime(buckets * 2, compiler);
            }
        }
    }

    return numbersDividingPrime(n, buckets);
}

Array64 ArrayRandom::numbersDividingPrime(int n, long long p) {
    auto a = Array64::id(n);
    for (auto& x: a) {
        x *= p;
    }
    return a;
}

long long ArrayRandom::nextPrime(
    unsigned long long x,
    UnorderedSetCompiler compiler)
{
    ENSURE(compiler == UnorderedSetCompiler::Gcc4);

    const static size_t SIZE =
        sizeof(impl::primeList) / sizeof(impl::primeList[0]);
    return *std::lower_bound(impl::primeList, impl::primeList + SIZE, x);
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

using jngen::rnda;
using jngen::UnorderedSetCompiler;
