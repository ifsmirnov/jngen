## Overview

Jngen is a library which helps you to generate standard objects for competitive problems: trees, graphs, strings and so. For some objects it defines classes (like *Array*, *Graph* or *Point*), for others STL is used (*std::string*).

<!-- Primitive generators are provided (like «generate a random tree»), as well as testsets which contain various tests which you would likely use in your problem anyway. -->

There are two ways of generating objects. The first is with static methods of the class.

```cpp
auto a = Array::random(n, maxSize);
auto t = Tree::bamboo(n);
```

[Arrays](array.md), [trees](tree.md) and [graphs](graph.md) are generated like this.

The second uses helper objects.


```cpp
auto polygon = rndg.convexPolygon(n, maxCoordinate);
auto stringPair = rnds.antiHash(10000, {{1000000007, 101}, {1000000009, 211}}, "a-z");
int p = rndm.randomPrime(100, int(1e9));
```

[Strings](strings.md), [geometric primitives](geometry.md), [primes and partitions](math.md) and simply [random numbers](random.md) are generated with such helpers.

For each Jngen object there are operators for printing to streams. There are modifiers which allow, for example, to switch between 0- and 1-indexation. Also Jngen allows printing standard containers like vectors and pairs. See section [printers](printers.md).

```cpp
cout << std::vector<int>{1, 2, 3} << endl;
cout << Array::id(5).shuffled().printN().add1() << endl;
---
1 2 3
5
5 2 4 3 1
```

The library also supplies a [command-line arguments parser](getopt.md) and a [tool for drawing geometric primitives](drawer.md).

Jngen is large, its compilation lasts for several seconds. It is possible to make it faster with precompiling a part of it. See [this chapter](library_build.md) for manual.

If you want to learn more about Jngen, please see all the docs listed at the [reference](/README.md#reference) section. Good luck!
