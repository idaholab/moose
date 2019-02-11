# ADComputeAxisymmetricRZIncrementalStrain

!syntax description /ADMaterials/ADComputeAxisymmetricRZIncrementalStrain<RESIDUAL>

## Description

The material `ADComputeAxisymmetricRZIncrementalStrain` calculates the small
incremental strain for Axisymmetric systems.

!include modules/tensor_mechanics/common/supplementalADAxisymmetricRZStrain.md

Once the deformation gradient is calculated for the specific 2D geometry, the
deformation gradient is passed to the strain and rotation methods used by
default 3D Cartesian simulations, as described in the
[Incremental Finite Strain Class](ADComputeIncrementalSmallStrain.md) page.

!syntax parameters /ADMaterials/ADComputeAxisymmetricRZIncrementalStrain<RESIDUAL>

!syntax inputs /ADMaterials/ADComputeAxisymmetricRZIncrementalStrain<RESIDUAL>

!syntax children /ADMaterials/ADComputeAxisymmetricRZIncrementalStrain<RESIDUAL>
