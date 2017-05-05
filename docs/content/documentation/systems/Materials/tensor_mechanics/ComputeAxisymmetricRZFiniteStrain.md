#ComputeAxisymmetricRZFiniteStrain
!description /Materials/ComputeAxisymmetricRZFiniteStrain


## Description
!include docs/content/documentation/modules/tensor_mechanics/common/supplementalAxisymmetricRZStrain.md

Only minimal changes are required to adapt a plane strain problem to an axisymmetric problem; thus,
`ComputeAxisymmetricRZFiniteStrain` inherits from `Compute2DFiniteStrain`.  The finite strain axisymmetric code overwrites the two methods used the calculation of the deformation gradient components:  the virtual function `computeDeformGradZZ` and its old deformation gradient equivalent `computeDeformGradZZOld`.

Once the deformation gradient is calculated for the specific 2D geometry, the deformation gradient is passed to the strain and rotation methods used by default 3D Cartesian simulations, as described in the [Finite Strain Class](ComputeFiniteStrain.md) page.

!parameters /Materials/ComputeAxisymmetricRZFiniteStrain

!inputfiles /Materials/ComputeAxisymmetricRZFiniteStrain

!childobjects /Materials/ComputeAxisymmetricRZFiniteStrain
