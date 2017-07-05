## Jngen: preparing tests made simpler

[DOWNLOAD](https://raw.githubusercontent.com/ifsmirnov/jngen/master/jngen.h)

* [Overview](#overview)
* [Reference](#reference)
* [Compatibility note](#compatibility-note)
* [Examples](#examples)
* [Why not testlib.h?](#why-not-testlibh)
* [What does Jngen mean?](#what-does-jngen-mean)

### Overview

Jngen is a C++ library for generating testss for various competitive programming problems.
It is written in pure C++ (compliant with C++11 standard) and distributed as a single header.

Among its features there are:
* random number generator with uniform and off-center distribution (known as wnext);
* command-line options parser, supporting named and positional arguments;
* wrapper for *std::vector* which makes sorting, printing and generating random sequences easier;
* printers for all standard types (no more trailing spaces and pain with 1-indexing!);
* various random and not-so graphs and trees generators, manipulation with graphs and trees;
* generating random geometrical primitives (including large convex polygons) and built-in drawing geometry in SVG; 
* random primes, partitions, anti-hash and anti-hashset tests;
* and else.

Check out the larger [overview](/doc/overview.md) to see more capabilities and examples
or see [Getting started](/doc/getting_started.md) section learn the simplest usecases immediately.

### Reference
* [[random.h] Random numbers generation](/doc/random.md)
* [[options.h] Parsing command-line options](/doc/getopt.md)
* [[array.h] Array: wrapper for std::vector](/doc/array.md)
* [[repr.h, printers.h] Printers and output modifiers](/doc/printers.md)
* [[generic_graph.h] Graphs and trees: basics](/doc/generic_graph.md)
    * [[graph.h] Graphs generation](/doc/graph.md)
    * [[tree.h] Trees generation](/doc/tree.md)
* [[math.h] Math: primes and partitions](/doc/math.md)
* [[rnds.h] Strings](/doc/strings.md)
* [[geometry.h] Geometric primitives](/doc/geometry.md)
* [[drawer/drawer.h] Drawing figures in SVG](/doc/drawer.md)

### Compatibility note
* **This is not a standard "provided as-is" legal warning!** Opposite to testlib.h, which is already well-tested and pretty stable, Jngen is only yet being developed. First, not everything was properly tested and there may be bugs. Second and more important: there is no backward compatibility at the moment. It means that if you download Jngen tomorrow and run the same code then it may produce different result. Do not blindly update Jngen header if you need that tests for your problem remain exactly the same.

### Examples
Find some real-world examples [here](/examples).

Generate a random tree on *n* vertices with a 3-letter string assigned to each edge:
```cpp
Tree t = Tree::random(5);
t.setEdgeWeights(TArray<string>::random(t.m(), "[a-z]{%d}", 3));
cout << t.add1().printN() << endl;
---
5
1 2 rqi
1 3 slv
1 4 foi
4 5 eju
```

Output a random permutation and its inverse:
```cpp
setMod().add1();

auto a = Array::id(10).shuffled();
cout << a.size() << endl;
cout << a << endl;
cout << a.inverse() << endl;
---
10
3 8 5 7 2 4 1 10 9 6
7 5 1 6 3 10 4 2 9 8
```

Output a bamboo on *n* vertices connected to a star on *m* vertices, probably shuffled:
```cpp
int main(int argc, char *argv[]) {
    parseArgs(argc, argv);
    int n, m;
    getPositional(n, m); 

    Tree t = Tree::bamboo(n).link(n - 1, Tree::star(m), 0); 
    if (getOptOr("shuffled", false)) {
        t.shuffle();
    }   

    cout << t.printN().add1() << endl;
}
```

Generate a connected graph with multi-edges:
```cpp
cout << Graph::random(n, m).connected().allowMulti() << endl;
```

### Why not testlib.h?
testlib.h is a wonderful library which has already saved hundreds of hours for contest writers. However, there are reasons why I did not build Jngen on top of existing testlib.h code.

* Testlib is multi-purpose. It also supports validators, checkers and interactors, while Jngen does not need it.
* There are not many things to borrow from testlib. *rnd*, pattern generation, maybe some internal helper functions.
* Testlib random is not very good. std::mt19937, which is used in Jngen under the hood, has much better distribution than hand-written linear congruential generator from testlib (though it is a bit slower).
* Also, it would be harder to introduce new features in *rnd* than to code it from scratch.
* I don't really like the code style of testlib, particularly naming convention and not using namespaces.
* Being dependant on testlib, Jngen would compile even longer than it does now.

### What does Jngen mean?
I don't know.

It sounds similar to Jinotega, my ACM-ICPC team, maybe that's the way how it (unconsciously) came to my mind. Also it is similar to Jungen – "Young" in German. Or "Just 'Nother GENerator library". Well, who the hell cares.
