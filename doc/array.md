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
#### template&lt;typename F, typename ...Args> <br> Array Array::randomf(size_t size, F func, Args... args)
#### template&lt;typename F, typename ...Args> <br> Array Array::randomfUnique(size_t size, F func, Args... args)
#### template&lt;typename F, typename ...Args> <br> Array Array::randomfAll(F func, Args... args)
* Returns: array of *size* random elements generated as *func(args...)*. In the second version all generated elements are distinct. In the third version generation runs until no new elements appear with high probability.
* Note: *randomfUnique* and *randomfAll* assume uniform distribution on data. I.e. if your method returns 1 with probability 0.999 and 2 with probability 0.001, *randomfUnique(2, ...)* will most likely terminate saying that there are not enough distinct elements.
* Complexity:
    * *randomf*: *size* calls of *func*;
    * *randomfUnique*: approiximately *O(size log size)* calls of *func*;
    * *randomfAll*: approximately *O(size log size)* calls of *func*, where *size* is the number of generated elements.
* Example:
```cpp
TArray<std::string>::randomf(
	10,
	[](const char* pattern) { return rnd.next(pattern); },
	"[a-z]{5}")
```
yields an array of 10 strings of 5 letters each.

#### template&lt;typename ...Args> <br> Array Array::random(size_t size, Args... args)
#### template&lt;typename ...Args> <br> Array Array::randomUnique(size_t size, Args... args)
#### template&lt;typename ...Args> <br> Array Array::randomAll(Args... args)
* Same as *randomf* versions, but function called is *rnd.next(args...)*.

#### Array Array::id(size_t size, T start = T())
* Generates an array of *size* elements: *start*, *start + 1*, ...
* Note: defined only for integer types.

### Modifiers
Most of modifiers have two versions: the one which modifies the object itself and the one which returns the modified copy. They are usually named as *verb* and *verb-ed*, e.g. *shuffle* and *shuffled*.

#### Array& Array::shuffle()
#### Array Array::shuffled() const
* Shuffle the array. The source of randomness is *rnd*.

#### Array& Array::reverse()
#### Array Array::reversed() const
* Reverse the array.

#### Array& Array::sort()
#### Array Array::sorted() const
* Sort the array in non-decreasing order.

####  template&lt;typename Comp> <br> Array& Array::sort(Comp&& comp)
#### template&lt;typename Comp> <br> Array Array::sorted(Comp&& comp)
* Sort the array in non-decreasing order using *comp* as a comparator.

#### Array& Array::unique()
#### Array Array::uniqued() const
* Remove consequent duplicates in the array. Equivalent to *std::erase(std::unique(a.begin(), a.end()), a.end())*.
* Note: as *std::unique*, this method doesn not remove all duplicated elements if the array is not sorted.

#### Array Array::inverse() const
* Returns: inverse permutation of the array.
* Note: defined only for integer types. Terminates if the array is not a permutation of \[0, n).
