## Configuration

Jngen has some built-in "sanity checks": if you want to generate an array of size 481927184, likely you have an uninitialize variable. Jngen will gracefully terminate and report it to you (instead of causing OOM error and possibly hanging the machine).

However, sometimes you know better and may want to turn these checks off. To do it, simply put a line at the beginning of *main*:
```cpp
config.optionName = true/false;
```

### List of configurable options (default value)
#### generateLargeObjects (false)
* Allow generating arrays, graphs and so of size exceeding 5 million.

#### largeOptionIndices (false)
* Allow calling *getOpt(n)* for *n >= 32*. This check is created to report if you accidentally call *getOpt('C')* (that is, with char instead of string).

#### normalizeEdges (true)
* If this option is set, edges of newly generated graphs are printed in sorted order to make output more human-readable. You may turn it off if you care about performance rather than presentation.
