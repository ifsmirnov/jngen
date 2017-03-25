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

BOOST_AUTO_TEST_CASE(test_choice) {
    BOOST_CHECK(true);

    std::vector<int> a;
    for (int i = 0; i < 10; ++i) {
        a.push_back(i);
    }

    shuffle(a.begin(), a.end());
    choice(a.begin(), a.end());
    choice(a);

    rnd.choice(a.begin(), a.end());
    rnd.choice(a);

    std::set<int> b(a.begin(), a.end());
    choice(b.begin(), b.end());
    choice(b);

    rnd.choice(b.begin(), b.end());
    rnd.choice(b);
}

BOOST_AUTO_TEST_CASE(wnext) {
    rnd.seed(987);

    rnd.wnext(0.1, 1);
    rnd.wnext(1.0, 2.0, 2);

    rnd.wnext(10, 3);
    rnd.wnext(10, 20, -3);

    rnd.wnext(100ll, 0);
    rnd.wnext(100ll, 200ll, -40);

    auto a = rnda.randomf(10000, []() { return rnd.wnext(1, 10, 2); });
    a.sort().unique();
    ensure(a.size() == 10u);
    ensure(a[0] == 1);
    ensure(a[9] == 10);
}

BOOST_AUTO_TEST_CASE(signed_bounds) {
    BOOST_CHECK(true);

    rnd.next(int(-2e9), int(2e9));
    rnd.wnext((long long)(-6e18), (long long)(6e18), 10);
    rnd.wnext((long long)(-6e18), (long long)(6e18), -4);
}
