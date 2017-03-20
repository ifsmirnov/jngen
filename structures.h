#pragma once

#include "array.h"

#include <type_traits>

namespace jngen {

// generator(TArray<T>) -> U
// combiner(TArray<U>) -> U
template<typename T, typename Generator, typename Combiner>
auto composition(
        const TArray<T>& elements,
        size_t numParts,
        Generator&& generator,
        Combiner&& combiner)
    -> decltype(generator(elements))
{
    using U = decltype(generator(elements));

    static_assert(
        std::is_same<U, decltype(combiner(TArray<U>{}))>::value,
        "result type of 'combiner(TArray<U>)' must be U");

    auto partition = elements.partitionNonEmpty(numParts);

    TArray<U> subObjects;
    for (const auto& part: partition) {
        subObjects.emplace_back(generator(part));
    }

    return combiner(subObjects);
}

} // namespace jngen

using jngen::composition;
