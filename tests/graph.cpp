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

    BOOST_CHECK_EQUAL(ss.str(), "4 6\n1 3\n1 4\n1 4\n2 4\n2 3\n2 3\n");
}
