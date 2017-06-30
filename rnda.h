#pragma once

#include "array.h"
#include "common.h"
#include "gcc_primes_list.h"
#include "math.h"

#include <algorithm>
#include <cstdlib>

namespace jngen {

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

    static Array64 antiUnorderedSetGcc4(int n, double maxLoadFactor = 1.0);

    static Array64 antiUnorderedSetWithReserveGcc4(
            int n, double maxLoadFactor = 1.0);

private:
    static Array64 numbersDividingPrime(int n, long long p);

    static long long nextPrimeGcc4(unsigned long long x);
};

JNGEN_EXTERN ArrayRandom rnda;

#ifndef JNGEN_DECLARE_ONLY

Array64 ArrayRandom::antiUnorderedSetGcc4(int n, double maxLoadFactor) {
    int buckets = 2;
    for (int size = 1; size <= n; ++size) {
        if (size + 1 > buckets * maxLoadFactor) {
            buckets = nextPrimeGcc4(buckets * 2);
        }
    }

    return numbersDividingPrime(n, buckets);
}

Array64 ArrayRandom::antiUnorderedSetWithReserveGcc4(
        int n, double maxLoadFactor)
{
    int buckets = nextPrimeGcc4(std::ceil(n / maxLoadFactor));
    return numbersDividingPrime(n, buckets);
}

Array64 ArrayRandom::numbersDividingPrime(int n, long long p) {
    auto a = Array64::id(n);
    for (auto& x: a) {
        x *= p;
    }
    return a;
}

long long ArrayRandom::nextPrimeGcc4(unsigned long long x) {
    const static size_t SIZE =
        sizeof(impl::primeList) / sizeof(impl::primeList[0]);
    return *std::lower_bound(impl::primeList, impl::primeList + SIZE, x);
}

#endif // JNGEN_DECLARE_ONLY

} // namespace jngen

using jngen::rnda;
