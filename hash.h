#pragma once

#include "common.h"

#include <iterator>
#include <unordered_set>
#include <vector>
#include <type_traits>
#include <utility>

namespace jngen {

template<typename T, typename Enable = void>
struct Hash;

namespace impl {

inline void hashCombine(uint64_t& h, uint64_t k) {
    const uint64_t m = 0xc6a4a7935bd1e995;
    const int r = 47;

    k *= m;
    k ^= k >> r;
    k *= m;

    h ^= k;
    h *= m;

    h += 0xe6546b64;
}

template<typename Iterator>
void hashCombine(uint64_t& h, Iterator begin, Iterator end) {
    Hash<typename std::iterator_traits<Iterator>::value_type> hash;
    while (begin != end) {
        hashCombine(h, hash(*begin++));
    }
}

} // namespace impl

template<typename T>
struct Hash<
        T,
        enable_if_t<std::is_integral<T>::value>>
{
    uint64_t operator()(const T& t) const {
        uint64_t h = 0;
        impl::hashCombine(h, t);
        return h;
    }
};

#define JNGEN_DEFINE_STD_HASH(Type)\
namespace std {\
template<>\
struct hash<Type> {\
    size_t operator()(const Type& value) const {\
        return jngen::Hash<Type>{}(value);\
    }\
};\
}

#define JNGEN_DEFINE_STD_HASH_TEMPLATE(T, Type)\
namespace std {\
template<typename T>\
struct hash<Type> {\
    size_t operator()(const Type& value) const {\
        return jngen::Hash<Type>{}(value);\
    }\
};\
}

template<typename T>
struct Hash<std::vector<T>> {
    uint64_t operator()(const std::vector<T>& elements) const {
        uint64_t h = 0;
        impl::hashCombine(h, elements.size());
        impl::hashCombine(h, elements.begin(), elements.end());
        return h;
    }
};

template<typename T, typename U>
struct Hash<std::pair<T, U>> {
    uint64_t operator()(const std::pair<T, U>& value) const {
        uint64_t h = 0;
        impl::hashCombine(h, Hash<T>{}(value.first));
        impl::hashCombine(h, Hash<U>{}(value.second));
        return h;
    }
};

} // namespace jngen

JNGEN_DEFINE_STD_HASH_TEMPLATE(T, std::vector<T>);

namespace std {
template<typename T, typename U>
struct hash<std::pair<T, U>> {
    size_t operator()(const std::pair<T, U>& value) const {
        return jngen::Hash<std::pair<T, U>>{}(value);
    }
};
} // namespace std
