#pragma once

#include "repr.h"
#include "printers.h"

#include <iostream>
#include <stdexcept>
#include <type_traits>

namespace jngen {

namespace variant_detail {

constexpr static int NO_TYPE = -1;

template<size_t Size, size_t Align, typename ... Args>
class VariantImpl;

template<size_t Size, size_t Align>
class VariantImpl<Size, Align> {
public:
    VariantImpl() {
        type_ = NO_TYPE;
    }

private:
    alignas(Align) char data_[Size];
    int type_;

protected:
    int& type() { return type_; }
    int type() const { return type_; }

    char* data() { return data_; }
    const char* data() const { return data_; }

    void doDestroy() {
        throw;
    }

    template<typename P>
    constexpr static int typeId() {
        return NO_TYPE;
    }

    void copy(char*) const {
        throw;
    }

    void move(char*) const {
        throw;
    }

    void setType(int) {
        throw;
    }

    template<typename V>
    typename V::return_type applyVisitor(V&&) const {
        throw;
    }

    void assign() {}
};

template<size_t Size, size_t Align, typename T, typename ... Args>
class VariantImpl<Size, Align, T, Args...> : public VariantImpl<
        (sizeof(T) > Size ? sizeof(T) : Size),
        (alignof(T) > Align ? alignof(T) : Align),
        Args...
    >
{
    using Base = VariantImpl<
        (sizeof(T) > Size ? sizeof(T) : Size),
        (alignof(T) > Align ? alignof(T) : Align),
        Args...
    >;

    constexpr static int MY_ID = sizeof...(Args);

protected:
    void doDestroy() {
        if (this->type() == MY_ID) {
            this->type() = NO_TYPE;
            reinterpret_cast<T*>(this->data())->~T();
        } else {
            Base::doDestroy();
        }
    }

    template<typename P>
    constexpr static int typeId() {
        return std::is_same<P, T>::value ?
            MY_ID :
            Base::template typeId<P>();
    }

    void copy(char* dst) const {
        if (this->type() == MY_ID) {
            new(dst) T(*reinterpret_cast<const T*>(this->data()));
        } else {
            Base::copy(dst);
        }
    }

    void move(char* dst) const {
        if (this->type() == MY_ID) {
            new(dst) T(std::move(*reinterpret_cast<const T*>(this->data())));
        } else {
            Base::copy(dst);
        }
    }

    void setType(int typeIndex) {
        if (typeIndex == MY_ID) {
            if (this->type() != NO_TYPE) {
                throw;
            }
            assign(T{});
        } else {
            Base::setType(typeIndex);
        }
    }

    template<typename V>
    typename V::return_type applyVisitor(V&& v) const {
        if (this->type() == MY_ID) {
            return v(*reinterpret_cast<const T*>(this->data()));
        } else {
            return Base::applyVisitor(std::forward<V>(v));
        }
    }

    using Base::assign;

    void assign(const T& t) {
        if (this->type() == NO_TYPE) {
            new(this->data()) T;
            this->type() = MY_ID;
        }

        ref() = t;
    }

private:
    T& ref() { return *reinterpret_cast<T*>(this->data()); }

public:
    operator T() const {
        if (this->type() == MY_ID) {
            return *reinterpret_cast<const T*>(this->data());
        } else {
            return T();
        }
    }
};

template<typename ... Args>
class Variant : public VariantImpl<0, 1, Args...> {
    using Base = VariantImpl<0, 1, Args...>;

public:
    Variant() { }

    Variant(const Variant<Args...>& other) {
        if (other.type() != NO_TYPE) {
            other.copy(this->data());
            unsafeType() = other.type();
        }
    }

    Variant& operator=(const Variant<Args...>& other) {
        if (&other == this) {
            return *this;
        }
        if (this->type() != NO_TYPE) {
            this->doDestroy();
        }
        if (other.type() != NO_TYPE) {
            other.copy(this->data());
            unsafeType() = other.type();
        }
        return *this;
    }

    Variant(Variant<Args...>&& other) {
        if (other.type() != NO_TYPE) {
            other.move(this->data());
            unsafeType() = other.type();
        } else {
            unsafeType() = other.type();
        }
    }

    Variant& operator=(Variant<Args...>&& other) {
        if (&other == this) {
            return *this;
        }
        if (this->type() != NO_TYPE) {
            this->doDestroy();
        }
        if (other.type() != NO_TYPE) {
            other.move(this->data());
            unsafeType() = other.type();
        }
        return *this;
    }

    ~Variant() {
        if (type() != NO_TYPE) {
            this->doDestroy();
        }
    }

    template<typename T>
    Variant(const T& t) : Variant() {
        this->assign(t);
    }

    template<typename T>
    T& ref() {
        return *ptr<T>();
    }

    template<typename T>
    const T& cref() const {
        auto ptr = cptr<T>();
        if (ptr == 0) {
            throw std::logic_error("jngen::Variant: taking a reference for"
                " a type which is not active now");
        }
        return *ptr;
    }

    bool operator==(const Variant& v) const { return compareTo(v) == 0; }
    bool operator!=(const Variant& v) const { return compareTo(v) != 0; }
    bool operator< (const Variant& v) const { return compareTo(v) <  0; }
    bool operator> (const Variant& v) const { return compareTo(v) >  0; }
    bool operator<=(const Variant& v) const { return compareTo(v) <= 0; }
    bool operator>=(const Variant& v) const { return compareTo(v) >= 0; }

    template<typename V>
    typename V::return_type applyVisitor(V&& v) const {
        return Base::applyVisitor(std::forward<V>(v));
    }

    int type() const { return Base::type(); }

    void setType(int typeIndex) {
        if (typeIndex == NO_TYPE) {
            throw std::logic_error("jngen::Variant::setType():"
                " calling with NO_TYPE is invalid");
        }
        if (this->type() == typeIndex) {
            return;
        }
        if (this->type() != NO_TYPE) {
            this->doDestroy();
        }
        Base::setType(typeIndex);
    }

    bool empty() const { return Base::type() == NO_TYPE; }

    template<typename T>
    constexpr static bool hasType() {
        return Base::template typeId<T>() != NO_TYPE;
    }

private:
    template<typename T_>
    decay_t<T_>* ptr() {
        using T = decay_t<T_>;
        if (type() != this->template typeId<T>()) {
            if (type() != NO_TYPE) {
                this->doDestroy();
            }
            ::new(this->data()) T;
            unsafeType() = this->template typeId<T>();
        }
        return reinterpret_cast<T*>(this->data());
    }

    template<typename T_>
    const decay_t<T_>* cptr() const {
        using T = decay_t<T_>;
        if (type() != this->template typeId<T>()) {
            return nullptr;
        }
        return reinterpret_cast<const T*>(this->data());
    }

    int& unsafeType() {
        return Base::type();
    }

    int compareTo(const Variant& other) const;
};

struct OstreamVisitor {
    using return_type = void;

    template<typename T>
    void operator()(const T& t) {
        JNGEN_PRINT(t);
    }
    std::ostream& out;
    const OutputModifier& mod;
};

template<typename V>
struct CompareToVisitor {
    using return_type = int;

    template<typename T>
    int operator()(const T& t) {
        if (t == variant.template cref<T>()) {
            return 0;
        }
        return t < variant.template cref<T>() ? -1 : 1;
    }

    const V& variant;
};

template<typename ... Args>
int Variant<Args...>::compareTo(const Variant& other) const {
    if (empty()) {
        return other.empty() ? 0 : -1;
    }
    if (other.empty()) {
        return 1;
    }

    if (type() != other.type()) {
        return type() > other.type() ? -1 : 1;
    }
    return applyVisitor(CompareToVisitor<Variant>{other});

}

} // namespace variant_detail

using variant_detail::Variant;

template<typename ... Args>
JNGEN_DECLARE_SIMPLE_PRINTER(Variant<Args...>, 5) {
    if (t.type() == jngen::variant_detail::NO_TYPE) {
        out << "{empty variant}";
    } else {
        t.applyVisitor(jngen::variant_detail::OstreamVisitor{out, mod});
    }
}

} // namespace jngen
