#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

using jngen::Dsu;

BOOST_AUTO_TEST_SUITE(generic_graph)

BOOST_AUTO_TEST_CASE(dsu) {
    Dsu d;

    BOOST_TEST(d.isConnected());

    d.extend(5);
    BOOST_TEST(!d.isConnected());

    BOOST_TEST(d.unite(1, 2));
    BOOST_TEST(d.unite(3, 4));
    BOOST_TEST(d.unite(1, 4));
    BOOST_TEST(!d.unite(2, 3));
    BOOST_TEST(d.unite(0, 3));
    BOOST_TEST(d.isConnected());

    BOOST_TEST(!d.unite(0, 0));
}

BOOST_AUTO_TEST_SUITE_END()
