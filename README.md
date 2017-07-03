## Jngen: preparing tests made simpler

* [Overview](#overview)
* [Reference](#reference)
* [Why not testlib.h?](#why-not-testlibh)
* [Examples](#examples)

### Overview

Jngen library provides several handy instruments for generating various kinds of competitive programming problems.
* random number generator with uniform and off-center distribution (known as wnext);
* command-line options parser, supporting named and positional arguments;
* wrapper for *std::vector* which makes sorting, printing and generating random sequences easier;
* printers for all standard types (no more trailing spaces and pain with 1-indexing!);
* various random and not-so graphs and trees generators, manipulation with graphs and trees;
* generating random geometrical primitives (including large convex polygons) and built-in drawing geometry in SVG; 
* random primes, partitions, anti-hash and anti-hashset tests;
* and else.

Check out the larger [overview](/doc/overview.md) to see more capabilities and examples.

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

### Why not testlib.h?
testlib.h is a wonderful library which has already saved hundreds of hours for contest writers. However, there are reasons why I did not build Jngen on top of existing testlib.h code.

* Testlib is multi-purpose. It also supports validators, checkers and interactors, while Jngen does not need it.
* There are not many things to borrow from testlib. *rnd*, pattern generation, maybe some internal helper functions.
* Testlib random is not very good. std::mt19937, which is used in Jngen under the hood, is much faster and has better distribution than hand-written linear congruental generator from testlib.
* Also, it would be harder to introduce new features in *rnd* than to code it from scratch.
* I don't really like the code style of testlib, particularly naming convention and not using namespaces.
* Being dependant on testlib, Jngen would compile even longer than it does now.

### Examples
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
