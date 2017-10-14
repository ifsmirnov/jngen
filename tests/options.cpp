#define BOOST_TEST_DYN_LINK
#include <boost/test/unit_test.hpp>
#include "../jngen.h"

#include <algorithm>
#include <iterator>
#include <string>
#include <vector>

BOOST_AUTO_TEST_SUITE(options)

void initVmap(const std::string& options) {
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
}

void check(
    const std::string& options,
    const char *v0,
    const char *v1,
    const char *v2,
    const char *va,
    const char *vb,
    const char *vc)
{
    initVmap(options);

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

BOOST_AUTO_TEST_CASE(get_opt) {
    initVmap("10 5.4 hello -a 10 -b 5.4 -c bye -d -e");

    BOOST_TEST(hasOpt(0));
    BOOST_TEST(hasOpt(1));
    BOOST_TEST(hasOpt(2));
    BOOST_TEST(!hasOpt(3));

    BOOST_TEST(hasOpt("a"));
    BOOST_TEST(hasOpt("d"));
    BOOST_TEST(hasOpt("e"));
    BOOST_TEST(!hasOpt("hello"));
    BOOST_TEST(!hasOpt("10"));

    BOOST_CHECK_THROW(getOpt(10), jngen::Exception);
    BOOST_CHECK_THROW(getOpt("uoiarnst"), jngen::Exception);

    // int casts
    int x = getOpt(0);
    BOOST_TEST(x == 10);
    x = getOpt(10, 15);
    BOOST_TEST(x == 15);
    x = getOpt("a");
    BOOST_TEST(x == 10);
    x = getOpt("b");
    BOOST_TEST(x == 5);
    x = getOpt(10, 0.4);
    BOOST_TEST(x == 0);

    // double casts
    double y = getOpt(0);
    BOOST_TEST(y == 10);
    y = getOpt("b", 10);
    BOOST_TEST(y == 5.4);
    y = getOpt("tiesrt", 1.23);
    BOOST_TEST(y == 1.23);

    // string casts
    std::string s = getOpt(0);
    BOOST_TEST(s == "10");
    s = getOpt("c");
    BOOST_TEST(s == "bye");
    s = static_cast<std::string>(getOpt("uwlfp", "nope"));
    BOOST_TEST(s == "nope");
    s = "yep";
    s = getOpt(2, s);
    BOOST_TEST(s == "hello");
}

BOOST_AUTO_TEST_SUITE_END()
