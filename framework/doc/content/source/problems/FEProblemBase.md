# FEProblemBase

The FEProblemBase class is an intermediate base class containing all of the common
logic for running the various MOOSE simulations. MOOSE has two built-in types of
problems [FEProblem.md] for solving "normal" physics problems and [EigenProblem.md]
for solving Eigenvalue problems. Additionally, MOOSE contains an [ExternalProblem.md]
problem useful for creating ["MOOSE-wrapped Apps"](moose_wrapped_apps.md optional=True).

## Convenience Zeros

One of the advantages of the MOOSE framework is the ease at building up Multiphysics
simulations. Coupling is a first-class feature and filling out residuals, or
materials properties with coupling is very natural. When coupling is optional, it
is often handy to have access to valid data structures that may be used in-place
of the actual coupled variables. This makes it possible to avoid branch statements
inside of your residual statements and other computationally intensive areas of
code. One of the ways MOOSE makes this possible is by making several different
types of "zero" variables available. The following statements illustrate how
optional coupling may be implemented with these zeros.

```cpp
// In the constructor initialization list of a Kernel

  _velocity_vector(isParamValid("velocity_vector") ? coupledGradient("velocity_vector") : _grad_zero)


// The residual statement

  return _test[_i][_qp] * (_velocity_vector[_qp] * _grad_u[_qp]);
```

## Selective Reinit

The system automatically determines which variables should be made available for use on the
current element ("reinit"-ed). Each variable is tracked on calls through the coupling interface.
Variables that are not needed are simply not prepared. This can save significant amounts
of time on systems that have several active variables.

!syntax description /Problem/FEProblem

!syntax parameters /Problem/FEProblem

!syntax inputs /Problem/FEProblem

!syntax children /Problem/FEProblem

!bibtex bibliography
