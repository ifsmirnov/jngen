## Trees generation

Jngen provides a *Tree* class. It offers some methods to manipulate with trees and static generators. As other Jngen objects, Tree can be printed to *std::ostream*. Here is a standard way to use generators:

```cpp
cout << Tree::random(100).shuffled() << endl;
```

### Generators
Note that all generators return trees with sorted edges to make tests more human-readable. More, numbering is not always:
#### random(size_t size, int elongation = 0)
* Returns: a random tree with given elongation. The most classical tree generator ever.
* Description: jkkk
