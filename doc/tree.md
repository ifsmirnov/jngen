## Trees generation

Jngen provides a *Tree* class. It offers some methods to manipulate with trees and static generators. As other Jngen objects, Tree can be printed to *std::ostream*. Here is a standard way to use generators:

```cpp
cout << Tree::random(100).shuffled() << endl;
```

### Generators
Note that all generators return trees with sorted edges to make tests more human-readable. More, numbering is not always random for same reason. Particularly, *Tree::random(size, elongation)* always returns a tree rooted at 0. You can always use *tree.shuffle()*  to renumerate vertices and shuffle edges.

#### random(size_t size, int elongation = 0)
* Returns: a random tree with given elongation. The most classical tree generator ever.
* Description: first, vertex no. 0 is selected as a root. Next, for each vertex from 1 to n-1 its parent is selected as *wnext(i, elongation)*. With *elongation = -1000000* you will likely get a star, with *elontation = 1000000* -- a bamboo (a path).

#### randomPrufer(size_t size)
* Returns: a completely random tree, selected uniformly over all n<sup>n-2</sup> trees. Name comes from the fact that this generator exploits Prüfer sequences.

#### bamboo(size_t size)
* Returns: a bamboo (or a path) of a kind 0 -- 1 -- ... -- n-1.

#### star(size_t size)
* Returns: a star graph with *size* vertices and vertex no. 0 in the center. Central vertex is counted, i.e. there are *size - 1* leaf vertices in general case.

#### caterpillar(size_t length, size_t size)
* Returns: a caterpillar tree with *size* vertices based on a path of length *length*.
* Description: first, a path of length *length* is generated. Vertices of the path are numbered in order. Next, other *size - length* vertices are connected to random vertices of the path.

### Tree methods

#### Tree& shuffle()
#### Tree shuffled() const
* Shuffle the tree. This means:
    * relabel vertices in random order;
    * shuffle edges;
    * randomly swap egdes' endpoints.

#### Array parents(int root) const
* Returns: array of size *n*, where *i*-th element is a parent of vertex *i* if the tree is rooted at *root*. Parent of *root* is *root* itself.

#### Tree link
