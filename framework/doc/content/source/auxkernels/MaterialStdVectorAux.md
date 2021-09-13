# MaterialStdVectorAux

!syntax description /AuxKernels/MaterialStdVectorAux

Converting a field from the material system, here a component of a vector material property,
to a variable may be desirable for several reasons: to match the format expected by certain
kernels (thus lagging the field between time steps) or for output/testing/debugging.

This is particularly useful to examine anisotropic material properties. For output
purposes only, an alternative is to use the `output_properties` argument of the `Material`
or specify `output_material_properties` in the parameters of the desired output type nested in
the `[Outputs]` block. This `AuxKernel` is used in the back-end by these parameters.

!alert note
The [MaterialRealVectorValueAux.md] provides similar functionality.

!alert note
The AD system currently does not support auxiliary variables. If you convert material properties, which
do support automatic differentiation, to auxiliary variables, the derivatives will be ignored.

## Example syntax

In this example, the `MaterialStdVectorAux` is being used to examine the first component
of an anisotropic permeability.

!listing porous_flow/test/tests/gravity/grav02g.i block=AuxKernels start=[relpermwater] end=[]

!syntax parameters /AuxKernels/MaterialStdVectorAux

!syntax inputs /AuxKernels/MaterialStdVectorAux

!syntax children /AuxKernels/MaterialStdVectorAux
