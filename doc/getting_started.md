## Getting started with Jngen

### Installation
Jngen is a single-header library. You only have to download the [jngen.h](https://raw.githubusercontent.com/ifsmirnov/jngen/master/jngen.h)
file and put it somewhere on your machine. `/usr/include` or the directory with your problem must work. And, of course, don't forget to include it
in your source file.

#### Note on compilers
Jngen is known to work with g++ of versions 4.8, 4.9, 5.3 and 6.2 and Clang of version 3.5. You should enable C++11 support (`-std=c++11`)
to work with it. C++14 is also fine.

MS Visual Studio is not supported at the moment, and it is known that Jngen fails to compile under it. Nothing is known about MinGW.

### Migrating from testlib.h
So let's write our first generator for an "A+B" problem!

```cpp
#include "jngen.h"
#include <iostream>
using namespace std;

int main(int argc, char *argv[]) {
    registerGen(argc, argv);
    parseArgs(argc, argv);

    int maxc = getOpt(0);

    int a = rnd.next(0, maxc);
    int b = rnd.next(0, maxc);
    
    cout << a << " " << b << endl;
}
```

At the first glance there is not much difference from testlib.h. The only new functions are *parseArgs* and *getOpt*.
They are for options parsing. *parseArgs* initializes the parser. *getOpt(0)* reads the first option and casts it to int
(or to any other type, whatever you want). Options parser is described in details [here](getopt.md).

*rnd.next(0, maxc)* returns a random integer from 0 to *maxc*, exactly the same as in testlib.

### The basic Jngen
My favorite and very common example is generating a permutation. I would expect to see something like this:

```cpp
int n = getOpt(0);
vector<int> a;
for (int i = 0; i < n; ++i) {
    a.push_back(i);
}
shuffle(a.begin(), a.end());
cout << n << "\n";
for (int i = 0; i < n; ++i) {
    cout << a[i] + 1;
    if (i+1 == n) {
        cout << "\n";
    } else {
        cout << " ";
    }
}
```

Freaking 14 lines of code! Now see Jngen version.

```cpp
cout << Array::id(getOpt(0)).shuffled().printN().add1() << endl;
```

Such wow, very short. Here we see many Jngen features at once.

* [Arrays](array.md). With *Array::something* you can generate various arrays (like permutations and random ones).
    After you can shuffle, sort and do anything else calling a method on the same object.
* Chaining. Syntax *object.doThis().doThat().andThat()* is very common in Jngen. You will see it when modifying objects
    (like sorting the array), dealing with output format (*printN* and *add1* here) or setting constraints for graphs generation.
* [Printing](printers.md). All containers can be put to *cout* and usually are printed in a least-surprising way. For vector
    and Array it is just space-separated elements. Or newline-separated for 2D; it is smart! With chaining you can print your
    object in 1-numeration and prepend its size to it.

### On the margins
You want [trees](tree.md)? [graphs](graph.md)? [convex polygons](geometry.md)? We have some, but this margin is too narrow to
    contain all of the examples.

```cpp
int h, w;
getPositional(h, w); // also a getOpt-like function
auto a = Tree::bamboo(h);
auto b = Tree::star(w);
cout << a.link(0, b, 0).shuffled() << endl;

cout << Graph::random(n, m).connected().allowMulti().printN().printM() << endl;

Drawer d;
d.polygon(rndg.convexPolygon(n, maxc));
d.dumpSvg("image.svg");
```

I hope that this description and pieces of code helped you to understand how Jngen is supposed to be used.
