#pragma once

#include "../array.h"
#include "../common.h"
#include "../hash.h"

#include <algorithm>
#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <unordered_set>
#include <vector>

namespace jngen {
namespace suites {

#define JNGEN_ADD_PRODUCER(...)\
    {\
        std::string name = #__VA_ARGS__;\
        if (name.empty()) {\
            name = format("noname%d", (int)names_.size());\
        }\
        if (std::find(names_.begin(), names_.end(), name) != names_.end()) {\
            ENSURE(false, format("Duplicated test name: '%s'", name.c_str()));\
        }\
        names_.emplace_back(name);\
    }\
    *std::back_inserter(producers_) = [] (JNGEN_PRODUCER_ARGS)

template<typename T, typename ... Args>
class BaseTestSuite {
public:
    explicit BaseTestSuite(const std::string& name) : name_(name) {  }

    size_t size() const {
        return producers_.size();
    }

    TArray<std::string> names() const {
        return names_;
    }

    T gen(size_t id, Args... args) const {
        ensure(
            0 < id && id <= producers_.size(),
            format("Cannot generate test #%d in suite '%s', valid numbers are "
                " from 1 to %d",
                (int)id, name_.c_str(), (int)producers_.size()));
        return producers_[id](args...);
    }

    T gen(const std::string& name, Args... args) const {
        size_t pos =
            std::find(names_.begin(), names_.end(), name) - names_.begin();
        ensure(
            pos < names_.size(),
            format("There is no test '%s' in suite '%s'",
                name.c_str(), name_.c_str()));
        return gen(pos, args...);
    }

    TArray<T> genMany(size_t count, Args... args) const {
        ensure(
            count <= producers_.size(),
            format("Cannot generate test #%d in suite '%s', valid numbers are "
                " from 1 to %d",
                (int)count, name_.c_str(), (int)producers_.size()));

        TArray<T> result;
        result.reserve(count);
        for (size_t id = 1; id <= count; ++id) {
            try {
                result.push_back(gen(id, args...));
            } catch (...) {
                std::cerr << "Failed to generate test #" << id << " of suite "
                    << name_ << "\n";
                throw;
            }
        }

        return result;
    }

protected:
    using Producer = std::function<T(Args...)>;

    std::vector<Producer> producers_;
    std::vector<std::string> names_;

private:
    std::string name_;
};

}} // namespace jngen::suites
