## Math-ish primitives

Jngen provides several free functions and a generator class *MathRandom* to help generating numbers and combinatorial primitives. All interaction with *MathRandom* goes via its global instance called *rndm*. The source of randomness is *rnd*.

### Standalone functions

#### bool isPrime(long long n)
* Returns: true if *n* is prime, false otherwise.
* Supported for all *n* from 1 to 3.8e18.
* Implemented with deterministic variation of the Miller-Rabin primality test so should work relatively fast (exact benchmark here).

### MathRandom methods

#### long long randomPrime(long long n)
#### long long randomPrime(long long l, long long r)
* Returns: random prime in range *[2, n)* or *[l, r]* respectively.
* Throws if no prime is found on the interval.

#### long long nextPrime(long long n)
#### long long previousPrime(long long n)
* Returns: the first prime larger (or smaller) than *n*, including *n*.

#### Array partition(int n, int numParts, int minSize = 0, int maxSize = -1)
* Returns: a random ordered partition of *n* into *numParts* parts, where the size of each part is between *minSize* and *maxSize*. If *maxSize* is *-1* (the default value) then sizes can be arbitrary large.

#### template&lt;typename T> <br> TArray&lt;TArray&lt;T>> partition(TArray&lt;T> elements, int numParts, int minSize = 0, int maxSize = -1)
* Returns: a random partition of the array *elements* into *numParts* parts.

#### template&lt;typename T> <br> TArray&lt;TArray&lt;T>> partition(TArray&lt;T> elements, const Array& sizes)
* Returns: a random partition of the array *elements* into parts, where the size of each part is specified.
* Note: sum(*sizes*) must be equal to *elements.size()*.
