# Just in Time Compilation

!media docs/media/phase_field/jitgraph_white.png width=50% padding-left=20px float=right caption=Performance test results for the JIT compile FParser module

The version of the _Function Parser_ library that ships with MOOSE contains a just in time (JIT) compilation  feature that is not present in the [upstream version](http://warp.povusers.org/FunctionParser/).

Include the JIT module (which also adds [automatic differentiation](Function Materials/Automatic Differentiation.md) support) with

```cpp
#include "libmesh/fparser_ad.hh"
```

The module provides the class

```cpp
FunctionParserADBase<Real>
```

which derives from

```cpp
FunctionParserBase<Real>
```

and provides the additional method

```cpp
bool JITCompile();
```

Calling this method on an FParser object will execute an attempt at
* generating a small temporary C++ file with a function that performs the calculation of the parsed function
* launching a compiler (the same compiler used to build libMesh) to compile the C++ file into a small dynamic library (`.so` file)
* using `dlopen()` to immediately load the library file
* using `dlsym()` to bind the compiled function to the FParser object

If all steps above succeed all further evaluations (calls to `Eval()`) of the FParser object will be redirected to the compiled function, which will yield a speedup of about an order of magnitude, depending on the complexity of the function.

All temporary files will be cleaned up and if JIT compilation succeeded the compiled library will be cached in a directory named `.jitcache` in the local directory. The name of the function file is constructed using a sha1 hash of the function byte code.

Compile times are around 100ms per function object. Once cached the `JITCompile()` call will return after about 1ms. Changing numeric constants in the function will usually not trigger a recompilation. The compiled function respects the current FParser `Epsilon` setting.

Almost all FParser opcodes are supported, _except_ `PCall` and `FCall`, which are function calls to other FParser objects and calls to custom functions.
