#pragma once

#include "common.h"

#include <algorithm>
#include <cctype>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

// TODO: adequate error messages

namespace jngen {

class Pattern {
    friend class Parser;
public:
    Pattern() : isOrPattern(false), min(1), max(1) {}
    Pattern(const std::string& s);

    std::string next(std::function<int(int)>&& rnd) const;

private:
    Pattern(Pattern p, std::pair<int, int> quantity);

    Pattern(std::vector<char> chars, std::pair<int, int> quantity);

    std::vector<char> chars;
    std::vector<Pattern> children;
    bool isOrPattern;
    int min;
    int max;

    static std::map<std::string, Pattern> cachedPatterns_;
};

class Parser {
public:
    Pattern parse(const std::string& s) {
        this->s = s;
        pos = 0;
        return parsePattern();
    }

private:
    static bool isControl(char c);

    static int control(int c);

    int next();

    int peek() const;

    int peekAndMove(size_t& newPos) const;

    // TODO: catch overflows
    int readInt();

    std::pair<int, int> parseRange();

    std::pair<int, int> tryParseQuantity();

    std::vector<char> parseBlock();

    Pattern parsePattern();

    std::string s;
    size_t pos;
};

} // namespace jngen

#ifndef JNGEN_DECLARE_ONLY
#define JNGEN_INCLUDE_PATTERN_INL_H
#include "impl/pattern_inl.h"
#undef JNGEN_INCLUDE_PATTERN_INL_H
#endif // JNGEN_DECLARE_ONLY

using jngen::Pattern;
