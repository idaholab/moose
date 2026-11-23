# ADComputeGlobalStrain

!syntax description /Materials/ADComputeGlobalStrain

## Description

The `ADComputeGlobalStrain` material consumes the diagonal and off-diagonal global strain scalar variables to build a symmetric global strain tensor $\overleftrightarrow{\epsilon}$. The provided tensor is then consumed by the constitituive system where $\overleftrightarrow{\epsilon}$ is applied to the other strains.

## AD Global Strain Objects

Used with the automatic differentiation version of Global Strain.

[ADGlobalStrain](/ADGlobalStrain.md)

[ADGlobalDisplacementAux](/ADGlobalDisplacementAux.md)

[GlobalStrainPeriodicDirUserObject](/GlobalStrainPeriodicDirUserObject.md)

!syntax parameters /Materials/ADComputeGlobalStrain

!syntax inputs /Materials/ADComputeGlobalStrain

!syntax children /Materials/ADComputeGlobalStrain
