#pragma once

#include <string>
#include <utility>

#include "variant.h"
#include "variant_array.h"

namespace jngen {

#define JNGEN_DEFAULT_WEIGHT_TYPES int, double, std::string, char, std::pair<int, int>

#if defined(JNGEN_EXTRA_WEIGHT_TYPES)
#define JNGEN_WEIGHT_TYPES JNGEN_DEFAULT_WEIGHT_TYPES , JNGEN_EXTRA_WEIGHT_TYPES
#else
#define JNGEN_WEIGHT_TYPES JNGEN_DEFAULT_WEIGHT_TYPES
#endif

using Weight = Variant<JNGEN_WEIGHT_TYPES>;
using WeightArray = VariantArray<JNGEN_WEIGHT_TYPES>;

} // namespace jngen

using jngen::Weight;
using jngen::WeightArray;
