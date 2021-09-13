# MaterialRankTwoTensorAux

!syntax description /AuxKernels/MaterialRankTwoTensorAux

Converting a field from the material system, here a component of the gradient of a material property,
to a variable may be desirable for several reasons: to match the format expected by certain
kernels (thus lagging the field between time steps) or for output/testing/debugging.

This is particularly useful to examine anisotropic material properties. For output
purposes only, an alternative is to use the `output_properties` argument of the `Material`
or specify `output_material_properties` in the parameters of the desired output type nested in
the `[Outputs]` block. This auxkernel is added automatically by the `MaterialOutputAction` if an
`outputs` parameter is specified in a Material block and a RankTwoTensor
material property is declared by the material.

!alert note
The AD system currently does not support auxiliary variables. If you convert material properties, which
do support automatic differentiation, to auxiliary variables, the derivatives will be ignored.

!syntax parameters /AuxKernels/MaterialRankTwoTensorAux

!syntax inputs /AuxKernels/MaterialRankTwoTensorAux

!syntax children /AuxKernels/MaterialRankTwoTensorAux

!bibtex bibliography
