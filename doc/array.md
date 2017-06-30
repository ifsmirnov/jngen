## Arrays

Jngen provides a template class *TArray&lt;T>* which is derived from *std::vector&lt;T>* and implements all its functionality... and some more handy things like single-argument sorting (*a.sort()*) , in-place generating of random arrays (*Array::random(n, maxValue)*) and more.

There are several typedefs for convenience:
```cpp
typedef TArray<int> Array;
typedef TArray<long long> Array64;
typedef TArray<double> Arrayf;
typedef TArray<std::pair<int, int>> Arrayp;
typedef TArray<TArray<int>> Array2d;
```
In this document *Array* will be mostly used instead of *TArray&lt;T>*. Usually it means that corresponding method works for arrays of any type; if not, it will be mentioned explicitly.

### Generators
#### template&lt;typename ...Args> <br> static Array Array::random(size_t size, Args... args)
#### template&lt;typename ...Args> <br> static Array Array::randomUnique(size_t size, Args... args)
#### template&lt;typename ...Args> <br> static Array Array::randomAll(Args... args)
* Returns: array of *size* random elements generated as *rnd.tnext&lt;T>(args...)*. In the second version all generated elements are distinct. In the third version generation runs until no new elements appear with high probability.
* Note: *randomUnique* and *randomAll* assume uniform distribution on data. I.e. if your method returns 1 with probability 0.999 and 2 with probability 0.001, *randomUnique(2, ...)* will most likely terminate saying that there are not enough distinct elements.
* Complexity:
    * *random*: *size* calls of *rnd.tnext*;
    * *randomUnique*: approximately *O(size log size)* calls of *rnd.tnext*;
    * *randomAll*: approximately *O(size log size)* calls of *rnd.tnext*, where *size* is the number of generated elements.
* Examples:
```cpp
Array::randomUnique(10, 10)
```
yields a random permutation on 10 elements (though more optimal way is *Array::id(10).shuffled()*);

```cpp
Arrayp::random(20, 10, 10, dpair)
```
yields edges of a random graph with 10 vertices and 20 edges, possibly containing multi-edges, but without loops.

#### template&lt;typename F, typename ...Args> <br> static Array Array::randomf(size_t size, F func, Args... args)
#### template&lt;typename F, typename ...Args> <br> static Array Array::randomfUnique(size_t size, F func, Args... args)
#### template&lt;typename F, typename ...Args> <br> static Array Array::randomfAll(F func, Args... args)
* Same as *Array::random*, but *func(args...)* is called instead of *rnd.tnext*.
* Example:
```cpp
TArray<std::string>::randomf(
	10,
	[](const char* pattern) { return rnd.next(pattern); },
	"[a-z]{5}")
```
yields an array of 10 strings of 5 letters each.

#### Array Array::id(size_t size, T start = T())
* Generates an array of *size* elements: *start*, *start + 1*, ...
* Note: defined only for integer types.

### Modifiers
Most of modifiers have two versions: the one which modifies the object itself and the one which returns the modified copy. They are usually named as *verb* and *verb-ed*, e.g. *shuffle* and *shuffled*.

#### Array& shuffle()
#### Array shuffled() const
* Shuffle the array. The source of randomness is *rnd*.

#### Array& reverse()
#### Array reversed() const
* Reverse the array.

#### Array& sort()
#### Array sorted() const
* Sort the array in non-decreasing order.

####  template&lt;typename Comp> <br> Array& sort(Comp&& comp)
#### template&lt;typename Comp> <br> Array sorted(Comp&& comp) const
* Sort the array in non-decreasing order using *comp* as a comparator.

#### Array& unique()
#### Array uniqued() const
* Remove consequent duplicates in the array. Equivalent to *std::erase(std::unique(a.begin(), a.end()), a.end())*.
* Note: as *std::unique*, this method doesn not remove all duplicated elements if the array is not sorted.

#### Array inverse() const
* Returns: inverse permutation of the array.
* Note: defined only for integer types. Terminates if the array is not a permutation of \[0, n).

#### void extend(size_t requiredSize);
* Equivalent to *resize(max(size(), requiredSize))*.

### Selectors
#### template&lt;typename Integer> <br> Array subseq(const std::vector<Integer>& indices) const;
#### template&lt;typename Integer> <br> Array subseq(const std::initializer_list<Integer>& indices) const;
* Returns: subsequence of the array denoted by *indices*.
* Example:
```cpp
a = a.subseq(Array::id(a.size()).shuffled());
```
effectively shuffles *a*. For example, this may be used to shuffle several arrays with the same permutation.

#### T choice() const;
* Returns: random element of the array.

#### Array choice(size_t count) const;
* Returns: an array of *count* elements of the array **without repetition**.
* Note: obviously, *count* should be not greater than *array.size()*.

#### Array choiceWithRepetition(size_t count) const;
* Returns: an array of *count* elements of the array, possibly repeating.

### Operators
#### Array& operator+=(const Array& other);
#### Array operator+(const Array& other) const;
* Inserts *other* to the end of the array.

#### Array& operator*=(int k);
#### Array operator*(int k) const;
* Repeats the array *k* times.

#### operator std::string() const;
* Casts TArray&lt;char> to std::string.
* Note: defined only for TArray&lt;char>.
