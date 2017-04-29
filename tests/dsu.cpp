#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

using jngen::Dsu;

BOOST_AUTO_TEST_CASE(dsu) {
    Dsu d;

    BOOST_CHECK(d.isConnected());

    d.extend(5);
    BOOST_CHECK(!d.isConnected());

    BOOST_CHECK(d.unite(1, 2));
    BOOST_CHECK(d.unite(3, 4));
    BOOST_CHECK(d.unite(1, 4));
    BOOST_CHECK(!d.unite(2, 3));
    BOOST_CHECK(d.unite(0, 3));
    BOOST_CHECK(d.isConnected());

    BOOST_CHECK(!d.unite(0, 0));
}
