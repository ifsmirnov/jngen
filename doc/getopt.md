## Parsing command-line options
Jngen provides a parser of command-line options. It supports both positional and named arguments. Here is the comprehensive example of usage.

```cpp
// ./main 10 -pi=3.14 20 -hw hello-world randomseedstring
int main(int argc, char *argv[]) {
    parseArgs(argc, argv);
    int n, m;
    double pi;
    string hw;

    n = getOpt(0); // n = 10
    pi = getOpt("pi"); // pi = 3.14

    n = getOpt(5, 100); // n = 100 as there is no option #5
    pi = getOpt("PI", 3.1415); // pi = 3.1415 as there is no option "PI"

    getPositional(n, m); // n = 10, m = 20
    getNamed(hw, pi); // hw = "hello-world", pi = 3.14

    cout << (int)getOpt("none", 10) << endl; // 10 as there is no "none" option
}
```

### Options format
* Any option not starting with "-" sign is a positional option;
* positional options are numbered from 0 sequentially (e.g. if there is a positional option, then named, then again positional, two positional options will have indices 0 and 1);
* named options can have form "-name=value" and "-name value", though the second is allowed if *value* does not start with a hyphen;
* if an option name immediately follows another option name (e.g. "-first -second ..." than the value of *first* is set to 1;
* single "-" sign is ignored;
* anything after "&dash;&dash;" (two minus signs) is ignored;

### Documentation

#### void parseArgs(int argc, char *argv)
* Parse arguments and prepare variable map. Required to be called before any *getOpt...* calls.

#### *unspecified_type* getOpt(size_t index)
#### *unspecified_type* getOpt(const std::string& name)
* Reads an option denoted by *index* (positional, 0-indexed) or *name*. Throws if the option does not exist.
* Return type can be casted to any other type. See the expected usage:
```cpp
int n = getOpt(0), m = getOpt(1);
double h = getOpt("height");
```
* Note: if the cast fails (e.g. you try to interpret "adsfasd" as int) the function throws.

#### template&lt;typename T> <br> *unspecified_type* getOpt(size_t index, T def)
#### template&lt;typename T> <br> *unspecified_type* getOpt(const std::string& name, T def)
* Same as *getOpt(index)* and *getOpt(name)*, but if the option doens't exist then *def* is returned.
* Note: the function still throws if the option exists but the cast fails.

#### bool hasOpt(size_t index)
#### bool hasOpt(const std::string& name)
* Checks if the option denoted by *index* or *name* is present. Its value is not examined.

#### int getPositional(Args&... args)
* Reads positional options to *args...* in order. Arguments which could not be read are not modified.
* Returns: number of succesfully read arguments.

#### int getNamed(Args&... args)
* Reads named arguments. Variable *x* is interpreted as having name *x*. Arguments which could not be read are not modified.
* Returns: number of succesfully read arguments.
* Note: this function is implemented with a define and may be not noticed by your autocompletion tool.
