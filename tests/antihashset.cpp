#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

#include <unordered_set>

BOOST_AUTO_TEST_SUITE(antihashset)

bool check(std::unordered_set<long long>& set, const Array64& data) {
    for (auto x: data) {
        set.insert(x);
    }
    for (auto x: data) {
        if (set.bucket(x) != set.bucket(data[0])) {
            return false;
        }
    }
    return true;
}

BOOST_AUTO_TEST_CASE(antihashset) {
#if __GNUC__ == 4
    using Set = std::unordered_set<long long>;
    Set s1;
    BOOST_CHECK(check(s1, rnda.antiUnorderedSet(1000)));

    Set s2;
    s2.max_load_factor(0.4);
    BOOST_CHECK(check(s2, rnda.antiUnorderedSet(1234, 0.4)));

    Set s3;
    s3.reserve(5000);
    BOOST_CHECK(check(s3, rnda.antiUnorderedSet(5000, 1, true)));

    /*
    // TODO: investigate later
    Set s4;
    s4.reserve(3141);
    s4.max_load_factor(0.31);
    BOOST_CHECK(check(s4, rnda.antiUnorderedSet(3141, 0.31, true)));
    */
#else
    // no antitests for other compilers yet
    BOOST_CHECK(true);
#endif
}

BOOST_AUTO_TEST_SUITE_END()
