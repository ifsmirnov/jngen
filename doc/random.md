### Random numbers generation

Jngen provides a class `Random` whose behavior is similar to `rnd` from testlib.h. E.g. you may write `rnd.next(100)`, `rnd.next("[a-z]{%d}, n)`, and so on.  Most of interaction with `Random` happens via its global instance of `Random` called `rnd`.

#### Generation

##### uint32_t next()
* returns: random integer in range [0, 2^32).
##### uint64_t next64()
* returns: random integer in range [0, 2^64).
##### double nextf()
* returns: random real in range [0, 1).
##### int next(int n) // also for long long, size\_t, double
* returns: random integer in range [0, n)
##### int next(int l, int r) // also for long long, size\_t, double
* returns: random integer in range [l, r]
##### int wnext(int n, int w) // also for long long, size\_t, double
* If w > 0, returns max(next(n), ..., next(n)) (w times). If w < 0, returns min(next(n), ..., next(n)) (-w times). If w = 0, similar to next(n).
##### int wnext(int l, int r, int w) // also for long long, size\_t, double
* Same as wnext(n, w), but the range is [l, r].
##### std::string next(const std::string& pattern)
* Should be compatible with testlib.h.
* returns: random string matching regex *pattern*.
* Regex has the following features:
    * any single character yields itself;
    * a set of characters inside square braces (*[abc123]*) yields random of them;
    * character ranges are allowed inside square braces (*[a-z1-9]*);
    * pattern followed by *{n}* is the same as the pattern repeated *n* times;
    * pattern followed by *{l,r}* is the same as the pattern repeated random number of times from *l* to *r*, inclusive;
    * "|" character yields either a pattern to its left or the pattern to its right equiprobably;
    * several "|" characters between patterns yield any pattern between them equiprobably, e.g. *(a|b|c){100}* yields a string of length 100 with almost equal number of *a*'s, *b*'s and *c*'s;
    * parentheses "()" are used for grouping.
* examples:
    * `rnd.next("[0-9][1-9]{1,2}")`:  random 2- or 3-digit number (note that the distribution on numbers is not uniform);
    * `rnd.next("a{10}{10}{10}")`: 1000 *a*'s;
    * `rnd.next("(ab|ba){10}|c{15}")`: either 15 *c*'s or a string of length 20 consisting of *ab*'s and *ba*'s.
##### std::string rnd.next(const std::string& pattern, ...)
* same as rnd.next(pattern), but pattern interpreted as printf-like format string.
##### template\<typename T, typename ...Args> tnext(Args... args)
* sdfds



> Written with [StackEdit](https://stackedit.io/).
