#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

#include <sstream>
#include <utility>
#include <vector>

template<typename T>
void checkOutput(const T& t, const std::string& s) {
    std::ostringstream ss;
    ss << t;
    BOOST_CHECK_EQUAL(ss.str(), s);
}

BOOST_AUTO_TEST_CASE(output_mod_array) {
    setMod().reset();

    Array a = Array::id(5);

    checkOutput(a, "0 1 2 3 4");
    checkOutput(a.add1(), "1 2 3 4 5");
    checkOutput(a.printN(), "5\n0 1 2 3 4");
    checkOutput(a.endl(), "0\n1\n2\n3\n4");
}

BOOST_AUTO_TEST_CASE(output_mod_invert) {
    setMod().reset().add1().printN().endl();

    Array a = Array::id(5);
    checkOutput(a, "5\n1\n2\n3\n4\n5");
    checkOutput(a.endl(false), "5\n1 2 3 4 5");

    setMod().endl(false);
    checkOutput(a, "5\n1 2 3 4 5");
    checkOutput(a.printN(false), "1 2 3 4 5");
    checkOutput(a.printN(false).add1(false), "0 1 2 3 4");
}

BOOST_AUTO_TEST_CASE(output_mod_graph_and_tree) {
    setMod().reset();

    Graph g;
    g.addEdge(0, 1);
    g.addEdge(1, 2);
    g.addEdge(0, 2);
    g.setN(4);

    checkOutput(g, "0 1\n1 2\n0 2");
    checkOutput(g.printN(), "4\n0 1\n1 2\n0 2");
    checkOutput(g.printN().printM(), "4 3\n0 1\n1 2\n0 2");
    checkOutput(g.printM().add1(), "3\n1 2\n2 3\n1 3");

    Tree t = Tree::bamboo(3);
    checkOutput(t, "0 1\n1 2");
    checkOutput(t.printN(), "3\n0 1\n1 2");
    checkOutput(t.add1().printN(), "3\n1 2\n2 3");
}

BOOST_AUTO_TEST_CASE(output_mod_reset) {
    setMod().reset().add1();

    Array a = Array::id(5);
    checkOutput(a, "1 2 3 4 5");
    checkOutput(a.add1(false), "0 1 2 3 4");
    checkOutput(a.add1(), "1 2 3 4 5");

    setMod().reset();

    checkOutput(a, "0 1 2 3 4");
    checkOutput(a.add1(false), "0 1 2 3 4");
    checkOutput(a.add1(), "1 2 3 4 5");

    setMod().add1();

    checkOutput(a, "1 2 3 4 5");
    checkOutput(a.add1(false), "0 1 2 3 4");
    checkOutput(a.add1(), "1 2 3 4 5");
}

BOOST_AUTO_TEST_CASE(external_repr) {
    setMod().reset();

    std::vector<int> x{0, 1, 2};
    checkOutput(repr(x), "0 1 2");
    checkOutput(repr(x).add1(), "1 2 3");

    std::pair<double, long long> y(0.123456, 31415);

    checkOutput(repr(y), "0.123456 31415");
    checkOutput(repr(y).add1(), "0.123456 31416");
}
