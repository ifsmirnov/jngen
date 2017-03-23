### Random numbers generation

Jngen provides a class `Random` whose behavior is similar to `rnd` from testlib.h. E.g. you may write `rnd.next(100)`, `rnd.next("[a-z]{%d}, n)`, and so on.  Most of interaction with `Random` happens via its global instance of `Random` called `rnd`.

#### Generation

`uint32_t next()`
* returns: random integer in range `[0, 2^32)`

`uint64_t next64()`
* returns: random integer in range `[0, 2^64)`


> Written with [StackEdit](https://stackedit.io/).

