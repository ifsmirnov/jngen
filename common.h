#pragma once

#include <chrono>
#include <cstdlib>
#include <iostream>
#include <map>
#include <stdexcept>
#include <string>
#include <type_traits>

#ifdef JNGEN_DECLARE_ONLY
#define JNGEN_EXTERN extern
#else
#define JNGEN_EXTERN
#endif

namespace jngen {

class Exception : public std::runtime_error {
public:
    explicit Exception(const std::string& s) :
        std::runtime_error("Assertion `" + s + "' failed.")
    {  }

    Exception(const std::string& assertMsg, const std::string& expl) :
        std::runtime_error(expl + " (assertion `" + assertMsg + "' failed).")
    {  }
};

class InternalException : public Exception {
public:
    explicit InternalException(const std::string& s) : Exception(s) {}

    InternalException(const std::string& assertMsg, const std::string& expl) :
        Exception(assertMsg, expl)
    {  }
};

} // namespace jngen

#define JNGEN_ENSURE1(exType, cond)\
do\
    if (!(cond)) {\
        throw exType(#cond);\
    }\
while (false)

#define JNGEN_ENSURE2(exType, cond, msg)\
do\
    if (!(cond)) {\
        throw exType(#cond, msg);\
    }\
while (false)

#define JNGEN_GET_MACRO(_1, _2, NAME, ...) NAME

#define ensure(...) JNGEN_GET_MACRO(__VA_ARGS__, JNGEN_ENSURE2, JNGEN_ENSURE1)\
    (jngen::Exception, __VA_ARGS__)
#define ENSURE(...) JNGEN_GET_MACRO(__VA_ARGS__, JNGEN_ENSURE2, JNGEN_ENSURE1)\
    (jngen::InternalException, __VA_ARGS__)

namespace jngen {

template<typename ... Args>
std::string format(const std::string& fmt, Args... args) {
    constexpr static char BUF_SIZE = 64;
    static char BUFFER[BUF_SIZE];

    int bufSize = BUF_SIZE;
    char *buf = BUFFER;

    while (true) {
        int ret = std::snprintf(buf, bufSize, fmt.c_str(), args...);
        if (ret < bufSize) {
            break;
        }

        if (bufSize != BUF_SIZE) {
            delete[] buf;
        }

        bufSize *= 2;
        buf = new char[bufSize];
    }

    std::string result(buf);

    if (bufSize != BUF_SIZE) {
        delete[] buf;
    }

    return result;
}

class ContextTimer {
public:
    ContextTimer(const std::string& name) : name_(name) {
        start_ = std::chrono::steady_clock::now();
    }

    ContextTimer() : ContextTimer("") {}

    ContextTimer(const ContextTimer&) = delete;
    ContextTimer& operator=(const ContextTimer&) = delete;
    ContextTimer(ContextTimer&&) = delete;
    ContextTimer& operator=(ContextTimer&&) = delete;

    ~ContextTimer() {
        auto dif = std::chrono::steady_clock::now() - start_;
        auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(dif);
        if (!name_.empty()) {
            std::cerr << "[" << name_ << "] ";
        }
        std::cerr << ms.count() << " ms\n";
    }

private:
    std::string name_;
    std::chrono::steady_clock::time_point start_;
};

template<typename F>
auto distribution(int n, F&& f) -> std::map<decltype(f()), int> {
    std::map<decltype(f()), int> dist;
    for (int i = 0; i < n; ++i) {
        ++dist[f()];
    }
    return dist;
}

inline void checkLargeParameter(int n) {
#ifdef JNGEN_I_WANT_LARGE_OBJECTS
    (void)n;
#else
    constexpr static int BOUND = 5e6;
    ensure(
        n <= BOUND,
        "If you want to generate an object of size > 5'000'000, please use "
        "#define JNGEN_I_WANT_LARGE_OBJECTS prior to including Jngen");
#endif // JNGEN_I_WANT_LARGE_OBJECTS
}

// Some type traits helpers. Based on ideas from TCPPPL v4.
template<bool B, typename T>
using enable_if_t = typename std::enable_if<B, T>::type;


} // namespace jngen

using jngen::format;
using jngen::ContextTimer;
using jngen::distribution;
