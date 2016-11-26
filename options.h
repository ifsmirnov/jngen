#pragma once

#include "common.h"

#include <bits/stdc++.h>

namespace impl {

namespace detail {

template<typename T>
void readVariable(T& var, const std::string& value) {
    std::istringstream ss(value);
    ss >> var;

    ensure(ss, "Failed to parse a value from a command line argument");
}

typedef std::vector<std::string>::const_iterator OptionIterator;

void getopts(OptionIterator)
{  }

template<typename T, typename ... Args>
void getopts(OptionIterator iter, T& var, Args& ...args) {
    readVariable(var, *iter);
    getopts(++iter, args...);
}

} // namespace detail

template<typename ... Args>
void getopts(const std::vector<std::string>& options, Args& ...args) {
    ensure(options.size() >= sizeof...(args), "Too few command-line arguments");
    detail::getopts(options.cbegin(), args...);
}

template<typename ... Args>
void getopts(int argc, char *argv[], Args& ...args) {
    return getopts(std::vector<std::string>(argv + 1, argv + argc), args...);
}

} // namespace impl

using impl::getopts;
