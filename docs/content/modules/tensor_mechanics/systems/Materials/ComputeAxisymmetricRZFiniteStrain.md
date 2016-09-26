#Compute Axisymmetric (RZ) Finite Strain
!description /Materials/ComputeAxisymmetricRZFiniteStrain

!devel /Materials/ComputeAxisymmetricRZFiniteStrain float=right width=auto margin=20px padding=20px background-color=#F8F8F8

## Description
{!content/modules/tensor_mechanics/common_documentation/supplementalAxisymmetricRZStrain.md!}

Only minimal changes are required to adapt a plane strain problem to an axisymmetric problem; thus,
`ComputeAxisymmetricRZFiniteStrain` inherits from `Compute2DFiniteStrain`.  The finite strain axisymmetric code overwrites the two methods used the calculation of the deformation gradient components:  the virtual function `computeDeformGradZZ` and its old deformation gradient equivalent `computeDeformGradZZOld`.

Once the deformation gradient is calculated for the specific 2D geometry, the deformation gradient is passed to the strain and rotation methods used by default 3D Cartesian simulations, as described in the [Finite Strain Class](auto::ComputeFiniteStrain) page.

!parameters /Materials/ComputeAxisymmetricRZFiniteStrain

!inputfiles /Materials/ComputeAxisymmetricRZFiniteStrain

!childobjects /Materials/ComputeAxisymmetricRZFiniteStrain
