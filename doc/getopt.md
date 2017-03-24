## Parsing command-line options
Jngen provides a parser of command-line options. It supports both positional and named arguments. Here is the comprehensive example of usage.
```cpp
// ./main 10 -pi=3.14 20 -hw=hello-world randomseedstring
int main(int argc, char *argv[]) {
    parseArgs(argc, argv);
    int n, m;
    double pi;
    string hw;

    getPositional(n, m); // n = 10, m = 20
    getNamed(hw, pi); // hw = "hello-world", pi = 3.14

    int otherN;
    double otherPi;
    getOpt(0, otherN); // otherN = 10, 0 is an index
    getOpt("pi", otherPi); // otherPi = 3.14, "pi" is a name
    cout << getOptOr("none", 10) << endl; // 10 as there is no "none" option
}
```

### Documentation
#### void parseArgs(int argc, char *argv)

* Parse arguments and prepare variable map. Required to be called before any *getOpt...* calls.
#### template&lt;typename T> <br> bool getOpt(size_t index, T& t)
#### template&lt;typename T> <br> bool getOpt(const std::string& name, T& t)
* Reads an option denoted by *index* (positional, 0-indexed) or *name*. In case if *index* is out of bound, *name* option does not exist or the given option fails to be read as *T*, *t* is not modified.
* Returns: true if option exist and *T* was succesfully read.

#### template&lt;typename T> <br> T getOptOr(size_t index, T def)
#### template&lt;typename T> <br> T getOptOr(const std::string& name, T def)
* Reads an option denoted by *index* or *name*. If it exists and can be interpreted as *T*, the read value is returned. Otherwise *def* is returned.

#### int getPositional(Args&... args)
* Reads positional options to *args...* in order. Arguments which could not be read are not modified.
* Returns: number of succesfully read arguments.

#### int getNamed(Args&... args)
* Reads named arguments. Variable *x* is interpreted as having name *x*. Arguments which could not be read are not modified.
* Returns: number of succesfully read arguments.
* Note: this function is implemented with a define and may be not noticed by your autocompletion tool.

