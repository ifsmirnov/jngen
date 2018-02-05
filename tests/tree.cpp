#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <utility>

BOOST_AUTO_TEST_SUITE(tree)

void checkEquals(const jngen::Tree& t1, const jngen::Tree& t2) {
    BOOST_TEST(t1.n() == t2.n());
    for (int i = 0; i < t1.n(); ++i) {
        BOOST_TEST(t1.edges(i).sorted() == t2.edges(i).sorted());
    }
}

BOOST_AUTO_TEST_CASE(manual_construction) {
    setMod().reset();

    Tree t;

    BOOST_TEST(t.n() == 1);
    BOOST_TEST(t.m() == 0);
    BOOST_TEST(t.isConnected());

    t.addEdge(0, 1);
    t.addEdge(2, 3);
    BOOST_TEST(!t.isConnected());

    t.addEdge(1, 2);

    BOOST_TEST(t.n() == 4);
    BOOST_TEST(t.m() == 3);

    std::ostringstream ss;
    ss << t.printN().add1() << std::endl;

    BOOST_TEST(ss.str() == "4\n1 2\n3 4\n2 3\n");
}

void dfs(int v, int anc, const Tree& t, std::vector<int>& dist) {
    for (int to: t.edges(v)) {
        if (to == anc) {
            continue;
        }
        dist[to] = dist[v] + 1;
        dfs(to, v, t, dist);
    }
}

/*
 * Find the diameter of the given tree.
 *
 * Returns: the diameter of the tree, measured in number of edges.
 * center: one or two vertices -- the center of the tree.
 * dist: distance from the first of the centers to all nodes.
 */
int findDiameter(
        const Tree& t,
        std::vector<int>& centers,
        std::vector<int>& dist)
{
    BOOST_REQUIRE(t.isConnected());
    std::vector<int> dist1(t.n(), -1);
    std::vector<int> dist2(t.n(), -1);

    dist1[0] = 0;
    dfs(0, -1, t, dist1);

    int left = std::max_element(dist1.begin(), dist1.end()) - dist1.begin();
    dist1[left] = 0;
    dfs(left, -1, t, dist1);
    int right = std::max_element(dist1.begin(), dist1.end()) - dist1.begin();
    dist2[right] = 0;
    dfs(right, -1, t, dist2);

    BOOST_REQUIRE(dist1[right] == dist2[left]);

    int diam = dist1[right];

    centers.clear();
    for (int v = 0; v < t.n(); ++v) {
        if (dist1[v] + dist2[v] == diam && std::abs(dist1[v] - dist2[v]) <= 1) {
            centers.push_back(v);
        }
    }

    if (diam % 2 == 1) {
        BOOST_REQUIRE(centers.size() == 2);
    } else {
        BOOST_REQUIRE(centers.size() == 1);
    }

    dist1[centers[0]] = 0;
    dfs(centers[0], -1, t, dist1);
    dist = dist1;

    return diam;
}

/* Mostly performs sanity check on the generators: check some basic properties
 * of the resulting trees such as diameter.
 */
BOOST_AUTO_TEST_CASE(generators) {
    setMod().reset();
    rnd.seed(12345);

    std::vector<int> centers, dist;

    auto b = Tree::bamboo(10);
    BOOST_TEST(findDiameter(b, centers, dist) == 9);
    BOOST_TEST(centers[0] == 4);
    BOOST_TEST(centers[1] == 5);

    std::ostringstream ss;

    ss << b;
    std::string first = ss.str();
    ss.clear();

    b.shuffle();

    ss << b;
    std::string second = ss.str();
    ss.clear();

    BOOST_TEST(first != second);

    auto s = Tree::star(10);
    BOOST_TEST(findDiameter(s, centers, dist) == 2);
    BOOST_TEST(centers[0] == 0);

    // probability of failure < 1e-5
    auto c = Tree::caterpillar(100, 10);
    BOOST_TEST(findDiameter(c, centers, dist) == 11);

    // probability of failure < 1e-3
    c = Tree::caterpillar(8004, 8000);
    BOOST_TEST(findDiameter(c, centers, dist) == 7999);

    // probability of failure unknown, but very low
    auto t = Tree::randomPrim(150, 15000);
    BOOST_TEST(t == Tree::bamboo(150));
}

BOOST_AUTO_TEST_CASE(prufer_all_trees) {
    auto a = TArray<Tree>::randomfUnique(120, []() {
        return Tree::random(5);
    });
    BOOST_TEST(a.size() == 5*4*3*2*1);

    auto b = TArray<Tree>::randomfAll([]() {
        return Tree::random(5);
    });
    BOOST_TEST(b.size() == 5*5*5);
}

BOOST_AUTO_TEST_CASE(check_link) {
    /*        0   (4-2)    1
            1   4   +    2   4
          2    3 5      0 3   5
                    =
              0
            1      4
          2     3 5    8
                     6 9 7
                          10
                            11
     */
    Tree t1;
    for (std::pair<int, int> edge:
            Arrayp{{0, 1}, {1, 2}, {0, 4}, {4, 3}, {4, 5}}) {
        t1.addEdge(edge.first, edge.second);
    }

    Tree t2;
    for (std::pair<int, int> edge:
            Arrayp{{1, 2}, {2, 0}, {2, 3}, {1, 4}, {4, 5}}) {
        t2.addEdge(edge.first, edge.second);
    }

    Tree t;
    for (std::pair<int, int> edge: Arrayp{
        {0, 1}, {1, 2}, {0, 4}, {4, 3}, {4, 5}, {4, 8}, {8, 6}, {8, 9},
        {8, 7}, {7, 10}, {10, 11}})
    {
        t.addEdge(edge.first, edge.second);
    }

    Tree linked = t1.link(4, t2, 2);

    BOOST_TEST(t1.n() + t2.n() == linked.n());
    BOOST_TEST(t1.m() + t2.m() + 1 == linked.m());
    checkEquals(linked, t);
}

BOOST_AUTO_TEST_CASE(check_glue) {
    /*        0   (4-2)    1
            1   4   .    2   4
          2    3 5      0 3   5
                    =
              0
            1      4
          2     3 5 6 8 7
                         9
                          10
     */
    Tree t1;
    for (std::pair<int, int> edge:
            Arrayp{{0, 1}, {1, 2}, {0, 4}, {4, 3}, {4, 5}}) {
        t1.addEdge(edge.first, edge.second);
    }

    Tree t2;
    for (std::pair<int, int> edge:
            Arrayp{{1, 2}, {2, 0}, {2, 3}, {1, 4}, {4, 5}}) {
        t2.addEdge(edge.first, edge.second);
    }

    Tree t;
    for (std::pair<int, int> edge: Arrayp{
        {0, 1}, {1, 2}, {0, 4}, {4, 3}, {4, 5}, {4, 6}, {8, 4}, {4, 7},
        {9, 7}, {9, 10}})
    {
        t.addEdge(edge.first, edge.second);
    }

    Tree glued = t1.glue(4, t2, 2);

    BOOST_TEST(t1.n() + t2.n() - 1 == glued.n());
    BOOST_TEST(t1.m() + t2.m() == glued.m());
    checkEquals(glued, t);
}

BOOST_AUTO_TEST_CASE(print_parents) {
    rnd.seed(123);
    setMod().reset();

    std::ostringstream ss;

    const std::string res1 = "5\ngrt kzw bar pja oap\n5 1 3 3\n";

    Tree t = Tree::random(5);
    t.setVertexWeights(TArray<std::string>::random(t.n(), "[a-z]{%d}", 3));
    ss << t.add1().printN().printParents() << std::endl;

    BOOST_TEST(ss.str() == res1);

    t.setVertexWeights(WeightArray(t.n()));

    ss.str("");
    ss << t.printParents(0);
    BOOST_TEST(ss.str() == "-1 4 0 2 2");

    ss.str("");
    ss << t.printParents(1);
    BOOST_TEST(ss.str() == "2 -1 4 2 1");
}

// TODO: add tests to check random generators exactly

BOOST_AUTO_TEST_SUITE_END()
