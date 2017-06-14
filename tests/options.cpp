#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(options)

void check(
    const std::string& options,
    const char *v0,
    const char *v1,
    const char *v2,
    const char *va,
    const char *vb,
    const char *vc)
{
    std::vector<std::string> tokens{""};
    std::string last;
    for (char c: options + ' ') {
        if (isspace(c)) {
            if (!last.empty()) {
                tokens.push_back(last);
            }
            last = "";
        } else {
            last += c;
        }
    }

    jngen::vmap = jngen::VariableMap{};
    std::vector<char*> argv;
    std::transform(
        tokens.begin(),
        tokens.end(),
        std::back_inserter(argv),
        [](const std::string& s) {
            return const_cast<char*>(s.data());
        }
    );
    parseArgs(argv.size(), argv.data());

    std::string x0, x1, x2, a, b, c;
    getPositional(x0, x1, x2);
    getNamed(a, b, c);

    BOOST_CHECK_EQUAL(x0, v0);
    BOOST_CHECK_EQUAL(x1, v1);
    BOOST_CHECK_EQUAL(x2, v2);
    BOOST_CHECK_EQUAL( a, va);
    BOOST_CHECK_EQUAL( b, vb);
    BOOST_CHECK_EQUAL( c, vc);
}

BOOST_AUTO_TEST_CASE(variable_map) {
    check("", "", "", "", "", "", "");
    check("-a", "","","","1","","");
    check("-a 2", "","","","2","","");
    check("-a=3", "","","","3","","");
    check("-a=3 -b 4", "","","","3","4","");
    check("-a=3 first -c -b4", "first","","","3","","1");
    check("-a -b=10 20", "20","","","1","10","");
}

BOOST_AUTO_TEST_SUITE_END()
