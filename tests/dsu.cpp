#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

using jngen::Dsu;

BOOST_AUTO_TEST_CASE(dsu) {
    Dsu d;

    BOOST_CHECK(d.isConnected());

    d.getParent(4);
    BOOST_CHECK(!d.isConnected());

    BOOST_CHECK(d.link(1, 2));
    BOOST_CHECK(d.link(3, 4));
    BOOST_CHECK(d.link(1, 4));
    BOOST_CHECK(!d.link(2, 3));
    BOOST_CHECK(d.link(0, 3));
    BOOST_CHECK(d.isConnected());

    BOOST_CHECK(!d.link(0, 0));
}
