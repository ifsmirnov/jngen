#pragma once

// This header is named 'math_jngen.h' and not 'math.h' because in the latter
// case it will replace the standard 'math.h' if you set jngen folder as the
// include path.

#include "array.h"
#include "common.h"
#include "random.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <type_traits>
#include <unordered_set>
#include <vector>

namespace jngen {

namespace detail {

inline int multiply(int x, int y, int mod) {
    return static_cast<long long>(x) * y % mod;
}

inline long long multiply(long long x, long long y, long long mod) {
#if defined(__SIZEOF_INT128__)
    return static_cast<__int128>(x) * y % mod;
#else
    long long res = 0;
    while (y) {
        if (y&1) {
            res = (static_cast<unsigned long long>(res) + x) % mod;
        }
        x = (static_cast<unsigned long long>(x) + x) % mod;
        y >>= 1;
    }
    return res;
#endif
}

inline int power(int x, int k, int mod) {
    int res = 1;
    while (k) {
        if (k&1) {
            res = multiply(res, x, mod);
        }
        x = multiply(x, x, mod);
        k >>= 1;
    }
    return res;
}

inline long long power(long long x, long long k, long long mod) {
    long long res = 1;
    while (k) {
        if (k&1) {
            res = multiply(res, x, mod);
        }
        x = multiply(x, x, mod);
        k >>= 1;
    }
    return res;
}

template<typename I>
bool millerRabinTest(I n, const std::vector<I>& witnesses) {
    static_assert(
        std::is_same<I, int>::value || std::is_same<I, long long>::value,
        "millerRabinTest<int/long long> only is supported");

    if (n == 1) {
        return false;
    }

    constexpr int LIMIT = 10000;

    if (n <= LIMIT) {
        for (int i = 2; i*i <= n; ++i) {
            if (n%i == 0) {
                return false;
            }
        }
        return true;
    }

    int r = 0;
    I d = n - 1;
    while (d % 2 == 0) {
        ++r;
        d /= 2;
    }

    for (I a: witnesses) {
        I x = power(a, d, n);
        if (x == 1 || x == n - 1) {
            continue;
        }

        bool composite = true;
        for (int i = 0; i < r - 1; ++i) {
            x = multiply(x, x, n);
            if (x == 1) {
                return false;
            }
            if (x == n - 1) {
                i = r;
                composite = false;
                continue;
            }
        }
        if (composite) {
            return false;
        }
    }
    return true;
}

} // namespace detail

inline bool isPrime(long long n) {
    const static std::vector<int> INT_WITNESSES{2, 7, 61};
    const static std::vector<long long> LONG_LONG_WITNESSES
        {2, 3, 5, 7, 11, 13, 17, 19, 23};
    // todo: experiment with base
    // 2, 325, 9375, 28178, 450775, 9780504, and 1795265022
    // (guaranteed for all integers < 2^64)

    // first strong pseudoprime to i64 bases is 3825123056546413051 ~= 3.8e18
    ensure(n > 0, "isPrime() is undefined for negative numbers");
    ensure(
        n <= static_cast<long long>(3.8e18),
        "isPrime() supports only numbers not greater than 3.8 * 10^18");

    if (n < std::numeric_limits<int>::max()) {
        return detail::millerRabinTest<int>(n, INT_WITNESSES);
    } else {
        return detail::millerRabinTest<long long>(n, LONG_LONG_WITNESSES);
    }
}

class MathRandom {
public:
    MathRandom() {
        static bool created = false;
        ensure(!created, "jngen::MathRandom should be created only once");
        created = true;
    }

    static long long randomPrime(long long n) {
        ensure(n > 2, format("There are no primes below %lld", n));
        return randomPrime(2, n - 1);
    }

    static long long randomPrime(long long l, long long r) {
        ensure(l <= r);
        std::unordered_set<long long> used;
        while (static_cast<long long>(used.size()) < r - l + 1) {
            long long x = rnd.next(l, r);
            if (used.count(x)) {
                continue;
            }
            used.insert(x);
            if (isPrime(x)) {
                return x;
            }
        }
        ensure(
            false,
            format(
                "There are no primes between %lld and %lld",
                l, r)
        );
    }

    static long long nextPrime(long long n) {
        while (!isPrime(n)) {
            ++n;
        }
        return n;
    }

    static long long previousPrime(long long n) {
        ensure(n >= 2, format("There are no primes less or equal to %lld", n));
        while (!isPrime(n)) {
            --n;
        }
        return n;
    }

    static Array partition(
            int n,
            int numParts,
            int minSize = 0,
            int maxSize = -1)
    {
        auto res = partition(
            static_cast<long long>(n),
            numParts,
            static_cast<long long>(minSize),
            static_cast<long long>(maxSize));
        return Array(res.begin(), res.end());
    }

    static Array64 partition(
            long long n,
            int numParts,
            long long minSize = 0,
            long long maxSize = -1)
    {
        if (maxSize == -1) {
            maxSize = n;
        }

        ensure(n >= 0);
        ensure(numParts >= 0);
        ensure(numParts * minSize <= n, "minSize is too large");
        ensure(numParts * maxSize >= n, "maxSize is too small");
        ensure(minSize <= maxSize);

        n -= minSize * numParts;

        auto delimiters = Array64::random(
                numParts - 1, 0, n).sorted();
        delimiters.insert(delimiters.begin(), 0);
        delimiters.push_back(n);

        Array64 partition(numParts);
        for (long long i = 0; i < numParts; ++i) {
            partition[i] = delimiters[i + 1] - delimiters[i];
        }
        partition.sort().reverse();

        long long remaining = 0;

        long long localMax = maxSize - minSize;
        for (auto& x: partition) {
            if (x > localMax) {
                remaining += x - localMax;
                x = localMax;
            }

            x += minSize;
        }

        // Here we try to distribute the remaining part in some even manner
        // between remaining slots. Looks like crap anyway, need a smarter way.

        for (int divisor: { 2, 1 }) {
            partition.shuffle();
            for (auto& x: partition) {
                if (x < maxSize) {
                    long long add = std::min(
                            remaining, (maxSize - x) / divisor);
                    x += add;
                    remaining -= add;
                }
            }
        }

        ensure(remaining == 0, "maxSize is too small");

        return partition;
    }

    template<typename T>
    TArray<TArray<T>> partition(
            TArray<T> elements,
            int numParts,
            int minSize = 0,
            int maxSize = -1)
    {
        return partition(
            std::move(elements),
            partition(
                static_cast<int>(elements.size()),
                numParts,
                minSize,
                maxSize));
    }

    template<typename T>
    TArray<TArray<T>> partition(TArray<T> elements, const Array& sizes) {
        size_t total = std::accumulate(sizes.begin(), sizes.end(), size_t(0));
        ensure(total == elements.size(), "sum(sizes) != elements.size()");
        elements.shuffle();
        TArray<TArray<T>> res;
        auto it = elements.begin();
        for (int size: sizes) {
            res.emplace_back();
            std::copy(it, it + size, std::back_inserter(res.back()));
            it += size;
        }

        return res;
    }
};

JNGEN_EXTERN MathRandom rndm;

} // namespace jngen

using jngen::isPrime;

using jngen::rndm;
