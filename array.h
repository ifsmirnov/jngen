#pragma once
#include <bits/stdc++.h>

#include "common.h"
#include "random.h"
#include "named_arguments.h"

namespace impl {

typedef std::pair<size_t, size_t> Range;

template<typename T>
class GenericArray : public std::vector<T> {
public:
    typedef std::vector<T> Base;

    GenericArray() {}
    GenericArray(const GenericArray<T>&) = default;
    GenericArray& operator=(const GenericArray<T>&) = default;
    /* implicit */ GenericArray(const Base& base) :
            Base(base)
    {  }
    template<typename... Args>
    GenericArray(Args... args) :
        Base(args...)
    {  }

    // syntaxic sugar for the god of C++
    size_t size() const {
        return Base::size();
    }
    T& at(size_t idx) {
        return Base::at(idx);
    }
    const T& at(size_t idx) const {
        return Base::at(idx);
    }

    static GenericArray<T> random(size_t size, T max);
    static GenericArray<T> random(size_t size, T min, T max);
    static GenericArray<T> random(const Range& size, T max);
    static GenericArray<T> random(const Range& size, T min, T max);

    GenericArray<T>& shuffle();
    GenericArray<T>& reverse();
};

template<typename T>
GenericArray<T> GenericArray<T>::random(size_t size, T max) {
    GenericArray<T> result(size);
    for (T& x: result) {
        x = rnd.next(max);
    }
    return result;
}

template<typename T>
GenericArray<T> GenericArray<T>::random(size_t size, T min, T max) {
    GenericArray<T> result(size);
    for (T& x: result) {
        x = rnd.next(min, max);
    }
    return result;
}

template<typename T>
GenericArray<T> GenericArray<T>::random(const Range& size, T max) {
    return GenericArray<T>::random(rnd.next(size.first, size.second), max);
}

template<typename T>
GenericArray<T> GenericArray<T>::random(const Range& size, T min, T max) {
    return GenericArray<T>::random(rnd.next(size.first, size.second), min, max);
}

template<typename T>
GenericArray<T>& GenericArray<T>::reverse() {
    std::reverse(this->begin(), this->end());
    return *this;
}

template<typename T>
GenericArray<T>& GenericArray<T>::shuffle() {
    for (size_t i = 1; i < size(); ++i) {
        std::swap(at(i), at(rnd.next(i + 1)));
    }
    return *this;
}

template<typename T>
const T& choice(const GenericArray<T>& array) {
    return array[rnd.next(array.size())];
}

template<typename T, typename... Traits>
const GenericArray<T> choice(
        const GenericArray<T>& array,
        size_t count,
        Traits... traits)
{
    TraitMap map = collectTraits(traits...);
    GenericArray<T> result(count);

    if (map.count("allowRepeats") && (int)map.at("allowRepeats")) {
        for (size_t i = 0; i < count; ++i) {
            result[i] = choice(array);
        }
    } else {
        ensure(count <= array.size());
        std::vector<size_t> indices = rnd.combination(array.size(), count);
        for (size_t i = 0; i < count; ++i) {
            result[i] = array[indices[i]];
        }
    }

    return result;
}

template<typename T>
using ArrayRepresentation = std::pair<GenericArray<T>, TraitMap>;

template<typename T, typename... Traits>
ArrayRepresentation<T> repr(const GenericArray<T>& array, Traits... traits) {
    return { array, collectTraits(traits...) };
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const ArrayRepresentation<T>& repr) {
    const GenericArray<T>& array = repr.first;
    const TraitMap& map = repr.second;

    if (!map.count("printSize") || (int)map.at("printSize")) {
        out << array.size() << "\n";
    }

    int addition = map.count("addOne") && (int)map.at("addOne") ? 1 : 0;
    const std::string sep = map.count("sep") ? (std::string)map.at("sep") : " ";

    bool first = true;
    for (const T& x: array) {
        if (!first) {
            out << sep;
        } else {
            first = false;
        }
        out << x + addition;
    }
    return out;
}

template<typename T>
std::ostream& operator<<(std::ostream& out, const GenericArray<T>& array) {
    return out << repr(array);
}


typedef GenericArray<int> Array;
typedef GenericArray<long long> Array64;
typedef GenericArray<double> Arrayf;

} // namespace impl

using impl::Array;
using impl::Array64;
using impl::Arrayf;

DECLARE_NAMED_PARAMETER(printSize);
DECLARE_NAMED_PARAMETER(sep);
DECLARE_NAMED_PARAMETER(addOne);
DECLARE_NAMED_PARAMETER(allowRepeats);
