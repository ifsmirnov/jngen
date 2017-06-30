## Geometry

Jngen provides two point classes: *Point* with *long long* coordinates and *Pointf* with *long double* coordinates. Standard operations like addition, subtraction, dot and cross products are supported. Similarly, classes *Polygon* and *Polygonf* are provided. A special class *GeometryRandom* is used for generating objects, all interaction goes via its global instance *rndg*.

Like most Jngen objects, *Point* and *Polygon* can be printed to streams and modified with [output modifiers](printers.md).

### Generators (*rndg* static methods)
#### Point point(long long C)
#### Pointf pointf(long double C)
* Returns: random point with coordinates between 0 and C, inclusive.

#### Point point(long long X, long long Y)
#### Pointf pointf(long double X, long double Y)
* Returns: random point with x-coordinate between 0 and X and y-coordinate between 0 and Y, inclusive.

#### Polygon convexPolygon(int n, long long C)
#### Polygon convexPolygon(int n, long long X, long long Y)
* Returns: random convex polygon with *n* vertices and coordinates lying in specified range.
* No three consecutive vertices lie on the same line, no two points coincide.
* Polygon is generated like following: convex hull of *10n* random points on an ellipse is taken,
    then *n* points are randomly selected from it.
* Throws if the are less than *n* points on the above convex hull.

#### TArray&lt;Point> pointsInGeneralPosition(int n, long long C)
#### TArray&lt;Point> pointsInGeneralPosition(int n, long long X, long long Y)
* Returns: *n* random points such that no two coincide and no three lie on the same line.
* Complexity: *O(n<sup>2</sup> log n)*.

### Point and Pointf operators
Here is the list of operators supported for *Point* and *Pointf*. All of them are declared *const*, excluding those which explicitly modify their arguments.

* _p1 + p2_, _p1 += p2_: coordinate-wise addition;
* _p1 - p2_, _p1 -= p2_: coordinate-wise subtraction;
* _p * x_, _p *= x_: coordinate-wise multiplication with scalar value;
* _p1 * p2_: dot product (_p1.x * p2.x + p1.y * p2.y_);
* _p1 % p2_: cross product (_p1.x * p2.y - p1.y * p2.x_);
* _p1 == p2_, _p1 != p2_: coordinate-wise equality comparison;
* _p1 < p2_: lexicographical coordinate-wise ordering.

For *Pointf* comparisons of floating point values are done with *eps* presision. The default value is *10<sup>-9</sup>*. It can be overridden with *setEps* function.

### Polygon and Polygonf methos
*Polygon* inherits *TArray&lt;Point>* so has it supports standard Array methods like *.sort()*, *.choice()* and so on. However, it provides a couple of additional methods.

#### Polygon& shift(const Point& vector)
#### Polygon shifted(const Point& vector) const
* Shift the polygon by given *vector*, i.e. add *vector* to each vertex of a polygon.
