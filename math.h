#pragma once

#include "array.h"
#include "common.h"
#include "random.h"

#include <algorithm>
#include <cmath>
#include <iterator>
#include <limits>
#include <type_traits>
#include <vector>

namespace jngen {

namespace detail {

int multiply(int x, int y, int mod) {
    return static_cast<long long>(x) * y % mod;
}

long long multiply(long long x, long long y, long long mod) {
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

int power(int x, int k, int mod) {
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

long long power(long long x, long long k, long long mod) {
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

bool isPrime(long long n) {
    const static std::vector<int> INT_WITNESSES{2, 7, 61};
    const static std::vector<long long> LONG_LONG_WITNESSES
        {2, 3, 5, 7, 11, 13, 17, 19, 23};

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
        return randomPrime(2, n - 1);
    }

    static long long randomPrime(long long l, long long r) {
        ensure(l <= r);
        int retries = std::log(l) * std::log(l);
        while (true) {
            long long x = rnd.next(l, r);
            if (isPrime(x)) {
                return x;
            }

            if (--retries == 0) {
                ensure(
                    false,
                    format(
                        "There are no primes between %lld and %lld",
                        l, r)
                );
            }
        }
    }

    static Array partition(int n, size_t numParts) {
        auto res = partition(static_cast<long long>(n), numParts);
        return Array(res.begin(), res.end());
    }

    static Array64 partition(long long n, size_t numParts) {
        auto res = partitionNonEmpty(
            static_cast<long long>(n + numParts), numParts);
        for (auto& x: res) {
            --x;
        }
        return res;
    }

    static Array partitionNonEmpty(int n, size_t numParts) {
        auto res = partitionNonEmpty(static_cast<long long>(n), numParts);
        return Array(res.begin(), res.end());
    }

    static Array64 partitionNonEmpty(long long n, size_t numParts) {
        ensure(static_cast<long long>(numParts) <= n);
        auto delimiters = Array64::randomUnique(numParts - 1, 1, n - 1).sorted();
        delimiters.insert(delimiters.begin(), 0);
        delimiters.push_back(n);
        Array64 res(numParts);
        for (size_t i = 0; i < numParts; ++i) {
            res[i] = delimiters[i + 1] - delimiters[i];
        }
        return res;
    }

    template<typename T>
    TArray<TArray<T>> partition(TArray<T> elements, size_t numParts) {
        return partition(
            std::move(elements),
            partition(static_cast<int>(elements.size()), numParts));
    }

    template<typename T>
    TArray<TArray<T>> partitionNonEmpty(TArray<T> elements, size_t numParts) {
        return partition(
            std::move(elements),
            partitionNonEmpty(static_cast<int>(elements.size()), numParts));
    }

    template<typename T>
    TArray<TArray<T>> partition(TArray<T> elements, const Array& sizes) {
        elements.shuffle();
        TArray<TArray<T>> res;
        auto it = elements.begin();
        for (int size: sizes) {
            res.emplace_back();
            std::copy(it, it + size, std::back_inserter(res.back()));
            it += size;
        }

        ensure(it == elements.end(), "sum(sizes) != elements.size()");

        return res;
    }
};

MathRandom rndm;

} // namespace jngen

using jngen::isPrime;

using jngen::rndm;
