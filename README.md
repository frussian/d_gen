# d_gen
Test data generator

## Semantics

### Types
`bool`, `int`, `string`, `arrays`, `char`

## Tool
The tool outputs test data using d_gen library as a json array of tests.

Arguments:
- -f<file_name> - path to algorithm file (required)
- -n<num_tests> - number of tests (required)
- -s<seed> (optional seed, otherwise unix time)

Example:
`d_gen_tool -fprefix_func.dg -n10 -s50`

## Build
### Build d_gen shared library
```
cmake -B build
cd build
make -j4
sudo make install #installs the so library and public headers to appropriate place
```
### Build d_gen tool
```
cd tool
cmake -B build
cd build
make -j4
```
### Build custom app
To build custom app you need 
- shared d_gen library installed (`so` and headers)
- Z3 `v4.12.2.0` `so` file

You can use the tool's CMake file as a template.
### Dependencies

#### Z3
[Follow installation rules](https://github.com/Z3Prover/z3/blob/master/README-CMake.md)

#### LLVM
[Install LLVM-13](https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.0)

#### ANTLR4
- ANLTR4 jar executable v4.12.0 under /usr/local/lib
- Runtime library is downloaded and built at the build time
