#define BOOST_TEST_MAIN
#include <boost/test/included/unit_test.hpp>
#include "../jngen.h"

#include <algorithm>
#include <set>
#include <sstream>
#include <utility>

BOOST_AUTO_TEST_CASE(manual_construction) {
    Tree t;

    BOOST_CHECK_EQUAL(t.n(), 1);
    BOOST_CHECK_EQUAL(t.m(), 0);
    BOOST_CHECK(t.isConnected());

    t.addEdge(0, 1);
    t.addEdge(2, 3);
    BOOST_CHECK(!t.isConnected());

    t.addEdge(1, 2);

    BOOST_CHECK_EQUAL(t.n(), 4);
    BOOST_CHECK_EQUAL(t.m(), 3);

    std::ostringstream ss;
    ss << t.printN().add1() << std::endl;

    BOOST_CHECK_EQUAL(ss.str(), "4\n1 2\n3 4\n2 3\n");
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
    std::vector<int> centers, dist;

    auto b = Tree::bamboo(10);
    BOOST_CHECK_EQUAL(findDiameter(b, centers, dist), 9);
    BOOST_CHECK_EQUAL(centers[0], 4);
    BOOST_CHECK_EQUAL(centers[1], 5);

    std::ostringstream ss;

    ss << b;
    std::string first = ss.str();
    ss.clear();

    b.shuffle();

    ss << b;
    std::string second = ss.str();
    ss.clear();

    BOOST_CHECK(first != second);

    auto s = Tree::star(10);
    BOOST_CHECK_EQUAL(findDiameter(s, centers, dist), 2);
    BOOST_CHECK_EQUAL(centers[0], 0);

    auto c = Tree::caterpillar(10, 100);
    BOOST_CHECK_EQUAL(findDiameter(c, centers, dist), 11);

    c = Tree::caterpillar(1000, 1005);
    BOOST_CHECK_EQUAL(findDiameter(c, centers, dist), 999);

    auto t = Tree::random(150, 0);
    BOOST_CHECK_EQUAL(t, Tree::bamboo(150));
}

BOOST_AUTO_TEST_CASE(prufer_all_trees) {
    auto a = TArray<Tree>::randomfUnique(120, []() {
        return Tree::randomPrufer(5);
    });
    BOOST_CHECK_EQUAL(a.size(), 5*4*3*2*1);
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

    BOOST_CHECK_EQUAL(t1.n() + t2.n(), linked.n());
    BOOST_CHECK_EQUAL(t1.m() + t2.m() + 1, linked.m());
    BOOST_CHECK_EQUAL(linked, t);
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

    BOOST_CHECK_EQUAL(t1.n() + t2.n() - 1, glued.n());
    BOOST_CHECK_EQUAL(t1.m() + t2.m(), glued.m());
    BOOST_CHECK_EQUAL(glued, t);
}

// TODO: add tests to check random generators exactly
