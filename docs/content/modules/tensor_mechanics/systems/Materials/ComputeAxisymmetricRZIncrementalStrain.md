#ComputeAxisymmetricRZIncrementalStrain
!description /Materials/ComputeAxisymmetricRZIncrementalStrain


## Description
{!docs/content/modules/tensor_mechanics/common_documentation/supplementalAxisymmetricRZStrain.md!}

Only minimal changes are required to adapt a plane strain problem to an axisymmetric problem; thus,
`ComputeAxisymmetricRZIncrementalStrain` inherits from `Compute2DIncrementalStrain`.  The finite strain axisymmetric code overwrites the two methods used the calculation of the deformation gradient components:  the virtual function `computeDeformGradZZ` and its old deformation gradient equivalent `computeDeformGradZZOld`.

Once the deformation gradient is calculated for the specific 2D geometry, the deformation gradient is passed to the strain and rotation methods used by default 3D Cartesian simulations, as described in the [Incremental Finite Strain Class](ComputeIncrementalSmallStrain.md) page.


!parameters /Materials/ComputeAxisymmetricRZIncrementalStrain

!inputfiles /Materials/ComputeAxisymmetricRZIncrementalStrain

!childobjects /Materials/ComputeAxisymmetricRZIncrementalStrain
