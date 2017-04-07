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

#### Array partition(int n, int numParts)
#### Array partitionNonEmpty(int n, int numParts)
#### Array64 partition(long long n, int numParts)
#### Array64 partitionNonEmpty(long long n, int numParts)
* Returns: a random ordered partition of *n* into *numParts* parts. In case of *partitionNonEmpty* each element of the result is positive.

#### template&lt;typename T> <br> TArray&lt;TArray&lt;T>> partition(TArray&lt;T> elements, int numParts)
#### template&lt;typename T> <br> TArray&lt;TArray&lt;T>> partitionNonEmpty(TArray&lt;T> elements, int numParts)
* Returns: a random partition of the array *elements* into *numParts* parts.

#### template&lt;typename T> <br> TArray&lt;TArray&lt;T>> partitionNonEmpty(TArray&lt;T> elements, const Array& sizes)
* Returns: a random partition of the array *elements* into parts, where the size of each part is specified.
* Note: sum(*sizes*) must be equal to *elements.size()*.
