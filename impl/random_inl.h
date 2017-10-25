#ifndef JNGEN_INCLUDE_RANDOM_INL_H
#error File "random_inl.h" must not be included directly.
#include "../random.h" // for completion engine
#endif

namespace jngen {

void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ENSURE(engine() == 822569775,
        "std::mt19937 doesn't conform to the C++ standard");
    ENSURE(engine() == 2137449171,
        "std::mt19937 doesn't conform to the C++ standard");
    ENSURE(engine() == 2671936806,
        "std::mt19937 doesn't conform to the C++ standard");
}

void assertIntegerSizes() {
    static_assert(
        std::numeric_limits<unsigned char>::max() == 255,
        "max(unsigned char) != 255");
    static_assert(sizeof(int) == 4, "sizeof(int) != 4");
    static_assert(sizeof(long long) == 8, "sizeof(long long) != 8");
    static_assert(
        sizeof(size_t) == 4 || sizeof(size_t) == 8,
        "sizeof(size_t) is neither 4 nor 8");
    static_assert(
        sizeof(std::size_t) == sizeof(size_t),
        "sizeof(size_t) != sizeof(std::size_t)");
}

void registerGen(int argc, char *argv[], int version) {
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

void Random::seed(uint32_t val) {
    randomEngine_.seed(val);
}

void Random::seed(const std::vector<uint32_t>& seed) {
    std::seed_seq seq(seed.begin(), seed.end());
    randomEngine_.seed(seq);
}

uint32_t Random::next() {
    return randomEngine_();
}

uint64_t Random::next64() {
    uint64_t a = next();
    uint64_t b = next();
    return (a << 32) ^ b;
}

double Random::nextf() {
    return (double)randomEngine_() / randomEngine_.max();
}

int Random::next(int n) {
    ensure(n > 0);
    return uniformRandom(n, *this, (uint32_t (Random::*)())&Random::next);
}

long long Random::next(long long n) {
    ensure(n > 0);
    return uniformRandom(n, *this, &Random::next64);
}

size_t Random::next(size_t n) {
    ensure(n > 0);
    return uniformRandom(n, *this, &Random::next64);
}

double Random::next(double n) {
    ensure(n >= 0);
    return nextf() * n;
}

int Random::next(int l, int r) {
    ensure(l <= r);
    uint32_t n = static_cast<uint32_t>(r) - l + 1;
    return l + uniformRandom(
        n, *this, (uint32_t (Random::*)())&Random::next);
}

long long Random::next(long long l, long long r) {
    ensure(l <= r);
    uint64_t n = static_cast<uint64_t>(r) - l + 1;
    return l + uniformRandom(n, *this, &Random::next64);
}

size_t Random::next(size_t l, size_t r) {
    ensure(l <= r);
    uint64_t n = static_cast<uint64_t>(r) - l + 1;
    return l + uniformRandom(n, *this, &Random::next64);
}

double Random::next(double l, double r) {
    ensure(l <= r);
    return l + next(r-l);
}

int Random::wnext(int n, int w) {
    ensure(n > 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<int>(w, n);
    } else {
        double t = realWnext(w);
        return n * t;
    }
}

long long Random::wnext(long long n, int w) {
    ensure(n > 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<long long>(w, n);
    } else {
        return n * realWnext(w);
    }
}

size_t Random::wnext(size_t n, int w) {
    ensure(n > 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<size_t>(w, n);
    } else {
        return n * realWnext(w);
    }
}

double Random::wnext(double n, int w) {
    ensure(n >= 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<double>(w, n);
    } else {
        return realWnext(w) * n;
    }
}

int Random::wnext(int l, int r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<int>(w, l, r);
    } else {
        uint32_t n = static_cast<uint32_t>(r) - l + 1;
        return l + static_cast<uint32_t>(n * realWnext(w));
    }
}

long long Random::wnext(long long l, long long r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<long long>(w, l, r);
    } else {
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + static_cast<uint64_t>(n * realWnext(w));
    }
}

size_t Random::wnext(size_t l, size_t r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<size_t>(w, l, r);
    } else {
        uint64_t n = static_cast<uint64_t>(r) - l + 1;
        return l + static_cast<uint64_t>(n * realWnext(w));
    }
}

double Random::wnext(double l, double r, int w) {
    ensure(l <= r);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<double>(w, l, r);
    } else {
        return realWnext(w) * (r - l) + l;
    }
}

int Random::nextByDistribution(const std::vector<int>& distribution) {
    long long total = std::accumulate(
            distribution.begin(),
            distribution.end(),
            0ll);
    long long value = rnd.next(total);
    size_t i = 0;
    while (distribution[i] <= value) {
        value -= distribution[i++];
        ENSURE(i < distribution.size());
    }
    return i;
}

std::string Random::next(const std::string& pattern) {
    return Pattern(pattern).next([this](int n) { return next(n); });
}

} // namespace jngen
