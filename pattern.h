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
    Pattern(Pattern p, std::pair<int, int> quantity) :
        isOrPattern(false),
        min(quantity.first),
        max(quantity.second)
    {
        children.push_back(std::move(p));
    }

    Pattern(std::vector<char> chars, std::pair<int, int> quantity) :
        chars(std::move(chars)),
        isOrPattern(false),
        min(quantity.first),
        max(quantity.second)
    {  }

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
    static bool isControl(char c) {
        static const std::string CONTROL_CHARS = "()[]{}|?";
        return CONTROL_CHARS.find(c) != std::string::npos;
    }

    static int control(int c) {
        return c >> 8;
    }

    int next() {
        size_t newPos;
        int result = peekAndMove(newPos);
        pos = newPos;
        return result;
    }

    int peek() const {
        size_t dummy;
        return peekAndMove(dummy);
    }

    int peekAndMove(size_t& newPos) const {
        newPos = pos;
        if (pos == s.size()) {
            return -1;
        }
        if (s[pos] == '\\') {
            ensure(
                pos+1 < s.size(),
                "Backslash at the end of the pattern is illegal");
            newPos += 2;
            return s[pos+1];
        }

        ++newPos;
        int ret = s[pos];
        return isControl(ret) ? (ret << 8) : ret;
    }

    // TODO: catch overflows
    int readInt() {
        ENSURE(std::isdigit(peek()));

        int res = 0;
        while (std::isdigit(peek())) {
            res = res * 10 + next() - '0';
        }
        return res;
    }

    std::pair<int, int> parseRange() {
        ENSURE(control(next()) == '{');

        int from = readInt();

        int nxt = next();
        if (control(nxt) == '}') {
            return {from, from};
        } else if (nxt == ',' || nxt == '-') {
            int to = readInt();
            ENSURE(control(next()) == '}');
            return {from, to};
        } else {
            ensure(false, "cannot parse character range");
        }
    }

    std::pair<int, int> tryParseQuantity() {
        std::pair<int, int> quantity = {1, 1};

        int qchar = peek();
        if (control(qchar) == '?') {
            quantity = {0, 1};
            next();
        } else if (control(qchar) == '{') {
            quantity = parseRange();
        }

        return quantity;
    }

    std::vector<char> parseBlock() {
        std::vector<char> allowed;
        char last = -1;
        bool inRange = false;
        while (control(peek()) != ']') {
            char c = next(); // buggy on cases like [a-}]
            ENSURE(c != -1);

            if (c == '-') {
                ensure(!inRange, "invalid pattern");
                inRange = true;
            } else if (inRange) {
                ensure(c >= last, "invalid pattern");
                for (char i = last; i <= c; ++i) {
                    allowed.push_back(i);
                }
                inRange = false;
                last = -1;
            } else {
                if (last != -1) {
                    allowed.push_back(last);
                }
                last = c;
            }
        }

        ENSURE(control(next()) == ']');

        ENSURE(!inRange);
        if (last != -1) {
            allowed.push_back(last);
        }

        std::sort(allowed.begin(), allowed.end());
        return allowed;
    }

    Pattern parsePattern() {
        std::vector<Pattern> orPatterns;
        Pattern cur;

        while (true) {
            int nxt = next();
            if (nxt == -1 || control(nxt) == ')') {
                break;
            } else if (control(nxt) == '(') {
                Pattern p = parsePattern();
                cur.children.push_back(Pattern(p, tryParseQuantity()));
            } else if (control(nxt) == '|') {
                orPatterns.emplace_back();
                std::swap(orPatterns.back(), cur);
            } else {
                std::vector<char> chars;
                if (control(nxt) == '[') {
                    chars = parseBlock();
                } else {
                    ENSURE(!control(nxt));
                    chars = {static_cast<char>(nxt)};
                }

                cur.children.push_back(Pattern(chars, tryParseQuantity()));
            }
        }

        if (orPatterns.empty()) {
            return cur;
        } else {
            orPatterns.emplace_back();
            std::swap(orPatterns.back(), cur);

            Pattern p;
            p.isOrPattern = true;
            p.children = orPatterns;
            return p;
        }
    }

    std::string s;
    size_t pos;
};

#ifndef JNGEN_DECLARE_ONLY

std::map<std::string, Pattern> Pattern::cachedPatterns_;

Pattern::Pattern(const std::string& s) {
    auto iter = cachedPatterns_.find(s);
    if (iter != cachedPatterns_.end()) {
        *this = iter->second;
        return;
    }
    *this = Parser().parse(s);
    cachedPatterns_[s] = *this;
}

std::string Pattern::next(std::function<int(int)>&& rnd) const {
    if (isOrPattern) {
        ENSURE(!children.empty());
        return children[rnd(children.size())].next(std::move(rnd));
    }

    ENSURE( (!!chars.empty()) ^ (!!children.empty()) );

    int count;
    if (min == max) {
        count = min;
    } else {
        count = min + rnd(max - min + 1);
    }

    std::string result;
    for (int i = 0; i < count; ++i) {
        if (!children.empty()) {
            for (const Pattern& p: children) {
                result += p.next(std::move(rnd));
            }
        } else {
            result += chars[rnd(chars.size())];
        }
    }

    return result;
}
#endif

} // namespace jngen

using jngen::Pattern;
