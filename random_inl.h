#ifndef JNGEN_INCLUDE_RANDOM_INL_H
#error File "random_inl.h" must not be included directly.
#endif

namespace jngen {

int Random::wnext(int n, int w) {
    ensure(n > 0);
    if (std::abs(w) <= WNEXT_LIMIT) {
        return smallWnext<int>(w, n);
    } else {
        double t = realWnext(w);
        std::cerr << "t = " << t << std::endl;
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

} // namespace jngen
