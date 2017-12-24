#pragma once

#include "common.h"
#include "range_option.h"

#include <map>
#include <sstream>
#include <string>
#include <vector>

namespace jngen {

namespace detail {

template<typename T>
constexpr bool isValidOptionType() {
    return std::is_same<T, std::string>::value ||
        (std::is_arithmetic<T>::value && !std::is_same<T, char>::value);
}

template<typename T>
using StringIfCharPtrElseT = typename std::conditional<
    std::is_same<typename std::decay<T>::type, const char*>::value ||
        std::is_same<typename std::decay<T>::type, char*>::value,
    std::string,
    T>::type;

} // namespace detail

struct Index {
    size_t index;
    std::string name;

    Index(size_t index) : index(index) {
        if (!config.largeOptionIndices) {
            ensure(
                index < 32,
                "Looks like you called getOpt('c'). Consider using "
                "getOpt(\"c\") or set 'config.largeOptionIndices = true' "
                "if you indeed have more than 32 options.");
        }
    }

    Index(const std::string& name) : name(name) {
        ensure(!name.empty(), "Variable name cannot be empty");
    }

    bool isNamed() const {
        return !name.empty();
    }
};

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
        return positional.at(pos);
    }

    std::string operator[](const std::string& name) const {
        if (!count(name)) {
            return "";
        }
        return named.at(name);
    }

    int count(const Index& index) const {
        if (index.isNamed()) {
            return count(index.name);
        } else {
            return count(index.index);
        }
    }

    std::string operator[](const Index& index) const {
        if (index.isNamed()) {
            return (*this)[index.name];
        } else {
            return (*this)[index.index];
        }
    }

    void assertExistence(const Index& index) const {
        if (count(index)) {
            return;
        }
        if (index.isNamed()) {
            ensure(false, format(
                    "There is no variable with name '%s'", index.name.c_str()));
        } else {
            ensure(false, format(
                    "There is no variable with index %d", index.index));
        }
    }

    bool initialized = false;
};

template<typename T>
class PendingVariable {
public:
    explicit PendingVariable(std::string value) :
        value_(std::move(value))
    {  }

    PendingVariable(std::string value, T defaultValue) :
        value_(std::move(value)),
        default_(std::move(defaultValue))
    {  }

    explicit PendingVariable(std::nullptr_t, T defaultValue) :
        valid_(false),
        default_(std::move(defaultValue))
    {  }

    // We need this check in order to make the following work:
    // string s = getOpt(0);
    // s = getOpt(0);
    // Weird things happen if we allow all kind of casts. See
    // https://stackoverflow.com/questions/46740341
    template<
        typename U,
        typename std::enable_if<
            detail::isValidOptionType<U>()>::type* = nullptr>
    operator U() const
    {
        if (!valid_) {
            return static_cast<U>(default_);
        }

        std::istringstream ss(value_);
        U t;
        if (ss >> t) {
            return t;
        } else {
            ensure(
                false,
                format(
                    "Cannot parse option. Raw value: '%s'",
                    value_.c_str()));
        }
    }

    // TODO: getOpt operators, like getOpt("n") == 100

private:
    bool valid_ = true;
    std::string value_;
    T default_;
};

template<>
class PendingVariable<void> {
public:
    explicit PendingVariable(std::string value) :
        value_(std::move(value))
    {  }

    // We need this check in order to make the following work:
    // string s = getOpt(0);
    // s = getOpt(0);
    // Weird things happen if we allow all kind of casts. See
    // https://stackoverflow.com/questions/46740341
    template<
        typename U,
        typename std::enable_if<
            detail::isValidOptionType<U>()>::type* = nullptr>
    operator U() const
    {
        std::istringstream ss(value_);
        U t;
        if (ss >> t) {
            return t;
        } else {
            ensure(
                false,
                format(
                    "Cannot parse option. Raw value: '%s'",
                    value_.c_str()));
        }
    }

private:
    std::string value_;
};

// TODO: think about seed as a last argument
inline VariableMap parseArguments(const std::vector<std::string>& args) {
    VariableMap result;

    auto setNamedVar = [&result](
            const std::string& name,
            const std::string& value)
    {
        ensure(
            !result.count(value),
            "Named arguments must have distinct names");
        result.named[name] = value;
    };

    std::string pendingVarName;

    for (const std::string& s: args) {
        if (s == "-") {
            continue;
        }
        if (s == "--") {
            break;
        }

        if (s[0] != '-') {
            if (!pendingVarName.empty()) {
                setNamedVar(pendingVarName, s);
                pendingVarName = "";
            } else {
                result.positional.push_back(s);
            }
            continue;
        }

        if (!pendingVarName.empty()) {
            result.named[pendingVarName] = "1";
            pendingVarName = "";
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
        if (foundEq) {
            setNamedVar(name, value);
        } else {
            pendingVarName = name;
        }

        setNamedVar(name, value);
    }

    if (!pendingVarName.empty()) {
        result.named[pendingVarName] = "1";
    }

    result.initialized = true;
    return result;
}

JNGEN_EXTERN VariableMap vmap;

namespace detail {

inline PendingVariable<void> getOpt(const Index& index) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getOpt(...)");
    vmap.assertExistence(index);
    return PendingVariable<void>(vmap[index]);
}

template<typename T, typename U = detail::StringIfCharPtrElseT<T>>
PendingVariable<U> getOpt(const Index& index, const T& defaultValue) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getOpt(...)");
    if (vmap.count(index)) {
        return PendingVariable<U>(vmap[index], U{defaultValue});
    } else {
        return PendingVariable<U>(nullptr, U{defaultValue});
    }
}

inline bool hasOpt(const Index& index) {
    return vmap.count(index);
}

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

inline int getNamedImpl(std::vector<std::string>::const_iterator) { return 0; }

template<typename T, typename ... Args>
int getNamedImpl(
    std::vector<std::string>::const_iterator it, T& var, Args&... args)
{
    T value;
    int res = 0;
    if (readVariable(vmap[*it], value)) {
        var = value;
        ++res;
    }
    res += getNamedImpl(++it, args...);
    return res;
}

inline int getPositionalImpl(size_t) { return 0; }

template<typename T, typename ... Args>
int getPositionalImpl(size_t index, T& var, Args&... args) {
    T value;
    int res = 0;
    if (readVariable(vmap[index], value)) {
        var = value;
        ++res;
    }
    res += getPositionalImpl(index + 1, args...);
    return res;
}

} // namespace detail

template<typename ... Args>
int doGetNamed(const std::string& names, Args&... args) {
    ensure(
        vmap.initialized,
        "parseArgs(args, argv) must be called before getNamed(...)");

    auto namesSplit = util::split(names, ',');

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

inline void parseArgs(int argc, char *argv[]) {
    vmap = parseArguments(std::vector<std::string>(argv + 1, argv + argc));
}

inline PendingVariable<void> getOpt(size_t index) {
    return detail::getOpt(Index(index));
}

inline PendingVariable<void> getOpt(const std::string& name) {
    return detail::getOpt(Index(name));
}

template<typename T, typename U = detail::StringIfCharPtrElseT<T>>
PendingVariable<U> getOpt(size_t index, const T& defaultValue) {
    return detail::getOpt(Index(index), defaultValue);
}

template<typename T, typename U = detail::StringIfCharPtrElseT<T>>
PendingVariable<U> getOpt(const std::string& name, const T& defaultValue) {
    return detail::getOpt(Index(name), defaultValue);
}

inline bool hasOpt(size_t index) {
    return vmap.count(index);
}

inline bool hasOpt(const std::string& name) {
    return vmap.count(name);
}

inline options::Range parseRange(const std::string& value) {
    return options::Range::fromString(value);

}

} // namespace jngen

using jngen::parseArgs;
using jngen::getOpt;
using jngen::hasOpt;
using jngen::parseRange;

using jngen::getPositional;

#define getNamed(...) ::jngen::doGetNamed(#__VA_ARGS__, __VA_ARGS__)
