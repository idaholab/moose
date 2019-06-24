# Auxiliary System

A system for direct calculation of field variables ("AuxVariables") that is designed for
postprocessing, coupling, and proxy calculations.

!---

The term "nonlinear variable" is defined, in MOOSE language, as a variable that is being solved for
using a nonlinear system of [!ac](PDEs) using `Kernel` and `BoundaryCondition` objects.


The term "auxiliary variable" is defined, in MOOSE language, as a variable that is directly
calculated using an `AuxKernel` object.

!---

## AuxVariables

Auxiliary variables are declared in the `[AuxVariables]` input file block

Auxiliary variables are field variables that are associated with finite element shape functions
and can serve as a proxy for nonlinear variables

Auxiliary variables currently come in two flavors:

- Element (constant or higher order monomials)
- Nodal (linear Lagrange)

Auxiliary variables have "old" and "older" states just like nonlinear variables

!---

### Elemental Auxiliary Variables

Element auxiliary variables compute average values per element (constant)

AuxKernel objects computing elemental values can couple to nonlinear variables and both element and
nodal auxiliary variables

```text
[AuxVariables]
  [aux]
    order = CONSTANT
    family = MONOMIAL
  []
[]
```

!---

### Nodal Auxiliary Variables

Element auxiliary variables are computed at each node and are stored as linear Lagrange variables

AuxKernel objects computing nodal values can +only+ couple to nonlinear variables and
other nodal auxiliary variables

```text
[AuxVariables]
  [aux]
    order = LAGRANGE
    family = FIRST
  []
[]
```

!---

## AuxKernel Objects

Directly compute AuxVariable values by overriding `computeValue()` and they can operate on
both elemental and nodal auxiliary variable.

When operating on a nodal variable `computeValue()` operates on each node; when operating
on a elemental variable it operates on each element.

!---

## AuxKernel Object Members

`_u`, `_grad_u`\\
Value and gradient of variable this AuxKernel is operating on

`_q_point`\\
Coordinates of the current q-point that is only valid for elemental AuxKernels, `_current_node`
should be used for nodal variables

`_qp`\\
Current quadrature point, this is used for both nodal and elemental variables for consistency

`_current_elem`\\
Pointer to the current element that is being operated on (elemental only)

`_current_node`\\
Pointer to the current node that is being operated on (nodal only)

!---

## VectorAuxKernel Objects

Directly compute a vector AuxVariable values by overriding `computeValue()`, with the difference
being the return value of a `RealVectorValue` instead of Real.

```text
[AuxVariables]
  [aux]
    order = FIRST
    family = LAGRANGE_VEC
  []
[]
```
