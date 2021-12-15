# MaterialRealEigenVectorAux

!syntax description /AuxKernels/MaterialRealEigenVectorAux

Converting a field from the material system, here a component of an array material property,
to a variable may be desirable for several reasons: to match the format expected by certain
kernels, for lagging the field between time steps or for output/testing/debugging.

This is particularly useful to examine anisotropic material properties. For output
purposes only, an alternative is to use the `output_properties` argument of the `Material`
or specify `output_material_properties` in the parameters of the desired output type nested in
the `[Outputs]` block.

!syntax parameters /AuxKernels/MaterialRealEigenVectorAux

!syntax inputs /AuxKernels/MaterialRealEigenVectorAux

!syntax children /AuxKernels/MaterialRealEigenVectorAux
