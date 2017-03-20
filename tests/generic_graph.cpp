#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

using jngen::GenericGraph;

#include <sstream>

BOOST_AUTO_TEST_CASE(basics) {
    GenericGraph gg;

    BOOST_CHECK(gg.n() == 0);
    BOOST_CHECK(gg.m() == 0);
    BOOST_CHECK(gg.isConnected());
    BOOST_CHECK(gg.edges().empty());

    gg.addEdge(0, 2);
    gg.addEdge(1, 3);
    BOOST_CHECK(gg.edges().size() == 2);
    BOOST_CHECK(gg.edges()[0] == std::make_pair(0, 2));
    BOOST_CHECK(gg.edges()[1] == std::make_pair(1, 3));
    BOOST_CHECK(gg.n() == 4);
    BOOST_CHECK(gg.m() == 2);
    BOOST_CHECK(!gg.isConnected());

    gg.addEdge(0, 3);
    BOOST_CHECK(gg.isConnected());
    gg.addEdge(0, 3);
    BOOST_CHECK(gg.m() == 4);

    auto g2 = gg;
    BOOST_CHECK(gg == g2);
    g2.addEdge(1, 2);
    BOOST_CHECK(!(gg == g2));
}
