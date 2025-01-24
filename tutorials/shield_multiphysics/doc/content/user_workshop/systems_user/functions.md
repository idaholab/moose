# [Function System](syntax/Functions/index.md)

A system for defining analytic expressions based on the spatial location ($x$, $y$, $z$) and
time, $t$.

A `Function` can depend an other functions, but not on material properties or variables.

!---

## Function Use

Many objects exist in MOOSE that utilize a function, such as:

- `FunctionDirichletBC`
- `FunctionNeumannBC`
- `FunctionIC`
- `BodyForce`

Each of these objects has a "function" parameter which is set in the input file, and controls which
`Function` object is used.

!---

## Parsed Function

A `ParsedFunction` allow functions to be defined by strings directly in the input file, e.g.:

!listing parsed/function.i block=Functions

It is possible to include other functions, as shown above, as well as scalar variables and
postprocessor values with the function definition.

!---

## Default Functions

Whenever an object uses a `Function`-name parameter, the user may elect to instead write
the expression of a x,y,z,t function directly in that parameter. A `ParsedFunction` will automatically
be created for this function.

A `ParsedFunction` or `ConstantFunction` object is automatically constructed based on the default
value if a function name is not supplied in the input file.

!---

## Using a Function in our model

Let's vary the water temperature in time

!style! fontsize=50%

```cpp

  [water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'water_boundary_inwards'
    T_infinity = 300.0
    heat_transfer_coefficient = 30
  []
```

!style-end!

The same  `BC`  object can accept a function parameter:

!style! fontsize=50%

```cpp

  [water_convection]
    type = ADConvectiveHeatFluxBC
    variable = T
    boundary = 'water_boundary_inwards'
    T_infinity_functor = water_T_ramp
    heat_transfer_coefficient = 30
  []

[Functions]
  [water_T_ramp]
    type = PiecewiseLinear
    x = '0 10 100'
    y = '300 290 290'
  []
[]
```

!style-end!

!---


Let's vary the power input in space

!style! fontsize=50%

```cpp

  [from_reactor]
    type = NeumannBC
    variable = T
    boundary = inner_cavity
    # 100 kW reactor, 108 m2 cavity area
    value = '${fparse 1e5 / 108}'
  []
```

!style-end!

This time we have to select a different object:

!style! fontsize=50%

```cpp

  [from_reactor]
    type = FunctionNeumannBC
    variable = T
    boundary = inner_cavity
    # 100 kW reactor, 108 m2 cavity area
    function = 'heat_flux'
  []

[Functions]
  [heat_flux]
    type = ParsedFunction
    # apply heat flux only above z=1m
    expression = 'if(z > 1, 1e5 / 108, 0)'
  []
[]
```

!style-end!

!---
