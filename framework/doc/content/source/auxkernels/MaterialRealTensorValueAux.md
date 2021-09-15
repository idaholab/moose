# MaterialRealTensorValueAux

!syntax description /AuxKernels/MaterialRealTensorValueAux

Converting a field from the material system, here a component of a tensor material property,
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

In this example, the `MaterialRealTensorValueAux` is used to examine the diagonal of an
anisotropic thermal conductivity in a porous flow simulation.

!listing modules/porous_flow/test/tests/thermal_conductivity/ThermalCondPorosity01.i block=AuxKernels

!syntax parameters /AuxKernels/MaterialRealTensorValueAux

!syntax inputs /AuxKernels/MaterialRealTensorValueAux

!syntax children /AuxKernels/MaterialRealTensorValueAux
