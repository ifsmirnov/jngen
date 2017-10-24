#pragma once

#include "repr.h"

#include <iostream>
#include <tuple>
#include <type_traits>
#include <typeinfo>
#include <utility>
#include <vector>

namespace jngen {

namespace detail {

// TODO: maybe make it more clear SFINAE, like boost::has_left_shift<X,Y>?
// TODO: make these defines namespace independent

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
    ::jngen::detail::Has ## name ## Helper<T>::value

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
    T(std::declval<T>() + 1)
)

JNGEN_DEFINE_FUNCTION_CHECKER(
    Container,
    std::distance(std::declval<T>().begin(), std::declval<T>().end())
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
    -> enable_if_t<constraint, void>

#define JNGEN_DECLARE_SIMPLE_PRINTER(type, priority)\
inline void printValue(std::ostream& out, const type& t,\
    const OutputModifier& mod, PTag<priority>)

#define JNGEN_PRINT(value)\
printValue(out, value, mod, PTagMax{})

#define JNGEN_PRINT_NO_MOD(value)\
printValue(out, value, OutputModifier{}, PTagMax{})

JNGEN_DECLARE_PRINTER(!JNGEN_HAS_OSTREAM(), 0)
{
    static bool locked = false;

    ensure(
            !locked,
            std::string{} + "You are trying to print a type for which "
            "operator<< is not defined: " + typeid(T).name());

    locked = true;
    (void)mod;
    out << t;
    locked = false;
}

JNGEN_DECLARE_PRINTER(JNGEN_HAS_OSTREAM(), 10)
{
    (void)mod;
    out << t;
}

JNGEN_DECLARE_PRINTER(
    JNGEN_HAS_OSTREAM() && JNGEN_HAS_FUNCTION(Plus), 11)
{
    if (std::is_integral<T>::value) {
        out << T(t + mod.addition);
    } else {
        out << t;
    }
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
            out << mod.sep;
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
        out << t.size() << (mod.printM ? " " : "");
    }
    if (mod.printM) {
        if (t.empty()) {
            out << 0;
        } else {
            auto size = t[0].size();
            out << size;
            for (const auto& vec: t) {
                ensure(size == vec.size(), "Size of all matrix elements must "
                        "be equal if printM is specified");
            }
        }
    }

    if ((mod.printN || mod.printM) && !t.empty()) {
        out << "\n";
    }

    auto tmp = mod;
    {
        auto mod = tmp;
        mod.printN = mod.printM = false;
        bool first = true;
        for (const auto& x: t) {
            if (first) {
                first = false;
            } else {
                out << '\n';
            }
            JNGEN_PRINT(x);
        }
    }
}

JNGEN_DECLARE_PRINTER(JNGEN_HAS_FUNCTION(Container), 2)
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

JNGEN_DECLARE_PRINTER(JNGEN_HAS_FUNCTION(Container)
    && std::tuple_size<typename T::value_type>::value == 2, 3)
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

// Following snippet allows writing
//     cout << pair<int, int>(1, 2) << endl;
// in user code. I have to put it into separate namespace because
//   1) I don't want to 'use' all operator<< from jngen
//   2) I cannot do it in global namespace because JNGEN_HAS_OSTREAM relies
// on that it is in jngen.
namespace namespace_for_fake_operator_ltlt {

template<typename T>
auto operator<<(std::ostream& out, const T& t)
    -> enable_if_t<
            !JNGEN_HAS_OSTREAM() && !std::is_base_of<BaseReprProxy, T>::value,
            std::ostream&
        >
{
    // not jngen::printValue, because relying on ADL here for printers declared
    // later (see, e.g., http://stackoverflow.com/questions/42833134)
    printValue(out, t, jngen::defaultMod, jngen::PTagMax{});
    return out;
}

} // namespace namespace_for_fake_operator_ltlt

// Calling this operator inside jngen namespace doesn't work without this line.
using namespace jngen::namespace_for_fake_operator_ltlt;

} // namespace jngen

using namespace jngen::namespace_for_fake_operator_ltlt;
