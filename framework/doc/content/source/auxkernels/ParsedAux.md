# ParsedAux

!syntax description /AuxKernels/ParsedAux

The parsed expression may contain:

- variables (`coupled_variables` parameter)

- real-valued material properties ([!param](/AuxKernels/ParsedAux/material_properties) parameter)

- automatic differentiation (AD) real-valued material properties ([!param](/AuxKernels/ParsedAux/ad_material_properties) parameter)

- functors (`functor_names` or `functor_symbols` parameter)

- coordinates in space and time (`use_xyzt` parameter)

- constants (`constant_names` for their name in the expression and `constant_expressions` for their values)


## Example syntax

In this example, the `ParsedAux` is being used to compute the multiplication of the simulation variable, `u`, by 2.

!listing test/tests/outputs/png/wedge.i block=AuxKernels

!syntax parameters /AuxKernels/ParsedAux

!syntax inputs /AuxKernels/ParsedAux

!syntax children /AuxKernels/ParsedAux
