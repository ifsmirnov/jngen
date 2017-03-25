#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

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

    BOOST_CHECK_EQUAL(ss.str(), "4 6\n1 3\n2 4\n1 4\n1 4\n2 3\n3 2\n");
}

BOOST_AUTO_TEST_CASE(weights_and_labelling) {
    rnd.seed(123);

    Graph g = rndg.random(10, 20);

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
    BOOST_CHECK_EQUAL(v1, g.n());
    BOOST_CHECK_EQUAL(v2, g.m());

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

    BOOST_CHECK_EQUAL(count, 1);
}
