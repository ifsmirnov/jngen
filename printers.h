#pragma once

#include <bits/stdc++.h>

#include "repr.h"

namespace impl {

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

#define JNGEN_DECLARE_SIMPLE_PRINTER(type, priority)\
void printValue(std::ostream& out, const type& t,\
    const OutputModifier& mod, PTag<priority>)

#define JNGEN_PRINT(value)\
printValue(out, value, mod, PTagMax{})

JNGEN_DECLARE_PRINTER(!JNGEN_HAS_OSTREAM(), 0)
{
    // can't just write 'false' here because assertion always fails
    static_assert(!std::is_same<T, T>::value, "operator<< is undefined");
//     (void)out;
//     (void)mod;
//     (void)t;
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
        JNGEN_PRINT(x);
    }
}

JNGEN_DECLARE_PRINTER(detail::VectorDepth<T>::value == 1 &&
    std::tuple_size<typename T::value_type>::value == 2, 4)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }

    bool first = true;
    for (const auto& x: t) {
        if (first) {
            first = false;
        } else {
            out << "\n";
        }
        JNGEN_PRINT(x);
    }
}

JNGEN_DECLARE_PRINTER(detail::VectorDepth<T>::value == 2, 4)
{
    if (mod.printN) {
        out << t.size() << "\n";
    }
    for (const auto& x: t) {
        JNGEN_PRINT(x);
        out << "\n";
    }
}

// http://stackoverflow.com/a/19841470/2159939
#define JNGEN_COMMA ,

template<typename Lhs, typename Rhs>
JNGEN_DECLARE_SIMPLE_PRINTER(std::pair<Lhs JNGEN_COMMA Rhs>, 3)
{
    JNGEN_PRINT(t.first);
    out << " ";
    JNGEN_PRINT(t.second);
}

#undef JNGEN_COMMA

} // namespace impl
