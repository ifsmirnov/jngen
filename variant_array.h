#pragma once

#include "array.h"
#include "repr.h"
#include "variant.h"

#include <iterator>
#include <vector>
#include <type_traits>

namespace jngen {

template<typename ... Args>
class VariantArray : public GenericArray<Variant<Args...>> {
public:
    using Base = GenericArray<Variant<Args...>>;
    using BaseVariant = Variant<Args...>;

    using Base::Base;

    VariantArray() {}

    /* implicit */ VariantArray(const Base& base) :
            Base(base)
    {  }

    template<typename T, typename = typename std::enable_if<
        BaseVariant::template hasType<T>()>::type>
    VariantArray(const std::vector<T>& other) {
        std::copy(other.begin(), other.end(), std::back_inserter(*this));
    }

    template<typename T, typename = typename std::enable_if<
        BaseVariant::template hasType<T>()>::type>
    VariantArray(std::vector<T>&& other) {
        std::move(other.begin(), other.end(), std::back_inserter(*this));
        GenericArray<T>().swap(other);
    }

    template<typename T, typename = typename std::enable_if<
        BaseVariant::template hasType<T>()>::type>
    operator GenericArray<T>() const
    {
        return GenericArray<T>(this->begin(), this->end());
    }

    bool hasNonEmpty() const {
        for (const auto& x: *this) {
            if (!x.empty()) {
                return true;
            }
        }
        return false;
    }

    int anyType() const {
        for (const auto& x: *this) {
            if (!x.empty()) {
                return x.type();
            }
        }
        return 0;
    }

};

} // namespace jngen
