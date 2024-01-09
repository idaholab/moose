# MooseVariableLinearFVReal

## Overview

This variable is used for finite volume simulations where a linear system is solved
with the following form:

!equation
A\vec{x} = \vec{b}~,

where $\vec{x}$ is the vector containing the degrees of freedom for the declared variable,
while $A$ and $\vec{b}$ are the corresponding system matrix and right hand side, respectively.

This variable can only be used with linear systems and cannot be used for systems which need
Jacobian and residual evaluations.

Similarly to [MooseVariableFV.md], this variable describes a field which has been discretized
using the cell-centered finite volume method.


## Example Input File Syntax

To create a `MooseLinearVariableFVReal` a user can do one of the following in their
input file:

```
[Variables]
  [v]
    type = MooseLinearVariableFVReal
  []
[]
```

## Functor spatial evaluation description

For background on the functor system see [Functors/index.md].

- `ElemArg`: returns the cell center value
- `ElemPointArg`: returns a two term expansion that is the sum of the cell
  center value and the product of the distance of the point from the cell center
  times the cell center gradient
- `FaceArg`: on internal faces this will return an interpolation defined by the
  `FaceArg` `limiter_type`. On external faces this will generally return a two
  term expansion using the cell center value and gradient unless
  `two_term_boundary_expansion` has been set to `false`. On Dirichlet faces this
  will return the Dirichlet value
- `ElemQpArg`: this forwards to `ElemPointArg` where the `point` is simply the
  quadrature point location
- `ElemSideQpArg`: same as `ElemQpArg`
- `NodeArg`: loops over connected elements, calling to the `ElemPointArg`
  overload for each element with the node's location as the `point` value,
  and performs an inverse distance weighting of the results

!syntax parameters /Variables/MooseLinearVariableFVReal

!syntax inputs /Variables/MooseLinearVariableFVReal

!syntax children /Variables/MooseLinearVariableFVReal
