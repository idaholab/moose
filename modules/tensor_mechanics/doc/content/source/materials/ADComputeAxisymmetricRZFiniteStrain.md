# ADComputeAxisymmetricRZFiniteStrain

!syntax description /Materials/ADComputeAxisymmetricRZFiniteStrain

## Description

The material `ADComputeAxisymmetricRZFiniteStrain` calculates the finite strain
for 2D Axisymmetric systems.

!include modules/tensor_mechanics/common/supplementalADAxisymmetricRZStrain.md

Once the deformation gradient is calculated for the specific 2D geometry, the
deformation gradient is passed to the strain and rotation methods used by
default 3D Cartesian simulations, as described in the
[Finite Strain Class](ADComputeFiniteStrain.md) page.

## Example Input File

!syntax parameters /Materials/ADComputeAxisymmetricRZFiniteStrain

!syntax inputs /Materials/ADComputeAxisymmetricRZFiniteStrain

!syntax children /Materials/ADComputeAxisymmetricRZFiniteStrain

!bibtex bibliography
