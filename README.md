# d_gen
d_gen is the tool for generating a set of tests for an algorithm written in the specifically designed language.
Basically, when interpreting a program every first access to an input variable triggers a newly generated
value being bound to it. If the first access to a variable appears to be inside the condition of the branch operator
d_gen uses [Z3 SMT Solver](https://www.microsoft.com/en-us/research/project/z3-3/) to find the value that satisfies the condition. A user is given the option to choose
the probability of executing the positive or the false branch.
This approach that uses the execution of concrete and symbolic values is usually called [Concolic execution](https://en.wikipedia.org/wiki/Concolic_testing).

## Motivation
Not every algorithm can be tested properly using randomly generated values.
For example, feeding the random string to the [prefix function](https://cp-algorithms.com/string/prefix-function.html) 
algorithm always results in generating an array where almost every element is zero. That makes it unable to test
every branch of an algorithm.
With d_gen you can choose what kind of output you want to see, whether it's the array with zero elements or 
the diverse array that corresponds to the string with highly repetitive characters.

As an example, you may consider the output from two versions of the prefix function algorithm.

[The good](examples/prefix_func.dg):
```
{
  tests: [
        {
                s: "zzzzz",
                prefix_func: [0,1,2,3,4]
        },
        {
                s: "s",
                prefix_func: [0]
        },
        {
                s: "zzyzzy",
                prefix_func: [0,1,0,1,2,3]
        }
  ]
}
```

[The bad](examples/bad_prefix_func.dg) that emulates random generation with useless access to the input string:
```
{
  tests: [
        {
                s: "",
                prefix_func: []
        },
        {
                s: "qnt",
                prefix_func: [0,0,0]
        },
        {
                s: "rhnkprqqu",
                prefix_func: [0,0,0,0,0,1,0,0,0]
        }
  ]
}
```

## Language
### Overview
Designed language is statically and (most of the times) strongly typed. It resembles a mix of C and Go.

Operators:
- `if`
- `for`
- `while`
- `continue`
- `break`
- `return`
- `increment/decrement` `(++/--)`
- `complex assignment` (like `+=`, `-=`, etc)
- `comparison operators` (like `==`, `>`, etc)

More information on grammar is listed in [ANLTR4 grammar file](d_gen.g4).

The type system includes:
- `int`
- `char`
- `bool`
- `string`
- `array` (including the nested ones)

Implicit conversions are permitted only when assigning between the next types:
`char <-> int` and `string <-> char[]`.
For example, 
```
int i = 97
char c = i + 2 // evaluates to 'c'

string str = char[100] // allocatings string as an array of chars

c = 'a' + 5  // triggers build error: different arguments types for sum operation
```

A program must contain one function that accepts at least one argument. Also, you can't assign to input variables.
The condition expression in `if` and `loop` operators must evaluate to the value of `bool` type.

You can find examples under the corresponding [directory](examples).

### Generation
By default, d_gen randomly generates input data. You're given the option to tune the generation. 
Before every conditional operator (`if`, `while`, `for`) you can insert a special construction - `[prob = <optional probability>; <optional conditional expression>]`.

The conditional expression is an additional constraint that you want to have on an input variable. 
It's just the regular conditional expression that must evaluate to `bool`.

Probability defines whether the condition in the corresponding operator evaluates to `true` or `false` most likely.
Based on it, d_gen will generate appropriate values for input variables inside the condition expression.

## Tool
The tool outputs test data using d_gen library as a json array of tests.

Arguments:
- -f<file_name> - path to algorithm file (required)
- -n<num_tests> - number of tests (required)
- -s<seed> (optional seed, otherwise unix time)

Example:
`./d_gen_tool -fprefix_func.dg -n10 -s50`

## Build
### Build d_gen shared library
```
cmake -B build
cd build
make -j4
sudo make install #installs the so library and public headers to the appropriate place
```
### Build d_gen tool
```
cd tool
cmake -B build
cd build
make -j4
```
### Build a custom app
To build the custom app you need 
- shared d_gen library installed (`so` and headers)
- Z3 `v4.12.2.0` `so` file

You can use the tool's CMake file as a template.
### Dependencies

#### Z3
[Follow installation rules](https://github.com/Z3Prover/z3/blob/master/README-CMake.md)

#### LLVM
[Install LLVM-13](https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.0)

#### ANTLR4
- Download ANLTR4 jar executable v4.12.0 under /usr/local/lib
- Runtime library is downloaded and built at the build time
