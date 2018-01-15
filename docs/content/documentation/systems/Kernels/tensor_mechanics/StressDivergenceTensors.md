#Stress Divergence Tensors
!syntax description /Kernels/StressDivergenceTensors


##Description
The `StressDivergenceTensors` kernel calculates the residual of the stress divergence for 1D, 2D, and 3D problems in the Cartesian coordinate system.
This kernel can be automatically created with the [TensorMechanics Master Action](/systems/Modules/TensorMechanics/Master/index.md). Use of the tensor mechanics master action is recommended to ensure the consistent setting of the _use_displaced_mesh_ parameter for the strain formulation selected.
For a detailed explanation of the settings for _use_displaced_mesh_ in mechanics problems and the TensorMechanics Master Action usage, see the [Introduction/StressDivergence](auto::/introduction/StressDivergence) page.

## Residual Calculation
!include docs/content/documentation/modules/tensor_mechanics/common/supplementalStressDivergenceKernels.md

## Example Input File syntax
The Cartesian `StressDivergenceTensors` is the default case for the tensor
mechanics master action
!listing modules/tensor_mechanics/test/tests/finite_strain_elastic/finite_strain_elastic_new_test.i block=Modules

Either 1, 2, or 3 displacement variables can be used in the stress divergence calculator for the Cartesian system.

!syntax parameters /Kernels/StressDivergenceTensors

!syntax inputs /Kernels/StressDivergenceTensors

!syntax children /Kernels/StressDivergenceTensors
