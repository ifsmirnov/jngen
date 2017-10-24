#pragma once

#include "array.h"
#include "common.h"
#include "random.h"
#include "sequence_ops.h"

#include <algorithm>
#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace jngen {

typedef std::pair<long long, long long> HashBase; // (mod, base)
typedef std::pair<std::string, std::string> StringPair;

class StringRandom {
public:
    StringRandom() {
        static bool created = false;
        ENSURE(!created, "jngen::StringRandom should be created only once");
        created = true;
    }

    static std::string random(int len, const std::string& alphabet = "a-z");

    template<typename ... Args>
    static std::string random(const std::string& pattern, Args... args) {
        return rnd.next(pattern, std::forward(args)...);
    }

    static std::string thueMorse(int len, char first = 'a', char second = 'b');

    static std::string abacaba(int len, char first = 'a');

    static StringPair antiHash(
            const std::vector<HashBase>& bases,
            const std::string& alphabet = "a-z",
            int length = -1);
};

JNGEN_EXTERN StringRandom rnds;

} // namespace jngen

using jngen::rnds;

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_RNDS_INL_H
#include "impl/rnds_inl.h"
#undef JNGEN_INCLUDE_RNDS_INL_H
#endif // JNGEN_DECLARE_ONLY
