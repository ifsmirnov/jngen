#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

#include <algorithm>
#include <utility>

BOOST_AUTO_TEST_SUITE(array)

BOOST_AUTO_TEST_CASE(basics) {
    Array a, b;

    a = Array::id(5);
    b = {0, 1, 2, 3, 4};
    BOOST_TEST(a == b);

    a = Array::id(5, 1);
    b = {1, 2, 3, 4, 5};
    BOOST_TEST(a == b);

    a.reverse();
    b = {5, 4, 3, 2, 1};
    BOOST_TEST(a == b);

    a = b.reversed();
    b.reverse();
    BOOST_TEST(a == b);

    a = {4, 2, 3, 1, 5};
    b = a.sorted();
    BOOST_TEST(a != b);
    a.sort();
    BOOST_TEST(a == b);

    auto comp = [](int x, int y) { return x > y; };
    a = {4, 2, 3, 1, 5};
    b = a.sorted(comp);
    BOOST_TEST(a != b);
    a.sort(comp);
    BOOST_TEST(a == b);
    b = {5, 4, 3, 2, 1};
    BOOST_TEST(a == b);

    a = {1, 3, 4, 0, 2};
    b = {3, 0, 4, 1, 2};
    BOOST_TEST(a.inverse() == b);
    BOOST_TEST(a == b.inverse());

    a = {1, 2, 3, 3, 2, 1};
    b = a.sorted().uniqued();
    BOOST_TEST(a != b);
    a.sort().unique();
    BOOST_TEST(a == b);
    b = {1, 2, 3};
    BOOST_TEST(a == b);

    a = Array::id(10, 1);
    b = {3, 5, 7};
    auto c = a.subseq({2, 4, 6});
    BOOST_TEST(b == c);
    c = a.subseq(std::vector<int>{2, 4, 6});
    BOOST_TEST(b == c);
}

BOOST_AUTO_TEST_CASE(operators) {
    auto a = Array::id(3, 1);
    a += Array{2};
    a = a + Array::id(3, 10);
    Array b{1, 2, 3, 2, 10, 11, 12};
    BOOST_TEST(a == b);

    b += b;
    b += b;
    a = a * 2;
    a *= 2;
    BOOST_TEST(a == b);
}

BOOST_AUTO_TEST_CASE(random_selection) {
    rnd.seed(123);

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
    rnd.seed(456);

    auto a = Array::random(100, 10);
    BOOST_TEST(a.sorted().uniqued() == Array::id(10));

    a = Array::random(100, 10, 20);
    BOOST_TEST(a.sorted().uniqued() == Array::id(11, 10));

    a = Array::randomUnique(11, 10, 20);
    BOOST_TEST(a.sorted().uniqued() == Array::id(11, 10));

    auto b = Arrayp::random(100, 10, opair);
    for (const auto& kv: b) {
        BOOST_TEST(kv.first <= kv.second);
    }

    b = Arrayp::randomUnique(100, 10);
    BOOST_TEST(b.size() == 100);

    b = Arrayp::randomUnique(55, 1, 10, opair);

    auto c = Arrayp::randomfUnique(55, []() {
        int x = rnd.next(1, 10);
        int y = rnd.next(1, 10);
        return std::make_pair(std::min(x, y), std::max(x, y));
    });

    BOOST_TEST(b.sorted() == c.sorted());

    c = Arrayp::randomf(1000, []() {
        int x = rnd.next(1, 10);
        int y = rnd.next(1, 10);
        return std::make_pair(std::min(x, y), std::max(x, y));
    });

    BOOST_TEST(b.sorted() == c.sorted().uniqued());
}

BOOST_AUTO_TEST_CASE(print_matrix) {
    Array2d a = {{1, 2, 3}, {4, 5, 6}};
    std::ostringstream out;

    setMod().reset();

    out.str("");
    out << a << std::endl;
    BOOST_TEST(out.str() == "1 2 3\n4 5 6\n");

    out.str("");
    out << a.printN() << std::endl;
    BOOST_TEST(out.str() == "2\n1 2 3\n4 5 6\n");

    out.str("");
    out << a.printN().printM() << std::endl;
    BOOST_TEST(out.str() == "2 3\n1 2 3\n4 5 6\n");

    out.str("");
    out << a.printN().printM().add1() << std::endl;
    BOOST_TEST(out.str() == "2 3\n2 3 4\n5 6 7\n");

    a.clear();
    out.str("");
    out << a.printN().printM().add1() << std::endl;
    BOOST_TEST(out.str() == "0 0\n");
}

BOOST_AUTO_TEST_CASE(interleave) {
    auto a = Array::id(3, 1);
    auto b = Array::id(3, 11);
    auto expected = Array{11, 1, 2, 12, 13, 3};

    rnd.seed(10);

    BOOST_TEST(jngen::interleave({a, b}) == expected);
}

BOOST_AUTO_TEST_SUITE_END()
