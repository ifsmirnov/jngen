#pragma once

#include "common.h"
#include "random.h"

#include <algorithm>
#include <iterator>
#include <numeric>

namespace jngen {

// TODO: deprecate random_shuffle as done in testlib.h

template<typename Iterator>
void shuffle(Iterator begin, Iterator end) {
    ensure(end >= begin, "Cannot shuffle range of negative length");
    size_t size = end - begin;
    for (size_t i = 1; i < size; ++i) {
        std::swap(*(begin + i), *(begin + rnd.next(i + 1)));
    }
}

template<typename Iterator>
auto choice(Iterator begin, Iterator end)
        -> typename std::iterator_traits<Iterator>::value_type
{
    return rnd.choice(begin, end);
}

template<typename Container>
typename Container::value_type choice(const Container& container) {
    return rnd.choice(container);
}

template<typename T>
T choice(std::initializer_list<T> ilist) {
    return choice(ilist.begin(), ilist.end());
}

namespace detail {

template<typename Collection2D>
typename Collection2D::value_type interleave(const Collection2D& collection) {
    std::vector<size_t> sizes;
    for (const auto& c: collection) {
        sizes.push_back(c.size());
    }
    size_t size = std::accumulate(sizes.begin(), sizes.end(), 0u);

    typename Collection2D::value_type result;
    while (size > 0) {
        size_t id = rnd.nextByDistribution(sizes);
        result.emplace_back(collection[id][collection[id].size() - sizes[id]]);
        --sizes[id];

        --size;
    }

    return result;
}

} // namespace detail

template<typename Collection2D>
typename Collection2D::value_type interleave(const Collection2D& collection) {
    return detail::interleave(collection);
}

template<typename Collection>
Collection interleave(const std::initializer_list<Collection>& ilist) {
    return detail::interleave<std::vector<Collection>>(ilist);
}

} // namespace jngen

using jngen::shuffle;
using jngen::choice;
using jngen::interleave;
