#pragma once

#include "common.h"
#include "hash.h"
#include "printers.h"
#include "random.h"
#include "sequence_ops.h"

#include <algorithm>
#include <initializer_list>
#include <numeric>
#include <set>
#include <string>
#include <type_traits>
#include <unordered_set>
#include <unordered_map>
#include <utility>
#include <vector>

namespace jngen {

template<typename T>
class GenericArray : public ReprProxy<GenericArray<T>>, public std::vector<T> {
public:
    typedef std::vector<T> Base;

    using Base::Base;

    GenericArray() {}
    GenericArray(const GenericArray<T>&) = default;
    GenericArray& operator=(const GenericArray<T>&) = default;
    GenericArray(GenericArray<T>&&) = default;
    GenericArray& operator=(GenericArray<T>&&) = default;

    ~GenericArray() {}

    /* implicit */ GenericArray(const Base& base) :
            Base(base)
    {  }

    using Base::at;
    using Base::size;
    using Base::resize;
    using Base::begin;
    using Base::end;
    using Base::insert;
    using Base::clear;
    using Base::erase;

    void extend(size_t requiredSize) {
        checkLargeParameter(requiredSize);
        if (requiredSize > size()) {
            resize(requiredSize);
        }
    }

    template<typename F, typename ...Args>
    static GenericArray<T> randomf(size_t size, F func, const Args& ... args);
    template<typename F, typename ...Args>
    static GenericArray<T> randomfUnique(
            size_t size, F func, const Args& ... args);
    template<typename F, typename ...Args>
    static GenericArray<T> randomfAll(F func, const Args& ... args);

    template<typename ...Args>
    static GenericArray<T> random(size_t size, const Args& ... args);
    template<typename ...Args>
    static GenericArray<T> randomUnique(size_t size, const Args& ... args);
    template<typename ...Args>
    static GenericArray<T> randomAll(const Args& ... args);

    static GenericArray<T> id(size_t size, T start = T{});

    GenericArray<T>& shuffle();
    GenericArray<T> shuffled() const;

    GenericArray<T>& reverse();
    GenericArray<T> reversed() const;

    GenericArray<T>& sort();
    GenericArray<T> sorted() const;

    template<typename Comp>
    GenericArray<T>& sort(Comp&& comp);
    template<typename Comp>
    GenericArray<T> sorted(Comp&& comp) const;

    GenericArray<T>& unique();
    GenericArray<T> uniqued() const;

    GenericArray<T> inverse() const;

    template<typename Integer>
    GenericArray<T> subseq(const std::vector<Integer>& indices) const;

    template<typename Integer>
    GenericArray<T> subseq(
        const std::initializer_list<Integer>& indices) const;

    T choice() const;
    GenericArray<T> choice(size_t count) const;
    GenericArray<T> choiceWithRepetition(size_t count) const;

    GenericArray<T>& operator+=(const GenericArray<T>& other);
    GenericArray<T> operator+(const GenericArray<T>& other) const;

    GenericArray<T>& operator*=(int k);
    GenericArray<T> operator*(int k) const;

    operator std::string() const;
};

template<typename T>
template<typename ...Args>
GenericArray<T> GenericArray<T>::random(size_t size, const Args& ... args) {
    checkLargeParameter(size);
    GenericArray<T> result(size);
    for (T& x: result) {
        x = rnd.tnext<T>(args...);
    }
    return result;
}

template<typename T>
template<typename F, typename ...Args>
GenericArray<T> GenericArray<T>::randomf(
        size_t size,
        F func,
        const Args& ... args)
{
    checkLargeParameter(size);
    GenericArray<T> result(size);
    for (T& x: result) {
        x = func(args...);
    }
    return result;
}

namespace detail {

template<typename T, typename Enable = std::size_t>
struct DictContainer {
    typedef std::set<T> type;
};

template<typename T>
struct DictContainer<T, typename std::hash<T>::result_type>
{
    typedef std::unordered_set<T> type;
};

} // namespace detail

template<typename T>
template<typename F, typename ...Args>
GenericArray<T> GenericArray<T>::randomfUnique(
        size_t size,
        F func,
        const Args& ... args)
{
    typename detail::DictContainer<T>::type set;
    checkLargeParameter(size);
    GenericArray<T> result;
    result.reserve(size);

    size_t retries = (size + 10) * log(size + 10) * 2;

    while (result.size() != size) {
        T t = func(args...);
        if (!set.count(t)) {
            set.insert(t);
            result.push_back(t);
        }

        if (--retries == 0) {
            ensure(false, "There are not enough unique elements");
        }

    }

    return result;
}

template<typename T>
template<typename ...Args>
GenericArray<T> GenericArray<T>::randomUnique(
        size_t size, const Args& ... args)
{
    return GenericArray<T>::randomfUnique(
        size,
        [](Args... args) { return rnd.tnext<T>(args...); },
        args...);
}

template<typename T>
template<typename F, typename ...Args>
GenericArray<T> GenericArray<T>::randomfAll(
        F func,
        const Args& ... args)
{
    typename detail::DictContainer<T>::type set;
    GenericArray<T> result;

    size_t timeAfterLastHit = 0;

    while (true) {
        T t = func(args...);
        if (!set.count(t)) {
            set.insert(t);
            result.push_back(t);
            timeAfterLastHit = 0;
        }

        ++timeAfterLastHit;

        // Probability of finding not all elements is about e^{-20} ~= 1e-9
        if (timeAfterLastHit > (result.size() + 10) * 20) {
            return result;
        }
    }
}

template<typename T>
template<typename ...Args>
GenericArray<T> GenericArray<T>::randomAll(const Args& ... args)
{
    return GenericArray<T>::randomfAll(
        [](Args... args) { return rnd.tnext<T>(args...); },
        args...);
}

template<typename T>
GenericArray<T> GenericArray<T>::id(size_t size, T start) {
    constexpr bool enable = std::is_integral<T>::value;
    static_assert(enable, "Cannot call Array<T>::id with non-integral T");
    checkLargeParameter(size);

    if (enable) {
        GenericArray<T> result(size);
        std::iota(result.begin(), result.end(), start);
        return result;
    } else {
        return {};
    }
}

template<typename T>
GenericArray<T>& GenericArray<T>::shuffle() {
    jngen::shuffle(begin(), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::shuffled() const {
    auto res = *this;
    res.shuffle();
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::reverse() {
    std::reverse(begin(), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::reversed() const {
    auto res = *this;
    res.reverse();
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::sort() {
    std::sort(begin(), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::sorted() const {
    auto res = *this;
    res.sort();
    return res;
}

template<typename T>
template<typename Comp>
GenericArray<T>& GenericArray<T>::sort(Comp&& comp) {
    std::sort(begin(), end(), comp);
    return *this;
}

template<typename T>
template<typename Comp>
GenericArray<T> GenericArray<T>::sorted(Comp&& comp) const {
    auto res = *this;
    res.sort(comp);
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::unique() {
    erase(std::unique(begin(), end()), end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::uniqued() const {
    auto res = *this;
    res.unique();
    return res;
}

template<typename T>
GenericArray<T> GenericArray<T>::inverse() const {
    static_assert(
        std::is_integral<T>::value,
        "Can only take inverse permutation of integral array");
    int n = size();

    if (n == 0) {
        return *this;
    }

    // sanity check
    ensure(*max_element(begin(), end()) == n-1 &&
        *min_element(begin(), end()) == 0,
        "Trying to take inverse of the array which is not a permutation");

    const static T NONE = static_cast<T>(-1);
    GenericArray<T> result(n, NONE);
    for (int i = 0; i < n; ++i) {
        ensure(result[at(i)] == NONE,
            "Trying to take inverse of the array which is not a permutation");
        result[at(i)] = i;
    }

    return result;
}

template<typename T>
template<typename Integer>
GenericArray<T> GenericArray<T>::subseq(
        const std::vector<Integer>& indices) const
{
    GenericArray<T> result;
    result.reserve(indices.size());
    for (Integer idx: indices) {
        result.push_back(at(idx));
    }
    return result;
}

// TODO(ifsmirnov): ever need to make it faster?
template<typename T>
template<typename Integer>
GenericArray<T> GenericArray<T>::subseq(
        const std::initializer_list<Integer>& indices) const
{
    return subseq(std::vector<T>(indices));
}

template<typename T>
T GenericArray<T>::choice() const {
    return jngen::choice(begin(), end());
}

template<typename T>
GenericArray<T> GenericArray<T>::choice(size_t count) const {
    ensure(
        count <= size(),
        "Use Array::choiceWithRepetition to select more than size() elements");

    size_t n = size();

    std::unordered_map<size_t, size_t> used;
    std::vector<size_t> res;
    for (size_t i = 0; i < count; ++i) {
        size_t oldValue = used.count(n-i-1) ? used[n-i-1] : n-i-1;
        size_t index = rnd.tnext<size_t>(n-i);
        res.push_back(used.count(index) ? used[index] : index);
        used[index] = oldValue;
    }

    return subseq(res);
}

template<typename T>
GenericArray<T> GenericArray<T>::choiceWithRepetition(size_t count) const {
    checkLargeParameter(count);
    GenericArray<T> res(count);
    for (T& t: res) {
        t = choice();
    }
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::operator+=(const GenericArray<T>& other) {
    if (&other == this) {
        return *this *= 2;
    }
    insert(end(), other.begin(), other.end());
    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::operator+(const GenericArray<T>& other) const {
    GenericArray<T> copy(*this);
    return copy += other;
}

template<typename T>
GenericArray<T>& GenericArray<T>::operator*=(int k) {
    if (k == 0) {
        clear();
        return *this;
    }

    this->reserve(size() * k);

    std::copy_n(begin(), size() * (k - 1), std::back_inserter(*this));

    return *this;
}

template<typename T>
GenericArray<T> GenericArray<T>::operator*(int k) const {
    GenericArray<T> copy(*this);
    return copy *= k;
}

template<typename T>
GenericArray<T>::operator std::string() const {
    static_assert(std::is_same<T, char>::value, "Must not cast"
        " TArray<T> to std::string with 'T' != 'char'");
    return std::string(begin(), end());
}

// JNGEN_EXTERN template class GenericArray<int>;

template<typename T>
using TArray = GenericArray<T>;

using Array = GenericArray<int>;
using Array2d = GenericArray<jngen::GenericArray<int>>;
using Array64 = GenericArray<long long>;
using Arrayf = GenericArray<double>;
using Arrayp = GenericArray<std::pair<int, int>>;

template<typename T>
jngen::GenericArray<T> makeArray(const std::vector<T>& values) {
    return jngen::GenericArray<T>(values);
}

template<typename T>
jngen::GenericArray<T> makeArray(const std::initializer_list<T>& values) {
    return jngen::GenericArray<T>(values);
}

template<typename T, typename U>
TArray<std::pair<T, U>> zip(const TArray<T>& lhs, const TArray<U>& rhs) {
    ensure(
        lhs.size() == rhs.size(),
        "In zip(a, b), a and b must have the same size");
    TArray<std::pair<T, U>> result;
    for (size_t i = 0; i < lhs.size(); ++i) {
        result.emplace_back(lhs[i], rhs[i]);
    }
    return result;
}

template<typename T, typename U>
TArray<T> arrayCast(const TArray<U>& array) {
    return TArray<T>(array.begin(), array.end());
}

template<typename T>
struct Hash<TArray<T>> {
    uint64_t operator()(const TArray<T>& elements) const {
        return Hash<std::vector<T>>{}(elements);
    }
};

} // namespace jngen

JNGEN_DEFINE_STD_HASH_TEMPLATE(T, jngen::TArray<T>);

using jngen::makeArray;
using jngen::zip;
using jngen::arrayCast;

using jngen::TArray;

using jngen::Array;
using jngen::Array2d;
using jngen::Array64;
using jngen::Arrayf;
using jngen::Arrayp;
