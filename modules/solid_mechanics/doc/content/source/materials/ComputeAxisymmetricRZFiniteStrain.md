# Compute Axisymmetric RZ Finite Strain

!syntax description /Materials/ComputeAxisymmetricRZFiniteStrain

## Description

The material `ComputeAxisymmetricRZFiniteStrain` calculates the finite strain for
2D Axisymmetric systems.

!include modules/tensor_mechanics/common/supplementalAxisymmetricRZStrain.md

Once the deformation gradient is calculated for the specific 2D geometry, the deformation gradient is
passed to the strain and rotation methods used by default 3D Cartesian simulations, as described in
the [Finite Strain Class](ComputeFiniteStrain.md) page.

## Example Input File

!listing modules/combined/test/tests/cavity_pressure/rz.i block=Materials/strain1

!syntax parameters /Materials/ComputeAxisymmetricRZFiniteStrain

!syntax inputs /Materials/ComputeAxisymmetricRZFiniteStrain

!syntax children /Materials/ComputeAxisymmetricRZFiniteStrain
