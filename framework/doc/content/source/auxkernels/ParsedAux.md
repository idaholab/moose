# ParsedAux

!syntax description /AuxKernels/ParsedAux

The parsed expression may contain:

- variables (`coupled_variables` parameter)

- coordinates in space and time (`use_xyzt` parameter)

- constants (`constant_names` for their name in the expression and `constant_expressions` for their values)


Material properties are currently not supported, but it would be really easy to add it so feel free to contact us.

## Example syntax

In this example, the `ParsedAux` is being used to compute the multiplication of the simulation variable, `u`, by 2.

!listing test/tests/outputs/png/wedge.i block=AuxKernels

!syntax parameters /AuxKernels/ParsedAux

!syntax inputs /AuxKernels/ParsedAux

!syntax children /AuxKernels/ParsedAux
