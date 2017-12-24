#pragma once

namespace jngen {

struct Config {
    bool generateLargeObjects = false;
    bool largeOptionIndices = false;
    bool normalizeEdges = true;
};

#ifdef JNGEN_DECLARE_ONLY
extern
#endif
Config config;

} // namespace jngen

using jngen::config;
