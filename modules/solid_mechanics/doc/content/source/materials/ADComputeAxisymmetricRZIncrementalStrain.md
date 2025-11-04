# ADComputeAxisymmetricRZIncrementalStrain

!syntax description /Materials/ADComputeAxisymmetricRZIncrementalStrain

## Description

The material `ADComputeAxisymmetricRZIncrementalStrain` calculates the small
incremental strain for Axisymmetric systems.

!include modules/solid_mechanics/common/supplementalADAxisymmetricRZStrain.md

Once the deformation gradient is calculated for the specific 2D geometry, the
deformation gradient is passed to the strain and rotation methods used by
default 3D Cartesian simulations, as described in the
[Incremental Finite Strain Class](ADComputeIncrementalStrain.md) page.

!syntax parameters /Materials/ADComputeAxisymmetricRZIncrementalStrain

!syntax inputs /Materials/ADComputeAxisymmetricRZIncrementalStrain

!syntax children /Materials/ADComputeAxisymmetricRZIncrementalStrain
