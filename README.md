# d_gen
Test data generator

## Build
```
cmake -B build
cd build
make -j4
```
### Dependencies

#### Z3
[Follow installation rules](https://github.com/Z3Prover/z3/blob/master/README-CMake.md)

#### LLVM
[Install LLVM-13](https://github.com/llvm/llvm-project/releases/tag/llvmorg-13.0.0)

#### ANTLR4
Downloaded and built at the build time
## Semantics

### Types
`bool`, `int`, `string`, `arrays`, `char`
