#pragma once

#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <string>

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
        int ret = snprintf(buf, bufSize, fmt.c_str(), args...);
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

} // namespace jngen

using jngen::format;
