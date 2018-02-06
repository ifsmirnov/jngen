#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

BOOST_AUTO_TEST_SUITE(exact_generators)

template<typename T>
void check(const T& t, const std::string& expected) {
    setMod().reset();
    std::stringstream ss;
    ss << t;
    BOOST_TEST(ss.str() == expected);
}

BOOST_AUTO_TEST_CASE(random) {
    rnd.seed(123);
    Array a;
    a.push_back(rnd.next(1, 100));
    a.push_back(rnd.next(1ll, 100ll));
    a.push_back(rnd.next(size_t(1), size_t(2)));
    a.push_back(rnd.next(100));
    a.push_back(rnd.next(100ll));
    a.push_back(rnd.next(size_t(100)));
    a.push_back(rnd.wnext(100000, 10));
    a.push_back(rnd.wnext(100000, 1));
    a.push_back(rnd.wnext(100000, -1));
    a.push_back(rnd.wnext(100000, -10));

    check(a, "67 99 2 83 86 96 95163 71200 33710 9734");

    BOOST_TEST(rnd.next("[a-z]{5,10}") == "oeaqerxd");
    BOOST_CHECK(rnd.nextp(100) == std::make_pair(2, 84));
    BOOST_TEST(rnd.choice({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) == 3);
    BOOST_TEST(rnd.choice({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) == 8);
    BOOST_TEST(rnd.choice({1, 2, 3, 4, 5, 6, 7, 8, 9, 10}) == 4);

    BOOST_TEST(rnd.nextByDistribution({1, 2, 3, 3}) == 2);
    BOOST_TEST(rnd.nextByDistribution({1, 2, 3, 3}) == 3);
    BOOST_TEST(rnd.nextByDistribution({1, 2, 3, 3}) == 1);
}

/* code for generation of such testcases:

template<typename T>
void print_(const T& t) {
    stringstream ss;
    ss << t;
    for (char c: ss.str()) {
        if (c == '\n') {
            cout << "\\n";
        } else {
            cout << c;
        }
    }
}

#define print(x) cout << "check(" << #x << ", " << '"', print_(x), cout << '"' << ");\n"
*/

BOOST_AUTO_TEST_CASE(tree) {
    rnd.seed(123);
    check(Tree::random(10), "0 2\n1 5\n1 6\n1 9\n2 4\n2 6\n3 7\n3 9\n6 8");
    check(Tree::randomPrim(10), "0 1\n0 6\n0 8\n1 2\n1 5\n2 3\n3 4\n3 9\n6 7");
    check(Tree::randomPrim(10, -2), "0 1\n0 2\n0 3\n0 4\n0 5\n1 7\n1 9\n3 8\n4 6");
    check(Tree::randomPrim(10, 2), "0 1\n0 2\n2 3\n3 4\n4 5\n4 6\n5 8\n6 7\n7 9");
    check(Tree::randomKruskal(10), "8 1\n2 1\n1 3\n5 9\n0 8\n1 6\n3 5\n9 7\n3 4");
    check(Tree::caterpillar(10, 5), "0 1\n0 7\n0 8\n1 2\n1 6\n2 3\n3 4\n3 5\n3 9");
    rnd.seed(555);
    check(Tree::random(10).shuffle(), "2 3\n4 2\n7 1\n8 0\n2 8\n0 9\n5 4\n4 7\n6 1");
    check(Tree::random(10).shuffleAllBut({0, 5}), "5 8\n7 0\n0 1\n7 4\n0 9\n2 8\n5 4\n3 0\n6 2");
}

BOOST_AUTO_TEST_CASE(graph) {
    rnd.seed(195);
    check(Graph::random(5, 6), "0 2\n0 3\n1 3\n1 4\n2 3\n3 4");
    check(Graph::random(7, 8).connected(), "0 1\n0 4\n0 6\n1 3\n2 3\n2 6\n3 4\n5 6");
    check(Graph::random(7, 8).connected().g().shuffle(), "0 2\n6 4\n0 6\n6 5\n4 0\n5 1\n2 3\n1 0");
    check(Graph::random(7, 8).directed().g().shuffle(), "5 6\n2 5\n2 3\n2 0\n3 5\n1 2\n1 5\n1 6");
    check(Graph::random(7, 10).directed().acyclic().g().shuffle(), "0 3\n0 2\n0 6\n4 6\n4 5\n2 4\n1 4\n6 5\n1 2\n4 3");
    check(Graph::complete(6).directed().acyclic(), "1 0\n2 0\n2 1\n3 0\n3 1\n3 2\n4 0\n4 1\n4 2\n4 3\n5 0\n5 1\n5 2\n5 3\n5 4");
    check(Graph::complete(5).directed().g().shuffle(), "4 1\n4 0\n1 2\n1 0\n4 2\n4 3\n0 3\n1 3\n3 2\n0 2");
    check(Graph::randomStretched(7, 9, 2, 2), "0 1\n0 2\n0 2\n1 2\n1 3\n1 4\n2 4\n3 6\n4 5");
    check(Graph::randomBipartite(3, 3, 5), "0 4\n0 5\n1 3\n1 5\n2 3");
}

BOOST_AUTO_TEST_CASE(math) {
    rnd.seed(256);
    check(rndm.randomPrime(100, 1000), "167");
    check(rndm.partition(20, 5, 3, 8), "4 4 3 6 3");
}

BOOST_AUTO_TEST_CASE(geometry) {
    rnd.seed(1994);
    check(rndg.point(10), "3 10");
    check(rndg.point(-5, -6, 50, 60), "15 57");
    check(rndg.convexPolygon(8, 50), "0 18\n1 15\n21 0\n49 29\n48 34\n39 45\n18 49\n3 38");
    check(rndg.pointsInGeneralPosition(8, 50), "30 13\n46 26\n27 41\n3 47\n11 42\n10 36\n43 17\n0 30");
}

BOOST_AUTO_TEST_CASE(string) {
    rnd.seed(80);
    check(rnds.random(10), "tswxpfujkt");
    check(rnds.antiHash({{424243, 71}}, "a-c"), "cbcbabcba aaabbccac");
}


BOOST_AUTO_TEST_SUITE_END()
