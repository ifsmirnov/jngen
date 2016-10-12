#pragma once

#include <bits/stdc++.h>

namespace impl {

template<int N> struct PTag : PTag<N-1> {};
template<> struct PTag<0> {};
struct PTagMax : PTag<20> {};

struct OutputModifier {
    int addition = 0;
    bool printN = false;
};

template<typename T>
class Repr {
    friend std::ostream& operator<<(std::ostream& out, const Repr& repr) {
        repr.print(out);
        return out;
    }

protected:
    Repr() :
        object_(nullptr)
    {  }

    Repr(const Repr<T>&) = default;
    Repr(Repr<T>&&) = default;

public:
    Repr(const T& object) :
        object_(&object)
    {  }

    Repr<T>& add1() {
        ++mod_.addition;
        return *this;
    }

    Repr<T>& printN(bool value = true) {
        mod_.printN = value;
        return *this;
    }

private:
    void print(std::ostream& out) const {
        if (object_) {
            printValue(out, *object_, mod_, PTagMax{});
        } else {
            printValue(out, static_cast<const T&>(*this), mod_, PTagMax{});
        }
    }

    const T* object_;
    OutputModifier mod_;
};


namespace detail {

#define JNGEN_DEFINE_FUNCTION_CHECKER(name, expr)\
template<typename T, typename Enable = void>\
class Has ## name ## Helper: public std::false_type {};\
\
template<typename T>\
class Has ## name ## Helper<T,\
    decltype(void(\
        expr\
    ))\
> : public std::true_type {};\

#define JNGEN_HAS_FUNCTION(name)\
    detail::Has ## name ## Helper<T>::value

JNGEN_DEFINE_FUNCTION_CHECKER(
    OstreamMethod,
    std::declval<std::ostream&>().operator<< (std::declval<T>())
)

JNGEN_DEFINE_FUNCTION_CHECKER(
    OstreamFreeFunction,
    std::operator<<(std::declval<std::ostream&>(), std::declval<T>())
)

JNGEN_DEFINE_FUNCTION_CHECKER(
    Plus,
    std::declval<T>() + 1
)

#define JNGEN_HAS_OSTREAM()\
    (JNGEN_HAS_FUNCTION(OstreamMethod) ||\
        JNGEN_HAS_FUNCTION(OstreamFreeFunction))

template<typename T>
struct VectorDepth {
    constexpr static int value = 0;
};

template<typename T, template <typename...> class C>
struct VectorDepth<C<T>> {
    constexpr static int value =
        std::is_base_of<
            std::vector<T>,
            C<T>
        >::value ? VectorDepth<T>::value + 1 : 0;
};

} // namespace detail

#define JNGEN_DECLARE_PRINTER(constraint, priority)\
template<typename T>\
auto printValue(\
    std::ostream& out, const T& t, const OutputModifier& mod, PTag<priority>)\
    -> typename std::enable_if<constraint, void>::type

JNGEN_DECLARE_PRINTER(!JNGEN_HAS_OSTREAM(), 0)
{
    // can't just write 'false' here because assertion always fails
    static_assert(!std::is_same<T, T>::value, "operator<< is undefined");
}

JNGEN_DECLARE_PRINTER(JNGEN_HAS_OSTREAM(), 1)
{
    (void)mod;
    out << t;
}

JNGEN_DECLARE_PRINTER(
    JNGEN_HAS_OSTREAM() && JNGEN_HAS_FUNCTION(Plus), 2)
{
    out << t + mod.addition;
}


JNGEN_DECLARE_PRINTER(detail::VectorDepth<T>::value == 1, 3)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }
    bool first = true;
    for (const auto& x: t) {
        if (first) {
            first = false;
        } else {
            out << " ";
        }
        printValue(out, x, mod, PTagMax{});
    }
}

JNGEN_DECLARE_PRINTER(detail::VectorDepth<T>::value == 2, 4)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }
    for (const auto& x: t) {
        printValue(out, x, mod, PTagMax{});
        out << "\n";
    }
}

} // namespace impl
