#pragma once

#include "array.h"
#include "common.h"
#include "random.h"
#include "rnda.h"

#include <algorithm>
#include <memory>
#include <utility>

namespace jngen {

struct QueryResult {
    enum {
        Single,
        Double,
        Function
    } type;

    std::string prefix = "";
    int first = 0;
    int second = 0;

    std::function<void(void)> function;
    uint64_t savedRandomState = 0;

    QueryResult(int first, std::string prefix = "") :
        type(Single),
        prefix(std::move(prefix)),
        first(first)
    {  }

    QueryResult(int first, int second, std::string prefix = "") :
        type(Double),
        prefix(std::move(prefix)),
        first(first),
        second(second)
    {  }

    QueryResult(std::function<void(void)> function) :
        type(Function),
        function(function),
        savedRandomState(rnd.next64())
    {  }

    QueryResult(const QueryResult&) = default;
    QueryResult& operator=(const QueryResult&) = default;
    QueryResult(QueryResult&&) = default;
    QueryResult& operator=(QueryResult&&) = default;

    friend std::ostream& operator<<(std::ostream& out, const QueryResult& res) {
        if (res.type == Function) {
            auto remRnd = rnd;
            rnd.seed({
                    static_cast<uint32_t>(res.savedRandomState),
                    static_cast<uint32_t>(res.savedRandomState >> 32)});
            res.function();
            rnd = remRnd;
            return out;
        }

        if (!res.prefix.empty()) {
            out << res.prefix << " ";
        }

        // printValue? Look at todo.
        if (res.type == Single) {
            out << res.first;
        } else if (res.type == Double) {
            out << res.first << " " << res.second;
        } else {
            ENSURE(false);
        }

        return out;
    }
};

class GenericQueryBuilder {
public:
    virtual ~GenericQueryBuilder() {}

    GenericQueryBuilder() {}
    GenericQueryBuilder(int n) : range_(0, n) {}
    GenericQueryBuilder(int l, int r) : range_(l, r+1) {}

    QueryResult next() {
        int sel = rnd.next(totalRatio());
        GenericQueryBuilder* ptr = this;
        while (sel >= ptr->ratio_) {
            sel -= ptr->ratio_;
            ptr = ptr->other_.get();
            ENSURE(ptr);
        }
        return ptr->doGenerate();
    }

private:
    int totalRatio() {
        return ratio_ + (other_ ? other_->totalRatio() : 0);
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

    std::pair<int, int> range_; // half-interval
    int ratio_ = 1;
    std::string prefix_ = "";

    // Should be unique_ptr but I don't want to disable copying and who cares.
    std::shared_ptr<GenericQueryBuilder> other_;
};

class PointQueryBuilder;
class SegmentQueryBuilder;
class FunctionQueryBuilder;

template<typename T>
class CrtpQueryBuilder : public GenericQueryBuilder {
public:
    T& prefix(std::string s) {
        prefix_ = std::move(s);
        return static_cast<T&>(*this);
    }

    T& ratio(int ratio) {
        ratio_ = ratio;
        return static_cast<T&>(*this);
    }

    PointQueryBuilder& addPoint();
    SegmentQueryBuilder& addSegment();
    FunctionQueryBuilder& addFunction(std::function<void(void)> func);

protected:
    using CrtpBase = CrtpQueryBuilder<T>;
    using GenericQueryBuilder::GenericQueryBuilder;
};

class PointQueryBuilder : public CrtpQueryBuilder<PointQueryBuilder> {
public:
    using CrtpBase::CrtpBase;

    PointQueryBuilder& setw(int w) {
        w_ = w;
        return *this;
    }

private:
    QueryResult doGenerate() const override {
        return QueryResult(w_, prefix_);
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
PointQueryBuilder& CrtpQueryBuilder<T>::addPoint() {
    auto ptr = std::make_shared<PointQueryBuilder>(
            range_.first, range_.second);
    addChild(ptr);
    return *ptr;
}

template<typename T>
SegmentQueryBuilder& CrtpQueryBuilder<T>::addSegment() {
    auto ptr = std::make_shared<SegmentQueryBuilder>(
            range_.first, range_.second);
    addChild(ptr);
    return *ptr;
}

template<typename T>
FunctionQueryBuilder& CrtpQueryBuilder<T>::addFunction(
        std::function<void(void)> generator)
{
    auto ptr = std::make_shared<FunctionQueryBuilder>(generator);
    addChild(ptr);
    return *ptr;
}

template<typename ... Args>
SegmentQueryBuilder rndq(Args... args) {
    return SegmentQueryBuilder(args...);
}

} // namespace jngen

using jngen::rndq;
