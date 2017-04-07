## Graph generation

* [Generators](#generators)
* [Modifiers](#modifiers)
* [Graph methods](#graph-methods)

This page is about *Graph* class and graph generators. To see the list of generic graphs methods please visit [this page](/generic_graph.md).

The *Graph* class has several static methods to generate random and special graphs, like *random(n, m)* or *complete(n)*. The source of randomness is *rnd*.

After calling a method you can add modifiers to allow or disallow loops, make graph connected etc. As you can see from the following example, *chaining* semantics is used. To support this semantics generation methods return not *Graph* itself but a special proxy class. To get a *Graph* itself, you may do one of the following:
* call *.g()* method after modifiers chain:
* cast the returned object to *Graph*;
* or directly print the proxy class to the stream, in this case the generated graph will be printed.

See the example for further clarifications.

```cpp
auto g = Graph::random(10, 20).connected().allowMulti().g().shuffled();
Graph g2 = Graph::randomStretched(100, 200, 2, 5);
cout << Graph::complete(5).allowLoops() << endl;
```

All graph generators return graph with sorted edges to make tests more human-readable. If you want to have your graph shuffled, use *.shuffle()* method, as in the example.

Currently only generation of undirected graphs is supported.

### Generators
#### random(int n, int m)
* Returns: a random graph with *n* vertices and *m* edges.
* Available modifiers: *connected*, *allowLoops*, *allowMulti*.

#### complete(int n)
* Returns: a complete graph with *n* vertices.
* Available modifiers: *allowLoops*.

#### cycle(int n)
* Returns: a cycle with *n* vertices, connected in order.
* Available modifiers: none.

#### empty(int n)
* Returns: an empty graph with *n* vertices.
* Available modifiers: none.

#### randomStretched(int n, int m, int elongation, int spread)
* Returns: a connected stretched graph with *n* vertices and *m* vertices.
* Available modifiers: *allowLoops*, *allowMulti*.
* Description: first a random tree on *n* vertices with given *elongation* (see [tree docs](/tree.md)) is generated. Then remaining *m*-*n*+*1* edges are added. One endpoint of an edge is selected at random. The second is a result of jumping to a tree parent of the first endoint a random number of times, from 0 to *spread*.

### Modifiers
All options are unset by default. If the generator contradicts some option (like *randomStretched*, which always produces a connected graph), it is ignored.
#### connected(bool value = true)
* Action: force the generated graph to be connected.
#### allowMulti(bool value = true)
* Action: allow multiple edges in the generated graph (i.e. several edges with the same endpoints).
#### allowLoops(bool value = true)
* Action: allow loops in the generated graph (i.e. edges from a vertex to itself).

### Graph methods
#### Graph(int n)
* Construct an empty graph with *n* vertices.
#### void setN(int n)
* Set the number of vertices of the graph to *n*.
* Note: this operation cannot lessen the number of vertices.

#### Graph& shuffle()
#### Graph shuffled() const
* Shuffle the graph. This means:
    * relabel vertices in random order;
    * shuffle edges;
    * randomly swap egdes' endpoints.
