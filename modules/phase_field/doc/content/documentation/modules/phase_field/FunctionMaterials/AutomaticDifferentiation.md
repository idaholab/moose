# Automatic Differentiation

Automatic differentiation is a means to construct derivatives of a given expression in a symbolic
way. Specifically this means _not_ performing finite differencing.  Derivatives show up the phase
field evolution equations (commonly derivatives of both the free energy and the
mobilities). Automatic differentiation shifts the burden of computing the derivatives of the
oftentimes complex known expressions for the free energies from the user to the software.

The version of the _Function Parser_ library that ships with MOOSE contains an automatic
differentiation feature that is not present in the
[upstream version](http://warp.povusers.org/FunctionParser/).

Include the automatic differentiation module (which also adds
[just in time compilation](JITCompile support) with

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
int AutoDiff(const std::string& var);
```

where `var` is the name of the variable to take the derivative with respect to. The `AutoDiff` method
must be called on an initialized and parsed (but not optimized) Function Parser object.

The automatic differentiation system transforms the compiled bytecode of the parsed function into the
bytecode of the derivative. The `AutoDiff` method can be called multiple times to generate high order
derivatives.

The helper method

```cpp
bool isZero();
```

can be called after optimizing the Function Parser object to check if the function is a constant zero
(i.e. the bytecode consists only of a `push 0` command).


## Limitations

Almost all FParser opcodes are supported, _except_ `PCall` and `FCall`, which are function calls to
other FParser objects and calls to custom functions.

The automatic differentiation will currently complain when taking derivatives of functions that are
not differentiable in a countable infinite number of points (such as `int()`), it will however take
derivatives of functions like the absolute value `abs()` that are not differentiable in only a single
point. This limitation is arbitrary and may be changed in future versions.

The current version of the AD module *does* support differentiating FParser objects that have been
previously optimized.

## Example

The following code snippet illustrates how to compute and evaluate a derivative:

```cpp
FunctionParserADBase<Real> fparser;
std::string func = "sin(a*x)+x^2*(3+sin(3*x))+a";

// Parse the input expression into bytecode
fparser.Parse(func, "x,a");

// transform F -> dF/dx
fparser.AutoDiff("x");

// run optimizer to simplify the derivative
fparser.Optimize();

// evaluate the derivative (method from FParserBase<Real>)
Real params[2] = {0.1, 1.7};
std::cout << fparser.Eval(params) << std::endl;

// print byte code of the derivative (needs debugging enabled)
fparser.PrintByteCode(std::cout);
```

Not that optimizing the FParser object using `.Optimize()` after taking the derivative is *highly*
recommended, as the automatic differentiation generates lots of trivial terms. For example the
derivative of `2*x` is `0*x+2*1`, which will be optimized to `2`.

## Performance considerations

The convenience of parsed and automatically differentiated functions does come with a performance
penalty. Despite optimizations the parsed functions are slower than hand coded and compiled
functions.

JIT (Just In Time) compilation is available for parsed functions. The JIT system is utilized by
adding the `enable_jit = true` (default) option in the
[`DerivativeParsedMaterial`](/DerivativeParsedMaterial.md) block. MOOSE will then attempt to compile
the functions and its derivatives into machine code and use it for the residual and Jacobian
calculations. This almost fully recovers the performance of hand coded free energies while retaining
the flexibility of automatic differentiation.

## See also

- [ExpressionBuilder](FunctionMaterials/ExpressionBuilder.md) - building FParser expressions at compile time using operator overloading
