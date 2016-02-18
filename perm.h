#pragma once
#include <bits/stdc++.h>

#include "array.h"

namespace impl {

class Perm : public Array {
public:
    Perm() {}
    Perm(const Perm&) = default;
    Perm& operator=(const Perm&) = default;

    Perm(size_t n);

    static Perm id(size_t n);
    static Perm random(size_t n);

    template<typename Sequence>
    Sequence apply(const Sequence& a);
};

Perm::Perm(size_t n) {
    *this = Perm::id(n);
}

Perm Perm::id(size_t n) {
    Perm res;
    res.resize(n);
    std::iota(res.begin(), res.end(), 0);
    return res;
}

Perm Perm::random(size_t n) {
    Perm res = Perm::id(n);
    res.shuffle();
    return res;
}

template<typename Sequence>
Sequence Perm::apply(const Sequence& a) {
    ensure(size() == a.size());

    Sequence result(a.size());
    for (size_t i = 0; i < size(); ++i) {
        result[i] = a[at(i)];
    }

    return result;
}

} // namespace impl

using impl::Perm;
