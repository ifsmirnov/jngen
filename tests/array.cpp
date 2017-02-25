#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

BOOST_AUTO_TEST_CASE(basics) {
    Array a, b;

    a = Array::id(5);
    b = {0, 1, 2, 3, 4};
    BOOST_CHECK_EQUAL(a, b);

    a = Array::id(5, 1);
    b = {1, 2, 3, 4, 5};
    BOOST_CHECK_EQUAL(a, b);

    a.reverse();
    b = {5, 4, 3, 2, 1};
    BOOST_CHECK_EQUAL(a, b);

    a = b.reversed();
    b.reverse();
    BOOST_CHECK_EQUAL(a, b);

    a = {4, 2, 3, 1, 5};
    b = a.sorted();
    BOOST_CHECK(a != b);
    a.sort();
    BOOST_CHECK_EQUAL(a, b);

    auto comp = [](int x, int y) { return x > y; };
    a = {4, 2, 3, 1, 5};
    b = a.sorted(comp);
    BOOST_CHECK(a != b);
    a.sort(comp);
    BOOST_CHECK_EQUAL(a, b);
    b = {5, 4, 3, 2, 1};
    BOOST_CHECK_EQUAL(a, b);

    a = {1, 3, 4, 0, 2};
    b = {3, 0, 4, 1, 2};
    BOOST_CHECK_EQUAL(a.inverse(), b);
    BOOST_CHECK_EQUAL(a, b.inverse());

    a = {1, 2, 3, 3, 2, 1};
    b = a.sorted().uniqued();
    BOOST_CHECK(a != b);
    a.sort().unique();
    BOOST_CHECK_EQUAL(a, b);
    b = {1, 2, 3};
    BOOST_CHECK_EQUAL(a, b);

    a = Array::id(10, 1);
    b = {3, 5, 7};
    auto c = a.subseq({2, 4, 6});
    BOOST_CHECK_EQUAL(b, c);
    c = a.subseq(std::vector<int>{2, 4, 6});
    BOOST_CHECK_EQUAL(b, c);
}

BOOST_AUTO_TEST_CASE(operators) {
    auto a = Array::id(3, 1);
    a += Array{2};
    a = a + Array::id(3, 10);
    Array b{1, 2, 3, 2, 10, 11, 12};
    BOOST_CHECK_EQUAL(a, b);

    b += b;
    b += b;
    a = a * 2;
    a *= 2;
    BOOST_CHECK_EQUAL(a, b);
}

BOOST_AUTO_TEST_CASE(random_selection) {
    auto a = Array::id(10);

    for (int i = 0; i < 10; ++i) {
        int x = a.choice();
        BOOST_CHECK(0 <= x && x < 10);
    }

    auto b = a.choice(5);
    for (int x: b) {
        BOOST_CHECK(0 <= x && x < 10);
    }

    b = a.choice(a.size());
    for (int x: b) {
        BOOST_CHECK(0 <= x && x < 10);
    }

    b = a.choiceWithRepetition(100);
    for (int x: a) {
        auto it = find(b.begin(), b.end(), x);
        BOOST_CHECK(it != b.end());
    }
}

BOOST_AUTO_TEST_CASE(random_generation) {
    auto a = Array::random(100, 10);
    BOOST_CHECK_EQUAL(a.sorted().uniqued(), Array::id(10));

    a = Array::random(100, 10, 20);
    BOOST_CHECK_EQUAL(a.sorted().uniqued(), Array::id(11, 10));

    a = Array::randomUnique(11, 10, 20);
    BOOST_CHECK_EQUAL(a.sorted().uniqued(), Array::id(11, 10));
}
