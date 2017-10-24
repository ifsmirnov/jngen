## Test suites

Having instruments for tests preparation is nice, but having already prepared tests is nicer, isn't it? Jngen has several test suites:
these are predefined sets of tests for several kinds of problems.

A typical generator looks like this:

```cpp
int id = getOpt("id");
const auto& suite = testSuites.tree;

ensure(id <= (int)suite.size());

Tree tree = suite.gen(id, n).shuffle();
tree.setEdgeWeights(Array::random(tree.m(), 1, w));
cout << tree.add1().printN() << endl;
```

And then you have a test script:

```
gen -id 1
gen -id 2
...
gen -id 50
```

Voi-la! A reasonable testset for a problem is ready in a minute. Of course, you still have to prepare some specific tests for your problem,
but you'd have to do it anyway: this tool saves you from inevitable routine work.

### Interface
All interaction happens with the object *testSuites*, specifically with its member (*testSuites.tree*, *testSuites.string* etc). To generate a test
you need to call the method *gen(id, ...)*, where *id* is a number of the the testcase and other parameters are specific for the generator.
For example, *tree* accepts a single parameter *n*, the number of vertices, and *string* needs *length* and (optional) *alphabet*.
This function returns an object (here, it is either *Tree* or *string*), you can print it immediately or modify somehow.

It is also possible to refer to a test by its name (`testSuites.tree.gen("sqrt_branches", 100000)`);

Testcases are numbered from 1. There is a method *size* which returns the number of tests. Thus *id* must be from *1* to *suite.size()*.

It is not guaranteed that all tests are distinct. However, it is highly likely if the size is large enough and each test is seeded
with a different value (and it is if you use the script above).

### Exact suites
This is the description of supported existing test suites. Numbering may change from version to version, so it is better to refer
to the exact testcase by name rather than by id.
* [tree](tree.md)
