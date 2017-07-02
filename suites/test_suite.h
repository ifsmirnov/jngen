#pragma once

#include "../array.h"
#include "../common.h"

#include <functional>
#include <iostream>
#include <iterator>
#include <string>
#include <vector>

namespace jngen {
namespace suites {

template<typename T, typename ... Args>
class BaseTestSuite {
public:
    explicit BaseTestSuite(const std::string& name) : name(name) {  }

    virtual ~BaseTestSuite() {}

    size_t size() {
        if (producers_.empty()) {
            populate();
        }
        return producers_.size();
    }

    T genSingle(size_t id, Args... args) {
        if (producers_.empty()) {
            populate();
        }
        ensure(
            id < producers_.size(),
            format("Cannot generate test #%d in suite '%s', there are only "
                "'%d'", (int)id, name.c_str(), (int)producers_.size()));
        return producers_[id](args...);
    }

    TArray<T> gen(size_t count, Args... args) {
        if (producers_.empty()) {
            populate();
        }

        ensure(
            count <= producers_.size(),
            format("Cannot generate %d tests in suite '%s', there are only "
                "'%d'", (int)count, name.c_str(), (int)producers_.size()));

        TArray<T> result;
        result.reserve(count);
        for (size_t id = 0; id < count; ++id) {
            try {
                result.push_back(genSingle(id, args...));
            } catch (...) {
                std::cerr << "Cannot generate test #" << id << " of suite "
                    << name << "\n";
            }
        }

        return result;
    }

protected:
    using Producer = std::function<T(Args...)>;

    virtual void populate() = 0;

    std::vector<Producer> producers_;

private:
    std::string name;
};

}} // namespace jngen::suites
