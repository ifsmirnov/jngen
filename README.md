## Jngen: library which makes preparing tests for contest problems easier

Jngen library provides several handy instruments for generating various kinds of competitive programming problems. It contains (but is not limited to):
* random number generator with uniform and off-center distribution (known as wnext);
* command-line options parser, supporting named and positional arguments;
* wrapper for *std::vector* which makes sorting, printing and generating random sequences easier;
* printers for all standard types (no more trailing spaces!), 1-indexing is not a pain anymore;
* various random and not-so graphs and trees generators, manipulation with graphs and trees;
* generating random geometrical primitives (including large convex polygons) and built-in drawing geometry in SVG; 
* random primes, partitions, highly composite numbers, anti-hash tests;
* and else.

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
// rndg stands for "random graphs"
cout << rndg.random(n, m).connected().allowMulti() << endl;
```
### Reference
* [[random.h] Random numbers generation](/doc/random.md)
* [[options.h] Parsing command-line options](/doc/getopt.md)
* [[array.h] Array: wrapper for std::vector](/doc/array.md)
* [[repr.h, printers.h] Printers and output modifiers](/doc/printers.md)