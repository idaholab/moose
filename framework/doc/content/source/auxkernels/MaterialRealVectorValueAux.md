# MaterialRealVectorValueAux

!syntax description /AuxKernels/MaterialRealVectorValueAux

Converting a field from the material system, here a component of a vector material property,
to a variable may be desirable for several reasons: to match the format expected by certain
kernels, for lagging the field between time steps or for output/testing/debugging.

This is particularly useful to examine anisotropic material properties. For output
purposes only, an alternative is to use the `output_properties` argument of the `Material`
or specify `output_material_properties` in the parameters of the desired output type nested in
the `[Outputs]` block.

!alert note
The AD system currently does not support auxiliary variables. If you convert material properties, which
do support automatic differentiation, to auxiliary variables, the derivatives will be ignored.

## Example syntax

In this example, the `MaterialRealVectorValueAux` is being used to examine different cracking criteria
for a smear cracking model.

!listing modules/tensor_mechanics/test/tests/smeared_cracking/cracking_rotation.i block=AuxKernels

!syntax parameters /AuxKernels/MaterialRealVectorValueAux

!syntax inputs /AuxKernels/MaterialRealVectorValueAux

!syntax children /AuxKernels/MaterialRealVectorValueAux
