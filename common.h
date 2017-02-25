#pragma once

#include <cassert>
#include <iostream>

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
