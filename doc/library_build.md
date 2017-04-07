## Accelerating Jngen build

Jngen is distributed as a single header. As the header is sufficiently large, compilation lasts fairly long. To speed it up you may use `JNGEN_DECLARE_ONLY` macro.

Many functions in the library look like this:

```cpp
#ifdef JNGEN_DECLARE_ONLY
void doSomething();
#else
void doSomething() {
    // crunching numbers
}
#endif
```

If `JNGEN_DECLARE_ONLY` is defined, the compiler expects to find the definitions in some other translation unit, otherwise the header is used standalone. When working with Jngen locally, you may create a static library which includes *jngen.h* and does nothing else, compile it with *g++ lib.cpp -c*, and then link your *main.cpp* with generated *lib.o*. If you add `#define JNGEN_DECLARE_ONLY` to the top of your *main.cpp* or specify `-DJNGEN_DECLARE_ONLY` flag in compiler options, function definitions will be taken from the static library and thus will be not recompiled every time.

```sh
$ echo '#include "jngen.h"' > lib.cpp
$ g++ -O2 -std=c++11 -Wall lib.cpp -c
$ g++ -O2 -std=c++11 -Wall -DJNGEN_DECLARE_ONLY main.cpp lib.o
```

On the author's laptop this trick reduces compilation time by approximately 2.5 times.

