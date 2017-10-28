#pragma once

#include "common.h"

#include <iterator>
#include <sstream>
#include <string>

namespace jngen {
namespace options {

struct Range {
    int first;
    int last;
    int step;

    Range(int first, int last, int step = 1) :
        first(first),
        last(last),
        step(step)
    {
        ensure(first <= last);
        ensure(step > 0);
    }

    class iterator {
    public:
        using iterator_category = std::forward_iterator_tag;
        using value_type = int;
        using reference = const int&;
        using pointer = const int*;
        using difference_type = int;

        iterator(int value, int step, int last) :
            value_(value),
            step_(step),
            last_(last)
        {  }

        iterator& operator++() {
            ensure(value_ <= last_, "Cannot increment past-the-end iterator");
            value_ += step_;
            return *this;
        }

        iterator operator++(int) {
            iterator copy = *this;
            ++*this;
            return copy;
        }

        int operator*() const {
            ensure(value_ <= last_, "Cannot dereference past-the-end iterator");
            return value_;
        }

        bool operator==(const iterator& other) const {
            if (value_ <= last_) {
                return value_ == other.value_;
            }
            return other.value_ > last_;
        }

        bool operator!=(const iterator& other) const {
            return !(*this == other);
        }

    private:
        int value_;
        int step_;
        int last_;
    };

    iterator begin() const {
        return { first, step, last };
    }

    iterator end() const {
        return { last + 1, step, last };
    }

    static Range fromString(std::string s) {
        for (char& c: s) {
            ensure(c != ' ', "No spaces allowed in range description");
            if (c == ':') {
                c = ' ';
            }
        }

        std::istringstream ss(s);
        int first = 1;
        int last = std::numeric_limits<int>::max() - 1;
        int step = 1;
        if (!(ss >> first)) {
            ensure(false, "Failed to parse range");
        }
        if (!(ss >> last)) {
            return Range(first, last, step);
        }
        if (!(ss >> step)) {
            return Range(first, last, step);
        }
        return Range(first, last, step);
    }
};

} // namespace options
} // namespace jngen
