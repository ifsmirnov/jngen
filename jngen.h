
#include <bits/stdc++.h>

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

#include <bits/stdc++.h>


namespace impl {

std::mt19937 randomEngine;

static void assertRandomEngineConsistency() {
    std::mt19937 engine(1234);
    ensure(engine() == 822569775);
    ensure(engine() == 2137449171);
    ensure(engine() == 2671936806);
}

template<typename T>
struct TypedRandom;

class Random {
public:
    Random() {
        assertRandomEngineConsistency();
        randomEngine.seed(std::random_device{}());

        static bool created = false;
        ensure(!created, "impl::Random should be created only once");
        created = true;
    }

    void seed(size_t val) {
        randomEngine.seed(val);
    }

    uint32_t next() {
        return randomEngine();
    }

    int next(int n) {
        // TODO(ifsmirnov): make random more uniform
        return randomEngine() % n;
    }

    long long next(long long n) {
        // TODO(ifsmirnov): make random more uniform
        return ((randomEngine() << 32) ^ randomEngine()) % n;
    }

    size_t next(size_t n) {
        // TODO(ifsmirnov): make random more uniform
        return ((randomEngine() << 32) ^ randomEngine()) % n;
    }

    double next(double n) {
        return (double)randomEngine() / randomEngine.max() * n;
    }

    int next(int l, int r) {
        return l + next(r-l+1);
    }

    long long next(long long l, long long r) {
        return r + next(r-l+1);
    }

    size_t next(size_t l, size_t r) {
        return l + next(r-l+1);
    }

    double next(double l, double r) {
        return l + next(r-l);
    }

    template<typename T, typename ... Args>
    T tnext(Args... args) {
        return TypedRandom<T>::next(args...);
    }
};

Random rnd;

template<>
struct TypedRandom<int> {
    static int next(int n) { return rnd.next(n); }
    static int next(int l, int r) { return rnd.next(l, r); }
};

template<>
struct TypedRandom<double> {
    static double next(double n) { return rnd.next(n); }
    static double next(double l, double r) { return rnd.next(l, r); }
};

template<>
struct TypedRandom<long long> {
    static long long next(long long n) { return rnd.next(n); }
    static long long next(long long l, long long r) { return rnd.next(l, r); }
};

template<>
struct TypedRandom<size_t> {
    static size_t next(size_t n) { return rnd.next(n); }
    static size_t next(size_t l, size_t r) { return rnd.next(l, r); }
};

struct OrderedPairTag {} opair;

template<>
struct TypedRandom<std::pair<int, int>> {
    static std::pair<int, int> next(int n) {
        // can't write 'return {rnd.next(n), rnd.next(n)}' because order of
        // evaluation of function arguments is unspecified.
        int first = rnd.next(n);
        int second = rnd.next(n);
        return {first, second};
    }
    static std::pair<int, int> next(int l, int r) {
        int first = rnd.next(l, r);
        int second = rnd.next(l, r);
        return {first, second};
    }

    static std::pair<int, int> next(int n, OrderedPairTag) {
        return ordered(next(n));
    }
    static std::pair<int, int> next(int l, int r, OrderedPairTag) {
        return ordered(next(l, r));
    }

private:
    static std::pair<int, int> ordered(std::pair<int, int> pair) {
        if (pair.first > pair.second) {
            std::swap(pair.first, pair.second);
        }
        return pair;
    }
};

} // namespace impl

using impl::rnd;
using impl::opair;

void registerGen(int argc, char *argv[]) {
    size_t val = 0;
    for (int i = 0; i < argc; ++i) {
        for (char *s = argv[i]; *s; ++s) {
            val = val * 10099 + *s;
        }
    }
    rnd.seed(val);
}

#include <bits/stdc++.h>

namespace impl {

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

    bool connected() const { return components == 1; }

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

} // namespace impl


#include <bits/stdc++.h>

namespace impl {

template<int N> struct PTag : PTag<N-1> {};
template<> struct PTag<0> {};
struct PTagMax : PTag<20> {};

struct OutputModifier {
    int addition = 0;
    bool printN = false;
    bool printM = false;

    bool printParents = false;
    bool printEdges = true;
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

    Repr<T>& add1() {
        ++mod_.addition;
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

private:
    void print(std::ostream& out) const {
        printValue(out, object_, mod_, PTagMax{});
    }

    const T& object_;

protected:
    OutputModifier mod_;
};

template<typename T>
class ReprProxy {
    friend std::ostream& operator<<(std::ostream& out, const ReprProxy& proxy) {
        Repr<T> repr(static_cast<const T&>(proxy));
        return out << repr;
    }

public:
    Repr<T> add1() {
        Repr<T> repr(static_cast<const T&>(*this));
        repr.add1();
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

} // namespace impl

using impl::repr;
using impl::setMod;

#include <bits/stdc++.h>


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

#include <bits/stdc++.h>


namespace impl {

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
    using Base::operator[];
    using Base::size;
    using Base::begin;
    using Base::end;
    using Base::insert;
    using Base::clear;

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

    GenericArray<T> inverse() const;

    template<typename Integer>
    GenericArray<T> subseq(const std::vector<Integer>& indices) const;

    template<typename Integer>
    GenericArray<T> subseq(
        const std::initializer_list<Integer>& indices) const;

    const T& choice() const;
    GenericArray<T> choice(size_t count) const;
    GenericArray<T> choiceWithRepetition(size_t count) const;

    GenericArray<T>& operator+=(const GenericArray<T>& other);
    GenericArray<T> operator+(const GenericArray<T>& other) const;

    GenericArray<T>& operator*=(int k);
    GenericArray<T> operator*(int k) const;
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
template<typename ...Args>
GenericArray<T> GenericArray<T>::randomUnique(
        size_t size, const Args& ... args)
{
    typename detail::DictContainer<T>::type set;
    GenericArray<T> result;
    result.reserve(size);

    size_t retries = (size + 10) * log(size + 10) * 2;

    while (result.size() != size) {
        if (--retries == 0) {
            ensure(false, "There are not enough unique elements");
        }

        T t = rnd.tnext<T>(args...);
        if (!set.count(t)) {
            set.insert(t);
            result.push_back(t);
        }
    }

    return result;
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
    for (size_t i = 1; i < size(); ++i) {
        std::swap(at(i), at(rnd.next(i + 1)));
    }
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
const T& GenericArray<T>::choice() const {
    return at(rnd.next(size()));
}

template<typename T>
GenericArray<T> GenericArray<T>::choice(size_t count) const {
    ensure(count <= size());

    size_t n = size();

    std::unordered_map<T, T> used;
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

} // namespace impl

template<typename T>
using TArray = impl::GenericArray<T>;

using Array = impl::GenericArray<int>;
using Array64 = impl::GenericArray<long long>;
using Arrayf = impl::GenericArray<double>;
using Arrayp = impl::GenericArray<std::pair<int, int>>;

template<typename T>
impl::GenericArray<T> makeArray(const std::vector<T>& values) {
    return impl::GenericArray<T>(values);
}

template<typename T>
impl::GenericArray<T> makeArray(const std::initializer_list<T>& values) {
    return impl::GenericArray<T>(values);
}

#include <bits/stdc++.h>


namespace impl {

class GenericGraph {
public:
    virtual ~GenericGraph() {}

    virtual int n() const { return adjList_.size(); }
    virtual int m() const { return numEdges_; }

    virtual void addEdge(int u, int v);
    virtual bool connected() const { return dsu_.connected(); }

    virtual int vertexLabel(int v) const { return vertexLabel_[v]; }
    virtual int vertexByLabel(int v) const { return vertexByLabel_[v]; }

    virtual const std::vector<int>& edges(int v) const {
        return adjList_[v];
    }

    virtual std::vector<std::pair<int, int>> edges() const {
        std::vector<std::pair<int, int>> result;
        for (int v = 0; v < n(); ++v) {
            for (int to: edges(v)) {
                if (v <= to) {
                    result.emplace_back(vertexLabel(v), vertexLabel(to));
                }
            }
        }
        return result;
    }

    virtual void doPrintEdges(
        std::ostream& out, const OutputModifier& mod) const;

    virtual bool operator==(const GenericGraph& other) const;
    virtual bool operator<(const GenericGraph& other) const;

protected:
    void doShuffle() {
        if (vertexLabel_.size() < static_cast<size_t>(n())) {
            vertexLabel_ = Array::id(n());
        }
        vertexLabel_.shuffle();
        vertexByLabel_ = vertexLabel_.inverse();
    }

    void extend(size_t size) {
        size_t oldSize = n();
        if (size > oldSize) {
            adjList_.resize(size);
            vertexLabel_ += Array::id(size - oldSize, oldSize);
            vertexByLabel_ += Array::id(size - oldSize, oldSize);
        }
    }

    void addEdgeUnsafe(int u, int v) {
        adjList_[u].push_back(v);
        if (u != v) {
            adjList_[v].push_back(u);
        }
    }

    int compareTo(const GenericGraph& other) const;

    int numEdges_ = 0;

    Dsu dsu_;
    std::vector<std::vector<int>> adjList_;
    Array vertexLabel_;
    Array vertexByLabel_;
};

inline void GenericGraph::addEdge(int u, int v) {
    extend(std::max(u, v) + 1);
    dsu_.link(u, v);
    addEdgeUnsafe(u, v);
    ++numEdges_;
}

inline void GenericGraph::doPrintEdges(
    std::ostream& out, const OutputModifier& mod) const
{
    Arrayp edges;
    for (int v = 0; v < n(); ++v) {
        for (int to: this->edges(v)) {
            if (v <= to) {
                edges.emplace_back(vertexLabel(v), vertexLabel(to));
            }
        }
    }

    if (mod.printN) {
        out << n();
        if (mod.printM) {
            out << " " << m();
        }
        out << "\n";
    } else if (mod.printM) {
        out << m() << "\n";
    }

    auto t(mod);
    {
        auto mod(t);
        mod.printN = false;
        JNGEN_PRINT(edges);
    }
}

inline bool GenericGraph::operator==(const GenericGraph& other) const {
    return compareTo(other) == 0;
}

inline bool GenericGraph::operator<(const GenericGraph& other) const {
    return compareTo(other) == -1;
}

inline int GenericGraph::compareTo(const GenericGraph& other) const {
    if (n() < other.n()) {
        return -1;
    }
    if (n() > other.n()) {
        return 1;
    }
    for (int i = 0; i < n(); ++i) {
        std::set<int> edges1(edges(i).begin(), edges(i).end());
        std::set<int> edges2(other.edges(i).begin(), other.edges(i).end());
        if (edges1 < edges2) {
            return -1;
        }
        if (edges1 > edges2) {
            return 1;
        }
    }
    return 0;
}

} // namespace impl


#include <bits/stdc++.h>


namespace impl {

class Tree : public ReprProxy<Tree>, public GenericGraph {
public:
    Tree() {
        extend(1);
    }
    Tree(const GenericGraph& gg) : GenericGraph(gg) {
        extend(1);
    }

    void addEdge(int u, int v);

    Tree& shuffle();
    Tree shuffled() const;

    static Tree bamboo(size_t size);
    static Tree randomPrufer(size_t size);
    static Tree random(size_t size, double elongation = 1.0);
};

inline void Tree::addEdge(int u, int v) {
    extend(std::max(u, v) + 1);

    int ret = dsu_.link(u, v);
    ensure(ret, "A cycle appeared in the tree :(");

    addEdgeUnsafe(u, v);
}

inline Tree& Tree::shuffle() {
    doShuffle();
    return *this;
}

inline Tree Tree::shuffled() const {
    Tree t = *this;
    return t.shuffle();
}

JNGEN_DECLARE_SIMPLE_PRINTER(Tree, 0) {
    ensure(t.connected(), "Tree is not connected :(");

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
    return t;
}

inline Tree Tree::random(size_t size, double elongation) {
    Tree t;
    for (size_t v = 1; v < size; ++v) {
        int parent = rnd.tnext<int>(v-1 - (v-1) * elongation, v-1);
        t.addEdge(parent, v);
    }
    return t;
}

} // namespace impl

using impl::Tree;

#include <bits/stdc++.h>


namespace impl {

// TODO: make GraphBuilder subclass of Graph
class GraphBuilder;

class Graph : public ReprProxy<Graph>, public GenericGraph {
public:
    virtual ~Graph() {}
    Graph() {}

    Graph(int n) {
        extend(n);
    }

    Graph(const GenericGraph& gg) : GenericGraph(gg) {}

    void setN(int n);

    static Graph random(int n, int m);
    static Graph randomConnected(int n, int m);

    Graph& allowLoops(bool value = true);
    Graph& allowMulti(bool value = true);

    int n() const { return self().GenericGraph::n(); }
    int m() const { return self().GenericGraph::m(); }
    void addEdge(int u, int v) {
        self().GenericGraph::addEdge(u, v);
    }
    bool connected() const {
        return self().GenericGraph::connected();
    }
    const std::vector<int>& edges(int v) const {
        return self().GenericGraph::edges(v);
    }
    std::vector<std::pair<int, int>> edges() const {
        return self().GenericGraph::edges();
    }
    int vertexLabel(int v) const {
        return self().GenericGraph::vertexLabel(v);
    }
    int vertexByLabel(int v) const {
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

    GraphBuilder(int n, int m, bool connected) :
        n_(n), m_(m), connected_(connected)
    {  }

    void allowLoops(bool value) {
        loops_ = value;
    }

    void allowMulti(bool value) {
        multiEdges_ = value;
    }

private:
    void build();

    int n_;
    int m_;
    bool connected_;
    bool multiEdges_ = false;
    bool loops_ = false;

    bool finalized_;
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

    int edgesNeeded = m;
    if (!loops_) {
        edgesNeeded += n;
    }
    if (!multiEdges_) {
        edgesNeeded = std::min<long long>(
            edgesNeeded, static_cast<long long>(n * (n+1) / 2));
    }

    Arrayp result(usedEdges.begin(), usedEdges.end());

    Arrayp candidates = multiEdges_ ?
        Arrayp::random(edgesNeeded, n, opair) :
        Arrayp::randomUnique(edgesNeeded, n, opair);

    for (const auto& edge: candidates) {
        if (result.size() == static_cast<size_t>(m)) {
            break;
        }
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
}

Graph Graph::random(int n, int m) {
    Graph g;
    auto builder = std::make_shared<GraphBuilder>(n, m, false);
    g.setBuilder(builder);
    return g;
}

Graph Graph::randomConnected(int n, int m) {
    Graph g;
    auto builder = std::make_shared<GraphBuilder>(n, m, true);
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

} // namespace impl

using impl::Graph;
