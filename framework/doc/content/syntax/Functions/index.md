# Functions System

## Overview

`Function`s are used to define functions depending only on spatial position and
time: $f(x,y,z,t)$. These objects can serve a wide variety of purposes, including
(but not limited to) the following:

- defining initial conditions,
- defining residual contributions (sources, boundary conditions, etc.), and
- defining post-processing quantities.

!alert note title=Dependency on Solution Values
Note that there are exceptions to the rule that `Function`s only depend on
space and time; for example, [MooseParsedFunction.md] may depend on post-processor
values (which may depend on the solution) and scalar variable values.

Moose `Function`s should override the following member functions

- `Real value(Real, Point)` - returning the value of the function at a point in space and time
- `Real value(ADReal, ADPoint)` - the AD enabled version of the above function
- `Real timeDerivative(Real, Point)` - retuning the derivative of the function with respect to the first argument (time)
- `RealVectorValue gradient(Real, Point)` -  the spatial derivative with respect to the second argument

For vector valued functions

- `RealVectorValue vectorValue(Real, Point)` - returning a vector value at a point in space and time
- `RealVectorValue vectorCurl(Real, Point)` - returning the curl of the function at a point in space and time

can be overridden. The optional `Real integral()` and `Real average()` methods
can also be overridden. Note that two overloads exist for the `value()` member
function. This enables evaluation of functions with dual numbers. As most legacy
function do not implement  an AD overload of the `value()` function, the
`Function` base class automatically provides one that uses the non-AD `value()`,
`timeDerivative()`, and `gradient()` member functions to construct an AD result.
Check out `PiecewiseBilinear` to see how to update a function to support AD by
using a templated `valueInternal()` function with virtual `value()` forwarders.


!syntax list /Functions objects=True actions=False subsystems=False

!syntax list /Functions objects=False actions=False subsystems=True

!syntax list /Functions objects=False actions=True subsystems=False
