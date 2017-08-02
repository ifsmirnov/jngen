## Graphs and trees: common interface

* [Documentation](#document)
* [Weights](#weights)
* [Labeling](#labeling)

Jngen provides a *GenericGraph* class. You will mostly use its two subclasses: *Graph* and *Tree*. They have different generators and methods, though there is a common generic part.

Graph vertices are always numbered from 0 to n-1, where n is the number of vertices. Other numerations will be supported later. Currently can output a graph in 1-numeration using *.add1()* output modifier.

You can assign weights to edges and vertices of a graph. Weight is implemented as (self-written, waiting for C++17) kinda *std::variant* with some predefined types: *int*, *double*, *string*, *pair&lt;int, int>*. However, you can add your own types. To do it define a macro *JNGEN_EXTRA_WEIGHT_TYPES* containing comma-separated extra types you want to use.

```cpp
#define JNGEN_EXTRA_WEIGHT_TYPES std::vector<int>, std::pair<char, double>
#include "jngen.h"
```

Like all containers in jngen, graphs support pretty-printing and output modifiers.

```cpp
Graph g;
g.addEdge(0, 1);
g.addEdge(1, 2);
g.setVertexWeights({"v1", "v2", "v3"});
g.setEdgeWeights({10, 20});

cout << g.printN().printM().add1() << endl;
---
3 2
v1 v2 v3
1 2 10
2 3 20
```

Graphs and trees are printed as following. If *.printN()* and *.printM()* modifiers are set, on the first line *n* and *m* are printed (you can set any of modifiers independently). If vertex weights are present, they are then printed on a separate line. After *m* lines with edges follow. Two endpoints of the edge are printed, optionally followed by edge weight.

**Output modifiers do not apply to vertex/edge weights**. When you set edge length to 10, you probably don't want it to increase to 11 when you switch to 1-numeration, right?

### Documentation

#### int n() const
* Returns: the number of vertices in the graph.
#### int m() const
* Returns: the number of edges in the graph.
#### void addEdge(int u, int v, const Weight& w = Weight{})
* Add an edge *(u, v)*, possbly, with weight *w*, to a graph.
#### bool isConnected() const
* Returns: true if and only if the graph is connected.
#### int vertexByLabel(int label) const
* Returns: the internal id of the vertex identified by *label*. See [*labeling*](#labeling) section at the end of this part. Most likely you'll never need this and the next method.
#### int vertexLabel(int v) const
* Returns: the label of the vertex with internal id *v*.
#### Array edges(int v) const
* Returns: array of vertices incident to *v*.
#### Arrayp edges() const
* Returns: array of all edges of the graph. // directed?
#### void setVertexWeights(const WeightArray& weights)
* Set weight of *i*-th vertex to *weights[i]*. Size of *weights* must be equal to *n*.
#### void setVertexWeight(int v, const Weight& weight)
* Set weight of a vertex *v* to *weight*.
#### void setEdgeWeights(const WeightArray& weights)
* Set weight of *i*-th edge to *weights[i]*. Size of *weights* must be equal to *m*.
#### void setEdgeWeight(size_t index, const Weight& weight)
* Set weight of an edge with index *index* to *weight*.
#### Weight vertexWeight(int v) const
* Returns: weight of the vertex *v*.
#### Weight edgeWeight(size_t index) const
* Returns: weight of an edge with index *index*.
#### bool operator==(const GenericGraph& other) const
#### bool operator!=(const GenericGraph& other) const
#### bool operator&lt;(const GenericGraph& other) const
#### bool operator&gt;(const GenericGraph& other) const
#### bool operator&lt;=(const GenericGraph& other) const
#### bool operator&gt;=(const GenericGraph& other) const
* Compare two graphs. If number of vertices in two graphs is different then one with lesser vertices is less than the other. Otherwise adjacency lists of vertices are compared lexicographicaly in natural order of vertices.
* Note: weights have no any effect on comparison result.
* Note: two identical graphs with shuffled adjacency lists are equal.

### Weights
All things you will probably ever do with *Weight* or *WeightArray* are shown in this snippet.

```cpp
Graph g(3); // construct an empty graph on 3 vertices

graph.setVertexWeight(1, 123);
int v = graph.vertexWeight(1); // v = 123
string s = graph.vertexWeight(1); // s = "" because weight holds int now.
cout << graph.vertexWeight(1) << endl; // 123. Value which is now held is printed.
graph.setVertexWeight(2, graph.vertexWeight(1)); // Weight is copyable as wwell.

Array a{1, 2, 3};
graph.setVertexWeights(a); // implicit cast from std::vector<T> to WeightArray
// is supported for each T which can be held by Weight.
std::vector<std::string> vs{"hello", "world", "42"};
graph.setVertexWeights(vs);
```

*Weight* type is implemented as a *jngen::Variant* class. Basically it is a type-safe union which can store the value of any of the predefined types. *jngen::Variant* is a bit different from *boost::variant* and *std::variant*. The first notable exception is that valueless state is valid, i.e. variant can be empty. The second is that *jngen::Variant* allows implicit casts to any of containing types which allows you writing something like

```cpp
int w = graph.vertexWeight(1);
string s = graph.edgeWeight(2);
```

Still, it may have some flaws (I'm far not Antony Polukhin), and I'll be happy to know about them.

### Labeling
Internally graph nodes are stored as integers from 0 to n-1. However, sometimes you need to change numeration (e.g. to shuffle the graph). That's why each vertex is assigned with a *label*, and end-user does all operations with vertices using their labels. Currently labels are always a permutation of [0, n-1]. Later Jngen is going to support arbitrary labeling.
