# [Auxiliary System](syntax/AuxVariables/index.md)

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

- Element (e.g. constant or higher order monomials)
- Nodal (e.g. linear Lagrange)

Auxiliary variables have "old" and "older" states, from previous time steps, just like nonlinear variables

!---

### Elemental Auxiliary Variables

Element auxiliary variables can compute:

- average values per element, if stored in a constant monomial variable
- spatial profiles using higher order variables

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

AuxKernel objects computing nodal values can +only+ couple to nodal nonlinear variables and
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

## VectorAuxKernel Objects

`VectorAuxKernels` can compute vector auxiliary variables.

The auxiliary variable will have to be one of the vector types (LAGRANCE_VEC, MONOMIAL_VEC or NEDELEC_VEC).

!style! fontsize=70%

```text
[AuxVariables]
  [aux]
    order = FIRST
    family = LAGRANGE_VEC
  []
[]
[AuxKernels]
  [parsed]
    type = ParsedVectorAux
    variable = aux
    expression_x = 'x + y'
    expression_y = 'x - y'
  []
[]
```

!style-end!
