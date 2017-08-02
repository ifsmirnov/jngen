#pragma once

#include "array.h"
#include "common.h"
#include "random.h"
#include "rnda.h"

#include <algorithm>
#include <utility>

namespace jngen {

class QueryBuilder {
public:
    QueryBuilder() {}

    QueryBuilder(int n);
    QueryBuilder(int l, int r);

    QueryBuilder& minLen(int value);
    QueryBuilder& maxLen(int value);
    QueryBuilder& range(int n);
    QueryBuilder& range(int l, int r);
    QueryBuilder& small();
    QueryBuilder& large();
    QueryBuilder& ordered(bool value);

    std::pair<int, int> next();
    Arrayp next(int m);

private:
    enum class QueryType {
        Default, Large, Small
    };

    QueryType queryType_ = QueryType::Default;

    std::pair<int, int> range_; // half-interval
    std::pair<int, int> lenRange_; // segment
    bool ordered_;
};

#ifndef JNGEN_DECLARE_ONLY

QueryBuilder::QueryBuilder(int n) :
    QueryBuilder(0, n - 1)
{  }

QueryBuilder::QueryBuilder(int l, int r) :
    range_(l, r + 1),
    lenRange_(1, r - l + 1),
    ordered_(true)
{
    ensure(l <= r);
}

QueryBuilder& QueryBuilder::minLen(int value) {
    lenRange_.first = value;
    return *this;
}

QueryBuilder& QueryBuilder::maxLen(int value) {
    lenRange_.second = value;
    return *this;
}

QueryBuilder& QueryBuilder::range(int n) {
    ensure(n > 0);
    return this->range(0, n - 1);
}

QueryBuilder& QueryBuilder::range(int l, int r) {
    ensure(l <= r);
    range_ = {l, r+1};
    lenRange_.second = std::min(lenRange_.second, r - l + 1);
    lenRange_.first = std::min(lenRange_.first, lenRange_.second);
    return *this;
}

QueryBuilder& QueryBuilder::small() {
    queryType_ = QueryType::Small;
    return *this;
}

QueryBuilder& QueryBuilder::large() {
    queryType_ = QueryType::Large;
    return *this;
}

QueryBuilder& QueryBuilder::ordered(bool value) {
    ordered_ = value;
    return *this;
}

std::pair<int, int> QueryBuilder::next() {
    switch (queryType_) {
    case QueryType::Default: {
        // This is inaccurate to say the least. I don't know how to
        // generate a random segment with length from l to r without
        // calling sqrt.
        int len = rnd.wnext(lenRange_.first, lenRange_.second, -1);
        int l = rnd.next(range_.first, range_.second - len);
        if (ordered_ || rnd.next(2)) {
            return {l, l + len - 1};
        } else {
            return {l + len - 1, l};
        }
    }
    case QueryType::Large: {
        ENSURE(false, "not implemented");
        break;
    }
    case QueryType::Small: {
        ENSURE(false, "not implemented");
        break;
    }
    default: ENSURE(false, "Nonexistent option");
    }
}

Arrayp QueryBuilder::next(int m) {
    return rnda.randomf(m, [this]() { return next(); });
}

#endif

template<typename ... Args>
QueryBuilder rndq(Args... args) {
    return QueryBuilder(args...);
}

} // namespace jngen

using jngen::rndq;
