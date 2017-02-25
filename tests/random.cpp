#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

#include <algorithm>
#include <string>
#include <vector>

BOOST_AUTO_TEST_CASE(basic_methods) {
    rnd.seed(123);

    BOOST_CHECK_EQUAL(rnd.next(), 2991312382);
    BOOST_CHECK_EQUAL(rnd.next64(), 13151704351418379646ull);
    BOOST_CHECK_CLOSE(rnd.nextf(), 0.42847092506, 1e-5);
    BOOST_CHECK_EQUAL(rnd.next(123), 92);
    BOOST_CHECK_EQUAL(rnd.next(1000000000000000000ll), 62439489592817382ll);
    BOOST_CHECK_CLOSE(rnd.next(123.0), 88.455487997, 1e-5);
    BOOST_CHECK_EQUAL(rnd.next(100, 500), 183);
    BOOST_CHECK_EQUAL(rnd.next(10000ll, 50000ll), 33766);
    BOOST_CHECK_CLOSE(rnd.next(0.1, 0.2), 0.14109243679, 1e-5);

    auto p = rnd.nextp(100);
    BOOST_CHECK_EQUAL(p.first, 96);
    BOOST_CHECK_EQUAL(p.second, 47);

    BOOST_CHECK_EQUAL(rnd.next("[a-z]{1,20}"), "aoapztoeaq");
}

template<typename T>
void checkEndpointsHit(const std::vector<T>& a, T min, T max) {
    BOOST_CHECK(!a.empty());
    BOOST_CHECK_EQUAL(*std::min_element(a.begin(), a.end()), min);
    BOOST_CHECK_EQUAL(*std::max_element(a.begin(), a.end()), max);
}

BOOST_AUTO_TEST_CASE(range_endpoints) {
    rnd.seed(123);
    std::vector<int> a;
    for (int i = 0; i < 100; ++i) {
        a.push_back(rnd.next(10));
    }
    checkEndpointsHit(a, 0, 9);

    a.clear();
    for (int i = 0; i < 100; ++i) {
        a.push_back(rnd.next(10, 20));
    }
    checkEndpointsHit(a, 10, 20);

    a.clear();
    for (int i = 0; i < 100; ++i) {
        a.push_back(rnd.next(-5, 5));
    }
    checkEndpointsHit(a, -5, 5);
}

BOOST_AUTO_TEST_CASE(mask_for_bound) {
    using jngen::maskForBound;
    BOOST_CHECK_EQUAL(maskForBound(1), 1);
    BOOST_CHECK_EQUAL(maskForBound(2), 1);
    BOOST_CHECK_EQUAL(maskForBound(10), 15);
    BOOST_CHECK_EQUAL(maskForBound(900), 1023);
    BOOST_CHECK_EQUAL(maskForBound(1023), 1023);
    BOOST_CHECK_EQUAL(maskForBound(1024), 1023);
    BOOST_CHECK_EQUAL(maskForBound(1025), 2047);
}

BOOST_AUTO_TEST_CASE(patterns) {
    // TODO: add tests for incorrect patterns
    BOOST_CHECK(true);

    std::vector<std::string> patterns = {
        "aa",
        "a|b",
        "a|b|c",
        "(a|b)|c",
        "a[z]{1}",
        "(a|b){1,2}",
        "(a|(a|b{4,10}|asfsf)){4324}|sadfjh{23423}(wr|sdf)"
    };
    for (auto s: patterns) {
        rnd.next(s);
    }
}

std::vector<uint32_t> generate(Random& random) {
    std::vector<uint32_t> result;
    for (int i = 0; i < 10; ++i) {
        result.push_back(random.next());
    }
    return result;
}

BOOST_AUTO_TEST_CASE(several_engines) {
    rnd.seed(123);
    auto etalon = generate(rnd);

    Random r1;
    r1.seed(123);
    BOOST_CHECK(etalon == generate(r1));

    r1.seed(123);
    BOOST_CHECK(etalon == generate(r1));

    etalon = generate(rnd);

    Random r2;
    r2.seed(123);
    generate(r2);

    BOOST_CHECK(etalon == generate(r2));
    BOOST_CHECK(etalon == generate(r1));
}
