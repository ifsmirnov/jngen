#ifndef JNGEN_INCLUDE_RANDOM_INL_H
#error File "random_inl.h" must not be included directly.
#endif

namespace jngen {

int Random::next(int l, int r) {
    return l + next(r-l+1);
}

long long Random::next(long long l, long long r) {
    return l + next(r-l+1);
}

size_t Random::next(size_t l, size_t r) {
    return l + next(r-l+1);
}

double Random::next(double l, double r) {
    return l + next(r-l);
}

int Random::wnext(int n, int w) {
    return baseWnext(n, w);
}

long long Random::wnext(long long n, int w) {
    return baseWnext(n, w);
}

size_t Random::wnext(size_t n, int w) {
    return baseWnext(n, w);
}

double Random::wnext(double n, int w) {
    return baseWnext(n, w);
}

int Random::wnext(int l, int r, int w) {
    return l + wnext(r-l+1, w);
}

long long Random::wnext(long long l, long long r, int w) {
    return l + wnext(r-l+1, w);
}

size_t Random::wnext(size_t l, size_t r, int w) {
    return l + wnext(r-l+1, w);
}

double Random::wnext(double l, double r, int w) {
    return l + wnext(r-l, w);
}

} // namespace jngen
