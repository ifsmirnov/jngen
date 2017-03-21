
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>

#define JNGEN_ENSURE1(cond)\
    assert(cond)

#define JNGEN_ENSURE2(cond, msg)\
do\
    if (!(cond)) {\
        std::cerr << "Error: " << msg << std::endl;\
        assert(cond);\
    }\
while (false)

#define JNGEN_GET_MACRO(_1, _2, NAME, ...) NAME

#define ensure(...) JNGEN_GET_MACRO(__VA_ARGS__, JNGEN_ENSURE2, JNGEN_ENSURE1)\
    (__VA_ARGS__)

namespace jngen {

template<typename ... Args>
std::string format(const std::string& fmt, Args... args) {
    constexpr static char BUF_SIZE = 64;
    static char BUFFER[BUF_SIZE];

    int bufSize = BUF_SIZE;
    char *buf = BUFFER;

    while (true) {
        int ret = snprintf(buf, bufSize, fmt.c_str(), args...);
        if (ret < bufSize) {
            break;
        }

        if (bufSize != BUF_SIZE) {
            delete[] buf;
        }

        bufSize *= 2;
        buf = new char[bufSize];
    }

    std::string result(buf);

    if (bufSize != BUF_SIZE) {
        delete[] buf;
    }

    return result;
}

} // namespace jngen

using jngen::format;

#include <algorithm>
#include <vector>

namespace jngen {

class Dsu {
public:
    int getParent(int x) {
        extend(x);

        return parent[x] == x ? x : (parent[x] = getParent(parent[x]));
    }

    bool link(int x, int y) {
        extend(std::max(x, y));

        x = parent[x];
        y = parent[y];
        if (x == y) {
            return false;
        }

        if (rank[x] > rank[y]) {
            std::swap(x, y);
        }
        if (rank[y] == rank[x]) {
            ++rank[y];
        }
        parent[x] = y;

        --components;

        return true;
    }

    bool isConnected() const { return components <= 1; }

private:
    std::vector<int> parent;
    std::vector<int> rank;

    int components = 0;

    void extend(size_t x) {
        size_t last = parent.size() - 1;
        while (parent.size() <= x) {
            ++components;
            parent.push_back(++last);
            rank.push_back(0);
        }
    }
};

} // namespace jngen



#include <sstream>
#include <string>
#include <vector>

namespace jngen {

namespace detail {

template<typename T>
void readVariable(T& var, const std::string& value) {
    if (value.empty()) {
        return;
    }

    std::istringstream ss(value);
    ss >> var;

    ensure(ss, "Failed to parse a value from a command line argument");
}

typedef std::vector<std::string>::const_iterator OptionIterator;

void getopts(OptionIterator)
{  }

template<typename T, typename ... Args>
void getopts(OptionIterator iter, T& var, Args& ...args) {
    readVariable(var, *iter);
    getopts(++iter, args...);
}

} // namespace detail

template<typename ... Args>
void getopts(std::vector<std::string> options, Args& ...args) {
    if (options.size() < sizeof...(args)) {
        options.resize(sizeof...(args));
    }
    detail::getopts(options.cbegin(), args...);
}

template<typename ... Args>
void getopts(int argc, char *argv[], Args& ...args) {
    return getopts(std::vector<std::string>(argv + 1, argv + argc), args...);
}

} // namespace jngen

using jngen::getopts;


#include <algorithm>
#include <cctype>
#include <functional>
#include <set>
#include <string>
#include <utility>
#include <vector>

// TODO: adequate error messages

namespace jngen {

class Pattern {
    friend class Parser;
public:
    Pattern() : isOrPattern(false), min(1), max(1) {}
    Pattern(const std::string& s);

    std::string next(std::function<int(int)> rnd) const;

private:
    Pattern(Pattern p, std::pair<int, int> quantity) :
        isOrPattern(false),
        min(quantity.first),
        max(quantity.second)
    {
        children.push_back(std::move(p));
    }

    Pattern(std::vector<char> chars, std::pair<int, int> quantity) :
        chars(std::move(chars)),
        isOrPattern(false),
        min(quantity.first),
        max(quantity.second)
    {  }

    std::vector<char> chars;
    std::vector<Pattern> children;
    bool isOrPattern;
    int min;
    int max;
};

class Parser {
public:
    Pattern parse(const std::string& s) {
        this->s = s;
        pos = 0;
        return parsePattern();
    }

private:
    static bool isControl(char c) {
        static const std::string CONTROL_CHARS = "()[]{}|?";
        return CONTROL_CHARS.find(c) != std::string::npos;
    }

    static int control(int c) {
        return c >> 8;
    }

    int next() {
        size_t newPos;
        int result = peekAndMove(newPos);
        pos = newPos;
        return result;
    }

    int peek() const {
        size_t dummy;
        return peekAndMove(dummy);
    }

    int peekAndMove(size_t& newPos) const {
        newPos = pos;
        if (pos == s.size()) {
            return -1;
        }
        if (s[pos] == '\\') {
            ensure(
                pos+1 < s.size(),
                "Backslash at the end of the pattern is illegal");
            newPos += 2;
            return s[pos+1];
        }

        ++newPos;
        int ret = s[pos];
        return isControl(ret) ? (ret << 8) : ret;
    }

    // TODO: catch overflows
    int readInt() {
        ensure(std::isdigit(peek()));

        int res = 0;
        while (std::isdigit(peek())) {
            res = res * 10 + next() - '0';
        }
        return res;
    }

    std::pair<int, int> parseRange() {
        ensure(control(next()) == '{');

        int from = readInt();

        int nxt = next();
        if (control(nxt) == '}') {
            return {from, from};
        } else if (nxt == ',' || nxt == '-') {
            int to = readInt();
            ensure(control(next()) == '}');
            return {from, to};
        } else {
            ensure(false, "cannot parseRange");
        }
    }

    std::pair<int, int> tryParseQuantity() {
        std::pair<int, int> quantity = {1, 1};

        int qchar = peek();
        if (control(qchar) == '?') {
            quantity = {0, 1};
            next();
        } else if (control(qchar) == '{') {
            quantity = parseRange();
        }

        return quantity;
    }

    std::vector<char> parseBlock() {
        std::vector<char> allowed;
        char last = -1;
        bool inRange = false;
        while (control(peek()) != ']') {
            char c = next(); // buggy on cases like [a-}]
            ensure(c != -1);

            if (c == '-') {
                ensure(!inRange);
                inRange = true;
            } else if (inRange) {
                ensure(c >= last);
                for (char i = last; i <= c; ++i) {
                    allowed.push_back(i);
                }
                inRange = false;
                last = -1;
            } else {
                if (last != -1) {
                    allowed.push_back(last);
                }
                last = c;
            }
        }

        ensure(control(next()) == ']');

        ensure(!inRange);
        if (last != -1) {
            allowed.push_back(last);
        }

        std::sort(allowed.begin(), allowed.end());
        return allowed;
    }

    Pattern parsePattern() {
        std::vector<Pattern> orPatterns;
        Pattern cur;

        while (true) {
            int nxt = next();
            if (nxt == -1 || control(nxt) == ')') {
                break;
            } else if (control(nxt) == '(') {
                Pattern p = parsePattern();
                cur.children.push_back(Pattern(p, tryParseQuantity()));
            } else if (control(nxt) == '|') {
                orPatterns.emplace_back();
                std::swap(orPatterns.back(), cur);
            } else {
                std::vector<char> chars;
                if (control(nxt) == '[') {
                    chars = parseBlock();
                } else {
                    ensure(!control(nxt));
                    chars = {static_cast<char>(nxt)};
                }

                cur.children.push_back(Pattern(chars, tryParseQuantity()));
            }
        }

        if (orPatterns.empty()) {
            return cur;
        } else {
            orPatterns.emplace_back();
            std::swap(orPatterns.back(), cur);

            Pattern p;
            p.isOrPattern = true;
            p.children = orPatterns;
            return p;
        }
    }

    std::string s;
    size_t pos;
};

Pattern::Pattern(const std::string& s) {
    *this = Parser().parse(s);
}

std::string Pattern::next(std::function<int(int)> rnd) const {
    if (isOrPattern) {
        ensure(!children.empty());
        return children[rnd(children.size())].next(rnd);
    }

    ensure( (!!chars.empty()) ^ (!!children.empty()) );

    int count;
    if (min == max) {
        count = min;
    } else {
        count = min + rnd(max - min + 1);
    }

    std::string result;
    for (int i = 0; i < count; ++i) {
        if (!children.empty()) {
            for (const Pattern& p: children) {
                result += p.next(rnd);
            }
        } else {
            result += chars[rnd(chars.size())];
        }
    }

    return result;
}

} // namespace jngen

using jngen::Pattern;


#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <iterator>
#include <limits>
#include <random>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

namespace jngen {

static void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ensure(engine() == 822569775);
    ensure(engine() == 2137449171);
    ensure(engine() == 2671936806);
}

class Random;

class BaseTypedRandom {
public:
    BaseTypedRandom(Random& random) : random(random) {}

protected:
    Random& random;
};

template<typename T>
struct TypedRandom;

uint64_t maskForBound(uint64_t bound) {
    --bound;
    uint64_t mask = ~0;
    if ((mask >> 32) >= bound) mask >>= 32;
    if ((mask >> 16) >= bound) mask >>= 16;
    if ((mask >> 8 ) >= bound) mask >>= 8 ;
    if ((mask >> 4 ) >= bound) mask >>= 4 ;
    if ((mask >> 2 ) >= bound) mask >>= 2 ;
    if ((mask >> 1 ) >= bound) mask >>= 1 ;
    return mask;
}

template<typename Result, typename Source>
Result uniformRandom(Result bound, Random& random, Source (Random::*method)()) {
    static_assert(sizeof(Result) <= sizeof(Source),
        "uniformRandom: Source type must be at least as large as Result type");
#ifdef JNGEN_FAST_RANDOM
    return (random.*method)() % bound;
#else
    Source mask = maskForBound(bound);
    while (true) {
        Source outcome = (random.*method)() & mask;
        if (outcome < static_cast<Source>(bound)) {
            return outcome;
        }
    }
#endif
}

class Random {
public:
    Random() {
        assertRandomEngineConsistency();
        seed(std::random_device{}());
    }

    void seed(uint32_t val) {
        randomEngine_.seed(val);
    }

    void seed(const std::vector<uint32_t>& seed) {
        std::seed_seq seq(seed.begin(), seed.end());
        randomEngine_.seed(seq);
    }

    uint32_t next() {
        return randomEngine_();
    }

    uint64_t next64() {
        uint64_t a = next();
        uint64_t b = next();
        return (a << 32) ^ b;
    }

    double nextf() {
        return (double)randomEngine_() / randomEngine_.max();
    }

    int next(int n) {
        ensure(n > 0);
        return uniformRandom(n, *this, (uint32_t (Random::*)())&Random::next);
    }

    long long next(long long n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    size_t next(size_t n) {
        ensure(n > 0);
        return uniformRandom(n, *this, &Random::next64);
    }

    double next(double n) {
        ensure(n >= 0);
        return nextf() * n;
    }

    int next(int l, int r);
    long long next(long long l, long long r);
    size_t next(size_t l, size_t r);
    double next(double l, double r);

    int wnext(int n, int w);
    long long wnext(long long n, int w);
    size_t wnext(size_t n, int w);
    double wnext(double n, int w);

    int wnext(int l, int r, int w);
    long long wnext(long long l, long long r, int w);
    size_t wnext(size_t l, size_t r, int w);
    double wnext(double l, double r, int w);

    std::string next(const std::string& pattern) {
        return Pattern(pattern).next([this](int n) { return next(n); });
    }

    template<typename ... Args>
    std::string next(const std::string& pattern, Args... args) {
        return next(format(pattern, args...));
    }

    template<typename T, typename ... Args>
    T tnext(Args... args) {
        return TypedRandom<T>{*this}.next(args...);
    }

    template<typename ... Args>
    std::pair<int, int> nextp(Args... args) {
        return tnext<std::pair<int, int>>(args...);
    }

    template<typename Iterator>
    typename Iterator::value_type choice(Iterator begin, Iterator end) {
        auto length = std::distance(begin, end);
        ensure(length > 0, "Cannot select from a range of negative length");
        size_t index = tnext<size_t>(length);
        std::advance(begin, index);
        return *begin;
    }

    template<typename Container>
    typename Container::value_type choice(const Container& container) {
        ensure(!container.empty(), "Cannot select from an empty container");
        return choice(container.begin(), container.end());
    }

private:
    template<typename T>
    T baseWnext(T n, int w) {
        static_assert(std::is_arithmetic<T>::value,
            "Only numeric types allowed for baseWnext<T>(T n, int w)");
        if (std::abs(w) <= WNEXT_LIMIT) {
            T result = next(n);
            while (w > 0) {
                result = std::max(result, next(n));
                --w;
            }
            while (w < 0) {
                result = std::min(result, next(n));
                ++w;
            }
            return result;
        }

        if (w < 0) {
            if (std::is_integral<T>::value) {
                return n - 1 - baseWnext(n, -w);
            } else {
                return n - baseWnext(n, -w);
            }
        }

        T upperLimit =
            std::is_integral<T>::value ? n-1 : n;

        double val = std::pow(nextf(), 1.0 / (w + 1));
        T result = val * n;
        return std::max(T(0), std::min(result, upperLimit));
    }

    std::mt19937 randomEngine_;
    constexpr static int WNEXT_LIMIT = 8;
};

Random rnd;

template<>
struct TypedRandom<int> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    int next(int n) { return random.next(n); }
    int next(int l, int r) { return random.next(l, r); }
};

template<>
struct TypedRandom<double> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    double next(double n) { return random.next(n); }
    double next(double l, double r) { return random.next(l, r); }
};

template<>
struct TypedRandom<long long> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    long long next(long long n) { return random.next(n); }
    long long next(long long l, long long r) { return random.next(l, r); }
};

template<>
struct TypedRandom<size_t> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    size_t next(size_t n) { return random.next(n); }
    size_t next(size_t l, size_t r) { return random.next(l, r); }
};

template<>
struct TypedRandom<char> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    char next(char n) { return random.next(n); }
    char next(char l, char r) { return random.next(l, r); }
};

template<typename T>
struct TypedRandom : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;
    template<typename ... Args>
    T next(Args... args) { return random.next(args...); }
};

struct OrderedPairTag {} opair;

template<>
struct TypedRandom<std::pair<int, int>> : public BaseTypedRandom {
    using BaseTypedRandom::BaseTypedRandom;

    std::pair<int, int> next(int n) {
        // can't write 'return {random.next(n), random.next(n)}' because order of
        // evaluation of function arguments is unspecified.
        int first = random.next(n);
        int second = random.next(n);
        return {first, second};
    }
    std::pair<int, int> next(int l, int r) {
        int first = random.next(l, r);
        int second = random.next(l, r);
        return {first, second};
    }

    std::pair<int, int> next(int n, OrderedPairTag) {
        return ordered(next(n));
    }
    std::pair<int, int> next(int l, int r, OrderedPairTag) {
        return ordered(next(l, r));
    }

private:
    std::pair<int, int> ordered(std::pair<int, int> pair) const {
        if (pair.first > pair.second) {
            std::swap(pair.first, pair.second);
        }
        return pair;
    }
};

} // namespace jngen

using jngen::Random;

using jngen::rnd;
using jngen::opair;

void registerGen(int argc, char *argv[], int version = 1) {
    (void)version; // unused, only for testlib.h compatibility

    std::vector<uint32_t> seed;
    for (int i = 1; i < argc; ++i) {
        int startPosition = seed.size();
        seed.emplace_back();
        for (char *s = argv[i]; *s; ++s) {
            ++seed[startPosition];
            seed.push_back(*s);
        }
    }
    rnd.seed(seed);
}

#define JNGEN_INCLUDE_RANDOM_INL_H
#ifndef JNGEN_INCLUDE_RANDOM_INL_H
#error File "random_inl.h" must not be included directly.
#endif

namespace jngen {

int Random::next(int l, int r) {
    return l + next(r-l+1);
}

long long Random::next(long long l, long long r) {
    return l + next(r-l+1);
}

size_t Random::next(size_t l, size_t r) {
    return l + next(r-l+1);
}

double Random::next(double l, double r) {
    return l + next(r-l);
}

int Random::wnext(int n, int w) {
    return baseWnext(n, w);
}

long long Random::wnext(long long n, int w) {
    return baseWnext(n, w);
}

size_t Random::wnext(size_t n, int w) {
    return baseWnext(n, w);
}

double Random::wnext(double n, int w) {
    return baseWnext(n, w);
}

int Random::wnext(int l, int r, int w) {
    return l + wnext(r-l+1, w);
}

long long Random::wnext(long long l, long long r, int w) {
    return l + wnext(r-l+1, w);
}

size_t Random::wnext(size_t l, size_t r, int w) {
    return l + wnext(r-l+1, w);
}

double Random::wnext(double l, double r, int w) {
    return l + wnext(r-l, w);
}

} // namespace jngen
#undef JNGEN_INCLUDE_RANDOM_INL_H

#include <iostream>
#include <type_traits>

namespace jngen {

template<int N> struct PTag : PTag<N-1> {};
template<> struct PTag<0> {};
struct PTagMax : PTag<20> {};

struct OutputModifier {
    int addition = 0;
    bool printN = false;
    bool printM = false;

    bool printParents = false;
    bool printEdges = true;

    char sep = ' ';
};

OutputModifier defaultMod;

template<typename T>
class Repr {
    friend std::ostream& operator<<(std::ostream& out, const Repr& repr) {
        repr.print(out);
        return out;
    }

    template<typename P>
    friend Repr<P> repr(const P& t);

    template<typename P>
    friend class ReprProxy;

protected:
    Repr() = delete;
    Repr(const Repr<T>&) = default;
    Repr<T>& operator=(const Repr<T>&) = default;
    Repr(Repr<T>&&) = default;
    Repr<T>& operator=(Repr<T>&&) = default;

public:
    Repr(const T& object) :
        object_(object),
        mod_(defaultMod)
    {  }

    Repr<T>& add1(bool value = true) {
        mod_.addition = value;
        return *this;
    }

    Repr<T>& printN(bool value = true) {
        mod_.printN = value;
        return *this;
    }

    Repr<T>& printM(bool value = true) {
        mod_.printM = value;
        return *this;
    }

    Repr<T>& printParents(bool value = true) {
        mod_.printParents = value;
        mod_.printEdges = !value;
        return *this;
    }

    Repr<T>& printEdges(bool value = true) {
        mod_.printEdges = value;
        mod_.printParents = !value;
        return *this;
    }

    Repr<T>& endl(bool value = true) {
        mod_.sep = value ? '\n' : ' ';
        return *this;
    }

private:
    void print(std::ostream& out) const {
        printValue(out, object_, mod_, PTagMax{});
    }

    const T& object_;

protected:
    OutputModifier mod_;
};

class BaseReprProxy {};

template<typename T>
class ReprProxy : public BaseReprProxy {
    friend std::ostream& operator<<(std::ostream& out, const ReprProxy& proxy) {
        Repr<T> repr(static_cast<const T&>(proxy));
        return out << repr;
    }

public:
    Repr<T> add1(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.add1(value);
        return repr;
    }

    Repr<T> printN(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printN(value);
        return repr;
    }

    Repr<T> printM(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printM(value);
        return repr;
    }

    Repr<T> printParents(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printParents(value);
        return repr;
    }

    Repr<T> printEdges(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.printEdges(value);
        return repr;
    }

    Repr<T> endl(bool value = true) {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.endl(value);
        return repr;
    }

protected:
    ReprProxy() {
        static_assert(
            std::is_base_of<ReprProxy<T>, T>::value,
            "ReprProxy<T> must be inherited by T");
    }
};

template<typename T>
Repr<T> repr(const T& t) {
    return Repr<T>(t);
}

class DefaultModSetter : public Repr<int> {
    friend DefaultModSetter setMod();

private:
    DefaultModSetter(int val) :
        Repr<int>(val)
    {  }

public:
    ~DefaultModSetter() {
        defaultMod = mod_;
    }

    Repr<int>& reset() {
        mod_ = OutputModifier();
        return *this;
    }
};

DefaultModSetter setMod() {
    static int dummy = 0;
    return DefaultModSetter(dummy);
}

} // namespace jngen

using jngen::repr;
using jngen::setMod;


#include <iostream>
#include <tuple>
#include <type_traits>
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

#define JNGEN_PRINT_NO_MOD(value)\
printValue(out, value, OutputModifier{}, PTagMax{})

JNGEN_DECLARE_PRINTER(!JNGEN_HAS_OSTREAM(), 0)
{
    // can't just write 'false' here because assertion always fails
    static_assert(!std::is_same<T, T>::value, "operator<< is undefined");
    (void)out;
    (void)mod;
    (void)t;
}

JNGEN_DECLARE_PRINTER(JNGEN_HAS_OSTREAM(), 1)
{
    (void)mod;
    out << t;
}

JNGEN_DECLARE_PRINTER(
    JNGEN_HAS_OSTREAM() && JNGEN_HAS_FUNCTION(Plus), 2)
{
    if (std::is_integral<T>::value) {
        out << t + mod.addition;
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

// Following snippet allows writing
//     cout << pair<int, int>(1, 2) << endl;
// in user code. I have to put it into separate namespace because
//   1) I don't want to 'use' all operator<< from jngen
//   2) I cannot do it in global namespace because JNGEN_HAS_OSTREAM relies
// on that it is in jngen.
namespace namespace_for_fake_operator_ltlt {

template<typename T>
auto operator<<(std::ostream& out, const T& t)
    -> typename std::enable_if<
            !JNGEN_HAS_OSTREAM() && !std::is_base_of<BaseReprProxy, T>::value,
            std::ostream&
        >::type
{
    jngen::printValue(out, t, jngen::defaultMod, jngen::PTagMax{});
    return out;
}

} // namespace namespace_for_fake_operator_ltlt

// Calling this operator inside jngen namespace doesn't work without this line.
using namespace jngen::namespace_for_fake_operator_ltlt;

} // namespace jngen

using namespace jngen::namespace_for_fake_operator_ltlt;


#include <algorithm>

namespace jngen {

// TODO: deprecate random_shuffle as done in testlib.h

template<typename Iterator>
void shuffle(Iterator begin, Iterator end) {
    ensure(end > begin, "Cannot shuffle range of negative length");
    size_t size = end - begin;
    for (size_t i = 1; i < size; ++i) {
        std::swap(*(begin + i), *(begin + rnd.next(i + 1)));
    }
}

template<typename Iterator>
typename Iterator::value_type choice(Iterator begin, Iterator end) {
    return rnd.choice(begin, end);
}

template<typename Container>
typename Container::value_type choice(const Container& container) {
    return rnd.choice(container);
}

} // namespace jngen

using jngen::shuffle;
using jngen::choice;


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

    // TODO(ifsmirnov): 'use' all methods and make inheritance private
    using Base::at;
    using Base::size;
    using Base::resize;
    using Base::begin;
    using Base::end;
    using Base::insert;
    using Base::clear;
    using Base::erase;

    void extend(size_t requiredSize) {
        if (requiredSize > size()) {
            resize(requiredSize);
        }
    }

    template<typename F, typename ...Args>
    static GenericArray<T> randomf(size_t size, F func, const Args& ... args);
    template<typename F, typename ...Args>
    static GenericArray<T> randomfUnique(
            size_t size, F func, const Args& ... args);

    template<typename ...Args>
    static GenericArray<T> random(size_t size, const Args& ... args);
    template<typename ...Args>
    static GenericArray<T> randomUnique(size_t size, const Args& ... args);

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
    GenericArray<T> result;
    result.reserve(size);

    size_t retries = (size + 10) * log(size + 10) * 2;

    while (result.size() != size) {
        if (--retries == 0) {
            ensure(false, "There are not enough unique elements");
        }

        T t = func(args...);
        if (!set.count(t)) {
            set.insert(t);
            result.push_back(t);
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
GenericArray<T> GenericArray<T>::id(size_t size, T start) {
    constexpr bool enable = std::is_integral<T>::value;
    static_assert(enable, "Cannot call Array<T>::id with non-integral T");

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
    ensure(count <= size());

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
    GenericArray<T> res(count);
    for (T& t: res) {
        t = choice();
    }
    return res;
}

template<typename T>
GenericArray<T>& GenericArray<T>::operator+=(const GenericArray<T>& other) {
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

    size_t size = this->size();
    while (k-- > 1) {
        insert(end(), begin(), begin() + size);
    }
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

} // namespace jngen

template<typename T>
using TArray = jngen::GenericArray<T>;

using Array = jngen::GenericArray<int>;
using Array64 = jngen::GenericArray<long long>;
using Arrayf = jngen::GenericArray<double>;
using Arrayp = jngen::GenericArray<std::pair<int, int>>;

template<typename T>
jngen::GenericArray<T> makeArray(const std::vector<T>& values) {
    return jngen::GenericArray<T>(values);
}

template<typename T>
jngen::GenericArray<T> makeArray(const std::initializer_list<T>& values) {
    return jngen::GenericArray<T>(values);
}


namespace jngen {

class ArrayRandom {
public:
    ArrayRandom() {
        static bool created = false;
        ensure(!created, "jngen::ArrayRandom should be created only once");
        created = true;
    }

    template<typename F, typename ...Args>
    static auto randomf(
            size_t size,
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomf(size, func, args...);
    }

    template<typename F, typename ...Args>
    static auto randomfUnique(
            size_t size,
            F func,
            Args... args) -> GenericArray<decltype(func(args...))>
    {
        typedef decltype(func(args...)) T;
        return GenericArray<T>::randomfUnique(size, func, args...);
    }
} rnda;

} // namespace jngen

using jngen::rnda;


#include <cstdlib>
#include <iostream>
#include <type_traits>

namespace jngen {

// TODO: why do we need this shit?
class EpsHolder {
private:
    EpsHolder() : eps(1e-9) {}

public:
    long double eps;

    static EpsHolder& instance() {
        static EpsHolder holder;
        return holder;
    }
};

inline void setEps(long double eps) {
    EpsHolder::instance().eps = eps;
}

inline long double eps() {
    return EpsHolder::instance().eps;
}

template<typename T, typename Enable = void>
class Comparator {
public:
    static bool eq(T a, T b) { return a == b; }
    static bool ne(T a, T b) { return !(a == b); }
    static bool lt(T a, T b) { return a < b; }
    static bool le(T a, T b) { return a <= b; }
    static bool gt(T a, T b) { return a > b; }
    static bool ge(T a, T b) { return a >= b; }
};

template<typename T>
class Comparator<T,
    typename std::enable_if<std::is_floating_point<T>::value, void>::type>
{
    static bool eq(T a, T b) { return std::abs(b - a) < eps(); }
    static bool ne(T a, T b) { return !(a == b); }
    static bool lt(T a, T b) { return a < b - eps; }
    static bool le(T a, T b) { return a <= b + eps; }
    static bool gt(T a, T b) { return a > b + eps; }
    static bool ge(T a, T b) { return a >= b - eps; }
};

// TODO: do something with eq(int, long long)
template<typename T> bool eq(T a, T b) { return Comparator<T>::eq(a, b); }
template<typename T> bool ne(T a, T b) { return Comparator<T>::ne(a, b); }
template<typename T> bool lt(T a, T b) { return Comparator<T>::lt(a, b); }
template<typename T> bool le(T a, T b) { return Comparator<T>::le(a, b); }
template<typename T> bool gt(T a, T b) { return Comparator<T>::gt(a, b); }
template<typename T> bool ge(T a, T b) { return Comparator<T>::ge(a, b); }

template<typename T>
struct TPoint : public ReprProxy<TPoint<T>> {
    T x, y;

    TPoint() : x(0), y(0) {}
    TPoint(T x, T y) : x(x), y(y) {}

    template<typename U>
    TPoint(const TPoint<U>& other) : x(other.x), y(other.y) {}

    TPoint<T> operator+(const TPoint<T>& other) const {
        return TPoint(x + other.x, y + other.y);
    }

    TPoint<T>& operator+=(const TPoint<T>& other) {
        x += other.x;
        y += other.y;
        return *this;
    }

    TPoint<T> operator-(const TPoint<T>& other) const {
        return TPoint(x - other.x, y - other.y);
    }

    TPoint<T>& operator-=(const TPoint<T>& other) {
        x -= other.x;
        y -= other.y;
        return *this;
    }

    TPoint<T> operator*(T factor) const {
        return TPoint<T>(x * factor, y * factor);
    }

    TPoint<T>& operator*=(T factor) {
        x *= factor;
        y *= factor;
        return *this;
    }

    T operator*(const TPoint<T>& other) const {
        return x * other.x + y * other.y;
    }

    T operator%(const TPoint<T>& other) const {
        return x * other.y - y * other.x;
    }

    bool operator==(const TPoint<T>& other) const {
        return eq(x, other.x) && eq(y, other.y);
    }

    bool operator!=(const TPoint<T>& other) const {
        return !(*this == other);
    }

    bool operator<(const TPoint<T>& other) const {
        if (eq(x, other.x)) {
            return lt(y, other.y);
        }
        return lt(x, other.x);
    }
};

using Point = TPoint<long long>;
using Pointf = TPoint<long double>;

template<typename T>
JNGEN_DECLARE_SIMPLE_PRINTER(TPoint<T>, 3) {
    (void)mod;
    out << t.x << " " << t.y;
}

// TODO: make polygon a class to support, e.g., shifting by a point
template<typename T>
using TPolygon = GenericArray<TPoint<T>>;

using Polygon = TPolygon<long long>;
using Polygonf = TPolygon<long double>;

template<typename T>
JNGEN_DECLARE_SIMPLE_PRINTER(TPolygon<T>, 5) {
    // I should avoid copy-paste from array printer here but need to output
    // points with '\n' separator. Maybe 'mod' should be made non-const?
    if (mod.printN) {
        out << t.size() << "\n";
    }
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

namespace detail {

// Please forgive me the liberty of using TPolygon instead of Array<Point<T>> :)
// (laxity?)
template<typename T>
TPolygon<T> convexHull(TPolygon<T> points) {
    points.sort().unique();

    if (points.size() <= 2u) {
        return points;
    }

    TPolygon<T> upper(points.begin(), points.begin() + 2);
    upper.reserve(points.size());
    int top = 1;
    for (size_t i = 2; i < points.size(); ++i) {
        while (top >= 1 && ge(
                (upper[top] - upper[top-1]) % (points[i] - upper[top]), 0ll))
        {
            upper.pop_back();
            --top;
        }
        upper.push_back(points[i]);
        ++top;
    }

    TPolygon<T> lower(points.begin(), points.begin() + 2);
    lower.reserve(points.size());
    top = 1;
    for (size_t i = 2; i < points.size(); ++i) {
        while (top >= 1 && le(
                (lower[top] - lower[top-1]) % (points[i] - lower[top]), 0ll))
        {
            lower.pop_back();
            --top;
        }
        lower.push_back(points[i]);
        ++top;
    }
    upper.pop_back();
    upper.erase(upper.begin());
    return lower + upper.reversed();
}

template<typename T>
TPolygon<T> convexPolygonByEllipse(
        int n, Pointf center, Pointf xAxis, Pointf yAxis)
{
    return convexHull(TPolygon<T>::randomf(
        n,
        [center, xAxis, yAxis] () -> TPoint<T> {
            static const long double PI = acosl(-1.0);
            long double angle = rnd.next(0., PI*2);
            long double sina = sinl(angle);
            long double cosa = cosl(angle);
            return center + xAxis * cosa + yAxis * sina;
        }
    ));
}

} // namespace detail

class GeometryRandom {
public:
    GeometryRandom() {
        static bool created = false;
        ensure(!created, "jngen::GeometryRandom should be created only once");
        created = true;
    }

    // point in [0, X] x [0, Y]
    static Point point(long long X, long long Y) {
        long long x = rnd.tnext<long long>(0, X);
        long long y = rnd.tnext<long long>(0, Y);
        return {x, y};
    }

    // point in [0, C] x [0, C]
    static Point point(long long C) {
        return point(C, C);
    }

    // Point in [x1, x2] x [y1, y2]
    static Point point(long long x1, long long y1, long long x2, long long y2) {
        long long x = rnd.tnext<long long>(x1, x2);
        long long y = rnd.tnext<long long>(y1, y2);
        return {x, y};
    }

    static Polygon convexPolygon(int n, long long X, long long Y) {
        Polygon res = detail::convexPolygonByEllipse<long long>(
            n * 10, // BUBEN!
            Point(X/2, Y/2),
            Point(X/2, 0),
            Point(0, Y/2)
        );
        for (auto& x: res) {
            ensure(x.x >= 0);
            ensure(x.x <= X);
            ensure(x.y >= 0);
            ensure(x.y <= Y);
        }

        ensure(
            static_cast<int>(res.size()) >= n,
            "Cannot generate a convex polygon with so much vertices");

        return res.subseq(Array::id(res.size()).choice(n).sort());
    }
} rndg;

} // namespace jngen

using jngen::Point;
using jngen::Pointf;

using jngen::Polygon;
using jngen::Polygonf;

using jngen::rndg;

using jngen::eps;
using jngen::setEps;


#include <cmath>
#include <map>
#include <set>
#include <string>
#include <utility>
#include <vector>

namespace jngen {

typedef std::pair<long long, long long> HashBase; // (mod, base)
typedef std::pair<std::string, std::string> StringPair;

class StringRandom {
public:
    StringRandom() {
        static bool created = false;
        ensure(!created, "jngen::StringRandom should be created only once");
        created = true;
    }

    static std::string thueMorse(int len, char first = 'a', char second = 'b');

    static std::string abacaba(int len, char first = 'a');

    static StringPair antiHash(
            const std::vector<HashBase>& bases,
            const std::string& alphabet = "a-z",
            int length = -1);
} rnds;

namespace detail {

inline int popcount(long long x) {
    int res = 0;
    while (x) {
        ++res;
        x &= x-1;
    }
    return res;
}

inline int trailingZeroes(long long x) {
    int res = 0;
    ensure(x != 0);
    while (!(x&1)) {
        ++res;
        x >>= 1;
    }
    return res;
}

inline std::string parseAllowedChars(std::string pattern) {
    std::set<char> result;
    pattern += "\0\0";
    for (size_t i = 0; i < pattern.length(); ++i) {
        if (pattern[i] == '-') {
            result.insert('-');
        } else if(pattern[i+1] == '-' && pattern[i+2] != '\0') {
            for (char c = pattern[i]; c <= pattern[i+2]; ++c) {
                result.insert(c);
            }
            i += 2;
        } else {
            result.insert(pattern[i]);
        }
    }
    return std::string(result.begin(), result.end());
}

inline std::vector<std::string> extendAntiHash(
        const std::vector<std::string>& chars,
        HashBase base,
        int count)
{
    ensure(count == 2, "Count != 2 is not supported (yet)");

    size_t baseLength = chars[0].size();
    for (const auto& s: chars) {
        ensure(s.size() == baseLength);
    }

    long long mod = base.first;
    long long p = base.second;

    long long pPower = 1;
    for (size_t i = 0; i != baseLength; ++i) {
        pPower = (pPower * p) % mod;
    }

    std::vector<long long> charHashes;
    for (const auto& s: chars) {
        long long hash = 0;
        for (char c: s) {
            hash = (hash * p + c) % mod;
        }
        charHashes.push_back(hash);
    }

    auto computeHash = [&charHashes, mod, pPower](const std::vector<int>& a) {
        long long hash = 0;
        for (int x: a) {
            hash = (hash * pPower + charHashes[x]) % mod;
        }
        return hash;
    };

    // This bounds were achieved empirically and should be justified.
    int needForMatch;
    if (count == 2) {
        needForMatch = 5 * pow(double(mod), 0.5);
    } else {
        ensure(false, "Only count = 2 is supported yet");
    }

    int length = 2;
    double wordCount = pow(double(chars.size()), double(length));

    while (true) {
        ++length;
        wordCount *= chars.size();
        if (wordCount < needForMatch) {
            continue;
        }

        std::vector<std::pair<long long, Array>> words;
        std::map<long long, int> hashCount;
        std::set<Array> used;

        for (int i = 0; i < needForMatch; ++i) {
            Array w = Array::random(length, chars.size());
            if (used.count(w)) {
                --i;
                continue;
            }
            used.insert(w);
            long long hash = computeHash(w);
            words.emplace_back(hash, w);
            if (++hashCount[hash] == count) {
                std::vector<std::string> result;
                for (const auto& kv: words) {
                    if (kv.first == hash) {
                        std::string word;
                        for (int c: kv.second) {
                            word += chars[c];
                        }
                        result.push_back(word);
                    }
                }
                return result;
            }
        }
    }
}

inline StringPair minimalAntiHashTest(
        std::vector<HashBase> bases,
        const std::string allowedChars)
{
    for (auto base: bases) {
        ensure(base.first >= 0, "0 < MOD must hold");
        ensure(
            base.first < (long long)(2e9),
            "Modules larger than 2'000'000'000 are not supported yet");
        ensure(
            0 < base.second && base.second < base.first,
            "0 <= P < MOD must hold");
    }

    std::vector<int> counts;
    if (bases.size() == 1) {
        counts = {2};
    } else if (bases.size() == 2) {
        counts = {2, 2};
    } else {
        counts.assign(bases.size(), 2);
    }

    std::vector<std::string> cur;
    for (char c: allowedChars) {
        cur.emplace_back(1, c);
    }

    for (size_t i = 0; i != bases.size(); ++i) {
        cur = extendAntiHash(cur, bases[i], counts[i]);
        ensure(static_cast<int>(cur.size()) == counts[i],
            "Cannot generate long enough pair with same hash");
    }

    return {cur[0], cur[1]};
}

} // namespace detail

std::string StringRandom::thueMorse(int len, char first, char second) {
    std::string res(len, ' ');
    for (int i = 0; i < len; ++i) {
        res[i] = detail::popcount(i)%2 == 0 ? first : second;
    }
    return res;
}

std::string StringRandom::abacaba(int len, char first) {
    std::string res(len, ' ');
    for (int i = 0; i < len; ++i) {
        res[i] = first + detail::trailingZeroes(~i);
    }
    return res;
}

StringPair StringRandom::antiHash(
        const std::vector<HashBase>& bases,
        const std::string& alphabet,
        int length)
{
    std::string allowedChars = detail::parseAllowedChars(alphabet);
    StringPair result = detail::minimalAntiHashTest(bases, allowedChars);

    if (length == -1) {
        return result;
    }

    ensure(
        static_cast<int>(result.first.length()) <= length,
        "Cannot generate enough long anti-hash test");

    int extraLength = length - result.first.length();
    int leftSize = rnd.next(0, extraLength);

    std::string left = rnd.next(format("[%s]{%d}", alphabet.c_str(), leftSize));
    std::string right =
        rnd.next(format("[%s]{%d}", alphabet.c_str(), extraLength - leftSize));

    return {
        left + result.first + right,
        left + result.second + right
    };
}

} // namespace jngen

using jngen::rnds;


#include <cstdio>
#include <cstdlib>

namespace jngen {

int getInitialTestNo() {
    char *envvar = getenv("TESTNO");
    int testno;
    if (!envvar || 1 != std::sscanf(envvar, "%d", &testno)) {
        return 1;
    }
    return testno;
}

int nextTestNo = -1;

void startTest(int testNo) {
    nextTestNo = testNo + 1;
    char filename[10];
    std::sprintf(filename, "%d", testNo);
    if (!std::freopen(filename, "w", stdout)) {
        ensure(false, "Cannot open the file");
    }
}

void startTest() {
    if (nextTestNo == -1) {
        nextTestNo = getInitialTestNo();
    }

    startTest(nextTestNo);
}

void setNextTestNumber(int testNo) {
    nextTestNo = testNo;
}

} // namespace jngen

using jngen::startTest;
using jngen::setNextTestNumber;


#include <cstring>
#include <iostream>
#include <stdexcept>
#include <type_traits>

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

    void setType(int) {
        throw;
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
        JNGEN_PRINT(t);
    }
    std::ostream& out;
    const OutputModifier& mod;
};

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


#include <iterator>
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
    VariantArray(const GenericArray<T>& other) {
        std::copy(other.begin(), other.end(), std::back_inserter(*this));
    }

    template<typename T, typename = typename std::enable_if<
        BaseVariant::template hasType<T>()>::type>
    VariantArray(GenericArray<T>&& other) {
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

#include <string>
#include <utility>


namespace jngen {

#define JNGEN_DEFAULT_WEIGHT_TYPES int, double, std::string, std::pair<int, int>

#if defined(JNGEN_EXTRA_WEIGHT_TYPES)
#define JNGEN_WEIGHT_TYPES JNGEN_DEFAULT_WEIGHT_TYPES , JNGEN_EXTRA_WEIGHT_TYPES
#else
#define JNGEN_WEIGHT_TYPES JNGEN_DEFAULT_WEIGHT_TYPES
#endif

using Weight = Variant<JNGEN_WEIGHT_TYPES>;
using WeightArray = VariantArray<JNGEN_WEIGHT_TYPES>;

} // namespace jngen

using jngen::Weight;
using jngen::WeightArray;


#include <algorithm>
#include <iostream>
#include <iterator>
#include <set>
#include <utility>
#include <vector>

namespace jngen {

class GenericGraph {
public:
    virtual ~GenericGraph() {}

    virtual int n() const { return adjList_.size(); }
    virtual int m() const { return numEdges_; }

    // u, v: labels
    virtual void addEdge(int u, int v, const Weight& w = Weight{});
    virtual bool isConnected() const { return dsu_.isConnected(); }

    virtual int vertexLabel(int v) const { return vertexLabel_[v]; }
    virtual int vertexByLabel(int v) const { return vertexByLabel_[v]; }

    // v: label
    // return: array<label>
    virtual Array edges(int v) const;

    // return: array<label, label>
    virtual Arrayp edges() const;

    // order: by labels
    // TODO: think about ordering here
    virtual void setVertexWeights(const WeightArray& weights) {
        ensure(static_cast<int>(weights.size()) == n());
        vertexWeights_.resize(n());
        for (int i = 0; i < n(); ++i) {
            vertexWeights_[i] = weights[vertexByLabel(i)];
        }
    }

    // v: label
    virtual void setVertexWeight(int v, const Weight& weight) {
        ensure(v < n());
        v = vertexByLabel(v);

        vertexWeights_.extend(v + 1);
        vertexWeights_[v] = weight;
    }

    virtual void setEdgeWeights(const WeightArray& weights) {
        ensure(static_cast<int>(weights.size()) == m());
        edgeWeights_ = weights;
    }

    virtual void setEdgeWeight(size_t index, const Weight& weight) {
        ensure(static_cast<int>(index) < m());
        edgeWeights_.extend(index + 1);
        edgeWeights_[index] = weight;
    }

    // v: label
    virtual Weight vertexWeight(int v) const {
        size_t index = vertexByLabel(v);
        if (index < vertexWeights_.size()) {
            return Weight{};
        }
        return vertexWeights_[index];
    }

    virtual Weight edgeWeight(size_t index) const {
        if (index < edgeWeights_.size()) {
            return Weight{};
        }
        return edgeWeights_[index];
    }

    // TODO: should it really be public?
    virtual void doPrintEdges(
        std::ostream& out, const OutputModifier& mod) const;

    // TODO: more operators!
    virtual bool operator==(const GenericGraph& other) const;
    virtual bool operator<(const GenericGraph& other) const;

protected:
    void doShuffle();

    void extend(size_t size);

    // u, v: edge numbers
    void addEdgeUnsafe(int u, int v);

    // v: edge number
    // returns: edge number
    int edgeOtherEnd(int v, int edgeId);

    void permuteEdges(const Array& order);

    void normalizeEdges();

    int compareTo(const GenericGraph& other) const;

    int numEdges_ = 0;

    bool directed_ = false;

    Dsu dsu_;
    std::vector<Array> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;
    Arrayp edges_;

    WeightArray vertexWeights_;
    WeightArray edgeWeights_;
};

Array GenericGraph::edges(int v) const {
    v = vertexByLabel(v);

    Array result;
    std::transform(
        adjList_[v].begin(),
        adjList_[v].end(),
        std::back_inserter(result),
        [this, v](int x) { return vertexLabel(edgeOtherEnd(v, x)); }
    );
    return result;
}

Arrayp GenericGraph::edges() const {
    auto edges = edges_;
    for (auto& e: edges) {
        e.first = vertexLabel(e.first);
        e.second = vertexLabel(e.second);
    }
    return edges;
}

inline void GenericGraph::doShuffle() {
    if (vertexLabel_.size() < static_cast<size_t>(n())) {
        vertexLabel_ = Array::id(n());
    }
    vertexLabel_.shuffle();
    vertexByLabel_ = vertexLabel_.inverse();

    if (!directed_) {
        for (auto& edge: edges_) {
            if (rnd.next(2)) {
                std::swap(edge.first, edge.second);
            }
        }
    }

    permuteEdges(Array::id(numEdges_).shuffled());
}

inline void GenericGraph::extend(size_t size) {
    size_t oldSize = n();
    if (size > oldSize) {
        adjList_.resize(size);
        vertexLabel_ += Array::id(size - oldSize, oldSize);
        vertexByLabel_ += Array::id(size - oldSize, oldSize);
    }
}

void GenericGraph::addEdgeUnsafe(int u, int v) {
    int id = numEdges_++;
    edges_.emplace_back(u, v);

    adjList_[u].push_back(id);
    if (u != v) {
        adjList_[v].push_back(id);
    }
}

int GenericGraph::edgeOtherEnd(int v, int edgeId) {
    ensure(edgeId < numEdges_);
    const auto& edge = edges_[edgeId];
    if (edge.first == v) {
        return edge.second;
    }
    ensure(!directed_);
    ensure(edge.second == v);
    return edge.first;
}

void GenericGraph::permuteEdges(const Array& order) {
    edges_ = edges_.subseq(order);

    auto newByOld = order.inverse();
    for (int v = 0; v < n(); ++v) {
        for (auto& x: adjList_[v]) {
            x = newByOld[x];
        }
    }

    if (edgeWeights_.hasNonEmpty()) {
        edgeWeights_.extend(m());
        edgeWeights_ = edgeWeights_.subseq(order);
    }
}

void GenericGraph::normalizeEdges() {
    ensure(
        vertexLabel_ == Array::id(n()),
        "Can call normalizeEdges() only on newly created graph");

    if (!directed_) {
        for (auto& edge: edges_) {
            if (edge.first > edge.second) {
                std::swap(edge.first, edge.second);
            }
        }
    }

    auto order = Array::id(numEdges_).sorted(
        [this](int i, int j) {
            return edges_[i] < edges_[j];
        });

    permuteEdges(order);
}

inline void GenericGraph::addEdge(int u, int v, const Weight& w) {
    extend(std::max(u, v) + 1);

    u = vertexByLabel(u);
    v = vertexByLabel(v);

    dsu_.link(u, v);
    addEdgeUnsafe(u, v);

    if (!w.empty()) {
        setEdgeWeight(m() - 1, w);
    }
}

namespace {

WeightArray prepareWeightArray(WeightArray a, int requiredSize) {
    ensure(a.hasNonEmpty(), "INTERNAL ASSERT");

    a.extend(requiredSize);
    int type = a.anyType();
    for (auto& x: a) {
        if (x.empty()) {
            x.setType(type);
        }
    }

    return a;
}

} // namespace

inline void GenericGraph::doPrintEdges(
    std::ostream& out, const OutputModifier& mod) const
{
    if (mod.printN) {
        out << n();
        if (mod.printM) {
            out << " " << m();
        }
        out << "\n";
    } else if (mod.printM) {
        out << m() << "\n";
    }

    if (vertexWeights_.hasNonEmpty()) {
        auto vertexWeights = prepareWeightArray(vertexWeights_, n());
        for (int i = 0; i < n(); ++i) {
            if (i > 0) {
                out << " ";
            }
            JNGEN_PRINT_NO_MOD(vertexWeights[vertexByLabel(i)]);
        }
        out << "\n";
    }

    auto t(mod);
    {
        auto mod(t);

        Arrayp edges = this->edges();
        mod.printN = false;
        if (edgeWeights_.hasNonEmpty()) {
            auto edgeWeights = prepareWeightArray(edgeWeights_, m());
            for (int i = 0; i < m(); ++i) {
                if (i > 0) {
                    out << "\n";
                }
                JNGEN_PRINT(edges[i]);
                out << " ";
                JNGEN_PRINT_NO_MOD(edgeWeights[i]);
            }
        } else {
            JNGEN_PRINT(edges);
        }
    }
}

inline bool GenericGraph::operator==(const GenericGraph& other) const {
    return compareTo(other) == 0;
}

inline bool GenericGraph::operator<(const GenericGraph& other) const {
    return compareTo(other) == -1;
}

inline int GenericGraph::compareTo(const GenericGraph& other) const {
    if (n() != other.n()) {
        return n() < other.n() ? -1 : 1;
    }
    for (int i = 0; i < n(); ++i) {
        Array e1 = Array(edges(i)).sorted();
        Array e2 = Array(other.edges(i)).sorted();
        if (e1 != e2) {
            return e1 < e2 ? -1 : 1;
        }
    }
    return 0;
}

} // namespace jngen



#include <algorithm>
#include <vector>

namespace jngen {

class Tree : public ReprProxy<Tree>, public GenericGraph {
public:
    Tree() {
        extend(1);
    }
    Tree(const GenericGraph& gg) : GenericGraph(gg) {
        extend(1);
    }

    void addEdge(int u, int v, const Weight& w = Weight{}) override;

    Tree& shuffle();
    Tree shuffled() const;

    Tree link(int vInThis, const Tree& other, int vInOther);
    Tree glue(int vInThis, const Tree& other, int vInOther);

    static Tree bamboo(size_t size);
    static Tree randomPrufer(size_t size);
    static Tree random(size_t size, double elongation = 1.0);
    static Tree star(size_t size);
    static Tree caterpillar(size_t length, size_t size);
};

inline void Tree::addEdge(int u, int v, const Weight& w) {
    extend(std::max(u, v) + 1);

    u = vertexByLabel(u);
    v = vertexByLabel(v);

    int ret = dsu_.link(u, v);
    ensure(ret, "A cycle appeared in the tree :(");

    addEdgeUnsafe(u, v);

    if (!w.empty()) {
        setEdgeWeight(m() - 1, w);
    }
}

inline Tree& Tree::shuffle() {
    doShuffle();
    return *this;
}

inline Tree Tree::shuffled() const {
    Tree t = *this;
    return t.shuffle();
}

Tree Tree::link(int vInThis, const Tree& other, int vInOther) {
    Tree t(*this);

    for (const auto& e: other.edges()) {
        t.addEdge(e.first + n(), e.second + n());
    }

    t.addEdge(vInThis, vInOther + n());

    return t;
}

Tree Tree::glue(int vInThis, const Tree& other, int vInOther) {
    auto newLabel = [vInThis, vInOther, &other, this] (int v) {
        if (v < vInOther) {
            return n() + v;
        } else if (v == vInOther) {
            return vInThis;
        } else {
            return n() + v - 1;
        }
    };

    Tree t(*this);

    for (const auto& e: other.edges()) {
        t.addEdge(newLabel(e.first), newLabel(e.second));
    }

    assert(t.n() == n() + other.n() - 1);

    return t;
}

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 2) {
    ensure(t.isConnected(), "Tree is not connected :(");

    if (mod.printParents) {
        out << "Printing parents is not supported yet";
    } else if (mod.printEdges) {
        t.doPrintEdges(out, mod);
    } else {
        ensure(false, "Print mode is unknown");
    }
}

// Tree generators go here

inline Tree Tree::bamboo(size_t size) {
    Tree t;
    for (size_t i = 0; i + 1 < size; ++i) {
        t.addEdge(i, i+1);
    }
    t.normalizeEdges();
    return t;
}

inline Tree Tree::randomPrufer(size_t size) {
    if (size == 1) {
        return Tree();
    }

    Array code = Array::random(size - 2, size);
    std::vector<int> degree(size, 1);
    for (int v: code) {
        ++degree[v];
    }

    std::set<int> leaves;
    for (size_t v = 0; v < size; ++v) {
        if (degree[v] == 1) {
            leaves.insert(v);
        }
    }

    Tree t;
    for (int v: code) {
        ensure(!leaves.empty());
        int to = *leaves.begin();
        leaves.erase(leaves.begin());
        if (--degree[v] == 1) {
            leaves.insert(v);
        }

        t.addEdge(v, to);
    }

    ensure(leaves.size() == 2u);
    t.addEdge(*leaves.begin(), *leaves.rbegin());
    t.normalizeEdges();
    return t;
}

inline Tree Tree::random(size_t size, double elongation) {
    Tree t;
    for (size_t v = 1; v < size; ++v) {
        int parent = rnd.tnext<int>(v-1 - (v-1) * elongation, v-1);
        t.addEdge(parent, v);
    }
    t.normalizeEdges();
    return t;
}

inline Tree Tree::star(size_t size) {
    Tree t;
    for (size_t i = 1; i < size; ++i) {
        t.addEdge(0, i);
    }
    t.normalizeEdges();
    return t;
}

inline Tree Tree::caterpillar(size_t length, size_t size) {
    ensure(length <= size);
    Tree t = Tree::bamboo(length);
    for (size_t i = length; i < size; ++i) {
        t.addEdge(rnd.next(length), i);
    }
    t.normalizeEdges();
    return t;
}

} // namespace jngen

using jngen::Tree;


#include <memory>
#include <set>
#include <utility>
#include <vector>

/* Directed graphs are not supported yet, and Graph class itself
 * is pretty useless. Sorry for now.
 */

namespace jngen {

// TODO: make GraphBuilder subclass of Graph
class GraphBuilder;

class Graph : public ReprProxy<Graph>, public GenericGraph {
    friend class GraphBuilder;
public:
    virtual ~Graph() {}
    Graph() {}

    Graph(int n) {
        extend(n);
    }

    Graph(const GenericGraph& gg) : GenericGraph(gg) {}

    void setN(int n);

    static Graph random(int n, int m);

    Graph& allowLoops(bool value = true);
    Graph& allowMulti(bool value = true);
    Graph& connected(bool value = true);

    int n() const override { return self().GenericGraph::n(); }
    int m() const override { return self().GenericGraph::m(); }
    void addEdge(int u, int v, const Weight& w = Weight{}) override {
        self().GenericGraph::addEdge(u, v, w);
    }
    bool isConnected() const override {
        return self().GenericGraph::isConnected();
    }
    Array edges(int v) const override {
        return self().GenericGraph::edges(v);
    }
    Arrayp edges() const override {
        return self().GenericGraph::edges();
    }
    virtual void setVertexWeights(const WeightArray& weights) override {
        self().GenericGraph::setVertexWeights(weights);
    }
    virtual void setVertexWeight(int v, const Weight& weight) override {
        self().GenericGraph::setVertexWeight(v, weight);
    }
    virtual void setEdgeWeights(const WeightArray& weights) override {
        self().GenericGraph::setEdgeWeights(weights);
    }
    virtual void setEdgeWeight(size_t index, const Weight& weight) override {
        self().GenericGraph::setEdgeWeight(index, weight);
    }
    virtual Weight vertexWeight(int v) const override {
        return self().GenericGraph::vertexWeight(v);
    }
    virtual Weight edgeWeight(size_t index) const override {
        return self().GenericGraph::edgeWeight(index);
    }
    int vertexLabel(int v) const override {
        return self().GenericGraph::vertexLabel(v);
    }
    int vertexByLabel(int v) const override {
        return self().GenericGraph::vertexByLabel(v);
    }

    Graph& shuffle();
    Graph shuffled() const;

private:
    void setBuilder(std::shared_ptr<GraphBuilder> builder) {
        builder_ = builder;
    }

    const Graph& self() const;
    Graph& self();

    std::shared_ptr<GraphBuilder> builder_;
};

class GraphBuilder {
public:
    const Graph& graph() {
        if (!finalized_) {
            build();
        }
        return graph_;
    }

    GraphBuilder(int n, int m) :
        n_(n), m_(m)
    {  }

    void allowLoops(bool value) {
        loops_ = value;
    }

    void allowMulti(bool value) {
        multiEdges_ = value;
    }

    void connected(bool value) {
        connected_ = value;
    }

private:
    void build();

    int n_;
    int m_;
    bool connected_ = false;
    bool multiEdges_ = false;
    bool loops_ = false;

    bool finalized_ = false;
    Graph graph_;
};

inline void Graph::setN(int n) {
    ensure(n >= this->n(), "Cannot lessen number of vertices in the graph");
    extend(n);
}

inline Graph& Graph::allowLoops(bool value) {
    ensure(builder_, "Cannot modify the graph which is already built");
    builder_->allowLoops(value);
    return *this;
}

inline Graph& Graph::allowMulti(bool value) {
    ensure(builder_, "Cannot modify the graph which is already built");
    builder_->allowMulti(value);
    return *this;
}

inline Graph& Graph::connected(bool value) {
    ensure(builder_, "Cannot modify the graph which is already built");
    builder_->connected(value);
    return *this;
}

inline void GraphBuilder::build() {
    // the probability distribution is not uniform in some cases
    // but we forget about it for now.

    ensure(!finalized_);
    finalized_ = true;

    int n = n_;
    int m = m_;

    if (!multiEdges_) {
        long long maxEdges = static_cast<long long>(n) *
            (n + (loops_ ? 1 : -1)) / 2;
        ensure(m_ <= maxEdges, "Too many edges in the graph");
    }

    std::set<std::pair<int, int>> usedEdges;

    if (connected_) {
        ensure(m_ >= n_ - 1, "Not enough edges for a connected graph");
        auto treeEdges = Tree::randomPrufer(n).edges();
        usedEdges.insert(treeEdges.begin(), treeEdges.end());
        ensure(usedEdges.size() == static_cast<size_t>(n - 1));
    }

    auto edgeIsGood = [&usedEdges, this](const std::pair<int, int>& edge) {
        if (!loops_ && edge.first == edge.second) {
            return false;
        }
        if (!multiEdges_ && usedEdges.count(edge)) {
            return false;
        }
        return true;
    };

    Arrayp result(usedEdges.begin(), usedEdges.end());

    while (result.size() < static_cast<size_t>(m)) {
        auto edge = rnd.tnext<std::pair<int, int>>(n, opair);
        if (edgeIsGood(edge)) {
            usedEdges.insert(edge);
            result.push_back(edge);
        }
    }

    ensure(result.size() == static_cast<size_t>(m),
        "[INTERNAL ASSERT] Not enough edges found");

    graph_.setN(n);
    for (const auto& edge: result) {
        graph_.addEdge(edge.first, edge.second);
    }

    graph_.normalizeEdges();
}

Graph Graph::random(int n, int m) {
    Graph g;
    auto builder = std::make_shared<GraphBuilder>(n, m);
    g.setBuilder(builder);
    return g;
}

inline Graph& Graph::shuffle() {
    self().doShuffle();
    return *this;
}

inline Graph Graph::shuffled() const {
    Graph g = self();
    return g.shuffle();
}

const Graph& Graph::self() const {
    return builder_ ? builder_->graph() : *this;
}

Graph& Graph::self() {
    if (builder_) {
        *this = builder_->graph();
        builder_.reset();
    }
    return *this;
}

JNGEN_DECLARE_SIMPLE_PRINTER(Graph, 2) {
    t.doPrintEdges(out, mod);
}

} // namespace jngen

using jngen::Graph;
