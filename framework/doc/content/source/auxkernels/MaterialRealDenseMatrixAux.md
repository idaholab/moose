# MaterialRealDenseMatrixAux

!syntax description /AuxKernels/MaterialRealDenseMatrixAux

Converting a field from the material system, here a component of a matrix material property,
to a variable may be desirable for several reasons: to match the format expected by certain
kernels (thus lagging the field between time steps) or for output/testing/debugging.

This is particularly useful to examine anisotropic material properties. For output
purposes only, an alternative is to use the `output_properties` argument of the `Material`
or specify `output_material_properties` in the parameters of the desired output type nested in
the `[Outputs]` block. This `AuxKernel` is used in the back-end by these parameters.

!alert note
The AD system currently does not support auxiliary variables. If you convert material properties, which
do support automatic differentiation, to auxiliary variables, the derivatives will be ignored.

## Example syntax

In this example, the `MaterialRealDenseMatrixAux` is used to examine the first row, first column element of
a matrix material property.

!listing test/tests/materials/types/test.i block=AuxKernels start=[./densemat00] end=[./densemat01]

!syntax parameters /AuxKernels/MaterialRealDenseMatrixAux

!syntax inputs /AuxKernels/MaterialRealDenseMatrixAux

!syntax children /AuxKernels/MaterialRealDenseMatrixAux
