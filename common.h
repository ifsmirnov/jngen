#pragma once

#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#define JNGEN_ENSURE1(cond)\
    assert(cond)

#define JNGEN_ENSURE2(cond, msg)\
do\
    if (!(cond)) {\
        std::cerr << "Error: " << msg << std::endl;\
        assert(cond);\
    }\
while (false)

#define JNGEN_GET_MACRO(_1, _2, NAME, ...) NAME

#define ensure(...) JNGEN_GET_MACRO(__VA_ARGS__, JNGEN_ENSURE2, JNGEN_ENSURE1)\
    (__VA_ARGS__)

namespace jngen {

template<typename ... Args>
std::string format(const std::string& format, Args... args) {
    constexpr static char BUF_SIZE = 64;
    static char BUFFER[BUF_SIZE];

    int bufSize = BUF_SIZE;
    char *buf = BUFFER;

    while (true) {
        int ret = snprintf(buf, bufSize, format.c_str(), args...);
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
