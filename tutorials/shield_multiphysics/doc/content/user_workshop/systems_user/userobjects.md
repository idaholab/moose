# [UserObject System](syntax/UserObjects/index.md)

A system for defining an arbitrary interface between MOOSE objects.

!---

The UserObject system provides data and calculation results to other MOOSE objects.

- Postprocessors are UserObjects that compute a single scalar value.
- VectorPostprocessors are UserObjects that compute vectors of data.
- UserObjects define their own interface, which other MOOSE objects can call to retrieve data.

!---

## Execution

UserObjects are computed at specified "times" by the execute_on option in the input file:

`execute_on = 'initial timestep_end'`\\
`execute_on = linear`\\
`execute_on = nonlinear`\\
`execute_on = 'timestep_begin final failed'`

They can be restricted to specific blocks, sidesets, and nodesets
