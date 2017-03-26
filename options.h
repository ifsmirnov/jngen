#pragma once

#include "common.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace jngen {

struct VariableMap {
    std::vector<std::string> positional;
    std::map<std::string, std::string> named;

    int count(size_t pos) const {
        return pos < positional.size();
    }

    int count(const std::string& name) const {
        return named.count(name);
    }

    std::string operator[](size_t pos) const {
        if (!count(pos)) {
            return "";
        }
        return positional[pos];
    }

    std::string operator[](const std::string& name) const {
        if (!count(name)) {
            return name;
        }
        return named.at(name);
    }

    bool initialized = false;
};

// TODO: think about seed as a last argument
inline VariableMap parseArguments(const std::vector<std::string>& args) {
    VariableMap result;

    for (const std::string& s: args) {
        if (s == "-") {
            continue;
        }
        if (s == "--") {
            break;
        }

        if (s[0] != '-') {
            result.positional.push_back(s);
            continue;
        }

        std::string name;
        std::string value;
        bool foundEq = false;
        for (char c: s.substr(1)) {
            if (!foundEq && c == '=') {
                foundEq = true;
            } else {
                if (foundEq) {
                    value += c;
                } else {
                    name += c;
                }
            }
        }
        if (!foundEq) {
            value = "1";
        }
        ensure(
            !result.count(value),
            "Named arguments must have distinct names");
        result.named[name] = value;
    }

    result.initialized = true;
    return result;
}

JNGEN_EXTERN VariableMap vmap;

template<typename T>
bool readVariable(const std::string& value, T& var) {
    std::istringstream ss(value);

    T t;
    if (ss >> t) {
        var = t;
        return true;
    }
    return false;
}

template<typename T>
bool getOpt(size_t index, T& var) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getOpt(...)");
    if (!vmap.count(index)) {
        return false;
    }
    return readVariable(vmap[index], var);
}

template<typename T>
bool getOpt(const std::string& name, T& var) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getOpt(...)");
    if (!vmap.count(name)) {
        return false;
    }
    return readVariable(vmap[name], var);
}

template<typename T>
T getOptOr(size_t index, T def) {
    getOpt(index, def);
    return def;
}

template<typename T>
T getOptOr(const std::string& name, T def) {
    getOpt(name, def);
    return def;
}

inline void parseArgs(int argc, char *argv[]) {
    vmap = parseArguments(std::vector<std::string>(argv + 1, argv + argc));
}

namespace detail {

inline std::vector<std::string> splitByComma(std::string s) {
    auto strip = [](std::string s) {
        size_t l = 0;
        while (l < s.size() && s[l] == ' ') {
            ++l;
        }
        s = s.substr(l);
        while (!s.empty() && s.back() == ' ') {
            s.pop_back();
        }
        return s;
    };

    std::vector<std::string> result;
    s += ',';
    std::string cur;

    for (char c: s) {
        if (c == ',') {
            result.push_back(strip(cur));
            cur.clear();
        } else {
            cur += c;
        }
    }

    return result;
}

inline int getNamedImpl(std::vector<std::string>::const_iterator) { return 0; }

template<typename T, typename ... Args>
int getNamedImpl(
    std::vector<std::string>::const_iterator it, T& var, Args&... args)
{
    int res = getOpt(*it, var);
    res += getNamedImpl(++it, args...);
    return res;
}

inline int getPositionalImpl(size_t) { return 0; }

template<typename T, typename ... Args>
int getPositionalImpl(size_t index, T& var, Args&... args) {
    int res = getOpt(index, var);
    res += getPositionalImpl(index + 1, args...);
    return res;
}

} // namespace detail

template<typename ... Args>
int doGetNamed(const std::string& names, Args&... args) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getNamed(...)");

    auto namesSplit = detail::splitByComma(names);

    ENSURE(
        namesSplit.size() == sizeof...(args),
        "Number of names is not equal to number of variables");

    return detail::getNamedImpl(namesSplit.begin(), args...);
}

template<typename ... Args>
int getPositional(Args&... args) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getPositional(...)");

    return detail::getPositionalImpl(0, args...);
}

} // namespace jngen

using jngen::parseArgs;
using jngen::getOpt;
using jngen::getOptOr;

using jngen::getPositional;

#define getNamed(...) jngen::doGetNamed(#__VA_ARGS__, __VA_ARGS__)
