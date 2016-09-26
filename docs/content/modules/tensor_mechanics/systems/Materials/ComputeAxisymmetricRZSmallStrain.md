#ComputeAxisymmetricRZSmallStrain
!description /Materials/ComputeAxisymmetricRZSmallStrain

!devel /Materials/ComputeAxisymmetricRZSmallStrain float=right width=auto margin=20px padding=20px background-color=#F8F8F8

## Description
{!content/modules/tensor_mechanics/common_documentation/supplementalAxisymmetricRZStrain.md!}

Only minimal changes are required to adapt a plane strain problem to an axisymmetric problem; thus,
`ComputeAxisymmetricRZISmallStrain` inherits from `Compute2DSmallStrain`.  The finite strain axisymmetric code overwrites the method used to calculate the strain component $\nabla u_{22}$, before calculating the total strain measure with the small strain assumption, as described in the [Introduction/Strains](auto::/introduction/Strains) page.

!parameters /Materials/ComputeAxisymmetricRZSmallStrain

!inputfiles /Materials/ComputeAxisymmetricRZSmallStrain

!childobjects /Materials/ComputeAxisymmetricRZSmallStrain
