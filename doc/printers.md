## Printing to ostreams

Tired of writing `cout << a[i] << " \n"[i+1 == n]`? We have a solution! Jngen declares ostream operators for all standard containers. Moreover, for Jngen containers there is a bunch of output modifiers which can toggle 0/1 numeration, automatically print the size of the array and something else.

### Outline
As a quick start, try to write something like
```cpp
cout << Array::random(5, 5) << endl;
cout << Arrayp::random(2, 10) << endl;
---
3 1 1 0 4
5 9
8 8
```

Or even
```cpp
vector<int> a{0, 1, 2};
pair<string, double> p{"hello", 4.2};
cout << a << endl;
cout << p << endl;
---
0 1 2
hello 4.2
```
Containers are printed in a least surprising way: sequences are separated with single spaces, sequences of pairs -- with line breaks, sequences of sequences are formatted as matrices. If you print a graph, it first prints *n* and *m* on the first line (if corresponding modifiers are set, see later), then, if present, a line of vertex weights, then *m* lines with edges in a most standard format.

Now a word about modifiers. C++ programmers are used to 0-indexing, while in problem statements usually arises 1-indexing. There is a *quick fix*, which at first glance looks as a dirty hack but later appears to be very convenient. Look how to output a random 1-indexed permutation:
```cpp
cout << Array::id(5).shuffled().add1().printN() << endl;
---
5
1 4 2 5 3
```
These *add1()* and *printN()* are called *output modifiers*. These modifiers can be applied to any container provided by Jngen, such as Array, Graph and Tree. If you want to use modifiers with other types (like std::vector or even int), you can do it like this:
```cpp
vector<int> a{1, 2, 3};
cout << repr(a).endl() << endl;
---
1
2
3
```

### Global modifier
Sometimes it may be more convenient to set modifiers once for the entire program. This can be done as following:
```cpp
setMod().printN().add1();
// now printN() and add1() modifiers apply to everything being printed
setMod().reset();
// global modifier has returned to default state, you should specify local modifiers manually
```

Note that Jngen does not interact with stl-defined operators. That mean that writing `cout << 123 << endl;` will print *123* regardless of which global modifiers are set. However, printing a std::vector **will** use global modifiers.

### Modifiers
#### add1(bool value = true)
* Action: adds 1 to each integer being output, **except for vertex/edge weights in graphs**.
* Default: unset.
#### printN(bool value = true)
* Action: print array size on a separate line before the array. Print number of vertices of a graph.
* Default: unset.
#### printM(bool value = true)
* Action: print number of edges of a graph.
* Default: unset.
#### printEdges(bool value = true)
* Action: when printing a tree, print a list of edges.
* Default: set.
#### printParents(int value = -1)
* Action: when printing a tree, print a parent of each vertex. Opposite to *printEdges*.
* Arguments: *value* stands for the root of the tree. If *value* is *0* or greater, then the parent of each vertex is printed, having root's parent as
    *-1* (*0* if *add1()* is present). *value = -1* is a special value: in this case tree is rooted at *0* and its parent is not printed (printing *n-1* values in total).
* Note: this option and *printEdges* cancel each other.
#### endl(bool value = true)
* Action: separate elements of the array with line breaks instead of spaces.
* Default: unset.
