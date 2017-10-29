#pragma once

#include "array.h"
#include "common.h"
#include "random.h"
#include "rnda.h"
#include "weight.h"
#include "printers.h"
#include "repr.h"

#include <algorithm>
#include <memory>
#include <utility>

namespace jngen {

struct QueryResult : public ReprProxy<QueryResult> {
    enum {
        Single,
        Double,
        Function
    } type;

    std::string prefix = "";
    int first = 0;
    int second = 0;
    Weight value;

    // waiting for Variant::operator<
    /*
    bool operator<(const QueryResult& other) const {
        return std::tie(type, prefix, first, second, value) <
            std::tie(other.type, other.prefix, other.first,
                    other.second, other.value);
    }

    bool operator==(const QueryResult& other) const {
        return std::tie(type, prefix, first, second, value) ==
            std::tie(other.type, other.prefix, other.first,
                    other.second, other.value);
    }
    */

    std::function<void(void)> function;
    uint64_t savedRandomState = 0;

    explicit QueryResult(int first) :
        type(Single),
        first(first)
    {  }

    explicit QueryResult(int first, int second) :
        type(Double),
        first(first),
        second(second)
    {  }

    explicit QueryResult(std::function<void(void)> function) :
        type(Function),
        function(function),
        savedRandomState(rnd.next64())
    {  }

    void setWeight(Weight w) {
        value = std::move(w);
    }

    void setPrefix(std::string p) {
        prefix = std::move(p);
    }

    QueryResult(const QueryResult&) = default;
    QueryResult& operator=(const QueryResult&) = default;
    QueryResult(QueryResult&&) = default;
    QueryResult& operator=(QueryResult&&) = default;

    void print(std::ostream& out, OutputModifier mod) const {
        if (type == Function) {
            auto remRnd = rnd;
            rnd.seed({
                    static_cast<uint32_t>(savedRandomState),
                    static_cast<uint32_t>(savedRandomState >> 32)});
            function();
            rnd = remRnd;
            return;
        }

        if (!prefix.empty()) {
            out << prefix << " ";
        }

        // printValue? Look at todo.
        if (type == Single) {
            JNGEN_PRINT(first);
        } else if (type == Double) {
            JNGEN_PRINT(first);
            out << " ";
            JNGEN_PRINT(second);
        } else {
            ENSURE(false);
        }

        if (!value.empty()) {
            out << " ";
            JNGEN_PRINT_NO_MOD(value);
        }
    }
};

namespace detail {

template<>
struct NeedEndlAfterSelf<QueryResult> : public std::true_type {};

} // namespace detail

JNGEN_DECLARE_SIMPLE_PRINTER(QueryResult, 3) {
    t.print(out, mod);
}

class ValueAdder {
public:
    virtual ~ValueAdder() {}
    virtual void addValue(QueryResult&) = 0;
};

class FunctionValueAdder : public ValueAdder {
public:
    FunctionValueAdder(std::function<Weight(void)> func) :
        func_(func)
    {  }

    void addValue(QueryResult& res) override {
        res.setWeight(func_());
    }

private:
    std::function<Weight(void)> func_;
};

template<typename ... Args>
class RndValueAdder : public ValueAdder {
public:
    RndValueAdder(Args... args) :
        func_([=]() {
                return rnd.next(args...);
        })
    {  }

    void addValue(QueryResult& res) override {
        res.setWeight(func_());
    }

private:
    using result_type = decltype(
            std::declval<Random>().next(std::declval<Args>()...));

    std::function<result_type()> func_;
};

class GenericQueryBuilder {
public:
    virtual ~GenericQueryBuilder() {}

    GenericQueryBuilder() {}
    GenericQueryBuilder(int n) : range_(0, n) {}
    GenericQueryBuilder(int l, int r) : range_(l, r+1) {}

    QueryResult next() {
        return nextByIndex(rnd.nextByDistribution(getDistribution()));
    }

    TArray<QueryResult> next(int count) {
        auto dist = getDistribution();
        long long total = std::accumulate(dist.begin(), dist.end(), 0ll);
        int rem = count;
        TArray<QueryResult> res;
        for (size_t i = 0; i < dist.size(); ++i) {
            for (int j = 0; j < 1ll * count * dist[i] / total; ++j) {
                --rem;
                res.push_back(nextByIndex(i));
            }
        }
        ENSURE(rem >= 0);
        while (rem) {
            res.push_back(nextByIndex(rem % dist.size()));
            --rem;
        }
        return res.shuffled();
    }

private:
    QueryResult nextByIndex(size_t index) {
        auto ptr = this;
        while (index) {
            ptr = ptr->other_.get();
            ENSURE(ptr);
            --index;
        }
        auto res = ptr->doGenerate();
        if (ptr->valueAdder_) {
            ptr->valueAdder_->addValue(res);
        }

        res.setPrefix(ptr->prefix_);

        return res;
    }

    std::vector<int> getDistribution() const {
        std::vector<int> dist;
        auto ptr = this;
        while (ptr) {
            dist.push_back(ptr->ratio_);
            ptr = ptr->other_.get();
        }
        return dist;
    }

protected:
    virtual QueryResult doGenerate() const = 0;

    void addChild(std::shared_ptr<GenericQueryBuilder> child) {
        if (other_) {
            other_->addChild(child);
        } else {
            other_ = child;
        }
    }

    void setValueAdder(std::shared_ptr<ValueAdder> valueAdder) {
        if (other_) {
            other_->setValueAdder(valueAdder);
        } else {
            valueAdder_ = valueAdder;
        }
    }

    std::pair<int, int> range_; // half-interval
    int ratio_ = 1;
    std::string prefix_ = "";

private:
    // Should be unique_ptr but I don't want to disable copying and who cares.
    std::shared_ptr<GenericQueryBuilder> other_;
    std::shared_ptr<ValueAdder> valueAdder_;
};

class PointQueryBuilder;
class SegmentQueryBuilder;
class FunctionQueryBuilder;

template<typename T>
class CrtpQueryBuilder : public GenericQueryBuilder {
public:
    T&& prefix(std::string s) {
        prefix_ = std::move(s);
        return static_cast<T&&>(*this);
    }

    T&& ratio(int ratio) {
        ratio_ = ratio;
        return static_cast<T&&>(*this);
    }

    T&& withValue(std::function<Weight(void)> func);
    template<typename ... Args>
    T&& withRandomValue(Args... args);

    PointQueryBuilder& addPoint() &;
    SegmentQueryBuilder& addSegment() &;
    FunctionQueryBuilder& addFunction(std::function<void(void)> func) &;

protected:
    using CrtpBase = CrtpQueryBuilder<T>;
    using GenericQueryBuilder::GenericQueryBuilder;
};

class PointQueryBuilder : public CrtpQueryBuilder<PointQueryBuilder> {
public:
    using CrtpBase::CrtpBase;

private:
    QueryResult doGenerate() const override {
        return QueryResult(rnd.next(range_.first, range_.second));
    }

    int w_;
};

class SegmentQueryBuilder : public CrtpQueryBuilder<SegmentQueryBuilder> {
public:
    SegmentQueryBuilder(int n) :
        CrtpBase(n),
        lenRange_(1, n)
    {  }

    SegmentQueryBuilder(int l, int r) :
        CrtpBase(l, r + 1),
        lenRange_(1, r - l + 1)
    {  }

    SegmentQueryBuilder& minLen(int value) {
        lenRange_.first = value;
        return *this;
    }

    SegmentQueryBuilder& maxLen(int value) {
        lenRange_.second = value;
        return *this;
    }

    SegmentQueryBuilder& small() {
        queryType_ = QueryType::Small;
        return *this;
    }

    SegmentQueryBuilder& large() {
        queryType_ = QueryType::Large;
        return *this;
    }

    SegmentQueryBuilder& ordered(bool value) {
        ordered_ = value;
        return *this;
    }

private:
    enum class QueryType {
        Default, Large, Small
    };

    QueryResult doGenerate() const override {
        switch (queryType_) {
        case QueryType::Default: {
            // This is inaccurate to say the least. I don't know how to
            // generate a random segment with length from l to r without
            // calling sqrt.
            int len = rnd.wnext(lenRange_.first, lenRange_.second, -1);
            int l = rnd.next(range_.first, range_.second - len);
            if (ordered_ || rnd.next(2)) {
                return QueryResult(l, l + len - 1);
            } else {
                return QueryResult(l + len - 1, l);
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

    QueryType queryType_ = QueryType::Default;

    std::pair<int, int> lenRange_; // segment
    bool ordered_ = true;

};

class FunctionQueryBuilder : public CrtpQueryBuilder<FunctionQueryBuilder> {
public:
    FunctionQueryBuilder(std::function<void(void)> generator) :
        generator_(generator)
    {  }

private:
    QueryResult doGenerate() const override {
        return QueryResult(generator_);
    }

    std::function<void(void)> generator_;
};

template<typename T>
PointQueryBuilder& CrtpQueryBuilder<T>::addPoint() & {
    auto ptr = std::make_shared<PointQueryBuilder>(
            range_.first, range_.second);
    addChild(ptr);
    return *ptr;
}

template<typename T>
SegmentQueryBuilder& CrtpQueryBuilder<T>::addSegment() & {
    auto ptr = std::make_shared<SegmentQueryBuilder>(
            range_.first, range_.second);
    addChild(ptr);
    return *ptr;
}

template<typename T>
FunctionQueryBuilder& CrtpQueryBuilder<T>::addFunction(
        std::function<void(void)> generator) &
{
    auto ptr = std::make_shared<FunctionQueryBuilder>(generator);
    addChild(ptr);
    return *ptr;
}

template<typename T>
T&& CrtpQueryBuilder<T>::withValue(std::function<Weight(void)> func) {
    setValueAdder(std::make_shared<FunctionValueAdder>(func));
    return static_cast<T&&>(*this);
}
template<typename T>
template<typename ... Args>
T&& CrtpQueryBuilder<T>::withRandomValue(Args... args) {
    setValueAdder(std::make_shared<RndValueAdder<Args...>>(args...));
    return static_cast<T&&>(*this);
}

struct Rndq {
    template<typename ... Args>
    PointQueryBuilder point(Args... args) {
        return PointQueryBuilder(args...);
    }

    template<typename ... Args>
    SegmentQueryBuilder segment(Args... args) {
        return SegmentQueryBuilder(args...);
    }

    template<typename ... Args>
    FunctionQueryBuilder function(Args... args) {
        return FunctionQueryBuilder(args...);
    }
};

JNGEN_EXTERN Rndq rndq;

} // namespace jngen

using jngen::rndq;
