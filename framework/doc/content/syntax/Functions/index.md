# Functions System

## Overview

`Function`s are used to define functions depending only on time and spatial
position: $f(t,x,y,z)$. These objects can serve a wide variety of purposes,
including (but not limited to) the following:

- defining initial conditions,
- defining residual contributions (sources, boundary conditions, etc.), and
- defining post-processing quantities.

!alert note title=Dependency on Solution Values
Note that there are exceptions to the rule that `Function`s only depend on
space and time; for example, [MooseParsedFunction.md] may depend on post-processor
values (which may depend on the solution) and scalar variable values.

Moose `Function`s should override the following member functions:

- `Real value(Real, Point)` - the value of the function at a point in space and time
- `Real value(ADReal, ADPoint)` - the AD enabled version of the above function
- `Real timeDerivative(Real, Point)` - the derivative of the function with respect to the first argument (time)
- `RealVectorValue gradient(Real, Point)` -  the spatial derivative with respect to the second argument

and may optionally override the following member functions, which is only needed
for some particular functionality:

- `Real timeIntegral(Real t1, Real t1, const Point & p)`, which computes the
  time integral of the function at the spatial point `p` between the time values
  `t1` and `t2`.

For vector valued functions

- `RealVectorValue vectorValue(Real, Point)` - returning a vector value at a point in space and time
- `RealVectorValue curl(Real, Point)` - returning the curl of the function at a point in space and time
- `Real div(Real, Point)` - returning the divergence of the function at a point in space and time

can be overridden. The optional `Real integral()` and `Real average()` methods
can also be overridden. Note that two overloads exist for the `value()` member
function. This enables evaluation of functions with dual numbers. As most legacy
function do not implement  an AD overload of the `value()` function, the
`Function` base class automatically provides one that uses the non-AD `value()`,
`timeDerivative()`, and `gradient()` member functions to construct an AD result.
Check out `PiecewiseBilinear` to see how to update a function to support AD by
using a templated `valueInternal()` function with virtual `value()` forwarders.


### Functions as Functors

Functions are [Functors](syntax/Functors/index.md). Functors are an abstraction, a base class, for
objects that can compute values at a location in space and time.

As `Functors`, they may be specified to objects such as the
[FunctorAux.md] in their [!param](/AuxKernels/FunctorAux/functor) parameter. This vastly expands the number
of objects that can use `Functions` to compute spatial quantities.

!alert note
When making a new object using `Functions` to contribute back to MOOSE,
we ask that you consider using [Functors](syntax/Functors/index.md) instead
to naturally enable its use with variables and functor material properties.

!syntax list /Functions objects=True actions=False subsystems=False

!syntax list /Functions objects=False actions=False subsystems=True

!syntax list /Functions objects=False actions=True subsystems=False
