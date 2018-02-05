#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

BOOST_AUTO_TEST_SUITE(graph)

BOOST_AUTO_TEST_CASE(output) {
    Graph g;
    g.addEdge(0, 2);
    g.addEdge(1, 3);
    g.addEdge(0, 3);
    g.addEdge(0, 3);
    g.addEdge(1, 2);
    g.addEdge(2, 1);

    std::ostringstream ss;
    ss << g.printN().printM().add1() << std::endl;

    BOOST_TEST(ss.str() == "4 6\n1 3\n2 4\n1 4\n1 4\n2 3\n3 2\n");
}

BOOST_AUTO_TEST_CASE(weights_and_labelling) {
    rnd.seed(123);

    Graph g = Graph::random(10, 20);

    g.setVertexWeights(Array::random(g.n(), 100));
    g.setEdgeWeights(Arrayf::random(g.m(), 1.5, 1.8));

    g.setVertexWeight(5, "five");
    g.setVertexWeight(8, "eight");
    g.addEdge(5, 8, "5-8");

    g.shuffle();

    std::stringstream ss;
    ss << g.printN().printM() << std::endl;

    int v1, v2;
    int v5 = -1, v8 = -1;
    std::string s;
    ss >> v1 >> v2;
    BOOST_TEST(v1 == g.n());
    BOOST_TEST(v2 == g.m());

    for (int i = 0; i < g.n(); ++i) {
        ss >> s;
        if (s == "five") {
            v5 = i;
        } else if (s == "eight") {
            v8 = i;
        }
    }

    BOOST_CHECK(v5 != -1 && v8 != -1);

    int count = 0;
    for (int i = 0; i < g.m(); ++i) {
        ss >> v1 >> v2 >> s;
        if (s == "5-8") {
            ++count;
            BOOST_CHECK( (v1 == v5 && v2 == v8) || (v1 == v8 && v2 == v5) );
        }
    }

    BOOST_TEST(count == 1);
}

template<typename T>
void generateWithTraitsMask(T&& generator, const std::string& name, int mask) {
    if (mask&(1<<0)) generator.allowAntiparallel();
    if (mask&(1<<1)) generator.allowLoops();
    if (mask&(1<<2)) generator.allowMulti();
    if (mask&(1<<3)) generator.connected();
    if (mask&(1<<4)) generator.directed();
    if (mask&(1<<5)) generator.acyclic();
    try {
        generator.g();
    } catch (jngen::Exception) {
        // directed acyclic cycle
        if (name == "cycle" && (mask & ((1<<4) | (1<<5)))) {
            return;
        }

        // connected empty graph
        if (name == "empty" && (mask & (1<<3))) {
            return;
        }

        throw;

        /*
        // left here for debug purposes
        std::cerr << name << ": ";
        for (int i = 0; i < 6; ++i) {
            if (mask&(1<<i)) {
                std::cerr << "+";
            } else {
                std::cerr << "-";
            }
        }
        std::cerr << std::endl;
        */
    }
}

BOOST_AUTO_TEST_CASE(various_traits) {
    BOOST_CHECK(true);

    for (int mask = 0; mask < (1<<6); ++mask) {
        generateWithTraitsMask(Graph::random(10, 15), "random", mask);
        generateWithTraitsMask(
                Graph::randomStretched(10, 15, 5, 5), "randomStretched", mask);
        generateWithTraitsMask(Graph::complete(10), "complete", mask);
        generateWithTraitsMask(Graph::cycle(10), "cycle", mask);
        generateWithTraitsMask(Graph::empty(10), "empty", mask);
    }
}

BOOST_AUTO_TEST_CASE(empty_output) {
    setMod().reset();

    std::stringstream ss;

    ss << Graph::random(10, 0).g().printN().printM().add1() << std::endl;
    BOOST_TEST(ss.str() == "10 0\n");
    ss.str("");

    ss << Graph::random(10, 0).g().printN().add1() << std::endl;
    BOOST_TEST(ss.str() == "10\n");
    ss.str("");

    Graph g = Graph::random(10, 0);
    g.setVertexWeights(Array::random(g.n(), 10, 20));
    ss << g.printN().printM() << std::endl;
    auto s = ss.str();
    s.pop_back();
    BOOST_TEST(s != "\n");
    ss.str("");

    g = Graph::empty(5);
    g.setVertexWeights(Array::id(5));
    ss << g << std::endl;
    BOOST_TEST(ss.str() == "0 1 2 3 4\n");
    ss.str("");

    g = Graph::empty(0);
    ss << g.printN().printM() << std::endl;
    BOOST_TEST(ss.str() == "0 0\n");
    ss.str("");
}

BOOST_AUTO_TEST_SUITE_END()
