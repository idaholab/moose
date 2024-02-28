# MooseVariableLinearFVReal

## Overview

This variable is used for finite volume simulations where a linear system is solved
with the following form:

!equation
A\vec{x} = \vec{b}~,

where $\vec{x}$ is the vector containing the degrees of freedom for the declared variable,
while $A$ and $\vec{b}$ are the corresponding system matrix and right hand side, respectively.

!alert note
This variable can only be used with linear systems and cannot be used for systems which need
Jacobian and residual evaluations, such as nonlinear systems being solved by Newton or
quasi-Newton methods.

Similarly to [MooseVariableFV.md], this variable describes a field which has been discretized
using the cell-centered finite volume method and can be evaluated using the
[functor system in MOOSE](Functors/index.md).


## Example Input File Syntax

To create a `MooseLinearVariableFVReal`, users can do the following in their
input files:

```
[Variables]
  [v]
    type = MooseLinearVariableFVReal
  []
[]
```

Note that unlike the case of finite element variables, the user needs to
explicitly define the type of the variable.

!syntax parameters /Variables/MooseLinearVariableFVReal

!syntax inputs /Variables/MooseLinearVariableFVReal

!syntax children /Variables/MooseLinearVariableFVReal
