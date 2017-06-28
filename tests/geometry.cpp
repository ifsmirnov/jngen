#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

BOOST_AUTO_TEST_SUITE(geometry)

BOOST_AUTO_TEST_CASE(comparators) {
    using namespace jngen;

    BOOST_CHECK(lt(0.0, 1e-5));
    BOOST_CHECK(lt((long double)0.0, 1e-5));
    BOOST_CHECK(lt(0, 1e-5));
    BOOST_CHECK(!eq(0, 1e-5));
    BOOST_CHECK(eq(1 + 1e-10, 1 - 1e-10));
}

BOOST_AUTO_TEST_CASE(polygon) {
    rnd.seed(123);
    auto p = rndg.convexPolygon(10, 100, 100);
    BOOST_CHECK_EQUAL(p.size(), 10);
}

BOOST_AUTO_TEST_CASE(points_in_general_position) {
    rnd.seed(123);

    int n = 5;
    for (int test = 0; test < 10; ++test) {
        n += 5;
        auto pts = rndg.pointsInGeneralPosition(n, n);
        for (int i = 0; i < n; ++i) {
            for (int j = 0; j < i; ++j) {
                for (int k = 0; k < j; ++k) {
                    BOOST_CHECK((pts[i] - pts[j]) % (pts[i] - pts[k]) != 0);
                }
            }
        }
    }
}

BOOST_AUTO_TEST_SUITE_END()
