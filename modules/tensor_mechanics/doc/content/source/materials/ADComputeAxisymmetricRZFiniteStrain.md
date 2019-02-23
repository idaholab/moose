# ADComputeAxisymmetricRZFiniteStrain

!syntax description /Materials/ADComputeAxisymmetricRZFiniteStrain<RESIDUAL>

## Description

The material `ADComputeAxisymmetricRZFiniteStrain` calculates the finite strain
for 2D Axisymmetric systems.

!include modules/tensor_mechanics/common/supplementalADAxisymmetricRZStrain.md

Once the deformation gradient is calculated for the specific 2D geometry, the
deformation gradient is passed to the strain and rotation methods used by
default 3D Cartesian simulations, as described in the
[Finite Strain Class](ADComputeFiniteStrain.md) page.

## Example Input File

!syntax parameters /Materials/ADComputeAxisymmetricRZFiniteStrain<RESIDUAL>

!syntax inputs /Materials/ADComputeAxisymmetricRZFiniteStrain<RESIDUAL>

!syntax children /Materials/ADComputeAxisymmetricRZFiniteStrain<RESIDUAL>

!bibtex bibliography
