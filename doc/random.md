## Random numbers generation

Jngen provides a class *Random* whose behavior is similar to *rnd* from testlib.h. E.g. you may write *rnd.next(100)*, *rnd.next("[a-z]{%d}", n)*, and so on.  Most of interaction with *Random* happens via its global instance of *Random* called *rnd*.

Default initialized *Random* is seeded with some hardware-generated random value, so subsequent executions of the program will produce different tests. This may be useful for local stress-testing, for example. If you want to fix the seed, use *registerGen(argc, argv)* at the beginning of your *main*.

### Generation

#### uint32_t next()
* Returns: random integer in range [0, 2^32).
#### uint64_t next64()
* Returns: random integer in range [0, 2^64).
#### double nextf()
* Returns: random real in range [0, 1).
#### int next(int n) // also for long long, size\_t, double
* Returns: random integer in range [0, n).
#### int next(int l, int r) // also for long long, size\_t, double
* Returns: random integer in range [l, r].
#### int wnext(int n, int w) // also for long long, size\_t, double
* If w > 0, returns max(next(n), ..., next(n)) (w times). If w &lt; 0, returns min(next(n), ..., next(n)) (-w times). If w = 0, same as next(n).
#### int wnext(int l, int r, int w) // also for long long, size\_t, double
* Same as wnext(n, w), but the range is [l, r].
#### std::string next(const std::string& pattern)
* Should be compatible with testlib.h.
* Returns: random string matching regex *pattern*.
* Regex has the following features:
    * any single character yields itself;
    * a set of characters inside square braces (*[abc123]*) yields random of them;
    * character ranges are allowed inside square braces (*[a-z1-9]*);
    * pattern followed by *{n}* is the same as the pattern repeated *n* times;
    * pattern followed by *{l,r}* is the same as the pattern repeated random number of times from *l* to *r*, inclusive;
    * "|" character yields either a pattern to its left or the pattern to its right equiprobably;
    * several "|" characters between patterns yield any pattern between them equiprobably, e.g. *(a|b|c|z){100}* yields a string of length 100 with almost equal number of *a*'s, *b*'s, *c*'s and *z*'s;
    * parentheses "()" are used for grouping.
* examples:
    * `rnd.next("[1-9][0-9]{1,2}")`:  random 2- or 3-digit number (note that the distribution on numbers is not uniform);
    * `rnd.next("a{10}{10}{10}")`: 1000 *a*'s;
    * `rnd.next("(ab|ba){10}|c{15}")`: either 15 *c*'s or a string of length 20 consisting of *ab*'s and *ba*'s.
#### std::string next(const std::string& pattern, ...)
* Same as rnd.next(pattern), but pattern interpreted as printf-like format string.
#### template&lt;typename T, typename ...Args> <br> tnext(Args... args)
* Calls *next(args...)*, forcing the return type to be *T* and casting arguments appropriately. E.g. *tnext&lt;int>(2.5, 10.1)* is equivalent to *rnd.next(2, 10)*, where both arguments are ints.
* Name origin: *typed* next.
#### std::pair&lt;int, int> nextp(int n, [RandomPairTraits])
#### std::pair&lt;int, int> nextp(int l, int r, [RandomPairTraits])
* Returns: random pair of integers, where both of them are in range [0, *n*) or [*l*, *r*] respectively. 
* RandomPairTraits denotes if the pair should be ordered (first element is less than or equal to second one) and if its two elements should be distinct. Several global constants are defined:
    * *opair*: ordered pair (first &lt;= second)
    * *dpair*: distinct pair (first != second)
    * *odpair*, *dopair*: ordered distinct pair
* Example of usage:  *rnd.nextp(1, 10, odpair)* yields a pair of random integers from 1 to 10 where first is strictly less than second. *rnd.nextp(1, 10)* returns any pair of integers from 1 to 10 (note that the *RandomPairTraits* argument is optional).
#### template&lt;typename Iterator> <br> Iterator::value_type choice(Iterator begin, Iterator end)
#### template&lt;typename Container> <br> Container::value_type choice(const Container& container)
* Returns: random element of a range or of a container, respectively.
* Note: *Container* may be *any* STL container, including *std::set*. In general case the runtime of this function is *O(container.size())*. However, if *Iterator* is a random-access iterator, the runtime is constant.

#### template&lt;typename N> <br> size_t nextByDistribution(const std::vector&lt;N>& distribution)
* Returns: a random integer from *0* to *distribution.size() - 1*, where probability of *i* is proportional to *distribution[i].
* Example: *rnd.nextByDistribution({1, 1, 100})* will likely return 2, but roughly each 50-th iteration will return 0 or 1.

### Seeding
#### void seed(uint32_t seed)
#### void seed(const std::vector&lt;uint32_t>& seed)
* Seed the generator with appropriate values. It is guaranteed that after identical *seed* calls the generator produces the same sequence of values.

### Related free functions
#### void registerGen(int argc, char* argv[], [int version])
* Seed the generator using command-line options. Different options will likely result in different generator states. The behavior is similar to the one of testlib.h.
* Note: parameter *version* is optional and is introduced only for compatibility with testlib.h.
