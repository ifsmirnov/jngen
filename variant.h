#pragma once

#include <stdexcept>
#include <type_traits>
#include <cstring>

#include <iostream>
#include <typeinfo>

namespace jngen {

namespace variant_detail {

constexpr static int NO_TYPE = -1;

template<typename T>
struct PlainType {
    using type = typename std::remove_cv<
        typename std::remove_reference<T>::type>::type;
};

template<size_t Size, typename ... Args>
class VariantImpl;

template<size_t Size>
class VariantImpl<Size> {
public:
    VariantImpl() {
        type_ = NO_TYPE;
    }

private:
    int type_;
    char data_[Size];

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

    void move(char* dst) const {
        memmove(dst, data(), Size);
    }

    template<typename V>
    void applyVisitor(V&&) const {
        throw;
    }

    void assign() {}
};

template<size_t Size, typename T, typename ... Args>
class VariantImpl<Size, T, Args...> : public VariantImpl<
        (sizeof(T) > Size ? sizeof(T) : Size),
        Args...
    >
{
    using Base = VariantImpl<(sizeof(T) > Size ? sizeof(T) : Size), Args...>;

    constexpr static size_t MY_ID = sizeof...(Args);

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

    template<typename V>
    void applyVisitor(V&& v) const {
        if (this->type() == MY_ID) {
            v(*reinterpret_cast<const T*>(this->data()));
        } else {
            Base::applyVisitor(v);
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
class Variant : public VariantImpl<0, Args...> {
    using Base = VariantImpl<0, Args...>;

public:
    Variant() {}

    Variant(const Variant<Args...>& other) {
        if (other.type() != NO_TYPE) {
            other.copy(this->data());
            unsafeType() = other.type();
        }
    }

    Variant& operator=(const Variant<Args...>& other) {
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
            other.unsafeType() = NO_TYPE;
        }
    }

    Variant& operator=(Variant<Args...>&& other) {
        if (this->type() != NO_TYPE) {
            this->doDestroy();
        }
        if (other.type() != NO_TYPE) {
            other.move(this->data());
            unsafeType() = other.type();
            other.unsafeType() = NO_TYPE;
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
    const T& cref() {
        auto ptr = cptr<T>();
        if (ptr == 0) {
            throw std::logic_error("jngen::Variant: taking a reference for"
                " a type which is not active now");
        }
        return *ptr;
    }

    template<typename V>
    void applyVisitor(V&& v) const {
        Base::applyVisitor(v);
    }

    int type() const { return Base::type(); }

private:
    template<typename T_, typename T = typename PlainType<T_>::type>
    T* ptr() {
        if (type() != this->template typeId<T>()) {
            if (type() != NO_TYPE) {
                this->doDestroy();
            }
            ::new(this->data()) T;
            unsafeType() = this->template typeId<T>();
        }
        return reinterpret_cast<T*>(this->data());
    }

    template<typename T_, typename T = typename PlainType<T_>::type>
    const T* cptr() const {
        if (type() != this->template typeId<T>()) {
            return nullptr;
        }
        return reinterpret_cast<const T*>(this->data());
    }

    int& unsafeType() {
        return Base::type();
    }
};

struct OstreamVisitor {
    template<typename T>
    void operator()(const T& t) {
        out << t;
    }
    std::ostream& out;
};

} // namespace variant_detail

using variant_detail::Variant;

} // namespace jngen

using jngen::Variant;

template<typename ... Args>
std::ostream& operator<<(std::ostream& out, const Variant<Args...>& v) {
    if (v.type() == jngen::variant_detail::NO_TYPE) {
        out << "{empty variant}";
    } else {
        v.applyVisitor(jngen::variant_detail::OstreamVisitor{out});
    }

    return out;
}
