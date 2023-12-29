# Compute Axisymmetric RZ Incremental Strain

!syntax description /Materials/ComputeAxisymmetricRZIncrementalStrain

## Description

The material `ComputeAxisymmetricRZIncrementalStrain` calculates the small incremental strain for
Axisymmetric systems.

!include modules/tensor_mechanics/common/supplementalAxisymmetricRZStrain.md

Once the deformation gradient is calculated for the specific 2D geometry, the deformation gradient is
passed to the strain and rotation methods used by default 3D Cartesian simulations, as described in
the [Incremental Finite Strain Class](ComputeIncrementalSmallStrain.md) page.

## Example Input File

!listing modules/contact/test/tests/verification/patch_tests/cyl_1/cyl1_mu_0_2_pen.i
         block=Materials/bot_strain

!syntax parameters /Materials/ComputeAxisymmetricRZIncrementalStrain

!syntax inputs /Materials/ComputeAxisymmetricRZIncrementalStrain

!syntax children /Materials/ComputeAxisymmetricRZIncrementalStrain
