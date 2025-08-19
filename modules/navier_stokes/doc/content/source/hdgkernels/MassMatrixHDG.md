# MassMatrixHDG

This class computes a mass for facet unkowns on interior faces just like [MassMatrixDGKernel.md]. However, in order to be compatible with static condensation it is slightly less efficient in that faces are visited multiple times.

!syntax parameters /HDGKernels/MassMatrixHDG

!syntax inputs /HDGKernels/MassMatrixHDG

!syntax children /HDGKernels/MassMatrixHDG
